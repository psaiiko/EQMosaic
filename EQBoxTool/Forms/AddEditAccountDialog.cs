using System;
using System.Linq;
using System.Windows.Forms;

namespace EQBoxTool
{
    public partial class AddEditAccountDialog : Form
    {
        public string AccountLoginName => loginNameTextBox.Text.Trim();
        public string AccountPassword => passwordTextBox.Text;
        public Guid SelectedInstallId
        {
            get
            {
                var selected = installComboBox.SelectedItem as EqInstall;
                return selected?.Id ?? Guid.Empty;
            }
        }

        public AddEditAccountDialog()
        {
            InitializeComponent();
        }

        public AddEditAccountDialog(EqInstall[] installs) : this()
        {
            PopulateInstalls(installs);
        }

        public AddEditAccountDialog(string loginName, Guid currentInstallId, EqInstall[] installs) : this(installs)
        {
            loginNameTextBox.Text = loginName;
            for (int i = 0; i < installComboBox.Items.Count; i++)
            {
                if (((EqInstall)installComboBox.Items[i]).Id == currentInstallId)
                {
                    installComboBox.SelectedIndex = i;
                    break;
                }
            }
        }

        public AddEditAccountDialog(string loginName, string password, Guid currentInstallId, EqInstall[] installs) : this(installs)
        {
            loginNameTextBox.Text = loginName;
            passwordTextBox.Text = password;
            for (int i = 0; i < installComboBox.Items.Count; i++)
            {
                if (((EqInstall)installComboBox.Items[i]).Id == currentInstallId)
                {
                    installComboBox.SelectedIndex = i;
                    break;
                }
            }
        }

        private void PopulateInstalls(EqInstall[] installs)
        {
            installComboBox.Items.AddRange(installs);
            if (installComboBox.Items.Count > 0)
                installComboBox.SelectedIndex = 0;
        }

        private void OkButton_Click(object sender, EventArgs e)
        {
            if (string.IsNullOrWhiteSpace(AccountLoginName))
            {
                MessageBox.Show(this, "Login name cannot be empty.", "Validation", MessageBoxButtons.OK, MessageBoxIcon.Warning);
                DialogResult = DialogResult.None;
                return;
            }
            if (string.IsNullOrWhiteSpace(AccountPassword))
            {
                MessageBox.Show(this, "Password cannot be empty.", "Validation", MessageBoxButtons.OK, MessageBoxIcon.Warning);
                DialogResult = DialogResult.None;
                return;
            }
            if (SelectedInstallId == Guid.Empty)
            {
                MessageBox.Show(this, "Please select an EQ install.", "Validation", MessageBoxButtons.OK, MessageBoxIcon.Warning);
                DialogResult = DialogResult.None;
                return;
            }
        }
    }
}
