	.set noreorder
	.text
	.globl f
f:
	lui $1,%hi(foo+1)
	sb $5,%lo(foo+1)($1)
	jr $31
	nop
	.comm foo,4
