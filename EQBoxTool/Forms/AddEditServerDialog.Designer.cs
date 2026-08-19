namespace EQBoxTool
{
    partial class AddEditServerDialog
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
            allAccessCheckBox = new System.Windows.Forms.CheckBox();
            nameLabel = new System.Windows.Forms.Label();
            okButton = new System.Windows.Forms.Button();
            cancelButton = new System.Windows.Forms.Button();
            SuspendLayout();
            //
            // nameTextBox
            //
            nameTextBox.Location = new System.Drawing.Point(78, 12);
            nameTextBox.Name = "nameTextBox";
            nameTextBox.Size = new System.Drawing.Size(294, 23);
            //
            // allAccessCheckBox
            //
            allAccessCheckBox.Location = new System.Drawing.Point(78, 44);
            allAccessCheckBox.Name = "allAccessCheckBox";
            allAccessCheckBox.Size = new System.Drawing.Size(200, 23);
            allAccessCheckBox.Text = "Requires All Access";
            //
            // nameLabel
            //
            nameLabel.Location = new System.Drawing.Point(12, 15);
            nameLabel.Name = "nameLabel";
            nameLabel.Size = new System.Drawing.Size(60, 23);
            nameLabel.Text = "Name:";
            //
            // okButton
            //
            okButton.DialogResult = System.Windows.Forms.DialogResult.OK;
            okButton.Location = new System.Drawing.Point(216, 80);
            okButton.Name = "okButton";
            okButton.Size = new System.Drawing.Size(75, 23);
            okButton.Text = "OK";
            okButton.Click += OkButton_Click;
            //
            // cancelButton
            //
            cancelButton.DialogResult = System.Windows.Forms.DialogResult.Cancel;
            cancelButton.Location = new System.Drawing.Point(297, 80);
            cancelButton.Name = "cancelButton";
            cancelButton.Size = new System.Drawing.Size(75, 23);
            cancelButton.Text = "Cancel";
            //
            // AddEditServerDialog
            //
            AutoScaleDimensions = new System.Drawing.SizeF(7F, 15F);
            AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            ClientSize = new System.Drawing.Size(384, 118);
            Controls.Add(nameTextBox);
            Controls.Add(nameLabel);
            Controls.Add(allAccessCheckBox);
            Controls.Add(okButton);
            Controls.Add(cancelButton);
            FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedDialog;
            MaximizeBox = false;
            MinimizeBox = false;
            Name = "AddEditServerDialog";
            ShowIcon = false;
            ShowInTaskbar = false;
            StartPosition = System.Windows.Forms.FormStartPosition.CenterScreen;
            Text = "Add Server";
            ResumeLayout(false);
        }

        private System.Windows.Forms.TextBox nameTextBox;
        private System.Windows.Forms.CheckBox allAccessCheckBox;
        private System.Windows.Forms.Label nameLabel;
        private System.Windows.Forms.Button okButton;
        private System.Windows.Forms.Button cancelButton;
    }
}
