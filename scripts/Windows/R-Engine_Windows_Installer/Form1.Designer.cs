namespace R_Engine_Windows_Installer
{
    partial class Form1
    {
        /// <summary>
        /// Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows Form Designer generated code

        /// <summary>
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            this.lblTitle = new System.Windows.Forms.Label();
            this.lblDescription = new System.Windows.Forms.Label();
            this.grpRequirements = new System.Windows.Forms.GroupBox();
            this.lblCompiler = new System.Windows.Forms.Label();
            this.lblNinja = new System.Windows.Forms.Label();
            this.lblCMake = new System.Windows.Forms.Label();
            this.picCompiler = new System.Windows.Forms.PictureBox();
            this.picNinja = new System.Windows.Forms.PictureBox();
            this.picCMake = new System.Windows.Forms.PictureBox();
            this.lblCompilerStatus = new System.Windows.Forms.Label();
            this.lblNinjaStatus = new System.Windows.Forms.Label();
            this.lblCMakeStatus = new System.Windows.Forms.Label();
            this.btnInstallMissing = new System.Windows.Forms.Button();
            this.btnCheckAgain = new System.Windows.Forms.Button();
            this.btnClose = new System.Windows.Forms.Button();
            this.progressBar = new System.Windows.Forms.ProgressBar();
            this.lblProgress = new System.Windows.Forms.Label();
            this.rtbLog = new System.Windows.Forms.RichTextBox();
            this.grpRequirements.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.picCompiler)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.picNinja)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.picCMake)).BeginInit();
            this.SuspendLayout();
            // 
            // lblTitle
            // 
            this.lblTitle.AutoSize = true;
            this.lblTitle.Font = new System.Drawing.Font("Microsoft Sans Serif", 16F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.lblTitle.Location = new System.Drawing.Point(24, 17);
            this.lblTitle.Margin = new System.Windows.Forms.Padding(6, 0, 6, 0);
            this.lblTitle.Name = "lblTitle";
            this.lblTitle.Size = new System.Drawing.Size(618, 51);
            this.lblTitle.TabIndex = 0;
            this.lblTitle.Text = "R-Engine Development Setup";
            // 
            // lblDescription
            // 
            this.lblDescription.AutoSize = true;
            this.lblDescription.Location = new System.Drawing.Point(28, 85);
            this.lblDescription.Margin = new System.Windows.Forms.Padding(6, 0, 6, 0);
            this.lblDescription.Name = "lblDescription";
            this.lblDescription.Size = new System.Drawing.Size(1062, 25);
            this.lblDescription.TabIndex = 1;
            this.lblDescription.Text = "This installer will check and install the required development tools for R-Engine" +
    " (CMake, Ninja, C++ Compiler).";
            // 
            // grpRequirements
            // 
            this.grpRequirements.Controls.Add(this.lblCompiler);
            this.grpRequirements.Controls.Add(this.lblNinja);
            this.grpRequirements.Controls.Add(this.lblCMake);
            this.grpRequirements.Controls.Add(this.picCompiler);
            this.grpRequirements.Controls.Add(this.picNinja);
            this.grpRequirements.Controls.Add(this.picCMake);
            this.grpRequirements.Controls.Add(this.lblCompilerStatus);
            this.grpRequirements.Controls.Add(this.lblNinjaStatus);
            this.grpRequirements.Controls.Add(this.lblCMakeStatus);
            this.grpRequirements.Location = new System.Drawing.Point(34, 135);
            this.grpRequirements.Margin = new System.Windows.Forms.Padding(6, 6, 6, 6);
            this.grpRequirements.Name = "grpRequirements";
            this.grpRequirements.Padding = new System.Windows.Forms.Padding(6, 6, 6, 6);
            this.grpRequirements.Size = new System.Drawing.Size(1000, 231);
            this.grpRequirements.TabIndex = 2;
            this.grpRequirements.TabStop = false;
            this.grpRequirements.Text = "Development Tools Status";
            // 
            // lblCompiler
            // 
            this.lblCompiler.AutoSize = true;
            this.lblCompiler.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.lblCompiler.Location = new System.Drawing.Point(100, 163);
            this.lblCompiler.Margin = new System.Windows.Forms.Padding(6, 0, 6, 0);
            this.lblCompiler.Name = "lblCompiler";
            this.lblCompiler.Size = new System.Drawing.Size(160, 26);
            this.lblCompiler.TabIndex = 8;
            this.lblCompiler.Text = "C++ Compiler";
            // 
            // lblNinja
            // 
            this.lblNinja.AutoSize = true;
            this.lblNinja.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.lblNinja.Location = new System.Drawing.Point(100, 106);
            this.lblNinja.Margin = new System.Windows.Forms.Padding(6, 0, 6, 0);
            this.lblNinja.Name = "lblNinja";
            this.lblNinja.Size = new System.Drawing.Size(67, 26);
            this.lblNinja.TabIndex = 7;
            this.lblNinja.Text = "Ninja";
            // 
            // lblCMake
            // 
            this.lblCMake.AutoSize = true;
            this.lblCMake.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.lblCMake.Location = new System.Drawing.Point(100, 48);
            this.lblCMake.Margin = new System.Windows.Forms.Padding(6, 0, 6, 0);
            this.lblCMake.Name = "lblCMake";
            this.lblCMake.Size = new System.Drawing.Size(86, 26);
            this.lblCMake.TabIndex = 6;
            this.lblCMake.Text = "CMake";
            // 
            // picCompiler
            // 
            this.picCompiler.Location = new System.Drawing.Point(30, 154);
            this.picCompiler.Margin = new System.Windows.Forms.Padding(6, 6, 6, 6);
            this.picCompiler.Name = "picCompiler";
            this.picCompiler.Size = new System.Drawing.Size(48, 46);
            this.picCompiler.TabIndex = 5;
            this.picCompiler.TabStop = false;
            // 
            // picNinja
            // 
            this.picNinja.Location = new System.Drawing.Point(30, 96);
            this.picNinja.Margin = new System.Windows.Forms.Padding(6, 6, 6, 6);
            this.picNinja.Name = "picNinja";
            this.picNinja.Size = new System.Drawing.Size(48, 46);
            this.picNinja.TabIndex = 4;
            this.picNinja.TabStop = false;
            // 
            // picCMake
            // 
            this.picCMake.Location = new System.Drawing.Point(30, 38);
            this.picCMake.Margin = new System.Windows.Forms.Padding(6, 6, 6, 6);
            this.picCMake.Name = "picCMake";
            this.picCMake.Size = new System.Drawing.Size(48, 46);
            this.picCMake.TabIndex = 3;
            this.picCMake.TabStop = false;
            // 
            // lblCompilerStatus
            // 
            this.lblCompilerStatus.AutoSize = true;
            this.lblCompilerStatus.Location = new System.Drawing.Point(260, 163);
            this.lblCompilerStatus.Margin = new System.Windows.Forms.Padding(6, 0, 6, 0);
            this.lblCompilerStatus.Name = "lblCompilerStatus";
            this.lblCompilerStatus.Size = new System.Drawing.Size(120, 25);
            this.lblCompilerStatus.TabIndex = 2;
            this.lblCompilerStatus.Text = "Checking...";
            // 
            // lblNinjaStatus
            // 
            this.lblNinjaStatus.AutoSize = true;
            this.lblNinjaStatus.Location = new System.Drawing.Point(260, 106);
            this.lblNinjaStatus.Margin = new System.Windows.Forms.Padding(6, 0, 6, 0);
            this.lblNinjaStatus.Name = "lblNinjaStatus";
            this.lblNinjaStatus.Size = new System.Drawing.Size(120, 25);
            this.lblNinjaStatus.TabIndex = 1;
            this.lblNinjaStatus.Text = "Checking...";
            // 
            // lblCMakeStatus
            // 
            this.lblCMakeStatus.AutoSize = true;
            this.lblCMakeStatus.Location = new System.Drawing.Point(260, 48);
            this.lblCMakeStatus.Margin = new System.Windows.Forms.Padding(6, 0, 6, 0);
            this.lblCMakeStatus.Name = "lblCMakeStatus";
            this.lblCMakeStatus.Size = new System.Drawing.Size(120, 25);
            this.lblCMakeStatus.TabIndex = 0;
            this.lblCMakeStatus.Text = "Checking...";
            // 
            // btnInstallMissing
            // 
            this.btnInstallMissing.Enabled = false;
            this.btnInstallMissing.Location = new System.Drawing.Point(34, 396);
            this.btnInstallMissing.Margin = new System.Windows.Forms.Padding(6, 6, 6, 6);
            this.btnInstallMissing.Name = "btnInstallMissing";
            this.btnInstallMissing.Size = new System.Drawing.Size(240, 58);
            this.btnInstallMissing.TabIndex = 3;
            this.btnInstallMissing.Text = "Install Missing Tools";
            this.btnInstallMissing.UseVisualStyleBackColor = true;
            this.btnInstallMissing.Click += new System.EventHandler(this.btnInstallMissing_Click);
            // 
            // btnCheckAgain
            // 
            this.btnCheckAgain.Location = new System.Drawing.Point(306, 396);
            this.btnCheckAgain.Margin = new System.Windows.Forms.Padding(6, 6, 6, 6);
            this.btnCheckAgain.Name = "btnCheckAgain";
            this.btnCheckAgain.Size = new System.Drawing.Size(200, 58);
            this.btnCheckAgain.TabIndex = 4;
            this.btnCheckAgain.Text = "Check Again";
            this.btnCheckAgain.UseVisualStyleBackColor = true;
            this.btnCheckAgain.Click += new System.EventHandler(this.btnCheckAgain_Click);
            // 
            // btnClose
            // 
            this.btnClose.Location = new System.Drawing.Point(884, 396);
            this.btnClose.Margin = new System.Windows.Forms.Padding(6, 6, 6, 6);
            this.btnClose.Name = "btnClose";
            this.btnClose.Size = new System.Drawing.Size(150, 58);
            this.btnClose.TabIndex = 5;
            this.btnClose.Text = "Close";
            this.btnClose.UseVisualStyleBackColor = true;
            this.btnClose.Click += new System.EventHandler(this.btnClose_Click);
            // 
            // progressBar
            // 
            this.progressBar.Location = new System.Drawing.Point(34, 485);
            this.progressBar.Margin = new System.Windows.Forms.Padding(6, 6, 6, 6);
            this.progressBar.Name = "progressBar";
            this.progressBar.Size = new System.Drawing.Size(1000, 38);
            this.progressBar.TabIndex = 6;
            this.progressBar.Visible = false;
            // 
            // lblProgress
            // 
            this.lblProgress.AutoSize = true;
            this.lblProgress.Location = new System.Drawing.Point(34, 548);
            this.lblProgress.Margin = new System.Windows.Forms.Padding(6, 0, 6, 0);
            this.lblProgress.Name = "lblProgress";
            this.lblProgress.Size = new System.Drawing.Size(0, 25);
            this.lblProgress.TabIndex = 7;
            // 
            // rtbLog
            // 
            this.rtbLog.Location = new System.Drawing.Point(34, 592);
            this.rtbLog.Margin = new System.Windows.Forms.Padding(6, 6, 6, 6);
            this.rtbLog.Name = "rtbLog";
            this.rtbLog.ReadOnly = true;
            this.rtbLog.Size = new System.Drawing.Size(996, 246);
            this.rtbLog.TabIndex = 8;
            this.rtbLog.Text = "";
            this.rtbLog.Visible = false;
            // 
            // Form1
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(12F, 25F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(1068, 502);
            this.Controls.Add(this.rtbLog);
            this.Controls.Add(this.lblProgress);
            this.Controls.Add(this.progressBar);
            this.Controls.Add(this.btnClose);
            this.Controls.Add(this.btnCheckAgain);
            this.Controls.Add(this.btnInstallMissing);
            this.Controls.Add(this.grpRequirements);
            this.Controls.Add(this.lblDescription);
            this.Controls.Add(this.lblTitle);
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedDialog;
            this.Margin = new System.Windows.Forms.Padding(6, 6, 6, 6);
            this.MaximizeBox = false;
            this.MinimizeBox = false;
            this.Name = "Form1";
            this.StartPosition = System.Windows.Forms.FormStartPosition.CenterScreen;
            this.Text = "R-Engine Development Setup";
            this.Load += new System.EventHandler(this.Form1_Load);
            this.grpRequirements.ResumeLayout(false);
            this.grpRequirements.PerformLayout();
            ((System.ComponentModel.ISupportInitialize)(this.picCompiler)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.picNinja)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.picCMake)).EndInit();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.Label lblTitle;
        private System.Windows.Forms.Label lblDescription;
        private System.Windows.Forms.GroupBox grpRequirements;
        private System.Windows.Forms.Label lblCMakeStatus;
        private System.Windows.Forms.Label lblNinjaStatus;
        private System.Windows.Forms.Label lblCompilerStatus;
        private System.Windows.Forms.PictureBox picCMake;
        private System.Windows.Forms.PictureBox picNinja;
        private System.Windows.Forms.PictureBox picCompiler;
        private System.Windows.Forms.Label lblCMake;
        private System.Windows.Forms.Label lblNinja;
        private System.Windows.Forms.Label lblCompiler;
        private System.Windows.Forms.Button btnInstallMissing;
        private System.Windows.Forms.Button btnCheckAgain;
        private System.Windows.Forms.Button btnClose;
        private System.Windows.Forms.ProgressBar progressBar;
        private System.Windows.Forms.Label lblProgress;
        private System.Windows.Forms.RichTextBox rtbLog;
    }
}