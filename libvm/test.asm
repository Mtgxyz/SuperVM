;
; Simple memory transform-and-copy program
;
_start:
	push 0xFFFF00
	cpget
	jmp @cls
	push 0x00FFFF
	cpget
	jmp @cls
	jmp @_start
	syscall

cls:
	bpget
	spget
	bpset
	push 0
cls_copy:
	dup
	cmp 1228800 ; 640*480*4 screen size
	[ex(z)=1] jmp @cls_stop
	
	get -2 ; Get color argument
	get 1  ; Get iterator
	[i0:arg] add 4096 ; Add screen buffer offset
	
	storei [ci:2] ; Write 32bit value
	
	; Increase counter
	[i0:arg] add 4
	jmp @cls_copy

cls_stop:
	bpget
	spset
	bpset
	ret