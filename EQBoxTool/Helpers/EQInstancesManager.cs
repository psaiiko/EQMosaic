using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;

namespace EQBoxTool
{
    public class EQInstancesManager : IDisposable
    {
        public enum ProcessStatus
        {
            New,
            Ready,
            Injecting,
            Injected,
        }

        public class InstanceInfo
        {
            public int ProcessId { get; }
            public string[] CommandLineParts { get; }
            public string LoginName { get; }
            public ProcessStatus Status { get; internal set; }
            public DateTime FirstSeen { get; internal set; }
            public bool IsInjected => Status == ProcessStatus.Injected;

            public InstanceInfo(int processId, string[] commandLineParts, string loginName, ProcessStatus status, DateTime firstSeen)
            {
                ProcessId = processId;
                CommandLineParts = commandLineParts;
                LoginName = loginName;
                Status = status;
                FirstSeen = firstSeen;
            }
        }

        private readonly object _lock = new object();
        private Dictionary<int, InstanceInfo> _processes = new Dictionary<int, InstanceInfo>();
        private System.Threading.Timer _timer;

        public EQInstancesManager(int pollIntervalMs = 1000)
        {
            _timer = new System.Threading.Timer(Poll, null, 0, pollIntervalMs);
        }

        public IReadOnlyList<InstanceInfo> GetInstances()
        {
            lock (_lock)
            {
                return _processes.Values.ToList().AsReadOnly();
            }
        }

        public int? GetProcessIdByLoginName(string loginName)
        {
            var instances = GetInstances();
            return instances.FirstOrDefault(i => i.LoginName == loginName)?.ProcessId;
        }

        private void Poll(object state)
        {
            try
            {
                var runningPids = new HashSet<int>(
                    Process.GetProcessesByName("eqgame").Select(p => p.Id));

                lock (_lock)
                {
                    foreach (var pid in runningPids)
                    {
                        if (_processes.ContainsKey(pid))
                            continue;

                        Process proc = null;
                        try
                        {
                            proc = Process.GetProcessById(pid);
                            var cmdLine = WindowsInterop.GetProcessCommandLineWmi(proc);
                            var parts = cmdLine?.Split(new char[] { ' ', ':' },
                                StringSplitOptions.RemoveEmptyEntries) ?? Array.Empty<string>();
                            string loginName = null;
                            for (int i = 0; i < parts.Length - 1; ++i)
                            {
                                if (parts[i] == "/login")
                                {
                                    loginName = parts[i + 1];
                                    break;
                                }
                            }

                            var alreadyInjected = WindowsInterop.IsModuleLoadedInProcess(
                                pid, GlobalsHelper.OverlayDllName);
                            var status = alreadyInjected ? ProcessStatus.Injected : ProcessStatus.New;

                            _processes[pid] = new InstanceInfo(
                                pid, parts, loginName, status, DateTime.UtcNow);
                        }
                        finally
                        {
                            proc?.Dispose();
                        }
                    }

                    var deadPids = _processes.Keys
                        .Where(p => !runningPids.Contains(p)).ToList();
                    foreach (var pid in deadPids)
                        _processes.Remove(pid);
                }


            }
            catch
            {
            }
        }

        public void Dispose()
        {
            _timer?.Dispose();
        }
    }
}
