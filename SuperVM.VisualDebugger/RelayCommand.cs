using System;
using System.Windows.Input;

namespace SuperVM.VisualDebugger
{
	internal class RelayCommand : ICommand
	{
		private Action action;

		public RelayCommand(Action action)
		{
			this.action = action;
		}

		public event EventHandler CanExecuteChanged;

		public bool CanExecute(object parameter) => true;

		public void Execute(object parameter)=> this.action();
	}
}