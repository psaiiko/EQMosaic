namespace EQBoxTool
{
    partial class MasterPasswordDialog
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
            promptLabel = new System.Windows.Forms.Label();
            passwordLabel = new System.Windows.Forms.Label();
            passwordTextBox = new System.Windows.Forms.TextBox();
            confirmLabel = new System.Windows.Forms.Label();
            confirmTextBox = new System.Windows.Forms.TextBox();
            okButton = new System.Windows.Forms.Button();
            cancelButton = new System.Windows.Forms.Button();
            SuspendLayout();
            //
            // promptLabel
            //
            promptLabel.Location = new System.Drawing.Point(12, 12);
            promptLabel.Name = "promptLabel";
            promptLabel.Size = new System.Drawing.Size(380, 30);
            //
            // passwordLabel
            //
            passwordLabel.Location = new System.Drawing.Point(12, 50);
            passwordLabel.Name = "passwordLabel";
            passwordLabel.Size = new System.Drawing.Size(80, 23);
            passwordLabel.Text = "Password:";
            //
            // passwordTextBox
            //
            passwordTextBox.Location = new System.Drawing.Point(98, 47);
            passwordTextBox.Name = "passwordTextBox";
            passwordTextBox.Size = new System.Drawing.Size(294, 23);
            passwordTextBox.UseSystemPasswordChar = true;
            //
            // confirmLabel
            //
            confirmLabel.Location = new System.Drawing.Point(12, 80);
            confirmLabel.Name = "confirmLabel";
            confirmLabel.Size = new System.Drawing.Size(80, 23);
            confirmLabel.Text = "Confirm:";
            //
            // confirmTextBox
            //
            confirmTextBox.Location = new System.Drawing.Point(98, 77);
            confirmTextBox.Name = "confirmTextBox";
            confirmTextBox.Size = new System.Drawing.Size(294, 23);
            confirmTextBox.UseSystemPasswordChar = true;
            //
            // okButton
            //
            okButton.DialogResult = System.Windows.Forms.DialogResult.OK;
            okButton.Location = new System.Drawing.Point(236, 120);
            okButton.Name = "okButton";
            okButton.Size = new System.Drawing.Size(75, 23);
            okButton.Text = "OK";
            //
            // cancelButton
            //
            cancelButton.DialogResult = System.Windows.Forms.DialogResult.Cancel;
            cancelButton.Location = new System.Drawing.Point(317, 120);
            cancelButton.Name = "cancelButton";
            cancelButton.Size = new System.Drawing.Size(75, 23);
            cancelButton.Text = "Cancel";
            //
            // MasterPasswordDialog
            //
            AutoScaleDimensions = new System.Drawing.SizeF(7F, 15F);
            AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            ClientSize = new System.Drawing.Size(404, 168);
            Controls.Add(promptLabel);
            Controls.Add(passwordLabel);
            Controls.Add(passwordTextBox);
            Controls.Add(confirmLabel);
            Controls.Add(confirmTextBox);
            Controls.Add(okButton);
            Controls.Add(cancelButton);
            FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedDialog;
            MaximizeBox = false;
            MinimizeBox = false;
            Name = "MasterPasswordDialog";
            ShowIcon = false;
            ShowInTaskbar = false;
            StartPosition = System.Windows.Forms.FormStartPosition.CenterScreen;
            ResumeLayout(false);
        }

        private System.Windows.Forms.Label promptLabel;
        private System.Windows.Forms.Label passwordLabel;
        private System.Windows.Forms.TextBox passwordTextBox;
        private System.Windows.Forms.Label confirmLabel;
        private System.Windows.Forms.TextBox confirmTextBox;
        private System.Windows.Forms.Button okButton;
        private System.Windows.Forms.Button cancelButton;
    }
}
