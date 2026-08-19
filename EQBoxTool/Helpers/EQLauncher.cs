using System;
using System.Diagnostics;
using System.IO;
using System.Runtime.InteropServices;
using System.Text;

namespace EQBoxTool
{
    public class EQLauncher
    {
        public Process LaunchNewInstance(string gameExePath, Account account, Server server, Character character)
        {
            string credentials = $"{account.LoginName}|{account.Password}|{server.Name}|{character.Name}";
            string encrypted = EncryptionHelper.ProtectString(credentials);
            string CommandLine = $" patchme /login:{account.LoginName} /autologin:{encrypted}";

            WindowsInterop.PROCESS_INFORMATION pInfo = new WindowsInterop.PROCESS_INFORMATION();
            WindowsInterop.STARTUPINFO sInfo = new WindowsInterop.STARTUPINFO();
            WindowsInterop.SECURITY_ATTRIBUTES pSec = new WindowsInterop.SECURITY_ATTRIBUTES();
            WindowsInterop.SECURITY_ATTRIBUTES tSec = new WindowsInterop.SECURITY_ATTRIBUTES();
            pSec.nLength = Marshal.SizeOf(pSec);
            tSec.nLength = Marshal.SizeOf(tSec);

            bool success = WindowsInterop.CreateProcess(gameExePath, CommandLine,
                ref pSec,
                ref tSec,
                false,
                 (uint)(WindowsInterop.CreationFlags.NORMAL_PRIORITY_CLASS | WindowsInterop.CreationFlags.CREATE_SUSPENDED | WindowsInterop.CreationFlags.DETACHED_PROCESS),
                IntPtr.Zero,
                Path.GetDirectoryName(gameExePath),
                ref sInfo,
                out pInfo);

            if (success)
            {
                var process = Process.GetProcessById(pInfo.dwProcessId);

                string dllPath = Path.Combine(
                    AppDomain.CurrentDomain.BaseDirectory, GlobalsHelper.OverlayDllName);

                InjectDLL(process.Handle, dllPath);

                WindowsInterop.ResumeThread(pInfo.hThread);
                return process;
            }
            return null;
        }

        private static void InjectDLL(IntPtr processHandle, string path)
        {
            byte[] pathAsUnicodeBytes = Encoding.Unicode.GetBytes(path + "\0");
            IntPtr remoteBuffer = WindowsInterop.VirtualAllocEx(processHandle, IntPtr.Zero, (uint)pathAsUnicodeBytes.Length, (uint)WindowsInterop.AllocationType.Commit, (uint)WindowsInterop.MemoryProtection.ReadWrite);
            int bytesWritten;
            WindowsInterop.WriteProcessMemory(processHandle, remoteBuffer, pathAsUnicodeBytes, pathAsUnicodeBytes.Length, out bytesWritten);
            var kernel32ModuleHandle = WindowsInterop.GetModuleHandle("Kernel32");
            var procAddress = WindowsInterop.GetProcAddress(kernel32ModuleHandle, "LoadLibraryW");
            var remoteThread = WindowsInterop.CreateRemoteThread(processHandle, IntPtr.Zero, 0, procAddress, remoteBuffer, 0, IntPtr.Zero);
        }
    }
}
