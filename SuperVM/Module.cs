using System.Collections.Generic;

namespace SuperVM
{
	public class Module
	{
		private readonly Instruction[] code;

		public Module(Instruction[] code)
		{
			this.code = code;
		}

		public IReadOnlyList<Instruction> Code => this.code;
	}
}