namespace EQBoxTool
{
    public static class EqClasses
    {
        public static readonly (string Name, string Abbv)[] All = new[]
        {
            ("Bard", "BRD"),
            ("Beastlord", "BST"),
            ("Berserker", "BER"),
            ("Cleric", "CLR"),
            ("Druid", "DRU"),
            ("Enchanter", "ENC"),
            ("Magician", "MAG"),
            ("Monk", "MON"),
            ("Necromancer", "NEC"),
            ("Paladin", "PAL"),
            ("Ranger", "RNG"),
            ("Rogue", "ROG"),
            ("Shadow Knight", "SHD"),
            ("Shaman", "SHM"),
            ("Warrior", "WAR"),
            ("Wizard", "WIZ"),
        };

        public static string GetAbbreviation(string className)
        {
            foreach (var c in All)
                if (c.Name == className)
                    return c.Abbv;
            return null;
        }
    }
}
