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

		private SerialPort _port;
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
			var p = PortCombo.Text;
			PortCombo.Items.Clear();
			var ports = SerialPort.GetPortNames();
			foreach(var port in ports)
			{
				PortCombo.Items.Add(port);
			}
			var idx = PortCombo.Items.Count-1;
			if(!string.IsNullOrWhiteSpace(p))
			{
				for(var i = 0; i < PortCombo.Items.Count; ++i)
				{
					if(p==(string)PortCombo.Items[i])
					{
						idx = i;
						break;
					}
				}
			}
            PortCombo.SelectedIndex = 0;
		}

		private void RefreshButton_Click(object sender, EventArgs e)
		{
			StartedCheckBox.Checked = false;
			RefreshPortList();
		}

		private void UpdateTimer_Tick(object sender, EventArgs e)
		{
            CollectSystemInfo();
			if(_port!=null)
			{
				ReadStatus data;
				data.CpuTemp = (byte)cpuTemp;
				data.CpuUsage = (byte)cpuUsage;
				data.GpuTemp = (byte)gpuTemp;
				data.GpuUsage = (byte)gpuUsage;
				data.CpuTempMax = (byte)cpuTjMax;
				data.GpuTempMax = (byte)gpuTjMax;
				try
				{
					if (StartedCheckBox.Checked)
					{

						_port.Open();
						var ba = new byte[] { 1 };
						_port.Write(ba, 0, ba.Length);

						_port.WriteStruct(data);
						_port.BaseStream.Flush();
						_port.Close();
					}
	
				}
				catch { }
				
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

        private void PortCombo_SelectedIndexChanged(object sender, EventArgs e)
		{
            if(_port!=null && _port.IsOpen)
			{
                _port.Close();
			}
            _port = new SerialPort(((ComboBox)sender).Text,115200);
            _port.Encoding = Encoding.ASCII;
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
    }
}
