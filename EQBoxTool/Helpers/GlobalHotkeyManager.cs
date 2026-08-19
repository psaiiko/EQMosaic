using System;
using System.Runtime.InteropServices;

namespace EQBoxTool
{
    /// <summary>
    /// Installs a global low-level keyboard hook (WH_KEYBOARD_LL) so a hotkey can be observed
    /// even when an eqgame instance window (not the EQBoxTool form) has focus. The hook only
    /// consumes the key when <see cref="HotkeyHandler"/> returns true — i.e. when an EQ instance
    /// is currently focused — otherwise the key passes through to the foreground app untouched.
    /// </summary>
    public sealed class GlobalHotkeyManager : IDisposable
    {
        private readonly WindowsInterop.LowLevelKeyboardProc _proc;
        private IntPtr _hook = IntPtr.Zero;

        // Latch state for one physical press: avoids firing on auto-repeat and tracks whether
        // this press was consumed (so repeats stay swallowed until the key is released).
        private bool _backtickDown;
        private bool _backtickReserved;

        /// <summary>
        /// Invoked on backtick key-down. Return true to consume the key (if an EQ instance is
        /// focused and a switch was initiated); return false to let it pass through normally.
        /// </summary>
        public Func<bool,bool> HotkeyHandler { get; set; }

        public GlobalHotkeyManager()
        {
            _proc = HookCallback;
        }

        public void Install()
        {
            if (_hook != IntPtr.Zero)
                return;

            using (var curProc = System.Diagnostics.Process.GetCurrentProcess())
            using (var curModule = curProc.MainModule)
            {
                // The hook callback is dispatched through this thread's message loop, so it must
                // be installed from a thread that pumps messages (the WinForms UI thread).
                _hook = WindowsInterop.SetWindowsHookEx(WindowsInterop.WH_KEYBOARD_LL, _proc,
                    WindowsInterop.GetModuleHandle(curModule.ModuleName), 0);
            }
        }

        private IntPtr HookCallback(int nCode, IntPtr wParam, IntPtr lParam)
        {
            if (nCode >= 0)
            {
                int msg = wParam.ToInt32();
                var data = Marshal.PtrToStructure<WindowsInterop.KBDLLHOOKSTRUCT>(lParam);

                bool shiftPressed = (WindowsInterop.GetAsyncKeyState(WindowsInterop.VK_SHIFT) & 0x8000) != 0;

                if (data.vkCode == WindowsInterop.VK_BACKTICK && msg == WindowsInterop.WM_KEYDOWN)
                {
                    if (!_backtickDown)
                    {
                        _backtickDown = true;
                        _backtickReserved = HotkeyHandler?.Invoke(shiftPressed) ?? false;
                        return _backtickReserved ? (IntPtr)1 : WindowsInterop.CallNextHookEx(_hook, nCode, wParam, lParam);
                    }

                    // Auto-repeat of an already-pressed (possibly reserved) backtick: keep it
                    // swallowed if this press was consumed, otherwise pass it through.
                    return _backtickReserved ? (IntPtr)1 : WindowsInterop.CallNextHookEx(_hook, nCode, wParam, lParam);
                }

                if (data.vkCode == WindowsInterop.VK_BACKTICK && (msg == WindowsInterop.WM_KEYUP || msg == WindowsInterop.WM_SYSKEYUP))
                {
                    _backtickDown = false;
                    _backtickReserved = false;
                }
            }

            return WindowsInterop.CallNextHookEx(_hook, nCode, wParam, lParam);
        }

        public void Dispose()
        {
            if (_hook != IntPtr.Zero)
            {
                WindowsInterop.UnhookWindowsHookEx(_hook);
                _hook = IntPtr.Zero;
            }
        }
    }
}
