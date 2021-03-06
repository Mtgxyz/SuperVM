@echo off
SET PATHSAVE=%PATH%
call "C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\vcvarsall.bat" amd64

SET INIT = /nologo
SET OPTIONS = /Zp1 /Ox

echo Building explink...
cl %INIT% explink.c getopt.c %OPTIONS%

echo Building expdump...
cl %INIT% expdump.c getopt.c mnemonics.c disassembler.c %OPTIONS%

echo Building emulator...
SET LIBS= /NXCOMPAT /DYNAMICBASE "SDL.lib" "kernel32.lib" "user32.lib" "gdi32.lib" "winspool.lib" "comdlg32.lib" "advapi32.lib" "shell32.lib" "ole32.lib" "oleaut32.lib" "uuid.lib" "odbc32.lib" "odbccp32.lib" /DEBUG /MACHINE:X64 /OPT:REF /INCREMENTAL:NO /SUBSYSTEM:CONSOLE /MANIFESTUAC:"level='asInvoker' uiAccess='false'" /OPT:ICF /ERRORREPORT:PROMPT /NOLOGO /TLBID:1 /NODEFAULTLIB:msvcrt.lib
cl %INIT% /I "SDL-1.2.15\include" emulator.c vm.c getopt.c %OPTIONS% /link %LIBS%

echo Building assembler...
cl %INIT% /DYY_NO_UNISTD_H as\as.c as\tokens.c mnemonics.c disassembler.c getopt.c %OPTIONS%

echo Cleaning up...
del *.obj
SET PATH=%PATHSAVE%
echo Done.