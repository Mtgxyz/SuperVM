using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SuperVM
{
	public sealed partial class Process
	{
		public const int StackSize = 1024;

		private readonly uint[] stack = new uint[StackSize];

		public event EventHandler<CommandExecutionEnvironment> SysCall;

		public event EventHandler<CommandExecutionEnvironment> HardwareIO;

		public Process()
		{
			this.CodePointer = 0;
			this.StackPointer = 0;
			this.BasePointer = 0;
			this.ZFlag = false;
			this.NFlag = false;
		}

		public void Push(uint value)
		{
			checked
			{
				this.stack[++this.StackPointer] = value;
			}
		}

		public uint Pop()
		{
			checked
			{
				return this.stack[this.StackPointer--];
			}
		}

		public uint Peek() => this.stack[this.StackPointer];

		public void Reset()
		{
			this.CodePointer = 0;
			this.BasePointer = 0;
			this.StackPointer = 0;
			this.NFlag = false;
			this.ZFlag = false;
		}

		public void Step()
		{
			Instruction instr = this.Module.Code[(int)this.CodePointer++];

			bool exec = true;
			switch (instr.ExecutionZ)
			{
				case ExecutionMode.Always:
				{/* Don't modify execution. */
					break;
				}
				case ExecutionMode.Zero:
				{
					if (this.ZFlag)
						exec = false;
					break;
				}
				case ExecutionMode.One:
				{
					if (!this.ZFlag)
						exec = false;
					break;
				}
				default: throw new InvalidOperationException("Invalid instruction: execZ undefined.");
			}
			switch (instr.ExecutionN)
			{
				case ExecutionMode.Always:
				{/* Don't modify execution. */
					break;
				}
				case ExecutionMode.Zero:
				{
					if (this.NFlag)
						exec = false;
					break;
				}
				case ExecutionMode.One:
				{
					if (!this.NFlag)
						exec = false;
					break;
				}
				default: throw new InvalidOperationException("Invalid instruction: execZ undefined.");
			}

			// Only do further instruction execution when
			// the execution condition is met.
			if (exec)
			{
				var cxe = new CommandExecutionEnvironment(this);
				switch (instr.Input0)
				{
					case InputType.Zero: cxe.Input0 = 0; break;
					case InputType.Pop: cxe.Input0 = this.Pop(); break;
					case InputType.Peek: cxe.Input0 = this.Peek(); break;
					case InputType.Argument: cxe.Input0 = instr.Argument; break;
					default: throw new InvalidOperationException("Invalid instruction: input0 undefined.");
				}

				switch (instr.Input1)
				{
					case InputType.Zero: cxe.Input1 = 0; break;
					case InputType.Pop: cxe.Input1 = this.Pop(); break;
					default: throw new InvalidOperationException("Invalid instruction: input1 undefined.");
				}

				cxe.Argument = instr.Argument;
				cxe.Additional = instr.CommandInfo;

				switch (instr.Command)
				{
					case Command.Copy: Cmd.Copy(cxe); break;
					case Command.Store: Cmd.Store(cxe); break;
					case Command.Load: Cmd.Load(cxe); break;
					case Command.Math: Cmd.Math(cxe); break;
					case Command.SysCall: this.SysCall?.Invoke(this, cxe); break;
					case Command.HwIO: this.HardwareIO?.Invoke(this, cxe); break;
					case Command.SpGet: Cmd.SpGet(cxe); break;
					case Command.SpSet: Cmd.SpSet(cxe); break;
					case Command.BpGet: Cmd.BpGet(cxe); break;
					case Command.BpSet: Cmd.BpSet(cxe); break;
					case Command.CpGet: Cmd.CpGet(cxe); break;
					case Command.Get: Cmd.Get(cxe); break;
					case Command.Set: Cmd.Set(cxe); break;
					default: throw new InvalidOperationException("Invalid instruction: command undefined.");
				}

				if (instr.ModifyFlags)
				{
					this.ZFlag = (cxe.Output == 0);
					this.NFlag = (cxe.Output & (1 << 31)) != 0;
				}

				switch (instr.Output)
				{
					case OutputType.Discard: break;
					case OutputType.Push: this.Push(cxe.Output); break;
					case OutputType.Jump: this.CodePointer = cxe.Output; break;
					case OutputType.JumpRelative:
					{
						checked
						{
							this.CodePointer = (uint)(this.CodePointer + (int)cxe.Output);
						}
						break;
					}
					default: throw new NotSupportedException("Invalid instruction: invalid output.");
				}
			}

		}

		public Module Module { get; set; }

		public Memory Memory { get; set; } = new Memory(16384);

		public uint CodePointer { get; set; }
		public uint StackPointer { get; set; }
		public uint BasePointer { get; set; }

		public bool ZFlag { get; set; }

		public bool NFlag { get; set; }

		public uint[] Stack => this.stack;




		public sealed class CommandExecutionEnvironment : EventArgs
		{
			public CommandExecutionEnvironment(Process process)
			{
				this.Process = process;
			}

			/// <summary>
			/// First input value
			/// </summary>
			public uint Input0 { get; set; }

			/// <summary>
			/// Second input value
			/// </summary>
			public uint Input1 { get; set; }

			/// <summary>
			/// The command argument
			/// </summary>
			public uint Argument { get; set; }

			/// <summary>
			/// The command info
			/// </summary>
			public uint Additional { get; set; }

			/// <summary>
			/// The output of the command.
			/// </summary>
			public uint Output { get; set; }

			/// <summary>
			/// The executing process.
			/// </summary>
			public Process Process { get; private set; }
		}
	}
}
