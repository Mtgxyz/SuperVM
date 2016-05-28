using System;

namespace SuperVM
{
	public sealed class MemoryChangedEventArgs : EventArgs
	{
		public MemoryChangedEventArgs(uint address) : 
			this(address, 1)
		{

		}

		public MemoryChangedEventArgs(uint address, uint length)
		{
			this.Address = address;
			this.Length = length;
		}

		public uint Address { get; private set; }
		public uint Length { get; private set; }
	}
}