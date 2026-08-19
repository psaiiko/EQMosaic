namespace EQBoxTool
{
    partial class AddEditCharacterDialog
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
            classComboBox = new System.Windows.Forms.ComboBox();
            levelNumericUpDown = new System.Windows.Forms.NumericUpDown();
            serverComboBox = new System.Windows.Forms.ComboBox();
            accountComboBox = new System.Windows.Forms.ComboBox();
            nameLabel = new System.Windows.Forms.Label();
            classLabel = new System.Windows.Forms.Label();
            levelLabel = new System.Windows.Forms.Label();
            serverLabel = new System.Windows.Forms.Label();
            accountLabel = new System.Windows.Forms.Label();
            okButton = new System.Windows.Forms.Button();
            cancelButton = new System.Windows.Forms.Button();
            ((System.ComponentModel.ISupportInitialize)levelNumericUpDown).BeginInit();
            SuspendLayout();
            //
            // nameTextBox
            //
            nameTextBox.Location = new System.Drawing.Point(100, 12);
            nameTextBox.Name = "nameTextBox";
            nameTextBox.Size = new System.Drawing.Size(272, 23);
            //
            // classComboBox
            //
            classComboBox.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            classComboBox.Location = new System.Drawing.Point(100, 40);
            classComboBox.Name = "classComboBox";
            classComboBox.Size = new System.Drawing.Size(272, 23);
            //
            // levelNumericUpDown
            //
            levelNumericUpDown.Location = new System.Drawing.Point(100, 68);
            levelNumericUpDown.Maximum = new decimal(new int[] { 130, 0, 0, 0 });
            levelNumericUpDown.Name = "levelNumericUpDown";
            levelNumericUpDown.Size = new System.Drawing.Size(80, 23);
            //
            // serverComboBox
            //
            serverComboBox.DisplayMember = "Name";
            serverComboBox.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            serverComboBox.Location = new System.Drawing.Point(100, 96);
            serverComboBox.Name = "serverComboBox";
            serverComboBox.Size = new System.Drawing.Size(272, 23);
            //
            // accountComboBox
            //
            accountComboBox.DisplayMember = "LoginName";
            accountComboBox.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            accountComboBox.Location = new System.Drawing.Point(100, 124);
            accountComboBox.Name = "accountComboBox";
            accountComboBox.Size = new System.Drawing.Size(272, 23);
            //
            // nameLabel
            //
            nameLabel.Location = new System.Drawing.Point(12, 15);
            nameLabel.Name = "nameLabel";
            nameLabel.Size = new System.Drawing.Size(80, 23);
            nameLabel.Text = "Name:";
            //
            // classLabel
            //
            classLabel.Location = new System.Drawing.Point(12, 43);
            classLabel.Name = "classLabel";
            classLabel.Size = new System.Drawing.Size(80, 23);
            classLabel.Text = "Class:";
            //
            // levelLabel
            //
            levelLabel.Location = new System.Drawing.Point(12, 71);
            levelLabel.Name = "levelLabel";
            levelLabel.Size = new System.Drawing.Size(80, 23);
            levelLabel.Text = "Level:";
            //
            // serverLabel
            //
            serverLabel.Location = new System.Drawing.Point(12, 99);
            serverLabel.Name = "serverLabel";
            serverLabel.Size = new System.Drawing.Size(80, 23);
            serverLabel.Text = "Server:";
            //
            // accountLabel
            //
            accountLabel.Location = new System.Drawing.Point(12, 127);
            accountLabel.Name = "accountLabel";
            accountLabel.Size = new System.Drawing.Size(80, 23);
            accountLabel.Text = "Account:";
            //
            // okButton
            //
            okButton.DialogResult = System.Windows.Forms.DialogResult.OK;
            okButton.Location = new System.Drawing.Point(216, 158);
            okButton.Name = "okButton";
            okButton.Size = new System.Drawing.Size(75, 23);
            okButton.Text = "OK";
            okButton.Click += OkButton_Click;
            //
            // cancelButton
            //
            cancelButton.DialogResult = System.Windows.Forms.DialogResult.Cancel;
            cancelButton.Location = new System.Drawing.Point(297, 158);
            cancelButton.Name = "cancelButton";
            cancelButton.Size = new System.Drawing.Size(75, 23);
            cancelButton.Text = "Cancel";
            //
            // AddEditCharacterDialog
            //
            AutoScaleDimensions = new System.Drawing.SizeF(7F, 15F);
            AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            ClientSize = new System.Drawing.Size(384, 218);
            Controls.Add(nameTextBox);
            Controls.Add(nameLabel);
            Controls.Add(classComboBox);
            Controls.Add(classLabel);
            Controls.Add(levelNumericUpDown);
            Controls.Add(levelLabel);
            Controls.Add(serverComboBox);
            Controls.Add(serverLabel);
            Controls.Add(accountComboBox);
            Controls.Add(accountLabel);
            Controls.Add(okButton);
            Controls.Add(cancelButton);
            FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedDialog;
            MaximizeBox = false;
            MinimizeBox = false;
            Name = "AddEditCharacterDialog";
            ShowIcon = false;
            ShowInTaskbar = false;
            StartPosition = System.Windows.Forms.FormStartPosition.CenterScreen;
            Text = "Add Character";
            ((System.ComponentModel.ISupportInitialize)levelNumericUpDown).EndInit();
            ResumeLayout(false);
        }

        private System.Windows.Forms.TextBox nameTextBox;
        private System.Windows.Forms.ComboBox classComboBox;
        private System.Windows.Forms.NumericUpDown levelNumericUpDown;
        private System.Windows.Forms.ComboBox serverComboBox;
        private System.Windows.Forms.ComboBox accountComboBox;
        private System.Windows.Forms.Label nameLabel;
        private System.Windows.Forms.Label classLabel;
        private System.Windows.Forms.Label levelLabel;
        private System.Windows.Forms.Label serverLabel;
        private System.Windows.Forms.Label accountLabel;
        private System.Windows.Forms.Button okButton;
        private System.Windows.Forms.Button cancelButton;
    }
}
