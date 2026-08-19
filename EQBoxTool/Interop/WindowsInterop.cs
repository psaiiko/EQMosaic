using System;
using System.Diagnostics;
using System.IO;
using System.Management;
using System.Runtime.InteropServices;
using System.Text;

namespace EQBoxTool
{
    public static class WindowsInterop
    {
        [DllImport("kernel32.dll", SetLastError = true, CharSet = CharSet.Auto)]
        public static extern bool CreateProcess(
        string lpApplicationName,
        string lpCommandLine,
        ref SECURITY_ATTRIBUTES lpProcessAttributes,
        ref SECURITY_ATTRIBUTES lpThreadAttributes,
        bool bInheritHandles,
        uint dwCreationFlags,
        IntPtr lpEnvironment,
        string lpCurrentDirectory,
        [In] ref STARTUPINFO lpStartupInfo,
        out PROCESS_INFORMATION lpProcessInformation);

        [DllImport("kernel32.dll", SetLastError = true)]
        public static extern uint ResumeThread(IntPtr hThread);

        [DllImport("kernel32.dll", SetLastError = true, ExactSpelling = true)]
        public static extern IntPtr VirtualAllocEx(IntPtr hProcess, IntPtr lpAddress,
            uint dwSize, uint flAllocationType, uint flProtect);

        [DllImport("kernel32.dll", SetLastError = true)]
        public static extern bool WriteProcessMemory(
          IntPtr hProcess,
          IntPtr lpBaseAddress,
          byte[] lpBuffer,
          Int32 nSize,
          out int lpNumberOfBytesWritten);

        [DllImport("kernel32", CharSet = CharSet.Ansi, ExactSpelling = true, SetLastError = true)]
        public static extern IntPtr GetProcAddress(IntPtr hModule, string procName);

        [DllImport("kernel32.dll", CharSet = CharSet.Unicode, SetLastError = true)]
        public static extern IntPtr GetModuleHandle([MarshalAs(UnmanagedType.LPWStr)] string lpModuleName);

        [DllImport("kernel32.dll")]
        public static extern IntPtr CreateRemoteThread(IntPtr hProcess,
           IntPtr lpThreadAttributes, uint dwStackSize, IntPtr lpStartAddress,
           IntPtr lpParameter, uint dwCreationFlags, IntPtr lpThreadId);

        [DllImport("kernel32.dll", SetLastError = true)]
        public static extern uint WaitForSingleObject(IntPtr hHandle, uint dwMilliseconds);

        [DllImport("kernel32.dll", SetLastError = true)]
        public static extern bool VirtualFreeEx(IntPtr hProcess, IntPtr lpAddress, uint dwSize, uint dwFreeType);

        public const uint INFINITE = 0xFFFFFFFF;
        public const uint MEM_RELEASE = 0x8000;


        [StructLayout(LayoutKind.Sequential)]
        public struct SECURITY_ATTRIBUTES
        {
            public int nLength;
            public  IntPtr lpSecurityDescriptor;
        }

        [Flags]
        public enum CreationFlags : uint
        {
            CREATE_BREAKAWAY_FROM_JOB = 0x01000000,
            CREATE_DEFAULT_ERROR_MODE = 0x04000000,
            CREATE_NEW_CONSOLE = 0x00000010,
            CREATE_NEW_PROCESS_GROUP = 0x00000200,
            CREATE_NO_WINDOW = 0x08000000,
            CREATE_PROTECTED_PROCESS = 0x00040000,
            CREATE_PRESERVE_CODE_AUTHZ_LEVEL = 0x02000000,
            CREATE_SECURE_PROCESS = 0x00400000,
            CREATE_SEPARATE_WOW_VDM = 0x00000800,
            CREATE_SHARED_WOW_VDM = 0x00001000,
            CREATE_SUSPENDED = 0x00000004,
            CREATE_UNICODE_ENVIRONMENT = 0x00000400,
            DEBUG_ONLY_THIS_PROCESS = 0x00000002,
            DEBUG_PROCESS = 0x00000001,
            DETACHED_PROCESS = 0x00000008,
            EXTENDED_STARTUPINFO_PRESENT = 0x00080000,
            INHERIT_PARENT_AFFINITY = 0x00010000,

            ABOVE_NORMAL_PRIORITY_CLASS = 0x00008000,
            BELOW_NORMAL_PRIORITY_CLASS = 0x00004000,
            HIGH_PRIORITY_CLASS = 0x00000080,
            IDLE_PRIORITY_CLASS = 0x00000040,
            NORMAL_PRIORITY_CLASS = 0x00000020,
            REALTIME_PRIORITY_CLASS = 0x00000100
        }

        [StructLayout(LayoutKind.Sequential)]
        public struct STARTUPINFO
        {
            public uint cb;
            public IntPtr lpReserved;
            public IntPtr lpDesktop;
            public IntPtr lpTitle;
            public uint dwX;
            public uint dwY;
            public uint dwXSize;
            public uint dwYSize;
            public uint dwXCountChars;
            public uint dwYCountChars;
            public uint dwFillAttributes;
            public uint dwFlags;
            public ushort wShowWindow;
            public ushort cbReserved;
            public IntPtr lpReserved2;
            public IntPtr hStdInput;
            public IntPtr hStdOutput;
            public IntPtr hStdErr;
        }

        [StructLayout(LayoutKind.Sequential)]
        public struct STARTUPINFOEX
        {
            public STARTUPINFO StartupInfo;
            public IntPtr lpAttributeList;
        }

        [StructLayout(LayoutKind.Sequential)]
        public struct PROCESS_INFORMATION
        {
            public IntPtr hProcess;
            public IntPtr hThread;
            public int dwProcessId;
            public int dwThreadId;
        }

        [Flags]
        public enum AllocationType
        {
            Commit = 0x1000,
            Reserve = 0x2000,
            Decommit = 0x4000,
            Release = 0x8000,
            Reset = 0x80000,
            Physical = 0x400000,
            TopDown = 0x100000,
            WriteWatch = 0x200000,
            LargePages = 0x20000000
        }

        [Flags]
        public enum MemoryProtection
        {
            Execute = 0x10,
            ExecuteRead = 0x20,
            ExecuteReadWrite = 0x40,
            ExecuteWriteCopy = 0x80,
            NoAccess = 0x01,
            ReadOnly = 0x02,
            ReadWrite = 0x04,
            WriteCopy = 0x08,
            GuardModifierflag = 0x100,
            NoCacheModifierflag = 0x200,
            WriteCombineModifierflag = 0x400
        }
        public const uint PROCESS_ALL_ACCESS = 0x001F0FFF;

        public const int VK_BACKTICK = 0xC0;
        public const int VK_SHIFT = 0x10;
        public const int VK_MENU = 0x12;

        public const uint KEYEVENTF_EXTENDEDKEY = 0x0001;
        public const uint KEYEVENTF_KEYUP = 0x0002;

        [StructLayout(LayoutKind.Sequential)]
        public struct KBDLLHOOKSTRUCT
        {
            public uint vkCode;
            public uint scanCode;
            public uint flags;
            public uint time;
            public IntPtr dwExtraInfo;
        }

        public delegate IntPtr LowLevelKeyboardProc(int nCode, IntPtr wParam, IntPtr lParam);


        [DllImport("kernel32.dll", SetLastError = true)]
        public static extern IntPtr OpenProcess(uint dwDesiredAccess, bool bInheritHandle, int dwProcessId);

        [DllImport("kernel32.dll", SetLastError = true)]
        public static extern bool ReadProcessMemory(IntPtr hProcess, IntPtr lpBaseAddress,
            [Out] byte[] lpBuffer, int dwSize, out int lpNumberOfBytesRead);

        [DllImport("kernel32.dll", SetLastError = true)]
        public static extern bool CloseHandle(IntPtr hObject);

        [DllImport("kernel32.dll", CharSet = CharSet.Unicode, SetLastError = true)]
        private static extern IntPtr OpenFileMapping(uint dwDesiredAccess, bool bInheritHandle, string lpName);

        [DllImport("kernel32.dll", SetLastError = true)]
        private static extern IntPtr MapViewOfFile(IntPtr hFileMappingObject, uint dwDesiredAccess, uint dwFileOffsetHigh, uint dwFileOffsetLow, IntPtr dwNumberOfBytesToMap);

        [DllImport("kernel32.dll", SetLastError = true)]
        private static extern bool UnmapViewOfFile(IntPtr lpBaseAddress);
        private const uint FILE_MAP_ALL_ACCESS = 0xF001F;

        [DllImport("user32.dll", SetLastError = true)]
        public static extern IntPtr SetWindowsHookEx(int idHook, LowLevelKeyboardProc lpfn, IntPtr hMod, uint dwThreadId);

        [DllImport("user32.dll", SetLastError = true)]
        [return: MarshalAs(UnmanagedType.Bool)]
        public static extern bool UnhookWindowsHookEx(IntPtr hhk);

        [DllImport("user32.dll")]
        public static extern IntPtr CallNextHookEx(IntPtr hhk, int nCode, IntPtr wParam, IntPtr lParam);

        public static bool TryReadMapFile(int processId, out long remoteRenderTextureHandle, out int lastGeneratedFrame)
        {
            remoteRenderTextureHandle = 0;
            lastGeneratedFrame = 0;
            IntPtr hMap = OpenFileMapping(FILE_MAP_ALL_ACCESS, false, "OverlayData_" + processId);
            if (hMap == IntPtr.Zero)
                return false;

            IntPtr pBuf = MapViewOfFile(hMap, FILE_MAP_ALL_ACCESS, 0, 0, IntPtr.Zero);
            if (pBuf == IntPtr.Zero)
            {
                CloseHandle(hMap);
                return false;
            }

            lastGeneratedFrame = (int)Marshal.ReadInt32(pBuf, 0);
            long lo = (long)Marshal.ReadInt32(pBuf, 4);
            long hi = (long)Marshal.ReadInt32(pBuf, 8);
            remoteRenderTextureHandle = lo | (hi << 32);

            UnmapViewOfFile(pBuf);
            CloseHandle(hMap);
            return true;
        }

        public static bool TrySendCommand(int processId, int commandID)
        {
            IntPtr hMap = OpenFileMapping(FILE_MAP_ALL_ACCESS, false, "OverlayData_" + processId);
            if (hMap == IntPtr.Zero)
                return false;

            IntPtr pBuf = MapViewOfFile(hMap, FILE_MAP_ALL_ACCESS, 0, 0, IntPtr.Zero);
            if (pBuf == IntPtr.Zero)
            {
                CloseHandle(hMap);
                return false;
            }

            Marshal.WriteInt32(pBuf, 12, commandID);

            UnmapViewOfFile(pBuf);
            CloseHandle(hMap);
            return true;
        }

        [DllImport("USER32.DLL")]
        public static extern bool SetForegroundWindow(IntPtr hWnd);

        [DllImport("user32.dll")]
        public static extern void keybd_event(byte bVk, byte bScan, uint dwFlags, IntPtr dwExtraInfo);

        [DllImport("USER32.DLL")]
        public static extern IntPtr GetForegroundWindow();

        [DllImport("user32.dll")]
        public static extern short GetAsyncKeyState(int vKey);

        [DllImport("user32.dll")]
        public static extern bool EnumWindows(EnumWindowsProc lpEnumFunc, IntPtr lParam);

        [DllImport("user32.dll")]
        public static extern uint GetWindowThreadProcessId(IntPtr hWnd, out int lpdwProcessId);

        [DllImport("kernel32.dll", SetLastError = false)]
        public static extern uint GetCurrentThreadId();

        [DllImport("user32.dll", CharSet = CharSet.Auto)]
        public static extern int GetClassName(IntPtr hWnd, System.Text.StringBuilder lpClassName, int nMaxCount);

        public delegate bool EnumWindowsProc(IntPtr hWnd, IntPtr lParam);

        public const int VK_SPACE = 0x20;

        [DllImport("user32.dll")]
        public static extern bool ShowWindow(IntPtr hWnd, int nCmdShow);

        [DllImport("user32.dll")]
        public static extern bool IsIconic(IntPtr hWnd);

        [DllImport("user32.dll")]
        public static extern bool AttachThreadInput(uint idAttach, uint idAttachTo, bool fAttach);

        public const int SW_RESTORE = 9;

        public const uint WM_SYSCOMMAND = 0x0112;
        public const int SC_RESTORE = 0xF120;

        public const int WH_KEYBOARD_LL = 13;
        public const int WM_KEYDOWN = 0x0100;
        public const int WM_KEYUP = 0x0101;
        public const int WM_SYSKEYDOWN = 0x0104;
        public const int WM_SYSKEYUP = 0x0105;

        [DllImport("user32.dll", CharSet = CharSet.Auto)]
        public static extern IntPtr SendMessage(IntPtr hWnd, uint Msg, IntPtr wParam, IntPtr lParam);

        [StructLayout(LayoutKind.Sequential)]
        public struct RECT
        {
            public int Left;
            public int Top;
            public int Right;
            public int Bottom;
        }

        public const int GWL_STYLE = -16;
        public const int GWL_EXSTYLE = -20;

        public const uint SWP_NOZORDER = 0x0004;
        public const uint SWP_NOACTIVATE = 0x0010;

        const int WS_THICKFRAME = 0x00040000;
        const int WS_CAPTION = 0x00C00000;
        const int WS_MINIMIZEBOX = 0x00020000;
        const int WS_MAXIMIZEBOX = 0x00010000;
        const int WS_SYSMENU = 0x00080000;

        [DllImport("user32.dll", EntryPoint = "SetWindowLong")]
        public static extern int SetWindowLong(IntPtr hWnd,int nIndex,int dwNewLong);

        [DllImport("user32.dll", SetLastError = true)]
        public static extern int GetWindowLong(IntPtr hWnd, int nIndex);

        [DllImport("user32.dll", SetLastError = true)]
        public static extern bool AdjustWindowRectEx(ref RECT lpRect, uint dwStyle, bool bMenu, uint dwExStyle);

        [DllImport("user32.dll", SetLastError = true)]
        public static extern bool SetWindowPos(IntPtr hWnd, IntPtr hWndInsertAfter, int X, int Y, int cx, int cy, uint uFlags);

        [DllImport("user32.dll")]
        static extern bool RedrawWindow(
    IntPtr hWnd,
    IntPtr lprcUpdate,
    IntPtr hrgnUpdate,
    uint flags);

        const uint RDW_INVALIDATE = 0x0001;
        const uint RDW_FRAME = 0x0400;
        const uint RDW_UPDATENOW = 0x0100;
        const uint RDW_ALLCHILDREN = 0x0080;

        public static void SetWindowClientRect(IntPtr hWnd, int clientX, int clientY, int clientWidth, int clientHeight)
        {
            int style = GetWindowLong(hWnd, GWL_STYLE);
            int exStyle = GetWindowLong(hWnd, GWL_EXSTYLE);

            if ((style & WS_THICKFRAME) == 0)
            {
                style |= WS_THICKFRAME;

                SetWindowLong(hWnd, GWL_STYLE, style);

                RedrawWindow(
                    hWnd,
                    IntPtr.Zero,
                    IntPtr.Zero,
                    RDW_FRAME |
                    RDW_INVALIDATE |
                    RDW_UPDATENOW |
                    RDW_ALLCHILDREN);
            }

            RECT rect = new RECT
            {
                Left = clientX,
                Top = clientY,
                Right = clientX + clientWidth,
                Bottom = clientY + clientHeight
            };

            AdjustWindowRectEx(ref rect, (uint)style, false, (uint)exStyle);

            SetWindowPos(hWnd, IntPtr.Zero,
                rect.Left, rect.Top,
                rect.Right - rect.Left,
                rect.Bottom - rect.Top,
                SWP_NOZORDER | SWP_NOACTIVATE);

        }

        public static void SetBorderlessWindowRect(IntPtr hWnd, int x, int y, int width, int height)
        {
            int style = GetWindowLong(hWnd, GWL_STYLE);
            style &= ~(WS_CAPTION | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_SYSMENU);
            SetWindowLong(hWnd, GWL_STYLE, style);

            RedrawWindow(hWnd, IntPtr.Zero, IntPtr.Zero,
                RDW_FRAME | RDW_INVALIDATE | RDW_UPDATENOW | RDW_ALLCHILDREN);

            SetWindowPos(hWnd, IntPtr.Zero, x, y, width, height, SWP_NOZORDER | SWP_NOACTIVATE);
        }

        public static IntPtr GetGameWindowHandle(int processId)
        {
            IntPtr result = IntPtr.Zero;
            EnumWindows((hWnd, lParam) =>
            {
                GetWindowThreadProcessId(hWnd, out int pid);
                if (pid == processId)
                {
                    var sb = new System.Text.StringBuilder(256);
                    GetClassName(hWnd, sb, sb.Capacity);
                    if (sb.ToString() == "_EverQuestwndclass")
                    {
                        result = hWnd;
                        return false;
                    }
                }
                return true;
            }, IntPtr.Zero);
            return result;
        }

        public static string GetProcessCommandLineWmi(Process process)
        {
            try
            {
                int processId = process.Id;
                using (var searcher = new ManagementObjectSearcher(
                    $"SELECT CommandLine FROM Win32_Process WHERE ProcessId = {processId}"))
                using (var results = searcher.Get().GetEnumerator())
                {
                    if (results.MoveNext())
                        return results.Current["CommandLine"]?.ToString();
                }
            }
            catch
            {
            }
            return null;
        }

        const uint LIST_MODULES_ALL = 0x03;

        [DllImport("psapi.dll", SetLastError = true)]
        static extern bool EnumProcessModulesEx(IntPtr hProcess, IntPtr[] lphModule,
            uint cb, out uint lpcbNeeded, uint dwFilterFlag);

        [DllImport("psapi.dll", SetLastError = true, CharSet = CharSet.Unicode)]
        static extern uint GetModuleFileNameEx(IntPtr hProcess, IntPtr hModule,
            StringBuilder lpFilename, uint nSize);

        public static bool IsModuleLoadedInProcess(int processId, string moduleName)
        {
            IntPtr hProcess = OpenProcess(PROCESS_ALL_ACCESS, false, processId);
            if (hProcess == IntPtr.Zero)
                return false;

            try
            {
                uint cbNeeded;
                if (!EnumProcessModulesEx(hProcess, null, 0, out cbNeeded, LIST_MODULES_ALL))
                    return false;

                int count = (int)(cbNeeded / IntPtr.Size);
                IntPtr[] modules = new IntPtr[count];
                if (!EnumProcessModulesEx(hProcess, modules, cbNeeded, out cbNeeded, LIST_MODULES_ALL))
                    return false;

                var sb = new StringBuilder(260);
                for (int i = 0; i < count; i++)
                {
                    sb.Clear();
                    uint len = GetModuleFileNameEx(hProcess, modules[i], sb, (uint)sb.Capacity);
                    if (len > 0)
                    {
                        string fileName = Path.GetFileName(sb.ToString());
                        if (string.Equals(fileName, moduleName, StringComparison.OrdinalIgnoreCase))
                            return true;
                    }
                }
                return false;
            }
            finally
            {
                CloseHandle(hProcess);
            }
        }

        public static void RelaxForegroundStealing()
        {
            // Windows only grants SetForegroundWindow to the process it considers to have
            // delivered the last input. A swallowed hook key doesn't qualify (input still
            // belongs to the foreground game), so standalone — no debugger to relax the lock —
            // this is refused and the taskbar flashes orange.
            // Injecting a momentary ALT keypress globally (to the game window currently in the foreground), will
            // open a brief grace period with relaxed checks for SetForegroundWindow.
            keybd_event(VK_MENU, 0, KEYEVENTF_EXTENDEDKEY, IntPtr.Zero);
            keybd_event(VK_MENU, 0, KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP, IntPtr.Zero);
        }

    }
}
