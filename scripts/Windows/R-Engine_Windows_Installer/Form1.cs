using System;
using System.Diagnostics;
using System.Drawing;
using System.IO;
using System.IO.Compression;
using System.Net.Http;
using System.Threading.Tasks;
using System.Windows.Forms;
using Microsoft.Win32;

namespace R_Engine_Windows_Installer
{
    /// <summary>
    /// Main form for R-Engine Windows installer. Manages the installation process
    /// for required development dependencies and building the R-Engine project.
    /// </summary>
    public partial class Form1 : Form
    {
        #region Private Fields

        private bool _cmakeInstalled = false;
        private bool _ninjaInstalled = false;
        private bool _compilerInstalled = false;
        private readonly HttpClient _httpClient = new HttpClient();

        // Win32 API constants
        private const uint WM_SETTINGCHANGE = 0x001a;
        private const uint SMTO_ABORTIFHUNG = 0x0002;
        private static readonly IntPtr HWND_BROADCAST = (IntPtr)0xffff;

        #endregion

        #region Constructor

        /// <summary>
        /// Initializes a new instance of the <see cref="Form1"/> class.
        /// </summary>
        public Form1()
        {
            InitializeComponent();
        }

        #endregion

        #region Event Handlers

        /// <summary>
        /// Handles the form load event by checking for required development tools.
        /// </summary>
        private async void Form1_Load(object sender, EventArgs e)
        {
            await CheckAllRequirementsAsync();
        }

        /// <summary>
        /// Handles the "Install Missing Tools" or "Build R-Engine" button click event.
        /// </summary>
        private async void btnInstallMissing_Click(object sender, EventArgs e)
        {
            // Check if this is a build operation or install operation
            bool allInstalled = _cmakeInstalled && _ninjaInstalled && _compilerInstalled;
            
            if (allInstalled)
            {
                // Build the project
                await BuildProjectAsync();
                return;
            }

            // Install missing dependencies
            await InstallMissingDependenciesAsync();
        }

        /// <summary>
        /// Handles the "Check Again" button click event.
        /// </summary>
        private async void btnCheckAgain_Click(object sender, EventArgs e)
        {
            await CheckAllRequirementsAsync();
        }

        /// <summary>
        /// Handles the "Close" button click event.
        /// </summary>
        private void btnClose_Click(object sender, EventArgs e)
        {
            this.Close();
        }

        #endregion

        #region Dependency Checking Methods

        /// <summary>
        /// Checks for all required development tools and updates the UI.
        /// </summary>
        private async Task CheckAllRequirementsAsync()
        {
            ResetStatusIcons();
            
            // Check CMake
            LogMessage("Checking for CMake installation...");
            _cmakeInstalled = await CheckCMakeInstallationAsync();
            UpdateStatusLabel(lblCMakeStatus, _cmakeInstalled);
            SetStatusIcon(picCMake, _cmakeInstalled);

            // Check Ninja
            LogMessage("Checking for Ninja installation...");
            _ninjaInstalled = await CheckNinjaInstallationAsync();
            UpdateStatusLabel(lblNinjaStatus, _ninjaInstalled);
            SetStatusIcon(picNinja, _ninjaInstalled);

            // Check C++ Compiler
            LogMessage("Checking for C++ Compiler installation...");
            _compilerInstalled = await CheckCompilerInstallationAsync();
            UpdateStatusLabel(lblCompilerStatus, _compilerInstalled);
            SetStatusIcon(picCompiler, _compilerInstalled);

            UpdateUIBasedOnDependencies();
        }

        /// <summary>
        /// Updates a status label based on installation status.
        /// </summary>
        /// <param name="label">The label to update.</param>
        /// <param name="isInstalled">Whether the dependency is installed.</param>
        private void UpdateStatusLabel(Label label, bool isInstalled)
        {
            label.Text = isInstalled ? "✓ Installed" : "✗ Not Found";
            label.ForeColor = isInstalled ? Color.Green : Color.Red;
        }

        /// <summary>
        /// Updates the UI controls based on the current dependency installation status.
        /// </summary>
        private void UpdateUIBasedOnDependencies()
        {
            bool allInstalled = _cmakeInstalled && _ninjaInstalled && _compilerInstalled;
            bool anyMissing = !allInstalled;
            
            btnInstallMissing.Enabled = true;
            btnInstallMissing.Text = allInstalled ? "Build R-Engine" : "Install Missing Tools";

            if (allInstalled)
            {
                LogMessage("All development tools are installed! You're ready to build R-Engine.");
                
                // Ask user if they want to proceed with building
                DialogResult result = MessageBox.Show(
                    "All dependencies are found! Would you like to build the R-Engine project now?", 
                    "Ready to Build", 
                    MessageBoxButtons.YesNo, 
                    MessageBoxIcon.Question);
                
                if (result == DialogResult.Yes)
                {
                    BuildProjectAsync().ConfigureAwait(false);
                }
            }
            else
            {
                LogMessage("Some development tools are missing. Click 'Install Missing Tools' to install them.");
            }
        }

        /// <summary>
        /// Checks if CMake is installed on the system.
        /// </summary>
        /// <returns>True if CMake is installed; otherwise, false.</returns>
        private async Task<bool> CheckCMakeInstallationAsync()
        {
            try
            {
                // Check if cmake is in PATH
                if (await CheckExecutableInPathAsync("cmake", "--version"))
                {
                    return true;
                }

                // Also check common installation paths
                string[] commonPaths = {
                    @"C:\Program Files\CMake\bin\cmake.exe",
                    @"C:\Program Files (x86)\CMake\bin\cmake.exe"
                };

                foreach (string path in commonPaths)
                {
                    if (File.Exists(path))
                    {
                        LogMessage($"CMake found at: {path}");
                        return true;
                    }
                }

                return false;
            }
            catch (Exception ex)
            {
                LogMessage($"CMake check failed: {ex.Message}");
                return false;
            }
        }

        /// <summary>
        /// Checks if Ninja build system is installed on the system.
        /// </summary>
        /// <returns>True if Ninja is installed; otherwise, false.</returns>
        private async Task<bool> CheckNinjaInstallationAsync()
        {
            try
            {
                return await CheckExecutableInPathAsync("ninja", "--version");
            }
            catch (Exception ex)
            {
                LogMessage($"Ninja check failed: {ex.Message}");
                return false;
            }
        }

        /// <summary>
        /// Checks if a C++ compiler is installed on the system.
        /// </summary>
        /// <returns>True if a C++ compiler is installed; otherwise, false.</returns>
        private async Task<bool> CheckCompilerInstallationAsync()
        {
            // Check for Visual Studio Build Tools or full Visual Studio
            if (CheckVisualStudioCompiler())
            {
                return true;
            }

            // Check for GCC
            if (await CheckExecutableInPathAsync("gcc", "--version"))
            {
                return true;
            }

            // Check for Clang
            if (await CheckExecutableInPathAsync("clang", "--version"))
            {
                return true;
            }

            return false;
        }

        /// <summary>
        /// Checks if Visual Studio or Visual Studio Build Tools are installed.
        /// </summary>
        /// <returns>True if Visual Studio is installed; otherwise, false.</returns>
        private bool CheckVisualStudioCompiler()
        {
            try
            {
                // Check registry for Visual Studio installations
                using (RegistryKey key = Registry.LocalMachine.OpenSubKey(@"SOFTWARE\Microsoft\VisualStudio\SxS\VS7"))
                {
                    if (key != null)
                    {
                        foreach (string version in key.GetValueNames())
                        {
                            string installPath = key.GetValue(version) as string;
                            if (!string.IsNullOrEmpty(installPath))
                            {
                                LogMessage($"Visual Studio found: {version} at {installPath}");
                                return true;
                            }
                        }
                    }
                }

                // Check for Build Tools
                string[] buildToolsPaths = {
                    @"C:\Program Files (x86)\Microsoft Visual Studio\2019\BuildTools",
                    @"C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools",
                    @"C:\Program Files\Microsoft Visual Studio\2019\BuildTools",
                    @"C:\Program Files\Microsoft Visual Studio\2022\BuildTools"
                };

                foreach (string path in buildToolsPaths)
                {
                    if (Directory.Exists(path))
                    {
                        LogMessage($"Visual Studio Build Tools found at: {path}");
                        return true;
                    }
                }
            }
            catch (Exception ex)
            {
                LogMessage($"Visual Studio check failed: {ex.Message}");
            }

            return false;
        }

        /// <summary>
        /// Checks if an executable is available in the system PATH.
        /// </summary>
        /// <param name="executable">The executable name to check.</param>
        /// <param name="arguments">The arguments to pass to the executable.</param>
        /// <returns>True if the executable is found and runs successfully; otherwise, false.</returns>
        private async Task<bool> CheckExecutableInPathAsync(string executable, string arguments)
        {
            try
            {
                ProcessStartInfo psi = new ProcessStartInfo
                {
                    FileName = executable,
                    Arguments = arguments,
                    UseShellExecute = false,
                    RedirectStandardOutput = true,
                    RedirectStandardError = true,
                    CreateNoWindow = true
                };

                using (Process process = Process.Start(psi))
                {
                    await Task.Run(() => process.WaitForExit());
                    if (process.ExitCode == 0)
                    {
                        string output = process.StandardOutput.ReadToEnd();
                        LogMessage($"{executable} found: {output.Split('\n')[0]}");
                        return true;
                    }
                }
            }
            catch (Exception ex)
            {
                LogMessage($"{executable} check failed: {ex.Message}");
            }

            return false;
        }

        /// <summary>
        /// Resets all status icons to the default state.
        /// </summary>
        private void ResetStatusIcons()
        {
            SetStatusIcon(picCMake, false);
            SetStatusIcon(picNinja, false);
            SetStatusIcon(picCompiler, false);
        }

        /// <summary>
        /// Sets the status icon for a dependency based on its installation status.
        /// </summary>
        /// <param name="pictureBox">The PictureBox to update.</param>
        /// <param name="isInstalled">Whether the dependency is installed.</param>
        private void SetStatusIcon(PictureBox pictureBox, bool isInstalled)
        {
            // Create simple colored rectangle as status indicator
            Bitmap bitmap = new Bitmap(24, 24);
            using (Graphics g = Graphics.FromImage(bitmap))
            {
                Color color = isInstalled ? Color.Green : Color.Red;
                using (SolidBrush brush = new SolidBrush(color))
                {
                    g.FillEllipse(brush, 0, 0, 24, 24);
                }
                
                // Add check mark or X
                using (Pen pen = new Pen(Color.White, 2))
                {
                    if (isInstalled)
                    {
                        // Draw check mark
                        g.DrawLine(pen, 6, 12, 10, 16);
                        g.DrawLine(pen, 10, 16, 18, 8);
                    }
                    else
                    {
                        // Draw X
                        g.DrawLine(pen, 6, 6, 18, 18);
                        g.DrawLine(pen, 18, 6, 6, 18);
                    }
                }
            }
            pictureBox.Image = bitmap;
        }

        #endregion

        #region Installation Methods

        /// <summary>
        /// Installs all missing dependencies.
        /// </summary>
        private async Task InstallMissingDependenciesAsync()
        {
            btnInstallMissing.Enabled = false;
            btnCheckAgain.Enabled = false;
            progressBar.Visible = true;
            progressBar.Value = 0;
            progressBar.Maximum = 3;
            rtbLog.Visible = true;
            this.Height = 450; // Expand form to show log

            try
            {
                lblProgress.Text = "Installing missing development tools...";

                if (!_cmakeInstalled)
                {
                    lblProgress.Text = "Installing CMake...";
                    await InstallCMakeAsync();
                    progressBar.Value++;
                }

                if (!_ninjaInstalled)
                {
                    lblProgress.Text = "Installing Ninja...";
                    await InstallNinjaAsync();
                    progressBar.Value++;
                }

                if (!_compilerInstalled)
                {
                    lblProgress.Text = "Installing C++ Compiler...";
                    await InstallCompilerAsync();
                    progressBar.Value++;
                }

                lblProgress.Text = "Installation completed. Checking tools again...";
                await CheckAllRequirementsAsync();
                lblProgress.Text = "Done!";
            }
            catch (Exception ex)
            {
                MessageBox.Show($"Installation failed: {ex.Message}", "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
            }
            finally
            {
                btnCheckAgain.Enabled = true;
                progressBar.Visible = false;
            }
        }

        /// <summary>
        /// Downloads and installs CMake.
        /// </summary>
        private async Task InstallCMakeAsync()
        {
            LogMessage("Downloading and installing CMake...");
            
            try
            {
                // Download CMake installer
                string downloadUrl = "https://github.com/Kitware/CMake/releases/download/v3.27.7/cmake-3.27.7-windows-x86_64.msi";
                string installerPath = Path.Combine(Path.GetTempPath(), "cmake-installer.msi");

                await DownloadFileAsync(downloadUrl, installerPath, "CMake installer");

                LogMessage("Running CMake installer...");
                ProcessStartInfo psi = new ProcessStartInfo
                {
                    FileName = "msiexec",
                    Arguments = $"/i \"{installerPath}\" /quiet ADD_CMAKE_TO_PATH=System",
                    UseShellExecute = false,
                    CreateNoWindow = true
                };

                using (Process process = Process.Start(psi))
                {
                    await Task.Run(() => process.WaitForExit());
                    if (process.ExitCode == 0)
                    {
                        LogMessage("CMake installed successfully!");
                    }
                    else
                    {
                        LogMessage($"CMake installation failed with exit code: {process.ExitCode}");
                    }
                }

                // Clean up
                if (File.Exists(installerPath))
                {
                    File.Delete(installerPath);
                }
            }
            catch (Exception ex)
            {
                LogMessage($"CMake installation error: {ex.Message}");
                throw;
            }
        }

        /// <summary>
        /// Downloads and installs Ninja build system.
        /// </summary>
        private async Task InstallNinjaAsync()
        {
            LogMessage("Downloading and installing Ninja...");
            
            try
            {
                string downloadUrl = "https://github.com/ninja-build/ninja/releases/download/v1.11.1/ninja-win.zip";
                string zipPath = Path.Combine(Path.GetTempPath(), "ninja.zip");
                string extractPath = @"C:\ninja";

                await DownloadFileAsync(downloadUrl, zipPath, "Ninja");

                LogMessage("Extracting Ninja...");
                if (Directory.Exists(extractPath))
                {
                    Directory.Delete(extractPath, true);
                }
                Directory.CreateDirectory(extractPath);

                ZipFile.ExtractToDirectory(zipPath, extractPath);

                LogMessage("Adding Ninja to PATH...");
                AddToSystemPath(extractPath);

                // Ensure ninja.exe is in the PATH or copy it to a directory already in PATH
                string ninjaExePath = Path.Combine(extractPath, "ninja.exe");
                string fallbackPath = Path.Combine(
                    Environment.GetFolderPath(Environment.SpecialFolder.LocalApplicationData),
                    @"Microsoft\WindowsApps");

                if (File.Exists(ninjaExePath) && Directory.Exists(fallbackPath))
                {
                    File.Copy(ninjaExePath, Path.Combine(fallbackPath, "ninja.exe"), true);
                    LogMessage($"Copied ninja.exe to {fallbackPath} as a fallback.");
                }

                LogMessage("Ninja installed successfully!");

                // Clean up
                if (File.Exists(zipPath))
                {
                    File.Delete(zipPath);
                }
            }
            catch (Exception ex)
            {
                LogMessage($"Ninja installation error: {ex.Message}");
                throw;
            }
        }

        /// <summary>
        /// Downloads and installs Visual Studio Build Tools with C++ compiler.
        /// </summary>
        private async Task InstallCompilerAsync()
        {
            LogMessage("Installing C++ Build Tools...");

            try
            {
                // Download Visual Studio Build Tools installer
                string downloadUrl = "https://aka.ms/vs/17/release/vs_buildtools.exe";
                string installerPath = Path.Combine(Path.GetTempPath(), "vs_buildtools.exe");

                await DownloadFileAsync(downloadUrl, installerPath, "Visual Studio Build Tools");

                LogMessage("Running Visual Studio Build Tools installer...");
                LogMessage("This may take several minutes...");

                ProcessStartInfo psi = new ProcessStartInfo
                {
                    FileName = installerPath,
                    Arguments = "--quiet --wait --add Microsoft.VisualStudio.Workload.VCTools --includeRecommended",
                    UseShellExecute = false,
                    CreateNoWindow = true
                };

                using (Process process = Process.Start(psi))
                {
                    await Task.Run(() => process.WaitForExit());
                    if (process.ExitCode == 0)
                    {
                        LogMessage("Visual Studio Build Tools installed successfully!");
                    }
                    else
                    {
                        LogMessage($"Build Tools installation completed with exit code: {process.ExitCode}");
                    }
                }

                // Clean up
                if (File.Exists(installerPath))
                {
                    File.Delete(installerPath);
                }
            }
            catch (Exception ex)
            {
                LogMessage($"C++ Compiler installation error: {ex.Message}");
                throw;
            }
        }

        /// <summary>
        /// Downloads a file from a URL to a local path.
        /// </summary>
        /// <param name="url">The URL to download from.</param>
        /// <param name="destinationPath">The local path to save the file to.</param>
        /// <param name="fileDescription">Description of the file for logging.</param>
        private async Task DownloadFileAsync(string url, string destinationPath, string fileDescription)
        {
            LogMessage($"Downloading {fileDescription}...");
            using (var response = await _httpClient.GetAsync(url))
            {
                response.EnsureSuccessStatusCode();
                using (var fileStream = File.Create(destinationPath))
                {
                    await response.Content.CopyToAsync(fileStream);
                }
            }
        }

        /// <summary>
        /// Adds a path to the system PATH environment variable.
        /// </summary>
        /// <param name="path">The path to add.</param>
        private void AddToSystemPath(string path)
        {
            try
            {
                const string name = "PATH";
                const string pathVariable = @"SYSTEM\CurrentControlSet\Control\Session Manager\Environment";

                using (RegistryKey environmentKey = Registry.LocalMachine.OpenSubKey(pathVariable, true))
                {
                    string currentPath = environmentKey.GetValue(name) as string;
                    if (currentPath != null && !currentPath.Contains(path))
                    {
                        environmentKey.SetValue(name, currentPath + ";" + path);
                        LogMessage($"Added {path} to system PATH");
                    }
                }

                // Notify system of environment variable changes
                SendMessageTimeout(HWND_BROADCAST, WM_SETTINGCHANGE, UIntPtr.Zero, "Environment", SMTO_ABORTIFHUNG, 5000, out _);
            }
            catch (Exception ex)
            {
                LogMessage($"Failed to add to PATH: {ex.Message}");
            }
        }

        #endregion

        #region Build Methods

        /// <summary>
        /// Builds the R-Engine project.
        /// </summary>
        private async Task BuildProjectAsync()
        {
            btnInstallMissing.Enabled = false;
            btnCheckAgain.Enabled = false;
            progressBar.Visible = true;
            progressBar.Style = ProgressBarStyle.Marquee;
            rtbLog.Visible = true;
            this.Height = 450;
            
            try
            {
                lblProgress.Text = "Building R-Engine project...";
                LogMessage("Starting R-Engine build process...");
                
                string buildScriptPath = await FindBuildScriptAsync();
                
                if (string.IsNullOrEmpty(buildScriptPath))
                {
                    LogMessage("Build cancelled - build.ps1 script not found or not selected.");
                    return;
                }
                
                string projectDir = Path.GetDirectoryName(buildScriptPath);
                LogMessage($"Found build script at: {buildScriptPath}");
                LogMessage($"Project directory: {projectDir}");
                
                await RunBuildScriptAsync(buildScriptPath, projectDir);
            }
            catch (Exception ex)
            {
                LogMessage($"Build error: {ex.Message}");
                MessageBox.Show($"Build failed: {ex.Message}", "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
            }
            finally
            {
                btnInstallMissing.Enabled = true;
                btnCheckAgain.Enabled = true;
                progressBar.Visible = false;
                progressBar.Style = ProgressBarStyle.Continuous;
                lblProgress.Text = "Ready";
            }
        }

        /// <summary>
        /// Finds the build.ps1 script in the project directory.
        /// </summary>
        /// <returns>The path to the build script or null if not found.</returns>
        private async Task<string> FindBuildScriptAsync()
        {
            // Find the build.ps1 script - start from current directory and traverse up to repository root
            string currentDir = Directory.GetCurrentDirectory();
            LogMessage($"Starting search from: {currentDir}");
            
            string buildScriptPath = FindBuildScript(currentDir);
            
            // If still not found, ask user to locate it
            if (string.IsNullOrEmpty(buildScriptPath))
            {
                LogMessage("build.ps1 script not found. Please locate it manually.");
                
                return await Task.Run(() => {
                    using (OpenFileDialog openFileDialog = new OpenFileDialog())
                    {
                        openFileDialog.Filter = "PowerShell scripts (*.ps1)|*.ps1|All files (*.*)|*.*";
                        openFileDialog.Title = "Please locate the build.ps1 script";
                        openFileDialog.FileName = "build.ps1";
                        
                        return openFileDialog.ShowDialog() == DialogResult.OK ? openFileDialog.FileName : null;
                    }
                });
            }
            
            return buildScriptPath;
        }

        /// <summary>
        /// Searches for the build.ps1 script starting from a directory and going up.
        /// </summary>
        /// <param name="startDirectory">The directory to start searching from.</param>
        /// <returns>The path to the build script or null if not found.</returns>
        private string FindBuildScript(string startDirectory)
        {
            string currentDir = Path.GetFullPath(startDirectory); // Ensure absolute path

            while (true)
            {
                string buildScriptPath = Path.Combine(currentDir, "build.ps1");
                LogMessage($"Checking: {buildScriptPath}");

                if (File.Exists(buildScriptPath))
                {
                    LogMessage($"Found build.ps1 at: {buildScriptPath}");
                    return buildScriptPath;
                }

                // Move up one directory level
                DirectoryInfo parent = Directory.GetParent(currentDir);
                if (parent == null)
                {
                    LogMessage("Reached filesystem root, stopping search.");
                    return null;
                }

                currentDir = parent.FullName;
                LogMessage($"Moving up to: {currentDir}");

                // Check for repository indicators after moving up
                bool hasRepoIndicators = File.Exists(Path.Combine(currentDir, "CMakeLists.txt")) ||
                                         Directory.Exists(Path.Combine(currentDir, ".git")) ||
                                         Directory.Exists(Path.Combine(currentDir, "src")) ||
                                         Directory.Exists(Path.Combine(currentDir, "include"));

                if (hasRepoIndicators && !File.Exists(Path.Combine(currentDir, "build.ps1")))
                {
                    LogMessage($"Found repository root at: {currentDir}, but no build.ps1");
                    return null;
                }
            }
        }

        /// <summary>
        /// Runs the build.ps1 script to build the R-Engine project.
        /// </summary>
        /// <param name="buildScriptPath">Path to the build script.</param>
        /// <param name="projectDir">The project directory.</param>
        private async Task RunBuildScriptAsync(string buildScriptPath, string projectDir)
        {
            // Execute the PowerShell script
            ProcessStartInfo psi = new ProcessStartInfo
            {
                FileName = "powershell.exe",
                Arguments = $"-ExecutionPolicy Bypass -File \"{buildScriptPath}\"",
                UseShellExecute = false,
                RedirectStandardOutput = true,
                RedirectStandardError = true,
                CreateNoWindow = true,
                WorkingDirectory = projectDir
            };

            using (Process process = Process.Start(psi))
            {
                // Read output asynchronously to show progress
                process.OutputDataReceived += (sender, e) => {
                    if (!string.IsNullOrEmpty(e.Data))
                    {
                        Invoke(new Action(() => LogMessage($"BUILD: {e.Data}")));
                    }
                };
                
                process.ErrorDataReceived += (sender, e) => {
                    if (!string.IsNullOrEmpty(e.Data))
                    {
                        Invoke(new Action(() => LogMessage($"ERROR: {e.Data}")));
                    }
                };
                
                process.BeginOutputReadLine();
                process.BeginErrorReadLine();
                
                await Task.Run(() => process.WaitForExit());
                
                if (process.ExitCode == 0)
                {
                    LogMessage("🎉 Build completed successfully!");
                    lblProgress.Text = "Build completed successfully!";
                    MessageBox.Show("R-Engine built successfully!", "Build Complete", MessageBoxButtons.OK, MessageBoxIcon.Information);
                }
                else
                {
                    LogMessage($"❌ Build failed with exit code: {process.ExitCode}");
                    lblProgress.Text = "Build failed!";
                    MessageBox.Show($"Build failed with exit code: {process.ExitCode}", "Build Failed", MessageBoxButtons.OK, MessageBoxIcon.Error);
                }
            }
        }

        #endregion

        #region Utility Methods

        /// <summary>
        /// Logs a message to the log text box with timestamp.
        /// </summary>
        /// <param name="message">The message to log.</param>
        private void LogMessage(string message)
        {
            string timestamp = DateTime.Now.ToString("HH:mm:ss");
            rtbLog.AppendText($"[{timestamp}] {message}\n");
            rtbLog.ScrollToCaret();
            Application.DoEvents();
        }

        #endregion

        #region Win32 API Import

        /// <summary>
        /// Sends a message to all top-level windows by broadcasting.
        /// </summary>
        [System.Runtime.InteropServices.DllImport("user32.dll", SetLastError = true, CharSet = System.Runtime.InteropServices.CharSet.Auto)]
        static extern IntPtr SendMessageTimeout(
            IntPtr hWnd, 
            uint Msg, 
            UIntPtr wParam, 
            string lParam, 
            uint fuFlags, 
            uint uTimeout, 
            out UIntPtr lpdwResult);

        #endregion
    }
}