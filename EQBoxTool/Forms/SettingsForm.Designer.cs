namespace EQBoxTool
{
    partial class SettingsForm
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
            _treeView = new System.Windows.Forms.TreeView();
            treePanel = new System.Windows.Forms.Panel();
            _settingsPanel = new System.Windows.Forms.Panel();
            splitter = new System.Windows.Forms.Splitter();
            treePanel.SuspendLayout();
            SuspendLayout();
            //
            // _treeView
            //
            _treeView.Dock = System.Windows.Forms.DockStyle.Fill;
            _treeView.FullRowSelect = true;
            _treeView.HideSelection = false;
            _treeView.Location = new System.Drawing.Point(0, 0);
            _treeView.Name = "_treeView";
            _treeView.ShowLines = true;
            _treeView.ShowPlusMinus = true;
            _treeView.ShowRootLines = true;
            _treeView.Size = new System.Drawing.Size(180, 448);
            _treeView.TabIndex = 0;
            _treeView.AfterSelect += TreeView_AfterSelect;
            //
            // treePanel
            //
            treePanel.Controls.Add(_treeView);
            treePanel.Dock = System.Windows.Forms.DockStyle.Left;
            treePanel.Location = new System.Drawing.Point(0, 0);
            treePanel.Name = "treePanel";
            treePanel.Padding = new System.Windows.Forms.Padding(10, 10, 0, 10);
            treePanel.Size = new System.Drawing.Size(190, 468);
            treePanel.TabIndex = 1;
            //
            // _settingsPanel
            //
            _settingsPanel.Dock = System.Windows.Forms.DockStyle.Fill;
            _settingsPanel.Location = new System.Drawing.Point(194, 0);
            _settingsPanel.Name = "_settingsPanel";
            _settingsPanel.Padding = new System.Windows.Forms.Padding(10);
            _settingsPanel.Size = new System.Drawing.Size(540, 468);
            _settingsPanel.TabIndex = 2;
            //
            // splitter
            //
            splitter.Dock = System.Windows.Forms.DockStyle.Left;
            splitter.Location = new System.Drawing.Point(190, 0);
            splitter.Name = "splitter";
            splitter.Size = new System.Drawing.Size(4, 468);
            splitter.TabIndex = 3;
            splitter.TabStop = false;
            //
            // SettingsForm
            //
            AutoScaleDimensions = new System.Drawing.SizeF(7F, 15F);
            AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            ClientSize = new System.Drawing.Size(734, 468);
            Controls.Add(_settingsPanel);
            Controls.Add(splitter);
            Controls.Add(treePanel);
            FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedDialog;
            MaximizeBox = false;
            MinimizeBox = false;
            Name = "SettingsForm";
            ShowIcon = false;
            ShowInTaskbar = true;
            StartPosition = System.Windows.Forms.FormStartPosition.CenterScreen;
            Text = "Settings";
            treePanel.ResumeLayout(false);
            ResumeLayout(false);
        }

        private System.Windows.Forms.TreeView _treeView;
        private System.Windows.Forms.Panel treePanel;
        private System.Windows.Forms.Panel _settingsPanel;
        private System.Windows.Forms.Splitter splitter;
    }
}
