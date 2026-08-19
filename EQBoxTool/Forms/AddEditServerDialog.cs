using System;
using System.Windows.Forms;

namespace EQBoxTool
{
    public partial class AddEditServerDialog : Form
    {
        public string ServerName => nameTextBox.Text.Trim();
        public bool RequiresAllAccess => allAccessCheckBox.Checked;

        public AddEditServerDialog()
        {
            InitializeComponent();
        }

        public AddEditServerDialog(string name, bool requiresAllAccess) : this()
        {
            nameTextBox.Text = name;
            allAccessCheckBox.Checked = requiresAllAccess;
        }

        private void OkButton_Click(object sender, EventArgs e)
        {
            if (string.IsNullOrWhiteSpace(ServerName))
            {
                MessageBox.Show(this, "Server name cannot be empty.", "Validation", MessageBoxButtons.OK, MessageBoxIcon.Warning);
                DialogResult = DialogResult.None;
            }
        }
    }
}
