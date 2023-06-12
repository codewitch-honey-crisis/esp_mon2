using System.Runtime.InteropServices;
namespace EspMon
{
	// the data to send over serial
	// pack for 32-bit systems
	[StructLayout(LayoutKind.Sequential,Pack = 4)]
	internal struct ReadStatus
	{
		// command = 1
		public int CpuUsage;
		public int CpuTemp;
		public int CpuTempMax;
		public int GpuUsage;
		public int GpuTemp;
		public int GpuTempMax;
	}
}
