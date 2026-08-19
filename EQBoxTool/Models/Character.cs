using System;
using System.Collections.Generic;
using System.Text.Json.Serialization;

namespace EQBoxTool
{
    public class Character
    {
        public Guid Id { get; set; }
        public string Name { get; set; }
        public string ClassName { get; set; }
        public int Level { get; set; }
        public Guid ServerId { get; set; }
        public Guid AccountId { get; set; }

        [JsonExtensionData]
        public Dictionary<string, object> ExtensionData { get; set; }
    }
}
