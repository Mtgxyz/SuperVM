


main:
	push 0 ; ptr
loop:
	[i0:peek] load [f:yes]
	[ex(z)=1] jmp @end
	[i0:pop] syscall [ci:1]
	[i0:arg] [i1:pop] add 1
	jmp @loop
end:
	syscall [ci:0] ; quit