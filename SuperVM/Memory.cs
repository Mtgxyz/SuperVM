using System;
using System.Text;

namespace SuperVM
{
	public class Memory
	{
		private readonly byte[] data;

		public event EventHandler<MemoryChangedEventArgs> Changed;

		public Memory(int size)
		{
			this.data = new byte[size];
		}

		public byte GetUInt8(uint address)
		{
			return this.data[address];
		}

		public UInt16 GetUInt16(uint address)
		{
			return BitConverter.ToUInt16(this.data, (int)address);
		}

		public UInt32 GetUint32(uint address)
		{
			return BitConverter.ToUInt32(this.data, (int)address);
		}

		public void SetUInt8(uint address, byte value)
		{
			this.data[address] = value;
			this.Changed?.Invoke(this, new MemoryChangedEventArgs(address, 1));
		}

		public void SetUInt16(uint address, UInt16 value)
		{
			Array.Copy(
				BitConverter.GetBytes(value), 0,
				this.data, address,
				2);
			this.Changed?.Invoke(this, new MemoryChangedEventArgs(address, 2));
		}

		public void SetUint32(uint address, UInt32 value)
		{
			Array.Copy(
				BitConverter.GetBytes(value), 0,
				this.data, address,
				4);
			this.Changed?.Invoke(this, new MemoryChangedEventArgs(address, 4));
		}

		public void LoadString(string text, int start)
		{
			var bits = Encoding.ASCII.GetBytes(text);
			for (int i = 0; i < bits.Length; i++)
			{
				this.data[start + i] = bits[i];
			}
			this.Changed?.Invoke(this, new MemoryChangedEventArgs((uint)start, (uint)bits.Length));
		}

		public byte this[uint address]
		{
			get { return this.data[address]; }
			set
			{
				this.data[address] = value;
				this.Changed?.Invoke(this, new MemoryChangedEventArgs(address, 1));
			}
		}

		public byte[] Raw => this.data;

		public int Size => this.data.Length;
	}
}