using System;
using System.Linq;
using System.Windows.Forms;

namespace EQBoxTool
{
    public partial class SettingsForm : Form
    {
        private AppConfig _config;
        private Action _onChanged;

        public SettingsForm()
        {
            InitializeComponent();
        }

        public SettingsForm(AppConfig config, Action onChanged = null) : this()
        {
            _config = config;
            _onChanged = onChanged;
            InitializeTree();
        }

        private void InitializeTree()
        {
            _treeView.BeginUpdate();
            var generalNode = _treeView.Nodes.Add("General");
            var launcherNode = _treeView.Nodes.Add("Launcher");
            launcherNode.Nodes.Add("Everquest Installs");
            launcherNode.Nodes.Add("Servers");
            launcherNode.Nodes.Add("Accounts");
            launcherNode.Nodes.Add("Characters");
            launcherNode.Nodes.Add("Profiles");
            _treeView.ExpandAll();
            _treeView.SelectedNode = generalNode;
            _treeView.EndUpdate();
            ShowGeneralSettings();
        }

        private void TreeView_AfterSelect(object sender, TreeViewEventArgs e)
        {
            _settingsPanel.Controls.Clear();

            switch (e.Node.Text)
            {
                case "General":
                    ShowGeneralSettings();
                    break;
                case "Everquest Installs":
                    ShowEverquestInstallsSettings();
                    break;
                case "Servers":
                    ShowServersSettings();
                    break;
                case "Accounts":
                    ShowAccountsSettings();
                    break;
                case "Characters":
                    ShowCharactersSettings();
                    break;
                case "Profiles":
                    ShowProfilesSettings();
                    break;
                default:
                    ShowPlaceholder(e.Node.Text);
                    break;
            }
        }

        private void ShowGeneralSettings()
        {
            var borderlessCheckbox = new CheckBox
            {
                Text = "Borderless game windows",
                Checked = _config.BorderlessGameWindows,
                Location = new System.Drawing.Point(10, 10),
                AutoSize = true,
            };
            borderlessCheckbox.CheckedChanged += (s, e) =>
            {
                _config.BorderlessGameWindows = borderlessCheckbox.Checked;
                ConfigManager.Save(_config);
            };
            _settingsPanel.Controls.Add(borderlessCheckbox);

            var changePasswordButton = new Button
            {
                Text = "Change Master Password...",
                Location = new System.Drawing.Point(10, 40),
                AutoSize = true,
            };
            changePasswordButton.Click += (s, e) =>
            {
                if (MasterPasswordDialog.TryChangePassword())
                    ConfigManager.Save(_config);
            };
            _settingsPanel.Controls.Add(changePasswordButton);
        }

        private void ShowEverquestInstallsSettings()
        {
            int listViewWidth = 420;
            int buttonX = listViewWidth + 20;
            int buttonWidth = 75;

            var listView = new ListView
            {
                Location = new System.Drawing.Point(10, 10),
                Size = new System.Drawing.Size(listViewWidth, 440),
                View = View.Details,
                FullRowSelect = true,
                HideSelection = false,
                HeaderStyle = ColumnHeaderStyle.Nonclickable,
                Anchor = AnchorStyles.Top | AnchorStyles.Bottom | AnchorStyles.Left,
            };
            listView.Columns.Add("Name", 120);
            listView.Columns.Add("Path", listViewWidth - 120 - 4);
            RefreshInstallList(listView);

            var addButton = new Button
            {
                Text = "Add",
                Location = new System.Drawing.Point(buttonX, 10),
                Size = new System.Drawing.Size(buttonWidth, 26),
            };
            addButton.Click += (s, e) =>
            {
                using (var dialog = new AddEditInstallDialog())
                {
                    if (dialog.ShowDialog(this) == DialogResult.OK)
                    {
                        _config.EverquestInstalls.Add(new EqInstall
                        {
                            Name = dialog.InstallName,
                            Path = dialog.InstallPath,
                        });
                        ConfigManager.Save(_config);
                        RefreshInstallList(listView);
                    }
                }
            };

            var editButton = new Button
            {
                Text = "Edit",
                Location = new System.Drawing.Point(buttonX, 42),
                Size = new System.Drawing.Size(buttonWidth, 26),
            };
            editButton.Click += (s, e) =>
            {
                var selected = GetSelectedInstall(listView);
                if (selected == null) return;

                using (var dialog = new AddEditInstallDialog(selected.Name, selected.Path)
                {
                    Text = "Edit EverQuest Install"
                })
                {
                    if (dialog.ShowDialog(this) == DialogResult.OK)
                    {
                        selected.Name = dialog.InstallName;
                        selected.Path = dialog.InstallPath;
                        ConfigManager.Save(_config);
                        RefreshInstallList(listView);
                    }
                }
            };

            var removeButton = new Button
            {
                Text = "Remove",
                Location = new System.Drawing.Point(buttonX, 74),
                Size = new System.Drawing.Size(buttonWidth, 26),
            };
            removeButton.Click += (s, e) =>
            {
                var selected = GetSelectedInstall(listView);
                if (selected == null) return;

                _config.EverquestInstalls.Remove(selected);
                ConfigManager.Save(_config);
                RefreshInstallList(listView);
            };

            listView.MouseDoubleClick += (s, e) => { if (listView.SelectedItems.Count > 0) editButton.PerformClick(); };
            _settingsPanel.Controls.AddRange(new Control[] { listView, addButton, editButton, removeButton });
        }

        private void ShowServersSettings()
        {
            int listViewWidth = 420;
            int buttonX = listViewWidth + 20;
            int buttonWidth = 75;

            var listView = new ListView
            {
                Location = new System.Drawing.Point(10, 10),
                Size = new System.Drawing.Size(listViewWidth, 440),
                View = View.Details,
                FullRowSelect = true,
                HideSelection = false,
                HeaderStyle = ColumnHeaderStyle.Nonclickable,
                Anchor = AnchorStyles.Top | AnchorStyles.Bottom | AnchorStyles.Left,
            };
            listView.Columns.Add("Name", 200);
            listView.Columns.Add("All Access", listViewWidth - 200 - 4);
            RefreshServerList(listView);

            var addButton = new Button
            {
                Text = "Add",
                Location = new System.Drawing.Point(buttonX, 10),
                Size = new System.Drawing.Size(buttonWidth, 26),
            };
            addButton.Click += (s, e) =>
            {
                using (var dialog = new AddEditServerDialog())
                {
                    if (dialog.ShowDialog(this) == DialogResult.OK)
                    {
                        _config.Servers.Add(new Server
                        {
                            Name = dialog.ServerName,
                            RequiresAllAccess = dialog.RequiresAllAccess,
                        });
                        ConfigManager.Save(_config);
                        RefreshServerList(listView);
                    }
                }
            };

            var editButton = new Button
            {
                Text = "Edit",
                Location = new System.Drawing.Point(buttonX, 42),
                Size = new System.Drawing.Size(buttonWidth, 26),
            };
            editButton.Click += (s, e) =>
            {
                var selected = GetSelectedServer(listView);
                if (selected == null) return;

                using (var dialog = new AddEditServerDialog(selected.Name, selected.RequiresAllAccess)
                {
                    Text = "Edit Server"
                })
                {
                    if (dialog.ShowDialog(this) == DialogResult.OK)
                    {
                        selected.Name = dialog.ServerName;
                        selected.RequiresAllAccess = dialog.RequiresAllAccess;
                        ConfigManager.Save(_config);
                        RefreshServerList(listView);
                    }
                }
            };

            var removeButton = new Button
            {
                Text = "Remove",
                Location = new System.Drawing.Point(buttonX, 74),
                Size = new System.Drawing.Size(buttonWidth, 26),
            };
            removeButton.Click += (s, e) =>
            {
                var selected = GetSelectedServer(listView);
                if (selected == null) return;

                _config.Servers.Remove(selected);
                ConfigManager.Save(_config);
                RefreshServerList(listView);
            };

            listView.MouseDoubleClick += (s, e) => { if (listView.SelectedItems.Count > 0) editButton.PerformClick(); };
            _settingsPanel.Controls.AddRange(new Control[] { listView, addButton, editButton, removeButton });
        }

        private void RefreshServerList(ListView listView)
        {
            listView.Items.Clear();
            foreach (var server in _config.Servers)
            {
                var item = new ListViewItem(server.Name);
                item.SubItems.Add(server.RequiresAllAccess ? "Yes" : "No");
                item.Tag = server;
                listView.Items.Add(item);
            }
        }

        private static Server GetSelectedServer(ListView listView)
        {
            if (listView.SelectedItems.Count == 0)
                return null;
            return listView.SelectedItems[0].Tag as Server;
        }

        private void ShowAccountsSettings()
        {
            int listViewWidth = 420;
            int buttonX = listViewWidth + 20;
            int buttonWidth = 75;

            var listView = new ListView
            {
                Location = new System.Drawing.Point(10, 10),
                Size = new System.Drawing.Size(listViewWidth, 440),
                View = View.Details,
                FullRowSelect = true,
                HideSelection = false,
                HeaderStyle = ColumnHeaderStyle.Nonclickable,
                Anchor = AnchorStyles.Top | AnchorStyles.Bottom | AnchorStyles.Left,
            };
            listView.Columns.Add("Account Name", 120);
            listView.Columns.Add("EQ Install", listViewWidth - 120 - 4);
            RefreshAccountList(listView);

            var addButton = new Button
            {
                Text = "Add",
                Location = new System.Drawing.Point(buttonX, 10),
                Size = new System.Drawing.Size(buttonWidth, 26),
            };
            addButton.Click += (s, e) =>
            {
                var installs = _config.EverquestInstalls.ToArray();
                if (installs.Length == 0)
                {
                    MessageBox.Show(this, "No EQ installs configured. Add one first.", "No Installs", MessageBoxButtons.OK, MessageBoxIcon.Information);
                    return;
                }

                using (var dialog = new AddEditAccountDialog(installs))
                {
                    if (dialog.ShowDialog(this) == DialogResult.OK)
                    {
                        _config.Accounts.Add(new Account
                        {
                            LoginName = dialog.AccountLoginName,
                            EqInstallId = dialog.SelectedInstallId,
                        });
                        ConfigManager.Save(_config);
                        RefreshAccountList(listView);
                    }
                }
            };

            var editButton = new Button
            {
                Text = "Edit",
                Location = new System.Drawing.Point(buttonX, 42),
                Size = new System.Drawing.Size(buttonWidth, 26),
            };
            editButton.Click += (s, e) =>
            {
                var selected = GetSelectedAccount(listView);
                if (selected == null) return;

                var installs = _config.EverquestInstalls.ToArray();
                if (installs.Length == 0)
                {
                    MessageBox.Show(this, "No EQ installs configured. Add one first.", "No Installs", MessageBoxButtons.OK, MessageBoxIcon.Information);
                    return;
                }

                using (var dialog = new AddEditAccountDialog(selected.LoginName, selected.Password ?? "", selected.EqInstallId, installs)
                {
                    Text = "Edit Account"
                })
                {
                    if (dialog.ShowDialog(this) == DialogResult.OK)
                    {
                        selected.LoginName = dialog.AccountLoginName;
                        selected.Password = dialog.AccountPassword;
                        selected.EqInstallId = dialog.SelectedInstallId;
                        ConfigManager.Save(_config);
                        RefreshAccountList(listView);
                    }
                }
            };

            var removeButton = new Button
            {
                Text = "Remove",
                Location = new System.Drawing.Point(buttonX, 74),
                Size = new System.Drawing.Size(buttonWidth, 26),
            };
            removeButton.Click += (s, e) =>
            {
                var selected = GetSelectedAccount(listView);
                if (selected == null) return;

                _config.Accounts.Remove(selected);
                ConfigManager.Save(_config);
                RefreshAccountList(listView);
            };

            listView.MouseDoubleClick += (s, e) => { if (listView.SelectedItems.Count > 0) editButton.PerformClick(); };
            _settingsPanel.Controls.AddRange(new Control[] { listView, addButton, editButton, removeButton });
        }

        private void RefreshAccountList(ListView listView)
        {
            listView.Items.Clear();
            foreach (var account in _config.Accounts)
            {
                var install = _config.EverquestInstalls.Find(i => i.Id == account.EqInstallId);
                var item = new ListViewItem(account.LoginName);
                item.SubItems.Add(install?.Name ?? "(unknown)");
                item.Tag = account;
                listView.Items.Add(item);
            }
        }

        private static Account GetSelectedAccount(ListView listView)
        {
            if (listView.SelectedItems.Count == 0)
                return null;
            return listView.SelectedItems[0].Tag as Account;
        }

        private void RefreshInstallList(ListView listView)
        {
            listView.Items.Clear();
            foreach (var install in _config.EverquestInstalls)
            {
                var item = new ListViewItem(install.Name);
                item.SubItems.Add(install.Path);
                item.Tag = install;
                listView.Items.Add(item);
            }
        }

        private static EqInstall GetSelectedInstall(ListView listView)
        {
            if (listView.SelectedItems.Count == 0)
                return null;
            return listView.SelectedItems[0].Tag as EqInstall;
        }

        private void ShowCharactersSettings()
        {
            int listViewWidth = 420;
            int buttonX = listViewWidth + 20;
            int buttonWidth = 75;

            var listView = new ListView
            {
                Location = new System.Drawing.Point(10, 10),
                Size = new System.Drawing.Size(listViewWidth, 440),
                View = View.Details,
                FullRowSelect = true,
                HideSelection = false,
                HeaderStyle = ColumnHeaderStyle.Nonclickable,
                Anchor = AnchorStyles.Top | AnchorStyles.Bottom | AnchorStyles.Left,
            };
            listView.Columns.Add("Name", 200);
            listView.Columns.Add("Server", listViewWidth - 200 - 86 - 4);
            listView.Columns.Add("Account", 80);
            RefreshCharacterList(listView);

            var addButton = new Button
            {
                Text = "Add",
                Location = new System.Drawing.Point(buttonX, 10),
                Size = new System.Drawing.Size(buttonWidth, 26),
            };
            addButton.Click += (s, e) =>
            {
                var servers = _config.Servers.ToArray();
                var accounts = _config.Accounts.ToArray();

                using (var dialog = new AddEditCharacterDialog(servers, accounts))
                {
                    if (dialog.ShowDialog(this) == DialogResult.OK)
                    {
                        _config.Characters.Add(new Character
                        {
                            Name = dialog.CharacterName,
                            ClassName = dialog.SelectedClass,
                            Level = dialog.SelectedLevel,
                            ServerId = dialog.SelectedServerId,
                            AccountId = dialog.SelectedAccountId,
                        });
                        ConfigManager.Save(_config);
                        RefreshCharacterList(listView);
                    }
                }
            };

            var editButton = new Button
            {
                Text = "Edit",
                Location = new System.Drawing.Point(buttonX, 42),
                Size = new System.Drawing.Size(buttonWidth, 26),
            };
            editButton.Click += (s, e) =>
            {
                var selected = GetSelectedCharacter(listView);
                if (selected == null) return;

                var servers = _config.Servers.ToArray();
                var accounts = _config.Accounts.ToArray();

                using (var dialog = new AddEditCharacterDialog(
                    selected.Name, selected.ClassName, selected.Level,
                    selected.ServerId, selected.AccountId,
                    servers, accounts)
                {
                    Text = "Edit Character"
                })
                {
                    if (dialog.ShowDialog(this) == DialogResult.OK)
                    {
                        selected.Name = dialog.CharacterName;
                        selected.ClassName = dialog.SelectedClass;
                        selected.Level = dialog.SelectedLevel;
                        selected.ServerId = dialog.SelectedServerId;
                        selected.AccountId = dialog.SelectedAccountId;
                        ConfigManager.Save(_config);
                        RefreshCharacterList(listView);
                    }
                }
            };

            var removeButton = new Button
            {
                Text = "Remove",
                Location = new System.Drawing.Point(buttonX, 74),
                Size = new System.Drawing.Size(buttonWidth, 26),
            };
            removeButton.Click += (s, e) =>
            {
                var selected = GetSelectedCharacter(listView);
                if (selected == null) return;

                _config.Characters.Remove(selected);
                ConfigManager.Save(_config);
                RefreshCharacterList(listView);
            };

            listView.MouseDoubleClick += (s, e) => { if (listView.SelectedItems.Count > 0) editButton.PerformClick(); };
            _settingsPanel.Controls.AddRange(new Control[] { listView, addButton, editButton, removeButton });
        }

        private void RefreshCharacterList(ListView listView)
        {
            listView.Items.Clear();
            foreach (var ch in _config.Characters)
            {
                var abbv = EqClasses.GetAbbreviation(ch.ClassName);
                string displayName;
                if (abbv != null && ch.Level > 0)
                    displayName = $"{ch.Name} ({abbv} {ch.Level})";
                else if (abbv != null)
                    displayName = $"{ch.Name} ({abbv})";
                else if (ch.Level > 0)
                    displayName = $"{ch.Name} ({(int)ch.Level})";
                else
                    displayName = ch.Name;

                var account = _config.Accounts.Find(a => a.Id == ch.AccountId);
                var server = _config.Servers.Find(s => s.Id == ch.ServerId);
                var item = new ListViewItem(displayName);
                item.SubItems.Add(server?.Name ?? "");
                item.SubItems.Add(account?.LoginName ?? "");
                item.Tag = ch;
                listView.Items.Add(item);
            }
        }

        private static Character GetSelectedCharacter(ListView listView)
        {
            if (listView.SelectedItems.Count == 0)
                return null;
            return listView.SelectedItems[0].Tag as Character;
        }

        private void ShowProfilesSettings()
        {
            int listViewWidth = 420;
            int buttonX = listViewWidth + 20;
            int buttonWidth = 75;

            var listView = new ListView
            {
                Location = new System.Drawing.Point(10, 10),
                Size = new System.Drawing.Size(listViewWidth, 440),
                View = View.Details,
                FullRowSelect = true,
                HideSelection = false,
                HeaderStyle = ColumnHeaderStyle.Nonclickable,
                Anchor = AnchorStyles.Top | AnchorStyles.Bottom | AnchorStyles.Left,
            };
            listView.Columns.Add("Active", 36);
            listView.Columns.Add("Name", 160);
            listView.Columns.Add("Characters", listViewWidth - 36 - 160 - 4);
            RefreshProfileList(listView);

            var addButton = new Button
            {
                Text = "Add",
                Location = new System.Drawing.Point(buttonX, 10),
                Size = new System.Drawing.Size(buttonWidth, 26),
            };
            addButton.Click += (s, e) =>
             {
                 var dialog = new AddEditProfileDialog(_config.Characters.ToArray(), _config.Servers.ToArray());

                 if (dialog.ShowDialog(this) == DialogResult.OK)
                 {
                     _config.Profiles.Add(new Profile
                     {
                         Id = Guid.NewGuid(),
                         Name = dialog.ProfileName,
                         Columns = dialog.ColumnsCount,
                         AutoRows = dialog.AutoRows,
                         Rows = dialog.RowsCount,
                         Entries = dialog.Entries.ToList(),
                     });
                     ConfigManager.Save(_config);
                     RefreshProfileList(listView);
                 }
             };

            var editButton = new Button
            {
                Text = "Edit",
                Location = new System.Drawing.Point(buttonX, 42),
                Size = new System.Drawing.Size(buttonWidth, 26),
            };
            editButton.Click += (s, e) =>
         {
             var selected = GetSelectedProfile(listView);
             if (selected == null) return;

             using (var dialog = new AddEditProfileDialog(selected.Name, selected.Entries, selected.Columns, selected.AutoRows, selected.Rows, _config.Characters.ToArray(), _config.Servers.ToArray())
             {
                 Text = "Edit Profile"
             })
             {
                 if (dialog.ShowDialog(this) == DialogResult.OK)
                 {
                     selected.Name = dialog.ProfileName;
                     selected.Columns = dialog.ColumnsCount;
                     selected.AutoRows = dialog.AutoRows;
                     selected.Rows = dialog.RowsCount;
                     selected.Entries = dialog.Entries.ToList();
                     ConfigManager.Save(_config);
                     RefreshProfileList(listView);
                     if (selected.Id == _config.CurrentProfileId)
                         _onChanged?.Invoke();
                 }
             }
         };

            var setActiveButton = new Button
            {
                Text = "Set Active",
                Location = new System.Drawing.Point(buttonX, 74),
                Size = new System.Drawing.Size(buttonWidth, 26),
            };
            setActiveButton.Click += (s, e) =>
            {
                var selected = GetSelectedProfile(listView);
                if (selected == null) return;

                _config.CurrentProfileId = selected.Id;
                ConfigManager.Save(_config);
                RefreshProfileList(listView);
                _onChanged?.Invoke();
            };

            var removeButton = new Button
            {
                Text = "Remove",
                Location = new System.Drawing.Point(buttonX, 106),
                Size = new System.Drawing.Size(buttonWidth, 26),
            };
            removeButton.Click += (s, e) =>
            {
                var selected = GetSelectedProfile(listView);
                if (selected == null) return;

                if (_config.CurrentProfileId == selected.Id)
                    _config.CurrentProfileId = null;

                _config.Profiles.Remove(selected);
                ConfigManager.Save(_config);
                RefreshProfileList(listView);
            };

            listView.MouseDoubleClick += (s, e) => { if (listView.SelectedItems.Count > 0) editButton.PerformClick(); };
            _settingsPanel.Controls.AddRange(new Control[] { listView, addButton, editButton, setActiveButton, removeButton });
        }

        private void RefreshProfileList(ListView listView)
        {
            listView.Items.Clear();
            foreach (var profile in _config.Profiles)
            {
                var chars = profile.Entries.Select(e =>
                {
                    if (e.CharacterId != Guid.Empty)
                    {
                        var character = _config.Characters.FirstOrDefault(c => c.Id == e.CharacterId);
                        if (character != null)
                        {
                            var server = _config.Servers.FirstOrDefault(s => s.Id == character.ServerId);
                            return server != null ? $"{character.Name} ({server.Name})" : character.Name;
                        }
                    }
                    return "Unknown";
                });
                var charSummary = string.Join(", ", chars);
                var item = new ListViewItem(_config.CurrentProfileId == profile.Id ? "*" : "");
                item.SubItems.Add(profile.Name);
                item.SubItems.Add(charSummary);
                item.Tag = profile;
                listView.Items.Add(item);
            }
        }

        private static Profile GetSelectedProfile(ListView listView)
        {
            if (listView.SelectedItems.Count == 0)
                return null;
            return listView.SelectedItems[0].Tag as Profile;
        }

        private void ShowPlaceholder(string category)
        {
            var label = new Label
            {
                Text = $"{category} settings coming soon.",
                Location = new System.Drawing.Point(10, 10),
                AutoSize = true,
                ForeColor = System.Drawing.Color.Gray,
            };
            _settingsPanel.Controls.Add(label);
        }
    }
}
