using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Text.RegularExpressions;
using System.Threading.Tasks;

using static System.Console;

namespace SuperVM.Assembler
{
	class Program
	{
		static void Main(string[] args)
		{
			// GenerateDisassemblerListingsForC();

			if (args.Contains("-gen-code"))
			{
				MnemonicParser.GenerateFromDocumentation(
					@"../supervm/supervm.md");
				return;
			}

			foreach (var file in args.Where(a => !a.StartsWith("-") && Path.GetExtension(a) == ".asm"))
			{
				var output = Path.ChangeExtension(file, ".bin");
				var assembly = Assembler.Assemble(File.ReadAllText(file));

				var code = assembly.Code;

				Console.WriteLine("{0}*{1}:", output, code.Count);
				for (int i = 0; i < code.Count; i++)
				{
					Console.Write("; {0:D3} ", i);
					PrintInstruction(code[i], assembly.Annotation[i]);
				}
				using (var fs = File.Open(output, FileMode.Create, FileAccess.Write))
				{
					for (int i = 0; i < code.Count; i++)
					{
						var bits = BitConverter.GetBytes(code[i]);
						if (BitConverter.IsLittleEndian == false)
						{
							bits = bits.Reverse().ToArray();
						}
						fs.Write(bits, 0, bits.Length);
					}
				}
			}
		}

		private static void GenerateDisassemblerListingsForC()
		{
			foreach (var mnemonic in Assembler.mnemonics)
			{
				var i = mnemonic.Value;

				Write("{{ \"{0}\", {{ ", mnemonic.Key);

				switch (i.ExecutionZ)
				{
					case ExecutionMode.Always: Write("VM_EXEC_X, "); break;
					case ExecutionMode.Zero: Write("VM_EXEC_0, "); break;
					case ExecutionMode.One: Write("VM_EXEC_1, "); break;
				}
				switch (i.ExecutionN)
				{
					case ExecutionMode.Always: Write("VM_EXEC_X, "); break;
					case ExecutionMode.Zero: Write("VM_EXEC_0, "); break;
					case ExecutionMode.One: Write("VM_EXEC_1, "); break;
				}
				switch (i.Input0)
				{
					case InputType.Zero: Write("VM_INPUT_ZERO, "); break;
					case InputType.Pop: Write("VM_INPUT_POP, "); break;
					case InputType.Peek: Write("VM_INPUT_PEEK, "); break;
					case InputType.Argument: Write("VM_INPUT_ARG, "); break;
				}
				switch (i.Input1)
				{
					case InputType.Zero: Write("VM_INPUT_ZERO, "); break;
					case InputType.Pop: Write("VM_INPUT_POP, "); break;
					case InputType.Peek: Write("VM_INPUT_PEEK, "); break;
					case InputType.Argument: Write("VM_INPUT_ARG, "); break;
				}
				switch (i.Command)
				{
					case Command.Copy: Write("VM_CMD_COPY, "); break;
					case Command.Store: Write("VM_CMD_STORE, "); break;
					case Command.Load: Write("VM_CMD_LOAD, "); break;
					case Command.Get: Write("VM_CMD_GET, "); break;
					case Command.Set: Write("VM_CMD_SET, "); break;
					case Command.BpGet: Write("VM_CMD_BPGET, "); break;
					case Command.BpSet: Write("VM_CMD_BPSET, "); break;
					case Command.CpGet: Write("VM_CMD_CPGET, "); break;
					case Command.Math: Write("VM_CMD_MATH, "); break;
					case Command.SpGet: Write("VM_CMD_SPGET, "); break;
					case Command.SpSet: Write("VM_CMD_SPSET, "); break;
					case Command.SysCall: Write("VM_CMD_SYSCALL, "); break;
					case Command.HwIO: Write("VM_CMD_HWIO, "); break;
				}

				Write("{0}, ", i.CommandInfo);

				switch (i.ModifyFlags)
				{
					case false: Write("VM_FLAG_NO, "); break;
					case true: Write("VM_FLAG_YES, "); break;
				}

				switch (i.Output)
				{
					case OutputType.Discard: Write("VM_OUTPUT_DISCARD"); break;
					case OutputType.Push: Write("VM_OUTPUT_PUSH"); break;
					case OutputType.Jump: Write("VM_OUTPUT_JUMP"); break;
					case OutputType.JumpRelative: Write("VM_OUTPUT_JUMPR"); break;
				}

				Console.WriteLine("} },");
			}
		}

		static void PrintInstruction(ulong instr, string comment)
		{
			var str = Convert.ToString((long)instr, 2).PadLeft(64, '0');

			var portions = new[]
			{
				new { Start = 0,  Length = 32, Color = ConsoleColor.Red },
				new { Start = 32, Length = 2,  Color = ConsoleColor.DarkGreen },
				new { Start = 34, Length = 1,  Color = ConsoleColor.Green },
				new { Start = 35, Length = 16, Color = ConsoleColor.Magenta },
				new { Start = 51, Length = 6,  Color = ConsoleColor.Yellow },
				new { Start = 57, Length = 1,  Color = ConsoleColor.DarkCyan },
				new { Start = 58, Length = 2,  Color = ConsoleColor.Cyan },
				new { Start = 60, Length = 2,  Color = ConsoleColor.DarkBlue },
				new { Start = 62, Length = 2,  Color = ConsoleColor.Blue },
			};

			var fg = Console.ForegroundColor;
			foreach (var portion in portions)
			{
				Console.ForegroundColor = portion.Color;
				Console.Write("{0} ", str.Substring(portion.Start, portion.Length));
			}
			Console.ForegroundColor = fg;
			Console.WriteLine(" {0}", comment);
		}
	}
}
