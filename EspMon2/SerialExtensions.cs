using System;
using System.IO.Ports;
using System.Runtime.InteropServices;
using System.Reflection;
using System.Text;
// provides extra serial functionality
internal static class SerialExtensions
{
    private static bool _IsValidMemberType(Type type)
    {
        return type == typeof(bool) ||
            type == typeof(byte) ||
            type == typeof(char) ||
            type == typeof(sbyte) ||
            type == typeof(short) ||
            type == typeof(ushort) ||
            type == typeof(int) ||
            type == typeof(uint) ||
            type == typeof(long) ||
            type == typeof(ulong) ||
            type == typeof(float) ||
            type == typeof(double) ||
            type == typeof(string);
    }
    private static void _WriteValue(SerialPort port, object value)
    {
        if (value == null) throw new ArgumentNullException("value");
        var type = value.GetType();
        if (type == typeof(bool))
        {
            port.Write(new byte[] { (byte)(((bool)value) ? 1 : 0)}, 0, 1);
        } else if (type == typeof(byte))
        {
            port.Write(new byte[] { (byte)value }, 0, 1);
        } else if (type == typeof(sbyte))
        {
            port.Write(new byte[] { (byte)(sbyte)value }, 0, 1);
        } else if (type == typeof(short))
        {
            var ba = BitConverter.GetBytes((short)value);
            port.Write(ba, 0, ba.Length);
        } else if (type == typeof(ushort))
        {
            var ba = BitConverter.GetBytes((ushort)value);
            port.Write(ba, 0, ba.Length);
        } else if (type == typeof(int))
        {
            var ba = BitConverter.GetBytes((int)value);
            port.Write(ba, 0, ba.Length);
        } else if (type == typeof(uint))
        {
            var ba = BitConverter.GetBytes((uint)value);
            port.Write(ba, 0, ba.Length);
        } else if (type == typeof(long))
        {
            var ba = BitConverter.GetBytes((long)value);
            port.Write(ba, 0, ba.Length);
        }
        else if (type == typeof(ulong))
        {
            var ba = BitConverter.GetBytes((ulong)value);
            port.Write(ba, 0, ba.Length);
        } else if (type == typeof(float))
		{
			var ba = BitConverter.GetBytes((float)value);
			port.Write(ba, 0, ba.Length);
		}
		else if (type == typeof(double))
		{
            var ba = BitConverter.GetBytes((double)value);
			port.Write(ba, 0, ba.Length);
		} else if(type==typeof(string))
        {
            var ba = Encoding.ASCII.GetBytes((string)value);
            var ba2 = new byte[ba.Length + 1];
            Buffer.BlockCopy(ba, 0, ba2, 0, ba.Length);
            ba2[ba.Length] = 0;
            port.Write(ba2, 0, ba2.Length);
        } else
        {
            throw new ArgumentException("Invalid value");
        }
	}
	public static void WriteStructReflection(this SerialPort _this, object value)
	{
        var t = value.GetType();
        var ma = t.GetMembers(BindingFlags.Instance|BindingFlags.Public);
        for (var i = 0; i < ma.Length; ++i)
        {
            var m = ma[i];
            var f = m as FieldInfo;
            if(f!=null)
            {
                if(!_IsValidMemberType(f.FieldType))
                {
                    throw new ArgumentException("The value contains an invalid member: " + m.Name);
                }
                var v = f.GetValue(value);
                _WriteValue(_this,v);
            }
            var p = m as PropertyInfo;
            if(p!=null && p.GetGetMethod()!=null)
            {
				if (!_IsValidMemberType(p.PropertyType))
				{
					throw new ArgumentException("The value contains an invalid member: " + m.Name);
				}
				var v = p.GetValue(value);
				_WriteValue(_this, v);
			}
            
        }
	}
    public static void WriteStruct(this SerialPort _this, object value)
    {
        var size = Marshal.SizeOf(value.GetType());
        var ptr = Marshal.AllocHGlobal(size);
        Marshal.StructureToPtr(value, ptr, false);
        var ba = new byte[size];
        Marshal.Copy(ptr, ba, 0, size);
        Marshal.FreeHGlobal(ptr);
        _this.Write(ba, 0, ba.Length);
    }
    public static void Write(this SerialPort _this, int value)
    {
		var bytes = BitConverter.GetBytes(value); 
        _this.Write(bytes,0, bytes.Length);
    }
    public static void Write(this SerialPort _this, uint value)
    {
        var bytes = BitConverter.GetBytes(value);
        _this.Write(bytes, 0, bytes.Length);
    }
    public static void Write(this SerialPort _this, short value)
    {
        var bytes = BitConverter.GetBytes(value);
        _this.Write(bytes, 0, bytes.Length);
    }
    public static void Write(this SerialPort _this, ushort value)
    {
        var bytes = BitConverter.GetBytes(value);
        _this.Write(bytes, 0, bytes.Length);
    }
    public static void Write(this SerialPort _this, long value)
    {
        var bytes = BitConverter.GetBytes(value);
        _this.Write(bytes, 0, bytes.Length);
    }
    public static void Write(this SerialPort _this, ulong value)
    {
        var bytes = BitConverter.GetBytes(value);
        _this.Write(bytes, 0, bytes.Length);
    }
    public static void Write(this SerialPort _this, byte value)
    {
        var bytes = BitConverter.GetBytes(value);
        _this.Write(bytes, 0, bytes.Length);
    }
    public static void Write(this SerialPort _this, sbyte value)
    {
        var bytes = BitConverter.GetBytes(value);
        _this.Write(bytes, 0, bytes.Length);
    }
    public static void Write(this SerialPort _this, float value)
    {
        var bytes = BitConverter.GetBytes(value);
        _this.Write(bytes, 0, bytes.Length);
    }
    public static void Write(this SerialPort _this, double value)
    {
        var bytes = BitConverter.GetBytes(value);
        _this.Write(bytes, 0, bytes.Length);
    }
    
    public static void Write(this SerialPort _this, bool value)
    {
        var bytes = BitConverter.GetBytes(value);
        _this.Write(bytes, 0, bytes.Length);
    }

    public static object ReadStruct(this SerialPort _this, Type type)
    {
        var bytes = new byte[Marshal.SizeOf(type)];
        if(bytes.Length!=_this.Read(bytes, 0, bytes.Length))
        {
            return null;
        }
        IntPtr ptr = Marshal.UnsafeAddrOfPinnedArrayElement(bytes, 0);
        return Marshal.PtrToStructure(ptr, type);
    }
    public static bool ReadStruct<T>(this SerialPort _this, out T outValue)
    {
        var bytes = new byte[Marshal.SizeOf(typeof(T))];
        if (bytes.Length != _this.Read(bytes, 0, bytes.Length))
        {
            outValue = default(T);
            return false;
        }
        IntPtr ptr = Marshal.UnsafeAddrOfPinnedArrayElement(bytes, 0);
        outValue = Marshal.PtrToStructure<T>(ptr);
        return true;
    }
    public static bool Read(this SerialPort _this, out bool outValue)
    {
        int i = _this.ReadByte();

        if (0 > i)
        {
            outValue = false;
            return false;
        }
        outValue = i!=0;
        return true;
    }

    public static bool Read(this SerialPort _this, out byte outValue) {
        int i = _this.ReadByte();

        if (0 > i)
        {
            outValue = 0;
            return false;
        }
        outValue = (byte)i;
        return true;
    }
    public static bool Read(this SerialPort _this, out sbyte outValue)
    {
        int i = _this.ReadByte();

        if (0 > i)
        {
            outValue = 0;
            return false;
        }
        outValue = (sbyte)i;
        return true;
    }
    public static bool Read(this SerialPort _this, out short outValue)
    {
        var bytes = new byte[sizeof(short)];
        if(bytes.Length!=_this.Read(bytes, 0, bytes.Length))
        {
            outValue = 0;
            return false;
        }
        outValue = BitConverter.ToInt16(bytes, 0);
        return true;
    }
    public static bool Read(this SerialPort _this, out ushort outValue)
    {
        var bytes = new byte[sizeof(ushort)];
        if (bytes.Length != _this.Read(bytes, 0, bytes.Length))
        {
            outValue = 0;
            return false;
        }
        outValue = BitConverter.ToUInt16(bytes, 0);
        return true;
    }
    public static bool Read(this SerialPort _this, out int outValue)
    {
        var bytes = new byte[sizeof(int)];
        if (bytes.Length != _this.Read(bytes, 0, bytes.Length))
        {
            outValue = 0;
            return false;
        }
        outValue = BitConverter.ToInt32(bytes, 0);
        return true;
    }
    public static bool Read(this SerialPort _this, out uint outValue)
    {
        var bytes = new byte[sizeof(uint)];
        if (bytes.Length != _this.Read(bytes, 0, bytes.Length))
        {
            outValue = 0;
            return false;
        }
        outValue = BitConverter.ToUInt32(bytes, 0);
        return true;
    }
    public static bool Read(this SerialPort _this, out long outValue)
    {
        var bytes = new byte[sizeof(long)];
        if (bytes.Length != _this.Read(bytes, 0, bytes.Length))
        {
            outValue = 0;
            return false;
        }
        outValue = BitConverter.ToInt64(bytes, 0);
        return true;
    }
    public static bool Read(this SerialPort _this, out ulong outValue)
    {
        var bytes = new byte[sizeof(ulong)];
        if (bytes.Length != _this.Read(bytes, 0, bytes.Length))
        {
            outValue = 0;
            return false;
        }
        outValue = BitConverter.ToUInt64(bytes, 0);
        return true;
    }
    public static bool Read(this SerialPort _this, out float outValue)
    {
        var bytes = new byte[sizeof(float)];
        if (bytes.Length != _this.Read(bytes, 0, bytes.Length))
        {
            outValue = 0;
            return false;
        }
        outValue = BitConverter.ToSingle(bytes, 0);
        return true;
    }
    public static bool Read(this SerialPort _this, out double outValue)
    {
        var bytes = new byte[sizeof(double)];
        if (bytes.Length != _this.Read(bytes, 0, bytes.Length))
        {
            outValue = 0;
            return false;
        }
        outValue = BitConverter.ToDouble(bytes, 0);
        return true;
    }
    
}

