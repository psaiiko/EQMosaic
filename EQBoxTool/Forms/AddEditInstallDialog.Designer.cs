namespace EQBoxTool
{
    partial class AddEditInstallDialog
    {
        private System.ComponentModel.IContainer components = null;

        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        private void InitializeComponent()
        {
            nameTextBox = new System.Windows.Forms.TextBox();
            pathTextBox = new System.Windows.Forms.TextBox();
            nameLabel = new System.Windows.Forms.Label();
            pathLabel = new System.Windows.Forms.Label();
            browseButton = new System.Windows.Forms.Button();
            okButton = new System.Windows.Forms.Button();
            cancelButton = new System.Windows.Forms.Button();
            SuspendLayout();
            //
            // nameTextBox
            //
            nameTextBox.Location = new System.Drawing.Point(78, 12);
            nameTextBox.Name = "nameTextBox";
            nameTextBox.Size = new System.Drawing.Size(344, 23);
            //
            // pathTextBox
            //
            pathTextBox.Location = new System.Drawing.Point(78, 44);
            pathTextBox.Name = "pathTextBox";
            pathTextBox.Size = new System.Drawing.Size(280, 23);
            //
            // nameLabel
            //
            nameLabel.Location = new System.Drawing.Point(12, 15);
            nameLabel.Name = "nameLabel";
            nameLabel.Size = new System.Drawing.Size(60, 23);
            nameLabel.Text = "Name:";
            //
            // pathLabel
            //
            pathLabel.Location = new System.Drawing.Point(12, 47);
            pathLabel.Name = "pathLabel";
            pathLabel.Size = new System.Drawing.Size(60, 23);
            pathLabel.Text = "Path:";
            //
            // browseButton
            //
            browseButton.Location = new System.Drawing.Point(364, 43);
            browseButton.Name = "browseButton";
            browseButton.Size = new System.Drawing.Size(58, 25);
            browseButton.Text = "...";
            browseButton.Click += BrowseButton_Click;
            //
            // okButton
            //
            okButton.DialogResult = System.Windows.Forms.DialogResult.OK;
            okButton.Location = new System.Drawing.Point(266, 85);
            okButton.Name = "okButton";
            okButton.Size = new System.Drawing.Size(75, 23);
            okButton.Text = "OK";
            okButton.Click += OkButton_Click;
            //
            // cancelButton
            //
            cancelButton.DialogResult = System.Windows.Forms.DialogResult.Cancel;
            cancelButton.Location = new System.Drawing.Point(347, 85);
            cancelButton.Name = "cancelButton";
            cancelButton.Size = new System.Drawing.Size(75, 23);
            cancelButton.Text = "Cancel";
            //
            // AddEditInstallDialog
            //
            AutoScaleDimensions = new System.Drawing.SizeF(7F, 15F);
            AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            ClientSize = new System.Drawing.Size(434, 128);
            Controls.Add(nameTextBox);
            Controls.Add(nameLabel);
            Controls.Add(pathTextBox);
            Controls.Add(pathLabel);
            Controls.Add(browseButton);
            Controls.Add(okButton);
            Controls.Add(cancelButton);
            FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedDialog;
            MaximizeBox = false;
            MinimizeBox = false;
            Name = "AddEditInstallDialog";
            ShowIcon = false;
            ShowInTaskbar = false;
            StartPosition = System.Windows.Forms.FormStartPosition.CenterScreen;
            Text = "Add EverQuest Install";
            ResumeLayout(false);
        }

        private System.Windows.Forms.TextBox nameTextBox;
        private System.Windows.Forms.TextBox pathTextBox;
        private System.Windows.Forms.Label nameLabel;
        private System.Windows.Forms.Label pathLabel;
        private System.Windows.Forms.Button browseButton;
        private System.Windows.Forms.Button okButton;
        private System.Windows.Forms.Button cancelButton;
    }
}
