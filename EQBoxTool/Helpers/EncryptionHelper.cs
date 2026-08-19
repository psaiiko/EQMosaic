using System;
using System.IO;
using System.Security.Cryptography;
using System.Text;

namespace EQBoxTool
{
    public static class EncryptionHelper
    {
        private const string Prefix = "$AES$";

        public static string EncryptString(string plaintext, byte[] key)
        {
            if (key == null || string.IsNullOrEmpty(plaintext))
                return plaintext;

            using (var aes = Aes.Create())
            {
                aes.Key = key;
                byte[] iv = aes.IV;
                using (var encryptor = aes.CreateEncryptor())
                {
                    byte[] plainBytes = Encoding.UTF8.GetBytes(plaintext);
                    byte[] cipherBytes = encryptor.TransformFinalBlock(plainBytes, 0, plainBytes.Length);

                    using (var ms = new MemoryStream())
                    {
                        ms.Write(iv, 0, iv.Length);
                        ms.Write(cipherBytes, 0, cipherBytes.Length);
                        return Prefix + Convert.ToBase64String(ms.ToArray());
                    }
                }
            }
        }

        public static string DecryptString(string ciphertext, byte[] key)
        {
            if (key == null || string.IsNullOrEmpty(ciphertext))
                return ciphertext;

            if (!ciphertext.StartsWith(Prefix))
                return ciphertext;

            try
            {
                byte[] data = Convert.FromBase64String(ciphertext.Substring(Prefix.Length));
                using (var aes = Aes.Create())
                {
                    aes.Key = key;
                    byte[] iv = new byte[16];
                    byte[] cipher = new byte[data.Length - 16];
                    Buffer.BlockCopy(data, 0, iv, 0, 16);
                    Buffer.BlockCopy(data, 16, cipher, 0, cipher.Length);
                    aes.IV = iv;

                    using (var decryptor = aes.CreateDecryptor())
                    {
                        byte[] plainBytes = decryptor.TransformFinalBlock(cipher, 0, cipher.Length);
                        return Encoding.UTF8.GetString(plainBytes);
                    }
                }
            }
            catch
            {
                return ciphertext;
            }
        }

        public static string ProtectString(string text)
        {
            if (string.IsNullOrEmpty(text))
                return string.Empty;

            byte[] plainBytes = Encoding.UTF8.GetBytes(text);

            byte[] protectedBytes = ProtectedData.Protect(
                plainBytes,
                null,
                DataProtectionScope.CurrentUser);

            return Convert.ToBase64String(protectedBytes);
        }

        public static string UnprotectString(string protectedText)
        {
            if (string.IsNullOrEmpty(protectedText))
                return string.Empty;

            byte[] protectedBytes = Convert.FromBase64String(protectedText);

            byte[] plainBytes = ProtectedData.Unprotect(
                protectedBytes,
                null,
                DataProtectionScope.CurrentUser);

            return Encoding.UTF8.GetString(plainBytes);
        }
    }
}
