namespace EQBoxTool
{
    partial class AddEditAccountDialog
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
            loginNameTextBox = new System.Windows.Forms.TextBox();
            passwordTextBox = new System.Windows.Forms.TextBox();
            installComboBox = new System.Windows.Forms.ComboBox();
            loginLabel = new System.Windows.Forms.Label();
            passwordLabel = new System.Windows.Forms.Label();
            installLabel = new System.Windows.Forms.Label();
            okButton = new System.Windows.Forms.Button();
            cancelButton = new System.Windows.Forms.Button();
            SuspendLayout();
            //
            // loginNameTextBox
            //
            loginNameTextBox.Location = new System.Drawing.Point(98, 12);
            loginNameTextBox.Name = "loginNameTextBox";
            loginNameTextBox.Size = new System.Drawing.Size(274, 23);
            //
            // passwordTextBox
            //
            passwordTextBox.Location = new System.Drawing.Point(98, 42);
            passwordTextBox.Name = "passwordTextBox";
            passwordTextBox.PasswordChar = '*';
            passwordTextBox.Size = new System.Drawing.Size(274, 23);
            //
            // installComboBox
            //
            installComboBox.DisplayMember = "Name";
            installComboBox.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            installComboBox.Location = new System.Drawing.Point(98, 72);
            installComboBox.Name = "installComboBox";
            installComboBox.Size = new System.Drawing.Size(274, 23);
            //
            // loginLabel
            //
            loginLabel.Location = new System.Drawing.Point(12, 15);
            loginLabel.Name = "loginLabel";
            loginLabel.Size = new System.Drawing.Size(80, 23);
            loginLabel.Text = "Login Name:";
            //
            // passwordLabel
            //
            passwordLabel.Location = new System.Drawing.Point(12, 45);
            passwordLabel.Name = "passwordLabel";
            passwordLabel.Size = new System.Drawing.Size(80, 23);
            passwordLabel.Text = "Password:";
            //
            // installLabel
            //
            installLabel.Location = new System.Drawing.Point(12, 75);
            installLabel.Name = "installLabel";
            installLabel.Size = new System.Drawing.Size(80, 23);
            installLabel.Text = "EQ Install:";
            //
            // okButton
            //
            okButton.DialogResult = System.Windows.Forms.DialogResult.OK;
            okButton.Location = new System.Drawing.Point(216, 107);
            okButton.Name = "okButton";
            okButton.Size = new System.Drawing.Size(75, 23);
            okButton.Text = "OK";
            okButton.Click += OkButton_Click;
            //
            // cancelButton
            //
            cancelButton.DialogResult = System.Windows.Forms.DialogResult.Cancel;
            cancelButton.Location = new System.Drawing.Point(297, 107);
            cancelButton.Name = "cancelButton";
            cancelButton.Size = new System.Drawing.Size(75, 23);
            cancelButton.Text = "Cancel";
            //
            // AddEditAccountDialog
            //
            AutoScaleDimensions = new System.Drawing.SizeF(7F, 15F);
            AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            ClientSize = new System.Drawing.Size(384, 168);
            Controls.Add(loginNameTextBox);
            Controls.Add(loginLabel);
            Controls.Add(passwordTextBox);
            Controls.Add(passwordLabel);
            Controls.Add(installComboBox);
            Controls.Add(installLabel);
            Controls.Add(okButton);
            Controls.Add(cancelButton);
            FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedDialog;
            MaximizeBox = false;
            MinimizeBox = false;
            Name = "AddEditAccountDialog";
            ShowIcon = false;
            ShowInTaskbar = false;
            StartPosition = System.Windows.Forms.FormStartPosition.CenterScreen;
            Text = "Add Account";
            ResumeLayout(false);
        }

        private System.Windows.Forms.TextBox loginNameTextBox;
        private System.Windows.Forms.TextBox passwordTextBox;
        private System.Windows.Forms.ComboBox installComboBox;
        private System.Windows.Forms.Label loginLabel;
        private System.Windows.Forms.Label passwordLabel;
        private System.Windows.Forms.Label installLabel;
        private System.Windows.Forms.Button okButton;
        private System.Windows.Forms.Button cancelButton;
    }
}
