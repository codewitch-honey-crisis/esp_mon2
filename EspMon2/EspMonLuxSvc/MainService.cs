using OpenHardwareMonitor.Hardware;

using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Diagnostics;
using System.IO.Ports;
using System.Linq;
using System.ServiceProcess;
using System.Text;
using System.Threading.Tasks;
using Microsoft.Win32;
using EspMonLux;
using System.IO.Pipes;
using System.IO;
using System.Threading;

namespace EspMonLuxSvc
{
	public partial class MainService : ServiceBase
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
		float gpuTemp;
		float cpuSpeed;
		float gpuSpeed;

		private string _port;
		private readonly Computer _computer = new Computer
		{
			CPUEnabled = true,
			GPUEnabled = true
		};
		public MainService()
		{
			InitializeComponent();
			StartHW();
		}
		private void StartHW()
		{
			string port = null;
			var baseKey = RegistryKey.OpenBaseKey(RegistryHive.LocalMachine, RegistryView.Default);
			var key = baseKey.OpenSubKey("Software");
			if (key != null)
			{
				key = key.OpenSubKey("HTCW");
				if (key != null)
				{
					key = key.OpenSubKey("EspMonLux");
					if (key != null)
					{
						if (key.GetValueKind("Port") == RegistryValueKind.String)
						{
							port = key.GetValue("Port") as string;
						}
					}
				}
			}
			_port = port;
			if (port != null)
			{
				_computer.Open();
				UpdateTimer.Enabled = true;
			}
			else
			{
				UpdateTimer.Enabled = false;
				try
				{
					_computer.Close();
				}
				catch { }
			}
		}
		// Defines the data protocol for reading and writing strings on our stream
		public class StreamString
		{
			private Stream ioStream;
			private Encoding streamEncoding;

			public StreamString(Stream ioStream)
			{
				this.ioStream = ioStream;
				streamEncoding = new ASCIIEncoding();
			}

			public string ReadString()
			{
				int len = 0;

				len = ioStream.ReadByte() * 256;
				len += ioStream.ReadByte();
				byte[] inBuffer = new byte[len];
				ioStream.Read(inBuffer, 0, len);

				return streamEncoding.GetString(inBuffer);
			}

			public int WriteString(string outString)
			{
				byte[] outBuffer = streamEncoding.GetBytes(outString);
				int len = outBuffer.Length;
				if (len > UInt16.MaxValue)
				{
					len = (int)UInt16.MaxValue;
				}
				ioStream.WriteByte((byte)(len / 256));
				ioStream.WriteByte((byte)(len & 255));
				ioStream.Write(outBuffer, 0, len);
				ioStream.Flush();

				return outBuffer.Length + 2;
			}
		}

		private static void PipeThread(object data)
		{
			NamedPipeServerStream pipeServer =
				new NamedPipeServerStream("EspMonLuxPipe", PipeDirection.In, 1);

			int threadId = Thread.CurrentThread.ManagedThreadId;

			// Wait for a client to connect
			pipeServer.WaitForConnection();

			try
			{
				// Read the request from the client. Once the client has
				// written to the pipe its security token will be available.



				// Verify our identity to the connected client using a
				// string that the client anticipates.
				var ss = new StreamString(pipeServer);
				var svc = data as MainService;
				svc._port = ss.ReadString();
				var baseKey = RegistryKey.OpenBaseKey(RegistryHive.LocalMachine, RegistryView.Default);
				bool found = false;
				for (var i = 0; i < 2 && !found; ++i)
				{
					var key = baseKey.OpenSubKey("Software");
					if (key != null)
					{
						key = key.OpenSubKey("HTCW");

						if (key != null)
						{
							key = key.OpenSubKey("EspMonLux",true);
							if (key != null)
							{
								key.SetValue("Port", svc._port);
								found = true;
							}
						}
					}
					if(!found)
					{
						key = baseKey.OpenSubKey("Software",true);
						key = key.CreateSubKey("HTCW",true);
						if (key != null) {
							key = key.CreateSubKey("EspMonLux", true);
							if (key != null)
							{
								key.SetValue("Port", svc._port);
								found = true;
							}
						}
						
					}
				}

			}
			// Catch the IOException that is raised if the pipe is broken
			// or disconnected.
			catch (IOException e)
			{
				Console.WriteLine("ERROR: {0}", e.Message);
			}
			pipeServer.Close();
		}
		private void StopHW()
		{
			_port = null;
			UpdateTimer.Enabled = false;
			try
			{
				_computer.Close();
			}
			catch { }
		}
		protected override void OnPause()
		{
			base.OnPause();
			UpdateTimer.Enabled = false;
		}
		protected override void OnContinue()
		{
			base.OnContinue();
			if (_port != null)
			{
				UpdateTimer.Enabled = true;
			}
		}
		protected override void OnShutdown()
		{
			base.OnShutdown();
			StopHW();
		}
		protected override void OnStart(string[] args)
		{
		}

		protected override void OnStop()
		{
		}
		void CollectSystemInfo()
		{
			var updateVisitor = new UpdateVisitor();
			_computer.Accept(updateVisitor);
			for (int i = 0; i < _computer.Hardware.Length; i++)
			{
				if (_computer.Hardware[i].HardwareType == HardwareType.CPU)
				{
					for (int j = 0; j < _computer.Hardware[i].Sensors.Length; j++)
					{
						var sensor = _computer.Hardware[i].Sensors[j];
						if (sensor.SensorType == SensorType.Temperature)
						{
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

		}
		private void UpdateTimer_Tick(object sender, EventArgs e)
		{
			try
			{
				CollectSystemInfo();
				if (_port != null)
				{
					ReadStatus data;
					data.CpuTemp = (byte)cpuTemp;
					data.CpuUsage = (byte)cpuUsage;
					data.GpuTemp = (byte)gpuTemp;
					data.GpuUsage = (byte)gpuUsage;
					try
					{
						var port = new SerialPort(_port, 115200);
						port.Open();
						var ba = new byte[] { 1 };
						port.Write(ba, 0, ba.Length);
						port.WriteStruct(data);
						port.BaseStream.Flush();
						port.Close();
					}
					catch { }

				}
			}
			catch { }

		}
	}
}
