using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text.Json;

namespace EQBoxTool
{
    public static class ConfigManager
    {
        private static readonly string ConfigDirectory = Path.Combine(
            Environment.GetFolderPath(Environment.SpecialFolder.LocalApplicationData),
            "EQBoxTool");

        private static readonly string ConfigPath = Path.Combine(ConfigDirectory, "config.json");

        public static AppConfig Load()
        {
            try
            {
                if (File.Exists(ConfigPath))
                {
                    var json = File.ReadAllText(ConfigPath);
                    var config = JsonSerializer.Deserialize<AppConfig>(json);
                    if (config == null)
                        return new AppConfig();

                    bool dirty = false;
                    foreach (var install in config.EverquestInstalls)
                    {
                        if (install.Id == Guid.Empty)
                        {
                            install.Id = Guid.NewGuid();
                            dirty = true;
                        }
                    }

                    foreach (var account in config.Accounts)
                    {
                        if (account.Id == Guid.Empty)
                        {
                            account.Id = Guid.NewGuid();
                            dirty = true;
                        }
                        account.LoginName = EncryptionHelper.DecryptString(account.LoginName, MasterKeyManager.MasterKey);
                        if (!string.IsNullOrEmpty(account.Password))
                        {
                            account.Password = EncryptionHelper.DecryptString(account.Password, MasterKeyManager.MasterKey);
                        }
                    }

                    foreach (var server in config.Servers)
                    {
                        if (server.Id == Guid.Empty)
                        {
                            server.Id = Guid.NewGuid();
                            dirty = true;
                        }
                    }

                    foreach (var ch in config.Characters)
                    {
                        if (ch.Id == Guid.Empty)
                        {
                            ch.Id = Guid.NewGuid();
                            dirty = true;
                        }
                        if (ch.ExtensionData != null)
                        {
                            if (ch.AccountId == Guid.Empty &&
                                ch.ExtensionData.TryGetValue("AccountLoginName", out var oldLogin))
                            {
                                var oldLoginStr = oldLogin?.ToString();
                                if (!string.IsNullOrEmpty(oldLoginStr))
                                {
                                    var matchedAccount = config.Accounts.Find(a => a.LoginName == oldLoginStr);
                                    if (matchedAccount != null)
                                        ch.AccountId = matchedAccount.Id;
                                }
                                dirty = true;
                            }
                            if (ch.ServerId == Guid.Empty &&
                                ch.ExtensionData.TryGetValue("ServerName", out var oldServer))
                            {
                                var oldServerStr = oldServer?.ToString();
                                if (!string.IsNullOrEmpty(oldServerStr))
                                {
                                    var matchedServer = config.Servers.Find(s => s.Name == oldServerStr);
                                    if (matchedServer != null)
                                        ch.ServerId = matchedServer.Id;
                                }
                                dirty = true;
                            }
                            ch.ExtensionData = null;
                        }
                    }

                    foreach (var profile in config.Profiles)
                    {
                        if (profile.Id == Guid.Empty)
                        {
                            profile.Id = Guid.NewGuid();
                            dirty = true;
                        }
                        if (profile.Columns == 0)
                        {
                            profile.Columns = 2;
                            dirty = true;
                        }
                        if (profile.Rows == 0)
                        {
                            profile.Rows = 2;
                            dirty = true;
                        }
                    }

                    if (dirty)
                        Save(config);

                    return config;
                }
            }
            catch
            {
            }
            return new AppConfig();
        }

        public static void Save(AppConfig config)
        {
            try
            {
                foreach (var account in config.Accounts)
                {
                    account.LoginName = EncryptionHelper.EncryptString(account.LoginName, MasterKeyManager.MasterKey);
                    if (!string.IsNullOrEmpty(account.Password))
                    {
                        account.Password = EncryptionHelper.EncryptString(account.Password, MasterKeyManager.MasterKey);
                    }
                }

                foreach (var profile in config.Profiles)
                {
                    foreach (var entry in profile.Entries)
                    {
                        if (entry.CharacterId == Guid.Empty)
                        {
                            // Try to find character by name only
                            var character = config.Characters.FirstOrDefault(c => c.Id == entry.CharacterId);
                            if (character != null)
                            {
                                entry.CharacterId = character.Id;
                            }
                        }
                    }
                }

                Directory.CreateDirectory(ConfigDirectory);
                var json = JsonSerializer.Serialize(config, new JsonSerializerOptions { WriteIndented = true });

                foreach (var account in config.Accounts)
                {
                    account.LoginName = EncryptionHelper.DecryptString(account.LoginName, MasterKeyManager.MasterKey);
                    if (!string.IsNullOrEmpty(account.Password))
                    {
                        account.Password = EncryptionHelper.DecryptString(account.Password, MasterKeyManager.MasterKey);
                    }
                }

                File.WriteAllText(ConfigPath, json);
            }
            catch
            {
            }
        }

        public static string GetCharacterDisplayText(Character character, AppConfig config)
        {
            if (character == null) return null;
            var server = config.Servers.FirstOrDefault(s => s.Id == character.ServerId);
            return server != null ? $"{character.Name} ({server.Name})" : character.Name;
        }

        public static Character GetCharacterByIdOrName(AppConfig config, string characterNameOrFullDisplay)
        {
            if (string.IsNullOrEmpty(characterNameOrFullDisplay))
                return null;

            var withParen = characterNameOrFullDisplay.Contains(" (");
            if (withParen)
            {
                var part1 = characterNameOrFullDisplay.Substring(0, characterNameOrFullDisplay.IndexOf(" ("));
                var part2 = characterNameOrFullDisplay.Substring(characterNameOrFullDisplay.IndexOf(" (") + 2,
                    characterNameOrFullDisplay.IndexOf(")") - characterNameOrFullDisplay.IndexOf(" (") - 1);

                return config.Characters.FirstOrDefault(c =>
                    c.Name == part1 &&
                    config.Servers.Any(s => s.Id == c.ServerId && s.Name == part2));
            }
            else
            {
                return config.Characters.FirstOrDefault(c => c.Name == characterNameOrFullDisplay);
            }
        }
    }
}


