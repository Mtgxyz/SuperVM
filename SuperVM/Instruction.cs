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
	}
}
