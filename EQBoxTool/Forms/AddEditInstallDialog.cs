using System;
using System.Windows.Forms;

namespace EQBoxTool
{
    public partial class AddEditInstallDialog : Form
    {
        public string InstallName => nameTextBox.Text.Trim();
        public string InstallPath => pathTextBox.Text.Trim();

        public AddEditInstallDialog()
        {
            InitializeComponent();
        }

        public AddEditInstallDialog(string name, string path) : this()
        {
            nameTextBox.Text = name;
            pathTextBox.Text = path;
        }

        private void BrowseButton_Click(object sender, EventArgs e)
        {
            using (var dialog = new FolderBrowserDialog())
            {
                dialog.Description = "Select EverQuest installation folder";
                if (!string.IsNullOrEmpty(pathTextBox.Text))
                    dialog.SelectedPath = pathTextBox.Text;
                if (dialog.ShowDialog(this) == DialogResult.OK)
                    pathTextBox.Text = dialog.SelectedPath;
            }
        }

        private void OkButton_Click(object sender, EventArgs e)
        {
            if (string.IsNullOrWhiteSpace(InstallName))
            {
                MessageBox.Show(this, "Name cannot be empty.", "Validation", MessageBoxButtons.OK, MessageBoxIcon.Warning);
                DialogResult = DialogResult.None;
                return;
            }
            if (string.IsNullOrWhiteSpace(InstallPath))
            {
                MessageBox.Show(this, "Path cannot be empty.", "Validation", MessageBoxButtons.OK, MessageBoxIcon.Warning);
                DialogResult = DialogResult.None;
                return;
            }
        }
    }
}
