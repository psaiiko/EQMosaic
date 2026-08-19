using System;
using System.Collections.Generic;
using System.Linq;
using System.Windows.Forms;

namespace EQBoxTool
{
    public partial class AddEditProfileDialog : Form
    {
        public string ProfileName => nameTextBox.Text.Trim();
        public int ColumnsCount => (int)columnsNumericUpDown.Value;
        public bool AutoRows => autoRowsCheckBox.Checked;
        public int RowsCount => (int)rowsNumericUpDown.Value;
        public List<ProfileEntry> Entries { get; } = new List<ProfileEntry>();

        private List<Character> _allCharacters;
        private List<Server> _allServers;

        public AddEditProfileDialog()
        {
            InitializeComponent();
        }

        public AddEditProfileDialog(Character[] allCharacters, Server[] allServers) : this()
        {
            _allCharacters = allCharacters.ToList();
            _allServers = allServers.ToList();
            RefreshAddDropdown();
        }

        public AddEditProfileDialog(string name, List<ProfileEntry> entries, int columns, bool autoRows, int rows, Character[] allCharacters, Server[] allServers)
           : this(allCharacters, allServers)
        {
            nameTextBox.Text = name;
            columnsNumericUpDown.Value = columns;
            autoRowsCheckBox.Checked = autoRows;
            rowsNumericUpDown.Value = rows;
            rowsNumericUpDown.Enabled = !autoRowsCheckBox.Checked;
            var sorted = entries.OrderBy(e => e.Order).ToList();
            for (int i = 0; i < sorted.Count; i++)
                Entries.Add(new ProfileEntry { CharacterId = sorted[i].CharacterId, Enabled = sorted[i].Enabled, Order = i });
            RefreshEntriesList();
        }

        private void AutoRowsCheckBox_CheckedChanged(object sender, EventArgs e)
        {
            rowsNumericUpDown.Enabled = !autoRowsCheckBox.Checked;
        }

        private void EntriesListView_ItemChecked(object sender, ItemCheckedEventArgs e)
        {
            var entry = e.Item.Tag as ProfileEntry;
            if (entry != null)
                entry.Enabled = e.Item.Checked;
        }

        private void AddButton_Click(object sender, EventArgs e)
        {
            var selected = _addCharComboBox.SelectedItem as CharacterDisplayItem;
            if (selected == null) return;

            if (Entries.Any(e2 => e2.CharacterId == selected.Character.Id))
            {
                MessageBox.Show(this, "Character is already in the profile.", "Duplicate", MessageBoxButtons.OK, MessageBoxIcon.Information);
                return;
            }

            Entries.Add(new ProfileEntry { CharacterId = selected.Character.Id, Enabled = true, Order = Entries.Count });
            RefreshEntriesList();
            RefreshAddDropdown();
        }

        private void UpButton_Click(object sender, EventArgs e) => MoveEntry(-1);

        private void DownButton_Click(object sender, EventArgs e) => MoveEntry(1);

        private void RemoveButton_Click(object sender, EventArgs e)
        {
            if (_entriesListView.SelectedItems.Count == 0) return;
            var entry = _entriesListView.SelectedItems[0].Tag as ProfileEntry;
            if (entry != null)
            {
                Entries.Remove(entry);
                RefreshEntriesList();
                RefreshAddDropdown();
            }
        }

        private void OkButton_Click(object sender, EventArgs e)
        {
            if (string.IsNullOrWhiteSpace(ProfileName))
            {
                MessageBox.Show(this, "Profile name cannot be empty.", "Validation", MessageBoxButtons.OK, MessageBoxIcon.Warning);
                DialogResult = DialogResult.None;
            }
        }

        private void MoveEntry(int direction)
        {
            if (_entriesListView.SelectedItems.Count == 0) return;
            var entry = _entriesListView.SelectedItems[0].Tag as ProfileEntry;
            if (entry == null) return;

            int index = Entries.IndexOf(entry);
            int swapIndex = index + direction;
            if (swapIndex < 0 || swapIndex >= Entries.Count) return;

            Entries.RemoveAt(index);
            Entries.Insert(swapIndex, entry);

            for (int i = 0; i < Entries.Count; i++)
                Entries[i].Order = i;

            RefreshEntriesList();
            foreach (ListViewItem item in _entriesListView.Items)
            {
                if (item.Tag == entry)
                {
                    item.Selected = true;
                    break;
                }
            }
        }

        private void RefreshEntriesList()
        {
            _entriesListView.Items.Clear();
            for (int i = 0; i < Entries.Count; i++)
            {
                var entry = Entries[i];
                string characterName = string.Empty;
                if (entry.CharacterId != Guid.Empty)
                {
                    var character = _allCharacters.FirstOrDefault(c => c.Id == entry.CharacterId);
                    if (character != null)
                    {
                        var server = _allServers.FirstOrDefault(s => s.Id == character.ServerId);
                        characterName = server != null ? $"{character.Name} ({server.Name})" : character.Name;
                    }
                }

                var item = new ListViewItem((i + 1).ToString());
                item.SubItems.Add(characterName ?? "Unknown");
                item.SubItems.Add(entry.Enabled ? "Yes" : "No");
                item.Checked = entry.Enabled;
                item.Tag = entry;
                _entriesListView.Items.Add(item);
            }
        }

        public void RefreshAddDropdown()
        {
            _addCharComboBox.Items.Clear();
            var added = new HashSet<Guid>(Entries.Select(e => e.CharacterId));

            foreach (var character in _allCharacters)
            {
                if (!added.Contains(character.Id))
                {
                    var server = _allServers.FirstOrDefault(s => s.Id == character.ServerId);
                    var displayName = server != null ? $"{character.Name} ({server.Name})" : character.Name;
                    _addCharComboBox.Items.Add(new CharacterDisplayItem { Character = character, DisplayName = displayName });
                }
            }
            if (_addCharComboBox.Items.Count > 0)
                _addCharComboBox.SelectedIndex = 0;
        }

        private class CharacterDisplayItem
        {
            public Character Character { get; set; }
            public string DisplayName { get; set; }

            public override string ToString()
            {
                return DisplayName;
            }
        }
    }
}
