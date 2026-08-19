using System;
using System.Collections.Generic;
using System.Text.Json.Serialization;

namespace EQBoxTool
{
    public class ProfileEntry
    {
        public Guid CharacterId { get; set; }
        public bool Enabled { get; set; }
        public int Order { get; set; }

        [JsonExtensionData]
        public Dictionary<string, object> ExtensionData { get; set; }
    }
}

