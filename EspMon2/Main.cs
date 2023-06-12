using System;
using System.Text;
using System.Windows.Forms;
using System.IO.Ports;
using OpenHardwareMonitor.Hardware;
using System.Collections.Generic;
using System.Management;
using System.Diagnostics;
using System.Linq;
using static EspMon.Main;

namespace EspMon
{
	public partial class Main : Form
	{
		public class UpdateVisitor : IVisitor
		{
			public void VisitComputer(IComputer computer)
			{
				computer.Traverse(this);
			}
			public void VisitHardware(IHardware hardware)
			{
				hardware.Update();
				foreach (IHardware subHardware in hardware.SubHardware) subHardware.Accept(this);
			}
			public void VisitSensor(ISensor sensor) { }
			public void VisitParameter(IParameter parameter) { }
		}
		float cpuUsage;
		float gpuUsage;
		float cpuTemp;
		float cpuTjMax;
		float gpuTemp;
		float gpuTjMax;
		float cpuSpeed;
		float gpuSpeed;
		struct PortData
		{
			public SerialPort Port;
			public PortData(SerialPort port)
			{
				Port = port;
			}
			public override string ToString()
			{
				if(Port!=null)
				{
					return Port.PortName;
				}
				return "<null>";
			}
			public override bool Equals(object obj)
			{
				if(!(obj is PortData)) return false;
				PortData other = (PortData)obj;
				return object.Equals(Port, other.Port);
			}
			public override int GetHashCode()
			{
				if(Port==null) return 0;

				return Port.GetHashCode();
			}
		}
		private readonly Computer _computer = new Computer
		{
			CPUEnabled = true,
			GPUEnabled = true
		};
		public Main()
		{
			InitializeComponent();
			
			Notify.Icon = System.Drawing.SystemIcons.Information;
			Show();
			RefreshPortList();
			_computer.Open();
		}
		public float CpuTempMax {
			get {
				return 0;
			}
		}
		protected override void OnClosed(EventArgs e)
		{
			base.OnClosed(e);
            _computer.Close();
		}
		void RefreshPortList()
		{
			var ports = new List<SerialPort>();
			foreach(var item in PortBox.CheckedItems)
			{
				ports.Add(((PortData)item).Port); 
			}
			PortBox.Items.Clear();
			var names = SerialPort.GetPortNames();
			foreach(var name in names)
			{
				SerialPort found = null;
				foreach(var ep in ports)
				{
					if(ep.PortName==name) {
						found = ep;
						break;
					}
				}
				var chk = false;
				if (found == null) {
					found = new SerialPort(name, 115200);
				} else
				{
					chk = true;
				}
				PortBox.Items.Add(new PortData(found));
				if(chk)
				{
					PortBox.SetItemChecked(PortBox.Items.IndexOf(new PortData(found)), true);
				}
			}
		}

		private void RefreshButton_Click(object sender, EventArgs e)
		{
			StartedCheckBox.Checked = false;
			RefreshPortList();
		}

		private void UpdateTimer_Tick(object sender, EventArgs e)
		{
            
			if (StartedCheckBox.Checked)
			{
				CollectSystemInfo();
				ReadStatus data;
				data.CpuTemp = (byte)cpuTemp;
				data.CpuUsage = (byte)cpuUsage;
				data.GpuTemp = (byte)gpuTemp;
				data.GpuUsage = (byte)gpuUsage;
				data.CpuTempMax = (byte)cpuTjMax;
				data.GpuTempMax = (byte)gpuTjMax;

				int i = 0;
				foreach(PortData pdata in PortBox.Items)
				{
					var port = pdata.Port;
					if (PortBox.GetItemChecked(i))
					{
						try
						{
							if (!port.IsOpen)
							{
								port.Open();
							}
							if (port.WriteBufferSize - port.BytesToWrite > 1 + System.Runtime.InteropServices.Marshal.SizeOf(data))
							{
								var ba = new byte[] { 1 };
								port.Write(ba, 0, ba.Length);

								port.WriteStruct(data);
							}
							port.BaseStream.Flush();
						}
						catch { }
					}
					else
					{
						if (port.IsOpen)
						{
							try { port.Close(); } catch { }
						}
					}
					++i;
				}
						
				//_port.Close();
			} 
	
				
			
		}


        void CollectSystemInfo()
        {
            var updateVisitor = new UpdateVisitor();
			_computer.Accept(updateVisitor);
			cpuTjMax = 100;
			gpuTjMax = 90;
			for (int i = 0; i < _computer.Hardware.Length; i++)
			{
				if (_computer.Hardware[i].HardwareType == HardwareType.CPU)
				{
					for (int j = 0; j < _computer.Hardware[i].Sensors.Length; j++)
					{
						
                        var sensor = _computer.Hardware[i].Sensors[j];
						
						
						if (sensor.SensorType == SensorType.Temperature && sensor.Name.Contains("CPU Package")) 
                        {
							for(int k = 0; k < sensor.Parameters.Length;++k)
							{
								var p = sensor.Parameters[i];
								if(p.Name.ToLowerInvariant().Contains("tjmax"))
								{
									cpuTjMax = (float)p.Value;
								}
							}
							cpuTemp = sensor.Value.GetValueOrDefault();
						}
						else if (sensor.SensorType == SensorType.Load && sensor.Name.Contains("CPU Total"))
						{
							// store
							cpuUsage = sensor.Value.GetValueOrDefault();
						}
						else if (sensor.SensorType == SensorType.Clock && sensor.Name.Contains("CPU Core #1"))
						{
							// store
							cpuSpeed = sensor.Value.GetValueOrDefault();
						}
					}
				}
				if (_computer.Hardware[i].HardwareType == HardwareType.GpuAti || _computer.Hardware[i].HardwareType == HardwareType.GpuNvidia)
				{
					for (int j = 0; j < _computer.Hardware[i].Sensors.Length; j++)
					{
						var sensor = _computer.Hardware[i].Sensors[j];
						if (sensor.SensorType == SensorType.Temperature && sensor.Name.Contains("GPU Core"))
						{
							// store
							gpuTemp = sensor.Value.GetValueOrDefault();
						}
						else if (sensor.SensorType == SensorType.Load && sensor.Name.Contains("GPU Core"))
						{
							// store
							gpuUsage = sensor.Value.GetValueOrDefault();
						}
						else if (sensor.SensorType == SensorType.Clock && sensor.Name.Contains("GPU Core"))
						{
							// store
							gpuSpeed = sensor.Value.GetValueOrDefault();
						}

					}
				}
			}
			//System.Diagnostics.Debug.WriteLine(_computer.GetReport());
        }

		private void EspMon_Resize(object sender, EventArgs e)
		{
            if(WindowState==FormWindowState.Minimized)
			{
                Hide();
                Notify.Visible = true;
			}
		}

		private void Notify_Click(object sender, EventArgs e)
		{
            Show();
            Size = MinimumSize;
            WindowState = FormWindowState.Normal;
            Activate();
        }

		private void PortBox_ItemCheck(object sender, ItemCheckEventArgs e)
		{
			if(e.Index>=0 && e.NewValue!=CheckState.Checked)
			{
				var port = ((PortData)PortBox.Items[e.Index]).Port;
				try { if (port.IsOpen) port.Close(); } catch { }
				
			}
		}
	}
}
