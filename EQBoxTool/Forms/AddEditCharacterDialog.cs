using System;
using System.Linq;
using System.Windows.Forms;

namespace EQBoxTool
{
    public partial class AddEditCharacterDialog : Form
    {
        public string CharacterName => nameTextBox.Text.Trim();
        public string SelectedClass => classComboBox.SelectedItem as string;
        public int SelectedLevel => (int)levelNumericUpDown.Value;
        public Guid SelectedServerId
        {
            get
            {
                var selected = serverComboBox.SelectedItem as Server;
                return selected?.Id ?? Guid.Empty;
            }
        }
        public Guid SelectedAccountId
        {
            get
            {
                var selected = accountComboBox.SelectedItem as Account;
                return selected?.Id ?? Guid.Empty;
            }
        }

        public AddEditCharacterDialog()
        {
            InitializeComponent();
        }

        public AddEditCharacterDialog(Server[] servers, Account[] accounts) : this()
        {
            PopulateData(servers, accounts);
        }

        public AddEditCharacterDialog(string name, string className, int level, Guid serverId, Guid accountId,
            Server[] servers, Account[] accounts) : this(servers, accounts)
        {
            nameTextBox.Text = name;
            levelNumericUpDown.Value = level;

            if (className != null)
            {
                for (int i = 0; i < classComboBox.Items.Count; i++)
                {
                    if ((string)classComboBox.Items[i] == className)
                    {
                        classComboBox.SelectedIndex = i;
                        break;
                    }
                }
            }

            if (serverId != Guid.Empty)
            {
                for (int i = 0; i < serverComboBox.Items.Count; i++)
                {
                    if (((Server)serverComboBox.Items[i]).Id == serverId)
                    {
                        serverComboBox.SelectedIndex = i;
                        break;
                    }
                }
            }

            if (accountId != Guid.Empty)
            {
                for (int i = 0; i < accountComboBox.Items.Count; i++)
                {
                    if (((Account)accountComboBox.Items[i]).Id == accountId)
                    {
                        accountComboBox.SelectedIndex = i;
                        break;
                    }
                }
            }
        }

        private void PopulateData(Server[] servers, Account[] accounts)
        {
            classComboBox.Items.AddRange(EqClasses.All.Select(c => c.Name).ToArray());
            classComboBox.SelectedIndex = -1;

            serverComboBox.Items.AddRange(servers);
            if (serverComboBox.Items.Count > 0)
                serverComboBox.SelectedIndex = 0;

            accountComboBox.Items.AddRange(accounts);
            if (accountComboBox.Items.Count > 0)
                accountComboBox.SelectedIndex = 0;
        }

        private void OkButton_Click(object sender, EventArgs e)
        {
            if (string.IsNullOrWhiteSpace(CharacterName))
            {
                MessageBox.Show(this, "Character name cannot be empty.", "Validation", MessageBoxButtons.OK, MessageBoxIcon.Warning);
                DialogResult = DialogResult.None;
            }
        }
    }
}
