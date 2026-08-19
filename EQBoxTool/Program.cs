using System;
using System.Windows.Forms;

namespace EQBoxTool
{
    static class Program
    {
        [STAThread]
        static void Main()
        {
            Application.EnableVisualStyles();
            Application.SetCompatibleTextRenderingDefault(false);

            if (!InitializeMasterKey())
                return;

            var config = ConfigManager.Load();

            var form = new CharacterSelectionForm();
            form.Initialize(config);

            var renderTimer = new Timer { Interval = 16 };
            renderTimer.Tick += (s, e) => form.OnDraw();
            renderTimer.Start();

            Application.Run(form);
        }

        private static bool InitializeMasterKey()
        {
            if (MasterKeyManager.IsMasterKeySet())
            {
                MasterKeyManager.LoadKeyFromRegistry();
                return true;
            }

            if (!MasterPasswordDialog.TrySetNewPassword(out var password))
                return false;

            MasterKeyManager.CreateMasterKey(password);
            return true;
        }
    }
}
