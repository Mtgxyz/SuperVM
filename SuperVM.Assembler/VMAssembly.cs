using System.Collections.Generic;

namespace SuperVM.Assembler
{
	public sealed class VMAssembly
	{
		private readonly ulong[] code;
		private readonly Instruction[] instructions;
		private readonly int[] lines;
		private readonly string[] origins;

		public VMAssembly(ulong[] code, Instruction[] instructions, int[] lines, string[] origins)
		{
			this.code = code;
			this.instructions = instructions;
			this.lines = lines;
			this.origins = origins;
		}

		public Module CreateModule()
		{
			return new Module(this.instructions);
		}

		public IReadOnlyList<ulong> Code { get { return this.code; } }
		public IReadOnlyList<Instruction> Instrucions { get { return this.instructions; } }
		public IReadOnlyList<int> OriginalLine { get { return this.lines; } }
		public IReadOnlyList<string> Annotation { get { return this.origins; } }
	}
}