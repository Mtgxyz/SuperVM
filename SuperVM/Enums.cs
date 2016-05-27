using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SuperVM
{
	public enum MathCommand
	{
		Add = 0,
		Subtract = 1,
		Multiplicate = 2,
		Divide = 3,
		Modulo = 4,
		And = 5,
		Or = 6,
		Xor = 7,
		Not = 8,
		RotShiftLeft = 9,
		RotShiftRight = 10,
		ArithmeticShiftLeft = 11,
		ArithmeticShiftRight = 12,
		LogicShiftLeft = 13,
		LogicShiftRight = 14,
	}

	public enum Command
	{
		Copy = 0,
		Store = 1,
		Load = 2,
		Get = 3,
		Set = 4,
		BpGet = 5,
		BpSet = 6,
		CpGet = 7,
		Math = 8,
		SpGet = 9,
		SpSet = 10,
		SysCall = 11,
		HwIO = 12,
	}

	public enum OutputType
	{
		Discard = 0,
		Push = 1,
		Jump = 2,
		JumpRelative = 3,
	}

	public enum ExecutionMode
	{
		Always = 0,
		Zero = 2,
		One = 3,
	}

	public enum InputType
	{
		Zero = 0,
		Pop = 1,
		Peek = 2,
		Argument = 3,
	}
}
