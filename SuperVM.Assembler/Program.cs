using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Text.RegularExpressions;
using System.Threading.Tasks;

namespace SuperVM.Assembler
{
	class Program
	{
		static void Main(string[] args)
		{
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
