using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Linq;
using System.Text;
using System.Windows.Input;
using SuperVM.Assembler;

namespace SuperVM.VisualDebugger
{
	public class VirtualMachineModel : INotifyPropertyChanged
	{
		public event PropertyChangedEventHandler PropertyChanged;

		private Process process;
		private VMAssembly assembly;

		public VirtualMachineModel()
		{
			this.process = new Process()
			{
				Memory = new Memory(1024),
			};

			this.Source = "\tpush 12\n\tpush 30\n\tadd\n\tdrop";

			this.Recompile();

			this.Reset();


			this.StepCommand = new RelayCommand(this.Step);
			this.ResetCommand = new RelayCommand(this.Reset);
			this.RecompileCommand = new RelayCommand(this.Recompile);
		}

		public void Recompile()
		{
			this.assembly = Assembler.Assembler.Assemble(this.Source);

			this.process.Module = assembly.CreateModule();

			this.Reset();
		}

		public void Reset()
		{
			this.process.Reset();
			this.OnVmChanged();
		}

		public void Step()
		{
			this.process.Step();
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

		public int CurrentSourceLine => this.assembly.OriginalLine[this.CodePointer] - 1;

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
			.Range(0, (int)this.process.StackPointer)
			.Select(i => new StackItem(i, this.process))
			.ToArray();

		public string Source { get; set; }

		public ICommand StepCommand { get; private set; }

		public ICommand ResetCommand { get; private set; }

		public ICommand RecompileCommand { get; private set; }
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
}
