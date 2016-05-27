using System;
using System.IO;
using System.Linq;

namespace SuperVM.Assembler
{
	internal class MnemonicParser
	{
		public static void GenerateFromDocumentation(string file)
		{
			var relevantLines = File
				.ReadAllLines(file)
				.SkipWhile(l => l != "## Assembler Mnemonics")
				.Skip(1)
				.SkipWhile(l => string.IsNullOrWhiteSpace(l))
				.TakeWhile(l => l.StartsWith("|"))
				.Skip(2)
				.ToArray();
			
			var instructions = relevantLines
				.Select(l => l
					.Split(new[] { '|' }, StringSplitOptions.RemoveEmptyEntries)
					.Select(s => s.Trim())
					.ToArray())
				.ToDictionary(a => a[0], a => new Instruction()
				{
					ExecutionN = ExecutionMode.Always,
					ExecutionZ = ExecutionMode.Always,
					Input0 = ToInputMode(a[2]),
					Input1 = ToInputMode(a[3]),
					Command = ToCommand(a[4]),
					CommandInfo = ushort.Parse(a[5]),
					ModifyFlags = (a[7].ToLower() == "yes"),
					Output = ToOutput(a[6]),
					Argument = 0,
				});

			var fields = typeof(Instruction).GetFields();
			foreach (var item in instructions)
			{
				// { "a", new Instruction() {  } },
				Console.Write("{{ \"{0}\", new Instruction() {{ ", item.Key);

				foreach (var field in fields)
				{
					var value = field.GetValue(item.Value);
					if (field.FieldType.IsEnum)
						Console.Write("{0} = {1}.{2}, ", field.Name, field.FieldType.Name, value);
					else
						Console.Write("{0} = {1}, ", field.Name, value.ToString().ToLower());
				}

				Console.WriteLine(" } },");
			}
		}

		private static OutputType ToOutput(string v)
		{
			switch (v.ToLower())
			{
				case "discard": return OutputType.Discard;
				case "push": return OutputType.Push;
				case "jump": return OutputType.Jump;
				default:
				throw new NotSupportedException();
			}
		}

		private static Command ToCommand(string v)
		{
			return (Command)Enum.Parse(typeof(Command), v, true);
		}

		private static InputType ToInputMode(string v)
		{
			switch (v.ToLower())
			{
				case "zero": return InputType.Zero;
				case "peek": return InputType.Peek;
				case "pop": return InputType.Pop;
				case "arg": return InputType.Argument;
				default:
				throw new NotSupportedException();
			}
		}
	}
}