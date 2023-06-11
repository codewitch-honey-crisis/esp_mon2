
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
			this.PortCombo = new System.Windows.Forms.ComboBox();
			this.RefreshButton = new System.Windows.Forms.Button();
			this.UpdateTimer = new System.Windows.Forms.Timer(this.components);
			this.Notify = new System.Windows.Forms.NotifyIcon(this.components);
			this.StartedCheckBox = new System.Windows.Forms.CheckBox();
			this.SuspendLayout();
			// 
			// PortCombo
			// 
			this.PortCombo.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
			this.PortCombo.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
			this.PortCombo.FormattingEnabled = true;
			this.PortCombo.Location = new System.Drawing.Point(11, 10);
			this.PortCombo.Margin = new System.Windows.Forms.Padding(3, 2, 3, 2);
			this.PortCombo.Name = "PortCombo";
			this.PortCombo.Size = new System.Drawing.Size(191, 24);
			this.PortCombo.TabIndex = 0;
			this.PortCombo.SelectedIndexChanged += new System.EventHandler(this.PortCombo_SelectedIndexChanged);
			// 
			// RefreshButton
			// 
			this.RefreshButton.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
			this.RefreshButton.Location = new System.Drawing.Point(206, 6);
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
			this.StartedCheckBox.Location = new System.Drawing.Point(12, 39);
			this.StartedCheckBox.Name = "StartedCheckBox";
			this.StartedCheckBox.Size = new System.Drawing.Size(72, 20);
			this.StartedCheckBox.TabIndex = 3;
			this.StartedCheckBox.Text = "Started";
			this.StartedCheckBox.UseVisualStyleBackColor = true;
			// 
			// Main
			// 
			this.AutoScaleDimensions = new System.Drawing.SizeF(8F, 16F);
			this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
			this.ClientSize = new System.Drawing.Size(307, 65);
			this.Controls.Add(this.StartedCheckBox);
			this.Controls.Add(this.RefreshButton);
			this.Controls.Add(this.PortCombo);
			this.Margin = new System.Windows.Forms.Padding(3, 2, 3, 2);
			this.MaximizeBox = false;
			this.MaximumSize = new System.Drawing.Size(325, 112);
			this.MinimumSize = new System.Drawing.Size(325, 112);
			this.Name = "Main";
			this.Text = "EspMon";
			this.Resize += new System.EventHandler(this.EspMon_Resize);
			this.ResumeLayout(false);
			this.PerformLayout();

		}

		#endregion

		private System.Windows.Forms.ComboBox PortCombo;
		private System.Windows.Forms.Button RefreshButton;
		private System.Windows.Forms.Timer UpdateTimer;
		private System.Windows.Forms.NotifyIcon Notify;
		private System.Windows.Forms.CheckBox StartedCheckBox;
	}
}

