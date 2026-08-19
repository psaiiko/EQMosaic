using System;

namespace EQBoxTool
{
    public class Account
    {
        public Guid Id { get; set; }
        public string LoginName { get; set; }
        public string Password { get; set; }
        public Guid EqInstallId { get; set; }
    }
}

