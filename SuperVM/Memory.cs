using System;

namespace SuperVM
{
	public class Memory
	{
		private readonly byte[] data;

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
		}

		public void SetUInt16(uint address, UInt16 value)
		{
			Array.Copy(
				BitConverter.GetBytes(value), 0,
				this.data, address,
				2);
		}

		public void SetUint32(uint address, UInt32 value)
		{
			Array.Copy(
				BitConverter.GetBytes(value), 0,
				this.data, address,
				4);
		}

		public byte this[uint address]
		{
			get { return this.data[address]; }
			set { this.data[address] = value; }
		}

		public byte[] Raw => this.data;

		public int Size => this.data.Length;
	}
}