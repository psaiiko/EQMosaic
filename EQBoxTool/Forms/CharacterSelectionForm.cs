using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Drawing;
using System.Linq;
using System.Runtime.InteropServices;
using System.Threading.Tasks;
using System.Windows.Forms;
using Vortice.Direct3D;
using Vortice.Direct3D11;
using Vortice.DXGI;

namespace EQBoxTool
{
    public partial class CharacterSelectionForm : Form
    {
        // WARNING: MAKE SURE TO ALIGN PROPERLY AND UPDATE THE BufferDescription BELOW TO MATCH THE SIZE!!!!!
        [StructLayout(LayoutKind.Sequential, Size = 32)]
        private struct PerFrameData
        {
            public float ShowBorder;        // 4
            public float AspectRatioOut;    // 8
            public float AspectRatioIn;     // 12
            public float CropToFill;        // 16
            public System.Numerics.Vector4 BorderColor; // 32
        }

        private class GameViewport
        {
            public ID3D11Texture2D backBuffer;
            public ID3D11RenderTargetView renderTargetView;
            public ID3D11Device device;
            public IDXGISwapChain swapChain;
            internal Control ctrl;
            internal ID3D11InputLayout layout;
            internal ID3D11Buffer vertices;
            internal ID3D11SamplerState sampler;
            internal ID3D11VertexShader vertexShader;
            internal ID3D11PixelShader pixelShader;
            internal ID3D11Buffer perFrameBuffer;
            internal string playerName;
            internal string loginName;
            internal long currentSharedHandle;
            internal ID3D11Texture2D sharedTexture;
            internal ID3D11ShaderResourceView sharedTextureView;
            internal uint sharedTextureWidth;
            internal uint sharedTextureHeight;
            internal Button launchButton;
            internal Label nameLabel;
            internal int lastFrameRendered;
            internal int laggedFrames;
        }

        private List<GameViewport> _viewports = new List<GameViewport>();
        private GameViewport _activeViewport = null;
        private ID3D11Device _sharedDevice;
        private IDXGIFactory1 _dxgiFactory;
        private AppConfig _config;
        private bool _spaceWasDown;

        public CharacterSelectionForm()
        {
            InitializeComponent();
            BackColor = Color.Black;
            notifyIcon1.Icon = new System.Drawing.Icon(System.IO.Path.Combine(Application.StartupPath, "appicon.ico"));
            notifyIcon1.DoubleClick += settingsToolStripMenuItem_Click;
            _instancesManager = new EQInstancesManager();
            FormClosing += (s, e) =>
            {
                _instancesManager.Dispose();
                _hotkeyManager?.Dispose();
            };

            _hotkeyManager = new GlobalHotkeyManager { HotkeyHandler = HandleInstanceSwitch };
            _hotkeyManager.Install();
        }

        private EQInstancesManager _instancesManager;
        private GlobalHotkeyManager _hotkeyManager;

        internal void OnDraw()
        {
            if (IsDisposed)
            {
                return;
            }

            CheckKeyboard();
            var foregroundWindow = WindowsInterop.GetForegroundWindow();

            foreach (var viewport in _viewports)
            {
                var ctx = viewport.device.ImmediateContext;
                ctx.ClearRenderTargetView(viewport.renderTargetView, new Vortice.Mathematics.Color4(0.1f, 0.1f, 0.1f, 1.0f));

                int? processId = _instancesManager.GetProcessIdByLoginName(viewport.loginName);
                bool isViewportActive = false;

                bool processRunning = processId.HasValue;
                if (viewport.launchButton != null)
                    viewport.launchButton.Visible = !processRunning;
                if (viewport.nameLabel != null)
                    viewport.nameLabel.Visible = processRunning;

                if (processId.HasValue)
                {
                    var gameHwnd = WindowsInterop.GetGameWindowHandle(processId.Value);
                    isViewportActive = (foregroundWindow == gameHwnd) || (foregroundWindow == Handle && _activeViewport == viewport);

                    if (WindowsInterop.TryReadMapFile(processId.Value, out long sharedHandle, out var generatedFrame))
                    {
                        if (viewport.lastFrameRendered == generatedFrame)
                            viewport.laggedFrames++;
                        else
                            viewport.laggedFrames = 0;

                        viewport.lastFrameRendered = generatedFrame;

                        if (sharedHandle != 0 && sharedHandle != viewport.currentSharedHandle)
                            OpenSharedTexture(viewport, sharedHandle);

                        if (viewport.sharedTexture != null)
                        {
                            ctx.OMSetRenderTargets(viewport.renderTargetView);
                            ctx.RSSetViewport(0, 0, viewport.ctrl.ClientSize.Width, viewport.ctrl.ClientSize.Height, 0.0f, 1.0f);

                            ctx.PSSetShaderResources(0, 1, new[] { viewport.sharedTextureView });
                            ctx.PSSetSamplers(0, 1, new[] { viewport.sampler });

                            var borderColorActive = new Vortice.Mathematics.Color4(1.0f, 1.0f, 0.9f);
                            var borderColorLag = new Vortice.Mathematics.Color4(1.0f, 0.2f, 0.2f);
                            var lagging = viewport.laggedFrames > 15;
                            var borderColor = lagging ? borderColorLag : borderColorActive;
                            bool showBorder = lagging || isViewportActive;
                            var perFrame = new PerFrameData
                            {
                                ShowBorder = showBorder ? 1.0f : 0.0f,
                                AspectRatioOut = viewport.ctrl.ClientSize.Width / (float)viewport.ctrl.ClientSize.Height,
                                AspectRatioIn = viewport.sharedTextureWidth / (float)viewport.sharedTextureHeight,
                                CropToFill = 1f,
                                BorderColor = borderColor
                            };

                            ctx.UpdateSubresource(in perFrame, viewport.perFrameBuffer, 0, 0, 0, null);

                            ctx.VSSetConstantBuffers(0, 1, new[] { viewport.perFrameBuffer });
                            ctx.PSSetConstantBuffers(0, 1, new[] { viewport.perFrameBuffer });

                            ctx.Draw(6, 0);
                        }
                    }
                }

                viewport.swapChain.Present(1, PresentFlags.None);
            }
        }

        private void CheckKeyboard()
        {
            if (WindowsInterop.GetForegroundWindow() != Handle)
                return;

            bool isDown = (WindowsInterop.GetAsyncKeyState(WindowsInterop.VK_SPACE) & 0x8000) != 0;
            if (isDown && !_spaceWasDown)
            {
                FormBorderStyle = FormBorderStyle == FormBorderStyle.None
                    ? FormBorderStyle.Sizable
                    : FormBorderStyle.None;
            }
            _spaceWasDown = isDown;
        }

        private void OpenSharedTexture(GameViewport vp, long handle)
        {
            if (vp.sharedTextureView != null)
            {
                vp.sharedTextureView.Dispose();
                vp.sharedTextureView = null;
            }
            if (vp.sharedTexture != null)
            {
                vp.sharedTexture.Dispose();
                vp.sharedTexture = null;
            }

            try
            {
                var ptr = new IntPtr(handle);
                vp.sharedTexture = vp.device.OpenSharedResource<ID3D11Texture2D>(ptr);
                vp.sharedTextureWidth = vp.sharedTexture.Description.Width;
                vp.sharedTextureHeight = vp.sharedTexture.Description.Height;
                vp.sharedTextureView = vp.device.CreateShaderResourceView(vp.sharedTexture);
                vp.currentSharedHandle = handle;
                Debug.WriteLine($"[BoxTool] Opened shared GPU texture for {vp.loginName}, handle=0x{handle:X}");
            }
            catch (Exception ex)
            {
                Debug.WriteLine($"[BoxTool] Failed to open shared texture: {ex.Message}. Overlay must be releasing");
                vp.currentSharedHandle = 0;
                vp.sharedTexture = null;
                vp.sharedTextureView = null;
            }
        }

        private void FocusWindowByIndex(int index)
        {
            var processId = _instancesManager.GetProcessIdByLoginName(_viewports[index].loginName);
            if (processId.HasValue)
            {
                IntPtr hwnd = WindowsInterop.GetGameWindowHandle(processId.Value);
                if (hwnd == IntPtr.Zero)
                    return;

                var screen = Screen.PrimaryScreen;
                var bounds = screen.Bounds;
                var workingArea = screen.WorkingArea;

                int clientWidth = (int)(bounds.Width * 0.75);
                int clientHeight = workingArea.Height;
                int clientX = bounds.Left;
                int clientY = bounds.Top;

                int capturedIndex = index;
                IntPtr capturedHwnd = hwnd;

                Task.Factory.StartNew(() =>
                {
                    if (WindowsInterop.IsIconic(capturedHwnd))
                    {
                        WindowsInterop.SendMessage(capturedHwnd, WindowsInterop.WM_SYSCOMMAND, (IntPtr)WindowsInterop.SC_RESTORE, IntPtr.Zero);
                    }

                    WindowsInterop.RelaxForegroundStealing();
                    WindowsInterop.SetForegroundWindow(capturedHwnd);

                    WindowsInterop.SetWindowClientRect(capturedHwnd, clientX, clientY, clientWidth, clientHeight);

                    BeginInvoke((Action)(() => _activeViewport = _viewports[capturedIndex]));
                });
            }
        }

        /// <summary>
        /// Backtick hotkey handler (called from the global keyboard hook). Switches focus to the
        /// next running instance (wrap-around) only when an EQ instance window is currently focused.
        /// Returns true if an instance was focused (key is consumed), false to pass the key through.
        /// </summary>
        private bool HandleInstanceSwitch(bool shiftPressed)
        {
            WindowsInterop.GetWindowThreadProcessId(WindowsInterop.GetForegroundWindow(), out int foregroundPid);
            if (foregroundPid == 0)
                return false;

            int current = -1;
            for (int i = 0; i < _viewports.Count; i++)
            {
                int? pid = _instancesManager.GetProcessIdByLoginName(_viewports[i].loginName);
                if (pid.HasValue && pid.Value == foregroundPid)
                {
                    current = i;
                    break;
                }
            }

            // Foreground window is not one of our EQ instances -> let the key pass through.
            if (current < 0)
                return false;

            int count = _viewports.Count;
            int direction = shiftPressed ? -1 : 1;

            for (int k = 1; k <= count; k++)
            {
                int next = (current + k * direction + count) % count;
                int? pid = _instancesManager.GetProcessIdByLoginName(_viewports[next].loginName);
                if (pid.HasValue && WindowsInterop.GetGameWindowHandle(pid.Value) != IntPtr.Zero)
                {
                    FocusWindowByIndex(next);
                    break;
                }
            }

            // An EQ instance was focused, so consume the key even if there was no next instance
            // to switch to (single running instance => no visible change).
            return true;
        }

        private GameViewport CreateViewport(Control ctrl, string playerName, string loginName)
        {
            var desc = new SwapChainDescription()
            {
                BufferDescription = new ModeDescription(
                    (uint)ctrl.ClientSize.Width, (uint)ctrl.ClientSize.Height,
                    new Rational(60, 1), Format.R8G8B8A8_UNorm),
                SampleDescription = new SampleDescription(1, 0),
                BufferUsage = Usage.Backbuffer | Usage.RenderTargetOutput,
                BufferCount = 1,
                Flags = SwapChainFlags.None,
                Windowed = true,
                OutputWindow = ctrl.Handle,
                SwapEffect = SwapEffect.Discard,
            };

            ID3D11Device device;
            IDXGISwapChain swapChain;
            if (_sharedDevice == null)
            {
                var result = D3D11.D3D11CreateDevice(
                    null,
                    DriverType.Hardware,
                    DeviceCreationFlags.None,
                    new[]
                    {
                        FeatureLevel.Level_11_1,
                        FeatureLevel.Level_11_0,
                        FeatureLevel.Level_10_1,
                        FeatureLevel.Level_10_0,
                    },
                    out device);

                if (result.Failure)
                    throw new InvalidOperationException("Failed to create D3D11 device");

                using (var dxgiDevice = device.QueryInterface<IDXGIDevice>())
                using (var adapter = dxgiDevice.GetAdapter())
                {
                    _dxgiFactory = adapter.GetParent<IDXGIFactory1>();
                }

                desc.OutputWindow = ctrl.Handle;
                swapChain = _dxgiFactory.CreateSwapChain(device, desc);
                _sharedDevice = device;
            }
            else
            {
                device = _sharedDevice;
                swapChain = _dxgiFactory.CreateSwapChain(device, desc);
            }

            var backBuffer = swapChain.GetBuffer<ID3D11Texture2D>(0);
            var renderTargetView = device.CreateRenderTargetView(backBuffer);

            var compiler = new ShaderCompiler(device);

            var vsBytecode = compiler.CompileFromFile(ShaderCompiler.ShaderType.VertexShader, "Graphics/ViewportShader.vs.hlsl", "main");
            var vertexShader = device.CreateVertexShader(vsBytecode);

            var psBytecode = compiler.CompileFromFile(ShaderCompiler.ShaderType.PixelShader, "Graphics/ViewportShader.ps.hlsl", "main");
            var pixelShader = device.CreatePixelShader(psBytecode);

            var layout = device.CreateInputLayout(
                new[]
                {
                    new InputElementDescription("POSITION", 0, Format.R32G32B32A32_Float, 0, 0, InputClassification.PerVertexData, 0),
                    new InputElementDescription("COLOR", 0, Format.R32G32B32A32_Float, 16, 0, InputClassification.PerVertexData, 0),
                    new InputElementDescription("TEXCOORD", 0, Format.R32G32_Float, 32, 0, InputClassification.PerVertexData, 0),
                },
                vsBytecode);

            var vertexData = new[]
            {
                -1.0f,  1.0f, 0.5f, 1.0f,      1.0f, 1.0f, 1.0f, 1.0f,      0.0f,  0.0f,
                 1.0f,  1.0f, 0.5f, 1.0f,      1.0f, 1.0f, 1.0f, 1.0f,      1.0f,  0.0f,
                -1.0f, -1.0f, 0.5f, 1.0f,      1.0f, 1.0f, 1.0f, 1.0f,      0.0f,  1.0f,
                 1.0f,  1.0f, 0.5f, 1.0f,      1.0f, 1.0f, 1.0f, 1.0f,      1.0f,  0.0f,
                 1.0f, -1.0f, 0.5f, 1.0f,      1.0f, 1.0f, 1.0f, 1.0f,      1.0f,  1.0f,
                -1.0f, -1.0f, 0.5f, 1.0f,      1.0f, 1.0f, 1.0f, 1.0f,      0.0f,  1.0f,
            };

            var vertexBufferDesc = new BufferDescription(
                (uint)(sizeof(float) * vertexData.Length),
                BindFlags.VertexBuffer,
                ResourceUsage.Immutable,
                CpuAccessFlags.None);

            ID3D11Buffer vertices;
            var vertexHandle = GCHandle.Alloc(vertexData, GCHandleType.Pinned);
            try
            {
                var initData = new SubresourceData(vertexHandle.AddrOfPinnedObject(), 0, 0);
                vertices = device.CreateBuffer(vertexBufferDesc, initData);
            }
            finally
            {
                vertexHandle.Free();
            }

            var samplerDesc = new SamplerDescription
            {
                Filter = Filter.MinMagMipPoint,
                AddressU = TextureAddressMode.Wrap,
                AddressV = TextureAddressMode.Wrap,
                AddressW = TextureAddressMode.Wrap,
                MipLODBias = 0,
                MaxAnisotropy = 1,
                ComparisonFunc = ComparisonFunction.Always,
                BorderColor = new Vortice.Mathematics.Color4(0, 0, 0, 0),
                MinLOD = 0,
                MaxLOD = float.MaxValue,
            };
            var sampler = device.CreateSamplerState(samplerDesc);

            var ctx = device.ImmediateContext;
            ctx.IASetInputLayout(layout);
            ctx.IASetPrimitiveTopology(PrimitiveTopology.TriangleList);
            ctx.IASetVertexBuffer(0, vertices, (uint)(sizeof(float) * 10), 0);

            ctx.VSSetShader(vertexShader);
            ctx.PSSetShader(pixelShader);

            var blendDesc = new BlendDescription
            {
                AlphaToCoverageEnable = false,
                IndependentBlendEnable = false,
            };
            blendDesc.RenderTarget[0].BlendEnable = false;
            blendDesc.RenderTarget[0].SourceBlend = Blend.SourceAlpha;
            blendDesc.RenderTarget[0].DestinationBlend = Blend.InverseSourceAlpha;
            blendDesc.RenderTarget[0].BlendOperation = BlendOperation.Add;
            blendDesc.RenderTarget[0].SourceBlendAlpha = Blend.Zero;
            blendDesc.RenderTarget[0].DestinationBlendAlpha = Blend.Zero;
            blendDesc.RenderTarget[0].BlendOperationAlpha = BlendOperation.Add;
            blendDesc.RenderTarget[0].RenderTargetWriteMask = ColorWriteEnable.All;

            ctx.OMSetBlendState(device.CreateBlendState(blendDesc));
            ctx.OMSetRenderTargets(renderTargetView);

            var perFrameBufferDesc = new BufferDescription(
                32,
                BindFlags.ConstantBuffer,
                ResourceUsage.Default,
                CpuAccessFlags.None,
                ResourceOptionFlags.None,
                0);
            var perFrameBuffer = device.CreateBuffer(perFrameBufferDesc);

            return new GameViewport
            {
                device = device,
                ctrl = ctrl,
                backBuffer = backBuffer,
                renderTargetView = renderTargetView,
                swapChain = swapChain,
                layout = layout,
                vertices = vertices,
                sampler = sampler,
                vertexShader = vertexShader,
                pixelShader = pixelShader,
                perFrameBuffer = perFrameBuffer,
                loginName = loginName,
                playerName = playerName,
            };
        }

        private void CharacterSelectionForm_Load(object sender, EventArgs e)
        {
        }

        public void Initialize(AppConfig config)
        {
            _config = config;
            StartPosition = FormStartPosition.Manual;
            var screen = Screen.PrimaryScreen;
            var wa = screen.WorkingArea;
            int w = screen.Bounds.Width / 4;
            SetBounds(wa.Right - w, wa.Top, w, wa.Height);

            var profile = config.Profiles.Find(p => p.Id == config.CurrentProfileId);
            if (profile == null)
            {
                var lbl = new Label
                {
                    Text = "No profile selected",
                    ForeColor = Color.Gray,
                    BackColor = Color.Transparent,
                    TextAlign = ContentAlignment.MiddleCenter,
                    Dock = DockStyle.Fill,
                    Font = new Font("Segoe UI", 16, FontStyle.Bold),
                    FlatStyle = FlatStyle.Flat,
                };
                Controls.Add(lbl);
                return;
            }

            var entries = profile.Entries.Where(e => e.Enabled).ToList();
            if (entries.Count == 0)
            {
                var lbl = new Label
                {
                    Text = "No profile selected",
                    ForeColor = Color.Gray,
                    BackColor = Color.Transparent,
                    TextAlign = ContentAlignment.MiddleCenter,
                    Dock = DockStyle.Fill,
                    Font = new Font("Segoe UI", 16, FontStyle.Bold),
                    FlatStyle = FlatStyle.Flat,
                };
                Controls.Add(lbl);
                return;
            }

            int columns = profile.Columns;
            int rows = profile.AutoRows ? (entries.Count + columns - 1) / columns : profile.Rows;
            int padding = 2;
            int cellW = (ClientSize.Width - padding * columns * 2) / columns;
            int cellH = (ClientSize.Height - padding * rows * 2) / rows;

            for (int i = 0; i < entries.Count; i++)
            {
                int col = i % columns;
                int row = i / columns;
                int x = padding + col * (cellW + padding * 2);
                int y = padding + row * (cellH + padding * 2);

                var entry = entries[i];
                var character = config.Characters.Find(c => c.Id == entry.CharacterId);
                if (character == null) continue;

                var account = config.Accounts.Find(a => a.Id == character.AccountId);
                string loginUsername = account?.LoginName ?? "";
                string displayName = character.Name;

                int labelHeight = 22;
                var pb = new PictureBox
                {
                    Location = new Point(x, y),
                    Size = new Size(cellW, cellH - labelHeight),
                    BackColor = Color.Black,
                };
                int index = i;
                pb.MouseUp += (sender, args) => { if (args.Button == MouseButtons.Left && args.Clicks == 1) FocusWindowByIndex(index); };

                Controls.Add(pb);
                var vp = CreateViewport(pb, displayName, loginUsername);
                _viewports.Add(vp);

                var capturedEntry = entry;
                var launchBtn = new Button
                {
                    Text = $"Launch {character.Name}",
                    Location = new Point(x, y + cellH - labelHeight),
                    Size = new Size(cellW, labelHeight),
                    FlatStyle = FlatStyle.Flat,
                    BackColor = Color.FromArgb(40, 90, 40),
                    ForeColor = Color.White,
                    Font = new Font("Segoe UI", 9, FontStyle.Bold),
                    Visible = false,
                };
                launchBtn.Click += (s, args) => LaunchSingleEntry(capturedEntry);
                vp.launchButton = launchBtn;
                Controls.Add(launchBtn);

                var label = new Label
                {
                    Text = displayName,
                    ForeColor = Color.White,
                    BackColor = Color.FromArgb(10, 10, 15),
                    TextAlign = ContentAlignment.MiddleCenter,
                    Location = new Point(x, y + cellH - labelHeight),
                    Size = new Size(cellW, labelHeight),
                    Font = new Font("Segoe UI", 9, FontStyle.Bold),
                    FlatStyle = FlatStyle.Flat,
                };
                vp.nameLabel = label;
                Controls.Add(label);
            }
        }

        private SettingsForm _settingsForm;

        private void closeToolStripMenuItem_Click(object sender, EventArgs e)
        {
            Application.Exit();
        }

        private void settingsToolStripMenuItem_Click(object sender, EventArgs e)
        {
            if (_settingsForm != null)
            {
                _settingsForm.Activate();
                return;
            }

            TopMost = false;
            _settingsForm = new SettingsForm(_config, RebuildViewports);
            _settingsForm.FormClosed += (s, args) =>
            {
                _settingsForm = null;
                TopMost = true;
            };
            _settingsForm.ShowDialog(this);
        }

        private void contextMenuStrip1_Opening(object sender, System.ComponentModel.CancelEventArgs e)
        {
            PopulateProfileMenu();
        }

        private void PopulateProfileMenu()
        {
            activateProfileToolStripMenuItem.DropDownItems.Clear();

            if (_config == null) return;

            foreach (var profile in _config.Profiles)
            {
                var profileItem = new ToolStripMenuItem(profile.Name);

                if (profile.Id == _config.CurrentProfileId)
                {
                    var launchItem = new ToolStripMenuItem("Launch");
                    launchItem.Click += (s, args) => LaunchProfile(profile);
                    profileItem.DropDownItems.Add(launchItem);
                }
                else
                {
                    var capturedProfile = profile;

                    var activateItem = new ToolStripMenuItem("Activate");
                    activateItem.Click += (s, args) =>
                    {
                        _config.CurrentProfileId = capturedProfile.Id;
                        ConfigManager.Save(_config);
                        RebuildViewports();
                    };
                    profileItem.DropDownItems.Add(activateItem);

                    var activateLaunchItem = new ToolStripMenuItem("Activate && Launch");
                    activateLaunchItem.Click += (s, args) =>
                    {
                        _config.CurrentProfileId = capturedProfile.Id;
                        ConfigManager.Save(_config);
                        RebuildViewports();
                        LaunchProfile(capturedProfile);
                    };
                    profileItem.DropDownItems.Add(activateLaunchItem);
                }

                activateProfileToolStripMenuItem.DropDownItems.Add(profileItem);
            }
        }

        private void LaunchSingleEntry(ProfileEntry entry)
        {
            var character = _config.Characters.FirstOrDefault(c => c.Id == entry.CharacterId);
            if (character == null) return;

            var account = _config.Accounts.Find(a => a.Id == character.AccountId);
            if (account == null) return;

            var install = _config.EverquestInstalls.Find(i => i.Id == account.EqInstallId);
            if (install == null) return;

            var server = _config.Servers.Find(i => i.Id == character.ServerId);
            if (server == null) return;

            var gameExePath = System.IO.Path.Combine(install.Path, "eqgame.exe");
            if (!System.IO.File.Exists(gameExePath)) return;

            if (_instancesManager.GetProcessIdByLoginName(account.LoginName).HasValue) return;

            var launcher = new EQLauncher();
            launcher.LaunchNewInstance(gameExePath, account, server, character);
        }

        private void LaunchProfile(Profile profile)
        {
            var entries = profile.Entries.Where(e => e.Enabled).ToList();
            if (entries.Count == 0) return;

            Task.Run(() =>
            {
                foreach (var entry in entries)
                    LaunchSingleEntry(entry);
            });
        }

        private void RebuildViewports()
        {
            foreach (var vp in _viewports)
            {
                vp.sharedTextureView?.Dispose();
                vp.sharedTexture?.Dispose();
                vp.renderTargetView?.Dispose();
                vp.backBuffer?.Dispose();
                vp.swapChain?.Dispose();
                vp.vertexShader?.Dispose();
                vp.pixelShader?.Dispose();
                vp.layout?.Dispose();
                vp.vertices?.Dispose();
                vp.sampler?.Dispose();
                vp.perFrameBuffer?.Dispose();
                if (vp.device != _sharedDevice)
                    vp.device?.Dispose();
            }
            _viewports.Clear();

            var toRemove = Controls.OfType<Control>()
                .Where(c => c is PictureBox || c is Button || (c is Label l && l.ForeColor == Color.White))
                .ToList();
            foreach (var c in toRemove)
                Controls.Remove(c);

            Initialize(_config);
        }
    }
}
