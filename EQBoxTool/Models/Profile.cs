using System;
using System.Collections.Generic;

namespace EQBoxTool
{
    public class Profile
    {
        public Guid Id { get; set; }
        public string Name { get; set; }
        public int Columns { get; set; } = 2;
        public bool AutoRows { get; set; } = true;
        public int Rows { get; set; } = 2;
        public List<ProfileEntry> Entries { get; set; } = new List<ProfileEntry>();
    }
}

