	.file	1 "T.I"

 # GNU C 2.6.0 [AL 1.1, MM 40] Sony Playstation compiled by GNU C

 # Cc1 defaults:
 # -mgas -msoft-float

 # Cc1 arguments (-G value = 0, Cpu = 3000, ISA = 1):
 # -O2 -G0 -quiet -o

gcc2_compiled.:
__gnu_compiled_c:
	.text
	.align	2
	.globl	f

	.loc	1 1
LM1:
	.ent	f
f:
	.frame	$sp,0,$31		# vars= 0, regs= 0/0, args= 0, extra= 0
	.mask	0x00000000,0
	.fmask	0x00000000,0
	sltu	$2,$4,6
	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L2
	sll	$2,$4,2
	.set	macro
	.set	reorder

	lw	$2,$L9($2)
	#nop
	j	$2
	.rdata
	.align	3
$L9:
	.word	$L3
	.word	$L4
	.word	$L5
	.word	$L6
	.word	$L7
	.word	$L8
	.text
$L3:
	.set	noreorder
	.set	nomacro
	j	$L11
	li	$2,0x00000003		# 3
	.set	macro
	.set	reorder

$L4:
	.set	noreorder
	.set	nomacro
	j	$L11
	li	$2,0x00000007		# 7
	.set	macro
	.set	reorder

$L5:
	.set	noreorder
	.set	nomacro
	j	$L11
	li	$2,0x00000009		# 9
	.set	macro
	.set	reorder

$L6:
	.set	noreorder
	.set	nomacro
	j	$L11
	li	$2,0x00000001		# 1
	.set	macro
	.set	reorder

$L7:
	.set	noreorder
	.set	nomacro
	j	$L11
	li	$2,0x0000000c		# 12
	.set	macro
	.set	reorder

$L8:
	.set	noreorder
	.set	nomacro
	j	$L11
	li	$2,0x0000002c		# 44
	.set	macro
	.set	reorder

$L2:
	move	$2,$0
$L11:
	j	$31
	.end	f
