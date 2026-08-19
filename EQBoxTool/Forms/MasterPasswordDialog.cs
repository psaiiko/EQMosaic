using System;
using System.Windows.Forms;

namespace EQBoxTool
{
    public partial class MasterPasswordDialog : Form
    {
        public string Password => passwordTextBox.Text;

        public MasterPasswordDialog()
        {
            InitializeComponent();
        }

        public static bool TrySetNewPassword(out string password)
        {
            using (var dialog = new MasterPasswordDialog())
            {
                dialog.Text = "Create Master Password";
                dialog.promptLabel.Text = "Create a master password to protect your EQBoxTool config\n(leave empty for no encryption):";
                dialog.confirmLabel.Visible = true;
                dialog.confirmTextBox.Visible = true;

                while (dialog.ShowDialog() == DialogResult.OK)
                {
                    if (dialog.passwordTextBox.Text != dialog.confirmTextBox.Text)
                    {
                        MessageBox.Show(dialog, "Passwords do not match.", "Validation", MessageBoxButtons.OK, MessageBoxIcon.Warning);
                        continue;
                    }
                    if (string.IsNullOrEmpty(dialog.passwordTextBox.Text))
                    {
                        var result = MessageBox.Show(dialog,
                            "An empty password means no encryption will be applied to your config data.\n\nAre you sure?",
                            "No Encryption",
                            MessageBoxButtons.YesNo,
                            MessageBoxIcon.Warning);
                        if (result != DialogResult.Yes)
                            continue;
                    }
                    password = dialog.Password;
                    return true;
                }
            }
            password = null;
            return false;
        }

        public static bool TryVerifyPassword(out string password)
        {
            using (var dialog = new MasterPasswordDialog())
            {
                dialog.Text = "Enter Master Password";
                dialog.promptLabel.Text = "Enter your EQBoxTool master password:";
                dialog.confirmLabel.Visible = false;
                dialog.confirmTextBox.Visible = false;

                while (dialog.ShowDialog() == DialogResult.OK)
                {
                    if (!MasterKeyManager.VerifyMasterKey(dialog.passwordTextBox.Text))
                    {
                        MessageBox.Show(dialog, "Incorrect password.", "Verification Failed", MessageBoxButtons.OK, MessageBoxIcon.Error);
                        continue;
                    }
                    password = dialog.Password;
                    return true;
                }
            }
            password = null;
            return false;
        }

        public static bool TryChangePassword()
        {
            bool hadPassword = MasterKeyManager.HasPassword();

            using (var dialog = new MasterPasswordDialog())
            {
                dialog.Text = "Change Master Password";
                dialog.Size = new System.Drawing.Size(420, 240);
                dialog.promptLabel.Text = hadPassword
                    ? "Enter your current password, then the new one:"
                    : "Set a new master password (leave empty for no encryption):";

                dialog.confirmLabel.Text = "New password:";
                dialog.confirmTextBox.Text = "";
                dialog.confirmLabel.Visible = true;
                dialog.confirmTextBox.Visible = true;

                var currentLabel = new Label
                {
                    Text = "Current:",
                    Location = new System.Drawing.Point(12, 50),
                    Size = new System.Drawing.Size(80, 23),
                    Visible = hadPassword,
                };

                var currentTextBox = new TextBox
                {
                    Location = new System.Drawing.Point(98, 47),
                    Size = new System.Drawing.Size(294, 23),
                    UseSystemPasswordChar = true,
                    Visible = hadPassword,
                };

                var newLabel = new Label
                {
                    Text = "New:",
                    Location = new System.Drawing.Point(12, hadPassword ? 80 : 50),
                    Size = new System.Drawing.Size(80, 23),
                };

                var newTextBox = new TextBox
                {
                    Location = new System.Drawing.Point(98, hadPassword ? 77 : 47),
                    Size = new System.Drawing.Size(294, 23),
                    UseSystemPasswordChar = true,
                };

                var confirmLabel = new Label
                {
                    Text = "Confirm:",
                    Location = new System.Drawing.Point(12, hadPassword ? 110 : 80),
                    Size = new System.Drawing.Size(80, 23),
                };

                var confirmTextBox = new TextBox
                {
                    Location = new System.Drawing.Point(98, hadPassword ? 107 : 77),
                    Size = new System.Drawing.Size(294, 23),
                    UseSystemPasswordChar = true,
                };

                int okY = hadPassword ? 145 : 120;
                var okButton = new Button
                {
                    Text = "OK",
                    DialogResult = DialogResult.OK,
                    Location = new System.Drawing.Point(236, okY),
                    Size = new System.Drawing.Size(75, 23),
                };

                var cancelButton = new Button
                {
                    Text = "Cancel",
                    DialogResult = DialogResult.Cancel,
                    Location = new System.Drawing.Point(317, okY),
                    Size = new System.Drawing.Size(75, 23),
                };

                dialog.Controls.Clear();
                dialog.Controls.AddRange(new Control[] {
                    dialog.promptLabel,
                    currentLabel, currentTextBox,
                    newLabel, newTextBox,
                    confirmLabel, confirmTextBox,
                    okButton, cancelButton
                });

                while (dialog.ShowDialog() == DialogResult.OK)
                {
                    if (hadPassword && !MasterKeyManager.VerifyMasterKey(currentTextBox.Text))
                    {
                        MessageBox.Show(dialog, "Current password is incorrect.", "Verification Failed", MessageBoxButtons.OK, MessageBoxIcon.Error);
                        continue;
                    }
                    if (newTextBox.Text != confirmTextBox.Text)
                    {
                        MessageBox.Show(dialog, "New passwords do not match.", "Validation", MessageBoxButtons.OK, MessageBoxIcon.Warning);
                        continue;
                    }
                    if (string.IsNullOrEmpty(newTextBox.Text))
                    {
                        var result = MessageBox.Show(dialog,
                            "An empty password means no encryption will be applied to your config data.\n\nAre you sure?",
                            "No Encryption",
                            MessageBoxButtons.YesNo,
                            MessageBoxIcon.Warning);
                        if (result != DialogResult.Yes)
                            continue;
                    }
                    MasterKeyManager.CreateMasterKey(newTextBox.Text);
                    return true;
                }
            }
            return false;
        }
    }
}
