namespace EQBoxTool
{
    partial class AddEditProfileDialog
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
            columnsNumericUpDown = new System.Windows.Forms.NumericUpDown();
            autoRowsCheckBox = new System.Windows.Forms.CheckBox();
            rowsNumericUpDown = new System.Windows.Forms.NumericUpDown();
            nameLabel = new System.Windows.Forms.Label();
            columnsLabel = new System.Windows.Forms.Label();
            rowsLabel = new System.Windows.Forms.Label();
            entriesLabel = new System.Windows.Forms.Label();
            _entriesListView = new System.Windows.Forms.ListView();
            _addCharComboBox = new System.Windows.Forms.ComboBox();
            addButton = new System.Windows.Forms.Button();
            upButton = new System.Windows.Forms.Button();
            downButton = new System.Windows.Forms.Button();
            removeButton = new System.Windows.Forms.Button();
            okButton = new System.Windows.Forms.Button();
            cancelButton = new System.Windows.Forms.Button();
            ((System.ComponentModel.ISupportInitialize)columnsNumericUpDown).BeginInit();
            ((System.ComponentModel.ISupportInitialize)rowsNumericUpDown).BeginInit();
            SuspendLayout();
            //
            // nameTextBox
            //
            nameTextBox.Location = new System.Drawing.Point(108, 12);
            nameTextBox.Name = "nameTextBox";
            nameTextBox.Size = new System.Drawing.Size(364, 23);
            //
            // columnsNumericUpDown
            //
            columnsNumericUpDown.Location = new System.Drawing.Point(108, 40);
            columnsNumericUpDown.Maximum = new decimal(new int[] { 10, 0, 0, 0 });
            columnsNumericUpDown.Minimum = new decimal(new int[] { 1, 0, 0, 0 });
            columnsNumericUpDown.Name = "columnsNumericUpDown";
            columnsNumericUpDown.Size = new System.Drawing.Size(50, 23);
            columnsNumericUpDown.Value = new decimal(new int[] { 2, 0, 0, 0 });
            //
            // autoRowsCheckBox
            //
            autoRowsCheckBox.AutoSize = true;
            autoRowsCheckBox.Checked = true;
            autoRowsCheckBox.CheckState = System.Windows.Forms.CheckState.Checked;
            autoRowsCheckBox.Location = new System.Drawing.Point(180, 40);
            autoRowsCheckBox.Name = "autoRowsCheckBox";
            autoRowsCheckBox.Size = new System.Drawing.Size(84, 19);
            autoRowsCheckBox.Text = "Auto Rows";
            autoRowsCheckBox.CheckedChanged += AutoRowsCheckBox_CheckedChanged;
            //
            // rowsNumericUpDown
            //
            rowsNumericUpDown.Enabled = false;
            rowsNumericUpDown.Location = new System.Drawing.Point(330, 40);
            rowsNumericUpDown.Maximum = new decimal(new int[] { 10, 0, 0, 0 });
            rowsNumericUpDown.Minimum = new decimal(new int[] { 1, 0, 0, 0 });
            rowsNumericUpDown.Name = "rowsNumericUpDown";
            rowsNumericUpDown.Size = new System.Drawing.Size(50, 23);
            rowsNumericUpDown.Value = new decimal(new int[] { 2, 0, 0, 0 });
            //
            // nameLabel
            //
            nameLabel.Location = new System.Drawing.Point(12, 15);
            nameLabel.Name = "nameLabel";
            nameLabel.Size = new System.Drawing.Size(90, 23);
            nameLabel.Text = "Profile Name:";
            //
            // columnsLabel
            //
            columnsLabel.Location = new System.Drawing.Point(12, 43);
            columnsLabel.Name = "columnsLabel";
            columnsLabel.Size = new System.Drawing.Size(60, 23);
            columnsLabel.Text = "Columns:";
            //
            // rowsLabel
            //
            rowsLabel.Location = new System.Drawing.Point(290, 43);
            rowsLabel.Name = "rowsLabel";
            rowsLabel.Size = new System.Drawing.Size(40, 23);
            rowsLabel.Text = "Rows:";
            //
            // entriesLabel
            //
            entriesLabel.Location = new System.Drawing.Point(12, 72);
            entriesLabel.Name = "entriesLabel";
            entriesLabel.Size = new System.Drawing.Size(100, 23);
            entriesLabel.Text = "Characters:";
            //
            // _entriesListView
            //
            _entriesListView.CheckBoxes = true;
            _entriesListView.Columns.Add("#", 30);
            _entriesListView.Columns.Add("Character", 320);
            _entriesListView.Columns.Add("Enabled", 100);
            _entriesListView.FullRowSelect = true;
            _entriesListView.HeaderStyle = System.Windows.Forms.ColumnHeaderStyle.Nonclickable;
            _entriesListView.HideSelection = false;
            _entriesListView.Location = new System.Drawing.Point(12, 94);
            _entriesListView.Name = "_entriesListView";
            _entriesListView.Size = new System.Drawing.Size(460, 220);
            _entriesListView.TabIndex = 6;
            _entriesListView.UseCompatibleStateImageBehavior = false;
            _entriesListView.View = System.Windows.Forms.View.Details;
            _entriesListView.ItemChecked += EntriesListView_ItemChecked;
            //
            // _addCharComboBox
            //
            _addCharComboBox.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            _addCharComboBox.Location = new System.Drawing.Point(12, 324);
            _addCharComboBox.Name = "_addCharComboBox";
            _addCharComboBox.Size = new System.Drawing.Size(220, 23);
            //
            // addButton
            //
            addButton.Location = new System.Drawing.Point(238, 323);
            addButton.Name = "addButton";
            addButton.Size = new System.Drawing.Size(60, 25);
            addButton.Text = "Add";
            addButton.Click += AddButton_Click;
            //
            // upButton
            //
            upButton.Location = new System.Drawing.Point(308, 323);
            upButton.Name = "upButton";
            upButton.Size = new System.Drawing.Size(30, 25);
            upButton.Text = "▲";
            upButton.Click += UpButton_Click;
            //
            // downButton
            //
            downButton.Location = new System.Drawing.Point(344, 323);
            downButton.Name = "downButton";
            downButton.Size = new System.Drawing.Size(30, 25);
            downButton.Text = "▼";
            downButton.Click += DownButton_Click;
            //
            // removeButton
            //
            removeButton.Location = new System.Drawing.Point(384, 323);
            removeButton.Name = "removeButton";
            removeButton.Size = new System.Drawing.Size(88, 25);
            removeButton.Text = "Remove";
            removeButton.Click += RemoveButton_Click;
            //
            // okButton
            //
            okButton.DialogResult = System.Windows.Forms.DialogResult.OK;
            okButton.Location = new System.Drawing.Point(316, 364);
            okButton.Name = "okButton";
            okButton.Size = new System.Drawing.Size(75, 23);
            okButton.Text = "OK";
            okButton.Click += OkButton_Click;
            //
            // cancelButton
            //
            cancelButton.DialogResult = System.Windows.Forms.DialogResult.Cancel;
            cancelButton.Location = new System.Drawing.Point(397, 364);
            cancelButton.Name = "cancelButton";
            cancelButton.Size = new System.Drawing.Size(75, 23);
            cancelButton.Text = "Cancel";
            //
            // AddEditProfileDialog
            //
            AutoScaleDimensions = new System.Drawing.SizeF(7F, 15F);
            AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            ClientSize = new System.Drawing.Size(484, 428);
            Controls.Add(nameTextBox);
            Controls.Add(nameLabel);
            Controls.Add(columnsNumericUpDown);
            Controls.Add(columnsLabel);
            Controls.Add(autoRowsCheckBox);
            Controls.Add(rowsNumericUpDown);
            Controls.Add(rowsLabel);
            Controls.Add(entriesLabel);
            Controls.Add(_entriesListView);
            Controls.Add(_addCharComboBox);
            Controls.Add(addButton);
            Controls.Add(upButton);
            Controls.Add(downButton);
            Controls.Add(removeButton);
            Controls.Add(okButton);
            Controls.Add(cancelButton);
            FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedDialog;
            MaximizeBox = false;
            MinimizeBox = false;
            Name = "AddEditProfileDialog";
            ShowIcon = false;
            ShowInTaskbar = false;
            StartPosition = System.Windows.Forms.FormStartPosition.CenterScreen;
            Text = "Add Profile";
            ((System.ComponentModel.ISupportInitialize)columnsNumericUpDown).EndInit();
            ((System.ComponentModel.ISupportInitialize)rowsNumericUpDown).EndInit();
            ResumeLayout(false);
        }

        private System.Windows.Forms.TextBox nameTextBox;
        private System.Windows.Forms.NumericUpDown columnsNumericUpDown;
        private System.Windows.Forms.CheckBox autoRowsCheckBox;
        private System.Windows.Forms.NumericUpDown rowsNumericUpDown;
        private System.Windows.Forms.Label nameLabel;
        private System.Windows.Forms.Label columnsLabel;
        private System.Windows.Forms.Label rowsLabel;
        private System.Windows.Forms.Label entriesLabel;
        private System.Windows.Forms.ListView _entriesListView;
        private System.Windows.Forms.ComboBox _addCharComboBox;
        private System.Windows.Forms.Button addButton;
        private System.Windows.Forms.Button upButton;
        private System.Windows.Forms.Button downButton;
        private System.Windows.Forms.Button removeButton;
        private System.Windows.Forms.Button okButton;
        private System.Windows.Forms.Button cancelButton;
    }
}
