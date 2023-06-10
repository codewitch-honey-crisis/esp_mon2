using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.IO.Pipes;
using System.IO.Ports;
using System.Linq;
using System.Security.Principal;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace EspMonLux
{
	public partial class EspMonLux : Form
	{
		public EspMonLux()
		{
			InitializeComponent();
			RefreshPortList();
		}
		void RefreshPortList()
		{
			var p = PortCombo.Text;
			PortCombo.Items.Clear();
			var ports = SerialPort.GetPortNames();
			foreach (var port in ports)
			{
				PortCombo.Items.Add(port);
			}
			var idx = PortCombo.Items.Count - 1;
			if (!string.IsNullOrWhiteSpace(p))
			{
				for (var i = 0; i < PortCombo.Items.Count; ++i)
				{
					if (p == (string)PortCombo.Items[i])
					{
						idx = i;
						break;
					}
				}
			}
			PortCombo.SelectedIndex = 0;
		}
		void SendClient()
		{
			var pipeClient =
				   new NamedPipeClientStream(".", "EspMonLuxPipe",
					   PipeDirection.Out, PipeOptions.None,
					   TokenImpersonationLevel.Impersonation);
			pipeClient.Connect();
			var ba = Encoding.ASCII.GetBytes(PortCombo.Text);
			pipeClient.Write(ba, 0, ba.Length);
			pipeClient.Flush();
			pipeClient.Close();
		}
		private void RefreshButton_Click(object sender, EventArgs e)
		{
			RefreshPortList();
		}
	}
}
