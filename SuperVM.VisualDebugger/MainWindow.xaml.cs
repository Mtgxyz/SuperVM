using ICSharpCode.AvalonEdit.Rendering;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;

namespace SuperVM.VisualDebugger
{
	/// <summary>
	/// Interaktionslogik für MainWindow.xaml
	/// </summary>
	public partial class MainWindow : Window
	{
		public static DependencyProperty CurrentLineProperty = DependencyProperty.Register(
			nameof(CurrentLine),
			typeof(int),
			typeof(MainWindow),
			new PropertyMetadata(-1, CurrentLineChanged));
		private readonly LineBackgroundRenderer renderer;

		private static void CurrentLineChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
		{
			var window = (MainWindow)d;
			window.codeEditor.TextArea.TextView.InvalidateVisual();
		}

		public int CurrentLine
		{
			get { return (int)GetValue(CurrentLineProperty); }
			set { SetValue(CurrentLineProperty, value); }
		}

		public MainWindow()
		{
			InitializeComponent();
			
			BindingOperations.SetBinding(this, CurrentLineProperty, new Binding("CurrentSourceLine"));

			this.codeEditor.TextArea.TextView.BackgroundRenderers.Add(this.renderer = new LineBackgroundRenderer(this));
		}

		private void MainWindow_DataContextChanged(object sender, DependencyPropertyChangedEventArgs e)
		{

		}

		public class LineBackgroundRenderer : IBackgroundRenderer
		{
			static Pen pen;
			static SolidColorBrush background;

			private readonly MainWindow window;

			static LineBackgroundRenderer()
			{
				background = new SolidColorBrush(Color.FromRgb(0xFF, 0xFF, 0x00)); background.Freeze();

				var blackBrush = new SolidColorBrush(Color.FromRgb(0, 0, 0)); blackBrush.Freeze();
				pen = new Pen(blackBrush, 0.0);
			}

			public LineBackgroundRenderer(MainWindow window)
			{
				this.window = window;
			}

			public KnownLayer Layer
			{
				get { return KnownLayer.Background; }
			}

			public void Draw(TextView textView, DrawingContext drawingContext)
			{
				var currentLine = (this.window.DataContext as VirtualMachineModel)?.CurrentSourceLine ?? -1;
				if (currentLine < 0)
					return;

				foreach (var v in textView.VisualLines)
				{
					var rc = BackgroundGeometryBuilder.GetRectsFromVisualSegment(textView, v, 0, 1000).First();
					// NB: This lookup to fetch the doc line number isn't great, we could
					// probably do it once then just increment.
					var linenum = v.FirstDocumentLine.LineNumber - 1;

					if (linenum != currentLine)
						continue;

					var brush = background;

					drawingContext.DrawRectangle(
						brush, pen,
						new Rect(0, rc.Top, textView.ActualWidth, rc.Height));
				}
			}
		}
	}
}
