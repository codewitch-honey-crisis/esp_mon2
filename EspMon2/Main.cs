using System;
using System.Windows.Forms;
using System.IO.Ports;
using OpenHardwareMonitor.Hardware;
using System.Collections.Generic;

namespace EspMon
{
	public partial class Main : Form
	{
		// traverses OHWM data
		public class UpdateVisitor : IVisitor
		{
			public void VisitComputer(IComputer computer)
			{
				computer.Traverse(this);
			}
			public void VisitHardware(IHardware hardware)
			{
				hardware.Update();
				foreach (IHardware subHardware in hardware.SubHardware) 
					subHardware.Accept(this);
			}
			public void VisitSensor(ISensor sensor) { }
			public void VisitParameter(IParameter parameter) { }
		}
		// list item that holds a com port
		struct PortData
		{
			public SerialPort Port;
			public PortData(SerialPort port)
			{
				Port = port;
			}
			public override string ToString()
			{
				if (Port != null)
				{
					return Port.PortName;
				}
				return "<null>";
			}
			public override bool Equals(object obj)
			{
				if (!(obj is PortData)) return false;
				PortData other = (PortData)obj;
				return object.Equals(Port, other.Port);
			}
			public override int GetHashCode()
			{
				if (Port == null) return 0;

				return Port.GetHashCode();
			}
		}
		// local members for system info
		float cpuUsage;
		float gpuUsage;
		float cpuTemp;
		float cpuTjMax;
		float gpuTemp;
		float gpuTjMax;
		private readonly Computer _computer = new Computer
		{
			CPUEnabled = true,
			GPUEnabled = true
		};
		// Populates the PortBox control with COM ports
		void RefreshPortList()
		{
			// get the active ports
			var ports = new List<SerialPort>();
			foreach (var item in PortBox.CheckedItems)
			{
				ports.Add(((PortData)item).Port);
			}
			// reset the portbox
			PortBox.Items.Clear();
			var names = SerialPort.GetPortNames();
			foreach (var name in names)
			{
				// check to see if the port is
				// one of the checked ports
				SerialPort found = null;
				foreach (var ep in ports)
				{
					if (ep.PortName == name)
					{
						found = ep;
						break;
					}
				}
				var chk = false;
				if (found == null)
				{
					// create a new port
					found = new SerialPort(name, 115200);
				}
				else
				{
					chk = true;
				}
				PortBox.Items.Add(new PortData(found));
				if (chk)
				{
					// if it's one of our previously
					// checked ports, check it
					PortBox.SetItemChecked(
						PortBox.Items.IndexOf(new PortData(found)), true);
				}
			}
		}
		private void UpdateTimer_Tick(object sender, EventArgs e)
		{
			// only process if we're started
			if (StartedCheckBox.Checked)
			{
				// gather the system info
				CollectSystemInfo();
				// put it in the struct for sending
				ReadStatus data;
				data.CpuTemp = (byte)cpuTemp;
				data.CpuUsage = (byte)cpuUsage;
				data.GpuTemp = (byte)gpuTemp;
				data.GpuUsage = (byte)gpuUsage;
				data.CpuTempMax = (byte)cpuTjMax;
				data.GpuTempMax = (byte)gpuTjMax;
				// go through all the ports
				int i = 0;
				foreach (PortData pdata in PortBox.Items)
				{
					var port = pdata.Port;
					// if it's checked
					if (PortBox.GetItemChecked(i))
					{
						try
						{
							// open if necessary
							if (!port.IsOpen)
							{
								port.Open();
							}
							// if there's enough write buffer left
							if (port.WriteBufferSize - port.BytesToWrite > 
								1 + System.Runtime.InteropServices.Marshal.SizeOf(data))
							{
								// write the command id
								var ba = new byte[] { 1 };
								port.Write(ba, 0, ba.Length);
								// write the data
								port.WriteStruct(data);
							}
							port.BaseStream.Flush();
						}
						catch { }
					}
					else
					{
						// make sure unchecked ports are closed
						if (port.IsOpen)
						{
							try { port.Close(); } catch { }
						}
					}
					++i;
				}
			}
		}
		void CollectSystemInfo()
		{
			// use OpenHardwareMonitorLib to collect the system info
			var updateVisitor = new UpdateVisitor();
			_computer.Accept(updateVisitor);
			cpuTjMax = (int)CpuMaxUpDown.Value;
			gpuTjMax = (int)GpuMaxUpDown.Value;
			for (int i = 0; i < _computer.Hardware.Length; i++)
			{
				if (_computer.Hardware[i].HardwareType == HardwareType.CPU)
				{
					for (int j = 0; j < _computer.Hardware[i].Sensors.Length; j++)
					{
						var sensor = _computer.Hardware[i].Sensors[j];
						if (sensor.SensorType == SensorType.Temperature && 
							sensor.Name.Contains("CPU Package"))
						{
							for (int k = 0; k < sensor.Parameters.Length; ++k)
							{
								var p = sensor.Parameters[i];
								if (p.Name.ToLowerInvariant().Contains("tjmax"))
								{
									cpuTjMax = (float)p.Value;
								}
							}
							cpuTemp = sensor.Value.GetValueOrDefault();
						}
						else if (sensor.SensorType == SensorType.Load && 
							sensor.Name.Contains("CPU Total"))
						{
							// store
							cpuUsage = sensor.Value.GetValueOrDefault();
						}
					}
				}
				if (_computer.Hardware[i].HardwareType == HardwareType.GpuAti || 
					_computer.Hardware[i].HardwareType == HardwareType.GpuNvidia)
				{
					for (int j = 0; j < _computer.Hardware[i].Sensors.Length; j++)
					{
						var sensor = _computer.Hardware[i].Sensors[j];
						if (sensor.SensorType == SensorType.Temperature && 
							sensor.Name.Contains("GPU Core"))
						{
							// store
							gpuTemp = sensor.Value.GetValueOrDefault();
						}
						else if (sensor.SensorType == SensorType.Load && 
							sensor.Name.Contains("GPU Core"))
						{
							// store
							gpuUsage = sensor.Value.GetValueOrDefault();
						}
					}
				}
			}
		}
		protected override void OnClosed(EventArgs e)
		{
			base.OnClosed(e);
			foreach (PortData portData in PortBox.CheckedItems)
			{
				try
				{
					if (portData.Port != null && portData.Port.IsOpen)
					{
						portData.Port.Close();
					}
				}
				catch { }
			}
			_computer.Close();
		}
		private void RefreshButton_Click(object sender, EventArgs e)
		{
			StartedCheckBox.Checked = false;
			RefreshPortList();
		}
		private void EspMon_Resize(object sender, EventArgs e)
		{
			// hide the window to the tray on minimize
			if (WindowState == FormWindowState.Minimized)
			{
				Hide();
				Notify.Visible = true;
			}
		}

		private void Notify_Click(object sender, EventArgs e)
		{
			// show the window on tray click
			Show();
			Size = MinimumSize;
			WindowState = FormWindowState.Normal;
			Activate();
		}

		private void PortBox_ItemCheck(object sender, ItemCheckEventArgs e)
		{
			// if the item is unchecked, close the port
			if (e.Index >= 0 && e.NewValue != CheckState.Checked)
			{
				var port = ((PortData)PortBox.Items[e.Index]).Port;
				try { if (port.IsOpen) port.Close(); } catch { }

			}
		}
		public Main()
		{
			InitializeComponent();

			Notify.Icon = System.Drawing.SystemIcons.Information;
			Show();
			RefreshPortList();
			_computer.Open();
		}


	}
}
