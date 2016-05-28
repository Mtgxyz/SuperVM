using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Linq;
using System.Text;
using System.Windows.Input;
using SuperVM.Assembler;
using System.Timers;
using System.Windows;

namespace SuperVM.VisualDebugger
{
	public class VirtualMachineModel : INotifyPropertyChanged
	{
		public event PropertyChangedEventHandler PropertyChanged;

		private Process process;
		private VMAssembly assembly;
		private Memory memory;
		private readonly Timer timer;

		public VirtualMachineModel()
		{
			this.memory = new Memory(1024);
			this.memory.LoadString("Hallo Welt!", 33);

			this.process = new Process()
			{
				Memory = this.memory,
			};
			this.process.SysCall += Process_SysCall;

			this.Source = "\tcpget\n\tjmp @main\n\tsyscall [ci: 0]\n; Here your code!\nmain:";

			this.StepCommand = new RelayCommand(this.Step);
			this.ResetCommand = new RelayCommand(this.Reset);
			this.RecompileCommand = new RelayCommand(this.Recompile);
			this.StartCommand = new RelayCommand(this.Start);
			this.StartSlowCommand = new RelayCommand(this.StartSlow);
			this.StopCommand = new RelayCommand(this.Stop);

			this.timer = new Timer();
			this.timer.Elapsed += Timer_Elapsed;

			this.Recompile();

			this.Reset();
		}

		private void Timer_Elapsed(object sender, ElapsedEventArgs e)
		{
			this.Step();
		}

		private void Stop()
		{
			this.timer.Stop();
		}

		private void Start()
		{
			this.timer.Interval = 10.0;
			this.timer.Start();
		}

		private void StartSlow()
		{
			this.timer.Interval = 100.0;
			this.timer.Start();
		}

		private void Process_SysCall(object sender, Process.CommandExecutionEnvironment e)
		{
			switch (e.Additional)
			{
				case 0: // Stop execution
				{
					this.Stop();
					break;
				}
				case 1: // Print char
				{
					this.Output += Encoding.ASCII.GetString(new[] { (byte)e.Input0 });
					this.OnPropertyChanged(nameof(Output));
					break;
				}
			}
		}

		public void Recompile()
		{
			this.Stop();
			try
			{
				this.assembly = Assembler.Assembler.Assemble(this.Source);
				this.process.Module = assembly.CreateModule();
			}
			catch (Exception ex)
			{
				MessageBox.Show(ex.Message, "SuperVM Assembler", MessageBoxButton.OK, MessageBoxImage.Warning);
			}
			this.Reset();
		}

		public void Reset()
		{
			this.timer.Stop();
			this.process.Reset();
			this.OnVmChanged();
		}

		public void Step()
		{
			try
			{
				this.process.Step();
			}
			catch (Exception ex)
			{
				this.Stop();
				MessageBox.Show(ex.Message, "SuperVM", MessageBoxButton.OK, MessageBoxImage.Warning);
			}
			this.OnVmChanged();
		}

		private void OnVmChanged()
		{
			OnPropertyChanged(nameof(StackPointer));
			OnPropertyChanged(nameof(BasePointer));
			OnPropertyChanged(nameof(CodePointer));
			OnPropertyChanged(nameof(FlagZ));
			OnPropertyChanged(nameof(FlagN));
			OnPropertyChanged(nameof(Stack));
			OnPropertyChanged(nameof(CurrentSourceLine));
		}

		private void OnPropertyChanged(string propertyName)
		{
			this.PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propertyName));
		}

		public int CurrentSourceLine => (this.CodePointer < this.assembly.OriginalLine.Count) ? (this.assembly.OriginalLine[this.CodePointer] - 1) : -1;

		public int StackPointer
		{
			get { return (int)process.StackPointer; }
			set { process.StackPointer = (uint)value; }
		}

		public int BasePointer
		{
			get { return (int)process.BasePointer; }
			set { process.BasePointer = (uint)value; }
		}

		public int CodePointer
		{
			get { return (int)process.CodePointer; }
			set { process.CodePointer = (uint)value; }
		}

		public bool FlagZ
		{
			get { return process.ZFlag; }
			set { process.ZFlag = value; }
		}

		public bool FlagN
		{
			get { return process.NFlag; }
			set { process.NFlag = value; }
		}

		public StackItem[] Stack => Enumerable
			.Range(1, (int)this.process.StackPointer)
			.Select(i => new StackItem(i, this.process))
			.ToArray();

		public MemoryProxy[] Memory => Enumerable
			.Range(0, this.memory.Size / 8)
			.Select(i => new MemoryProxy(this.memory, (uint)(8 * i)))
			.ToArray();

		public string Source { get; set; }

		public string Output { get; set; }

		public ICommand StepCommand { get; private set; }
		public ICommand ResetCommand { get; private set; }
		public ICommand RecompileCommand { get; private set; }
		public ICommand StartCommand { get; private set; }
		public ICommand StopCommand { get; private set; }
		public ICommand StartSlowCommand { get; private set; }
	}

	public class StackItem
	{
		private readonly Process process;

		public StackItem(int index, Process process)
		{
			this.Index = index;
			this.process = process;
		}

		public int Index { get; private set; }

		public int Value
		{
			get { return (int)this.process.Stack[this.Index]; }
			set { this.process.Stack[this.Index] = (uint)value; }
		}
	}

	public class MemoryProxy : INotifyPropertyChanged
	{
		private readonly Memory mem;
		private readonly uint offset;

		public event PropertyChangedEventHandler PropertyChanged;

		public MemoryProxy(Memory mem, uint offset)
		{
			this.mem = mem;
			this.offset = offset;

			this.Offset = Convert.ToString(this.offset, 16);

			this.mem.Changed += Mem_Changed;
		}

		private void Mem_Changed(object sender, MemoryChangedEventArgs e)
		{
			var upper = this.offset + 7;
			if (e.Address >= this.offset && (e.Address + e.Length) <= upper)
			{
				var idx = (e.Address - offset) % 8;
				this.PropertyChanged?.Invoke(this, new PropertyChangedEventArgs("D" + idx));
			}
		}

		private static string Read(byte val) => Convert.ToString(val, 16).ToUpper().PadLeft(2, '0');

		private static byte Write(string val) => Convert.ToByte(val, 16);

		public string Offset { get; private set; }

		public string D0
		{
			get { return Read(this.mem[this.offset + 0]); }
			set { this.mem[this.offset + 0] = Write(value); }
		}

		public string D1
		{
			get { return Read(this.mem[this.offset + 1]); }
			set { this.mem[this.offset + 1] = Write(value); }
		}

		public string D2
		{
			get { return Read(this.mem[this.offset + 2]); }
			set { this.mem[this.offset + 2] = Write(value); }
		}

		public string D3
		{
			get { return Read(this.mem[this.offset + 3]); }
			set { this.mem[this.offset + 3] = Write(value); }
		}

		public string D4
		{
			get { return Read(this.mem[this.offset + 4]); }
			set { this.mem[this.offset + 4] = Write(value); }
		}

		public string D5
		{
			get { return Read(this.mem[this.offset + 5]); }
			set { this.mem[this.offset + 5] = Write(value); }
		}

		public string D6
		{
			get { return Read(this.mem[this.offset + 6]); }
			set { this.mem[this.offset + 6] = Write(value); }
		}

		public string D7
		{
			get { return Read(this.mem[this.offset + 7]); }
			set { this.mem[this.offset + 7] = Write(value); }
		}
	}
}
