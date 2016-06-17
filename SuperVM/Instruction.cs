using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SuperVM
{
	public struct Instruction
	{
		public ExecutionMode ExecutionZ;
		public ExecutionMode ExecutionN;
		public InputType Input0;
		public InputType Input1;
		public Command Command;
		public ushort CommandInfo;
		public bool ModifyFlags;
		public OutputType Output;
		public uint Argument;

		public override string ToString() => $"({Input0},{Input1}) {Command} -> {Output}";

		public ulong Encode()
		{
			ulong encoded = 0;
			encoded |= ((uint)(this.ExecutionZ) << 0);
			encoded |= ((uint)(this.ExecutionN) << 2);
			encoded |= ((uint)(this.Input0) << 4);
			encoded |= ((uint)(this.Input1) << 6);
			encoded |= ((uint)(this.Command) << 7);
			encoded |= ((uint)(this.CommandInfo) << 13);
			encoded |= ((uint)(this.ModifyFlags ? 1 : 0) << 29);
			encoded |= ((uint)(this.Output) << 30);
			encoded |= ((ulong)this.Argument << 32);
			return encoded;
		}

		public static Instruction Decode(ulong encoded)
		{
			var instr = new Instruction();

			instr.ExecutionZ = (ExecutionMode)((encoded >> 0) & 3);
			instr.ExecutionN = (ExecutionMode)((encoded >> 2) & 3);
			instr.Input0 = (InputType)((encoded >> 4) & 3);
			instr.Input1 = (InputType)((encoded >> 6) & 1);
			instr.Command = (Command)((encoded >> 7) & 63);
			instr.CommandInfo = (ushort)((encoded >> 13) & 65535);
			instr.ModifyFlags = ((encoded >> 29) & 1) != 0;
			instr.Output = (OutputType)((encoded >> 30) & 3);
			instr.Argument = (uint)(encoded >> 32);
			
			return instr;
		}
	}
}
