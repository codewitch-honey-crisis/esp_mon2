
namespace EspMon
{
	partial class Main
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
			this.components = new System.ComponentModel.Container();
			this.RefreshButton = new System.Windows.Forms.Button();
			this.UpdateTimer = new System.Windows.Forms.Timer(this.components);
			this.Notify = new System.Windows.Forms.NotifyIcon(this.components);
			this.StartedCheckBox = new System.Windows.Forms.CheckBox();
			this.label1 = new System.Windows.Forms.Label();
			this.PortBox = new System.Windows.Forms.CheckedListBox();
			this.SuspendLayout();
			// 
			// RefreshButton
			// 
			this.RefreshButton.Location = new System.Drawing.Point(84, 6);
			this.RefreshButton.Margin = new System.Windows.Forms.Padding(3, 2, 3, 2);
			this.RefreshButton.Name = "RefreshButton";
			this.RefreshButton.Size = new System.Drawing.Size(86, 30);
			this.RefreshButton.TabIndex = 2;
			this.RefreshButton.Text = "Refresh";
			this.RefreshButton.UseVisualStyleBackColor = true;
			this.RefreshButton.Click += new System.EventHandler(this.RefreshButton_Click);
			// 
			// UpdateTimer
			// 
			this.UpdateTimer.Enabled = true;
			this.UpdateTimer.Tick += new System.EventHandler(this.UpdateTimer_Tick);
			// 
			// Notify
			// 
			this.Notify.BalloonTipText = "EspMon";
			this.Notify.BalloonTipTitle = "EspMon";
			this.Notify.Text = "EspMon";
			this.Notify.Click += new System.EventHandler(this.Notify_Click);
			// 
			// StartedCheckBox
			// 
			this.StartedCheckBox.AutoSize = true;
			this.StartedCheckBox.Location = new System.Drawing.Point(182, 10);
			this.StartedCheckBox.Name = "StartedCheckBox";
			this.StartedCheckBox.Size = new System.Drawing.Size(72, 20);
			this.StartedCheckBox.TabIndex = 3;
			this.StartedCheckBox.Text = "Started";
			this.StartedCheckBox.UseVisualStyleBackColor = true;
			// 
			// label1
			// 
			this.label1.AutoSize = true;
			this.label1.Location = new System.Drawing.Point(13, 6);
			this.label1.Name = "label1";
			this.label1.Size = new System.Drawing.Size(71, 16);
			this.label1.TabIndex = 5;
			this.label1.Text = "COM Ports";
			// 
			// PortBox
			// 
			this.PortBox.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
			this.PortBox.FormattingEnabled = true;
			this.PortBox.Location = new System.Drawing.Point(16, 50);
			this.PortBox.Name = "PortBox";
			this.PortBox.Size = new System.Drawing.Size(238, 140);
			this.PortBox.TabIndex = 6;
			// 
			// Main
			// 
			this.AutoScaleDimensions = new System.Drawing.SizeF(8F, 16F);
			this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
			this.ClientSize = new System.Drawing.Size(265, 197);
			this.Controls.Add(this.PortBox);
			this.Controls.Add(this.label1);
			this.Controls.Add(this.StartedCheckBox);
			this.Controls.Add(this.RefreshButton);
			this.Margin = new System.Windows.Forms.Padding(3, 2, 3, 2);
			this.MinimumSize = new System.Drawing.Size(283, 244);
			this.Name = "Main";
			this.Text = "EspMon";
			this.Resize += new System.EventHandler(this.EspMon_Resize);
			this.ResumeLayout(false);
			this.PerformLayout();

		}

		#endregion
		private System.Windows.Forms.Button RefreshButton;
		private System.Windows.Forms.Timer UpdateTimer;
		private System.Windows.Forms.NotifyIcon Notify;
		private System.Windows.Forms.CheckBox StartedCheckBox;
		private System.Windows.Forms.Label label1;
		private System.Windows.Forms.CheckedListBox PortBox;
	}
}

