using System;
using System.Collections.Generic;

namespace EQBoxTool
{
    public class AppConfig
    {
        public bool BorderlessGameWindows { get; set; }
        public List<EqInstall> EverquestInstalls { get; set; } = new List<EqInstall>();
        public List<Server> Servers { get; set; } = new List<Server>();
        public List<Account> Accounts { get; set; } = new List<Account>();
        public List<Character> Characters { get; set; } = new List<Character>();
        public List<Profile> Profiles { get; set; } = new List<Profile>();
        public Guid? CurrentProfileId { get; set; }
    }
}

