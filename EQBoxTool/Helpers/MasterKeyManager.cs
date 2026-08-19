using System;
using System.Security.Cryptography;
using Microsoft.Win32;

namespace EQBoxTool
{
    public static class MasterKeyManager
    {
        private const string RegistryPath = @"Software\EQBoxTool";
        private const string HashValueName = "MasterKeyHash";
        private const string EmptyMarker = "empty:";

        public static byte[] MasterKey { get; private set; }

        public static bool IsMasterKeySet()
        {
            try
            {
                using (var key = Registry.CurrentUser.OpenSubKey(RegistryPath))
                {
                    var val = key?.GetValue(HashValueName) as string;
                    return !string.IsNullOrEmpty(val);
                }
            }
            catch
            {
                return false;
            }
        }

        public static bool HasPassword()
        {
            try
            {
                using (var key = Registry.CurrentUser.OpenSubKey(RegistryPath))
                {
                    var val = key?.GetValue(HashValueName) as string;
                    return !string.IsNullOrEmpty(val) && val != EmptyMarker;
                }
            }
            catch
            {
                return false;
            }
        }

        public static void LoadKeyFromRegistry()
        {
            try
            {
                using (var key = Registry.CurrentUser.OpenSubKey(RegistryPath))
                {
                    string stored = key?.GetValue(HashValueName) as string;
                    if (stored == null)
                    {
                        MasterKey = null;
                        return;
                    }

                    if (stored == EmptyMarker)
                    {
                        MasterKey = null;
                        return;
                    }

                    var parts = stored.Split(':');
                    if (parts.Length == 2)
                    {
                        MasterKey = Convert.FromBase64String(parts[1]);
                    }
                }
            }
            catch
            {
                MasterKey = null;
            }
        }

        public static void CreateMasterKey(string password)
        {
            if (string.IsNullOrEmpty(password))
            {
                using (var key = Registry.CurrentUser.CreateSubKey(RegistryPath))
                    key.SetValue(HashValueName, EmptyMarker);
                MasterKey = null;
                return;
            }

            byte[] salt = new byte[32];
            using (var rng = RandomNumberGenerator.Create())
                rng.GetBytes(salt);

            byte[] hash = DeriveKey(password, salt);
            string stored = Convert.ToBase64String(salt) + ":" + Convert.ToBase64String(hash);

            using (var key = Registry.CurrentUser.CreateSubKey(RegistryPath))
                key.SetValue(HashValueName, stored);

            MasterKey = hash;
        }

        public static void ClearMasterKey()
        {
            try
            {
                using (var key = Registry.CurrentUser.CreateSubKey(RegistryPath))
                    key.DeleteValue(HashValueName, false);
            }
            catch
            {
            }
            MasterKey = null;
        }

        public static bool VerifyMasterKey(string password)
        {
            try
            {
                using (var key = Registry.CurrentUser.OpenSubKey(RegistryPath))
                {
                    string stored = key?.GetValue(HashValueName) as string;
                    if (stored == null || stored == EmptyMarker) return false;

                    var parts = stored.Split(':');
                    if (parts.Length != 2) return false;

                    byte[] salt = Convert.FromBase64String(parts[0]);
                    byte[] expectedHash = Convert.FromBase64String(parts[1]);
                    byte[] actualHash = DeriveKey(password, salt);

                    if (CryptographicOperations.FixedTimeEquals(expectedHash, actualHash))
                    {
                        MasterKey = actualHash;
                        return true;
                    }
                    return false;
                }
            }
            catch
            {
                return false;
            }
        }

        private static byte[] DeriveKey(string password, byte[] salt)
        {
            return Rfc2898DeriveBytes.Pbkdf2(password, salt, 600000, HashAlgorithmName.SHA256, 32);
        }
    }
}
