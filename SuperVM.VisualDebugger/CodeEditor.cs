using ICSharpCode.AvalonEdit;
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;

namespace SuperVM.VisualDebugger
{
	public sealed class CodeEditor : TextEditor, INotifyPropertyChanged
	{
		public static readonly DependencyProperty CodeProperty =
			 DependencyProperty.Register(nameof(Code), typeof(string), typeof(CodeEditor),
			 new PropertyMetadata((obj, args) =>
			 {
				 var target = (CodeEditor)obj;
				 target.Dispatcher.BeginInvoke(new Action(() =>
				 {
					 target.preventUpdate = true;
					 var cursor = target.CaretOffset;
					 target.Text = (string)args.NewValue;
					 target.CaretOffset = cursor;
					 target.preventUpdate = false;
				 }));
			 })
			);

		private bool preventUpdate = false;

		public string Code
		{
			get { return (string)GetValue(CodeProperty); }
			set
			{
				SetValue(CodeProperty, value);
				base.Text = value;
			}
		}

		protected override void OnTextChanged(EventArgs e)
		{
			if(this.preventUpdate == false)
				SetValue(CodeProperty, this.Text);
			base.OnTextChanged(e);
		}

		public event PropertyChangedEventHandler PropertyChanged;
		public void RaisePropertyChanged(string info)
		{
			if (PropertyChanged != null)
				PropertyChanged(this, new PropertyChangedEventArgs(info));
		}
	}
}
