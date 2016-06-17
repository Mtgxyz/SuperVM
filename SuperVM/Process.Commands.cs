using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SuperVM
{
	partial class Process
	{
		public Process(Module module, Memory memory)
		{
			this.Module = module;
			this.Memory = memory;
		}

		private static class Cmd
		{

			public static void Copy(CommandExecutionEnvironment cxe)
			{
				cxe.Output = cxe.Input0;
			}

			public static void Load(CommandExecutionEnvironment cxe)
			{
				switch (cxe.Additional)
				{
					case 0: cxe.Output = cxe.Process.Memory.GetUInt8(cxe.Input0); break;
					case 1: cxe.Output = cxe.Process.Memory.GetUInt16(cxe.Input0); break;
					case 2: cxe.Output = cxe.Process.Memory.GetUint32(cxe.Input0); break;
					default: throw new InvalidOperationException();
				}
			}

			public static void Store(CommandExecutionEnvironment cxe)
			{
				cxe.Output = cxe.Input1;
				switch (cxe.Additional)
				{
					case 0:
					{
						cxe.Process.Memory.SetUInt8(cxe.Input0, (byte)cxe.Input1);
						cxe.Output &= 0xFF;
						break;
					}
					case 1:
					{
						cxe.Process.Memory.SetUInt16(cxe.Input0, (ushort)cxe.Input1);
						cxe.Output &= 0xFFFF;
						break;
					}
					case 2:
					{
						cxe.Process.Memory.SetUint32(cxe.Input0, cxe.Input1);
						break;
					}
					default: throw new InvalidOperationException();
				}
			}

			public static void SpGet(CommandExecutionEnvironment cxe)
			{
				cxe.Output = cxe.Process.StackPointer;
			}

			public static void SpSet(CommandExecutionEnvironment cxe)
			{
				cxe.Output = cxe.Process.StackPointer = cxe.Input0;
			}

			public static void BpGet(CommandExecutionEnvironment cxe)
			{
				cxe.Output = cxe.Process.BasePointer;
			}

			public static void BpSet(CommandExecutionEnvironment cxe)
			{
				cxe.Output = cxe.Process.BasePointer = cxe.Input0;
			}

			public static void CpGet(CommandExecutionEnvironment cxe)
			{
				cxe.Output = cxe.Process.CodePointer + cxe.Additional;
			}

			public static void Get(CommandExecutionEnvironment cxe)
			{
				unchecked
				{
					cxe.Output = cxe.Process.stack[cxe.Process.BasePointer + (int)cxe.Input0];
				}
			}

			public static void Set(CommandExecutionEnvironment cxe)
			{
				cxe.Output = cxe.Process.stack[cxe.Process.BasePointer + (int)cxe.Input0] = cxe.Input1;
			}

			public static void Math(CommandExecutionEnvironment cxe)
			{
				switch ((MathCommand)cxe.Additional)
				{
					case MathCommand.Add: cxe.Output = cxe.Input1 + cxe.Input0; break;
					case MathCommand.Subtract: cxe.Output = cxe.Input1 - cxe.Input0; break;
					case MathCommand.Multiplicate: cxe.Output = cxe.Input1 * cxe.Input0; break;
					case MathCommand.Divide: cxe.Output = cxe.Input1 / cxe.Input0; break;
					case MathCommand.Modulo: cxe.Output = cxe.Input1 % cxe.Input0; break;
					case MathCommand.And: cxe.Output = cxe.Input1 & cxe.Input0; break;
					case MathCommand.Or: cxe.Output = cxe.Input1 | cxe.Input0; break;
					case MathCommand.Xor: cxe.Output = cxe.Input1 ^ cxe.Input0; break;
					case MathCommand.Not: cxe.Output = ~cxe.Input0; break;
				}
			}
		}
	}
}
