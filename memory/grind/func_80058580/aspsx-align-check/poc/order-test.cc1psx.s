	.file	1 "T.C"

 # GNU C 2.6.0 [AL 1.1, MM 40] Sony Playstation compiled by GNU C

 # Cc1 defaults:
 # -mgas -msoft-float

 # Cc1 arguments (-G value = 0, Cpu = 3000, ISA = 1):
 # -O2 -G0 -quiet -o

gcc2_compiled.:
__gnu_compiled_c:
	.rdata
	.align	2
$LC0:
	.ascii	"one\000"
	.text
	.align	2
	.globl	f1

	.loc	1 2
LM1:
	.ent	f1
f1:
	.frame	$sp,24,$31		# vars= 0, regs= 1/0, args= 16, extra= 0
	.mask	0x80000000,-8
	.fmask	0x00000000,0
	subu	$sp,$sp,24
	sltu	$2,$4,6
	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L2
	sw	$31,16($sp)
	.set	macro
	.set	reorder

	sll	$2,$4,2
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
	la	$4,$LC0
	jal	g
	j	$L11
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
	li	$2,0x0000000b		# 11
	.set	macro
	.set	reorder

$L7:
	.set	noreorder
	.set	nomacro
	j	$L11
	li	$2,0x00000002		# 2
	.set	macro
	.set	reorder

$L8:
	.set	noreorder
	.set	nomacro
	j	$L11
	li	$2,0x00000003		# 3
	.set	macro
	.set	reorder

$L2:
	move	$2,$0
$L11:
	lw	$31,16($sp)
	addu	$sp,$sp,24
	j	$31
	.end	f1
	.rdata
	.align	2
$LC1:
	.ascii	"hello\000"
	.text
	.align	2
	.globl	f2

	.loc	1 3
LM2:
	.ent	f2
f2:
	.frame	$sp,24,$31		# vars= 0, regs= 1/0, args= 16, extra= 0
	.mask	0x80000000,-8
	.fmask	0x00000000,0
	subu	$sp,$sp,24
	sw	$31,16($sp)
	la	$4,$LC1
	jal	g
	lw	$31,16($sp)
	addu	$sp,$sp,24
	j	$31
	.end	f2
	.rdata
	.align	2
$LC2:
	.ascii	"three\000"
	.text
	.align	2
	.globl	f3

	.loc	1 4
LM3:
	.ent	f3
f3:
	.frame	$sp,24,$31		# vars= 0, regs= 1/0, args= 16, extra= 0
	.mask	0x80000000,-8
	.fmask	0x00000000,0
	subu	$sp,$sp,24
	sltu	$2,$4,6
	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L14
	sw	$31,16($sp)
	.set	macro
	.set	reorder

	sll	$2,$4,2
	lw	$2,$L21($2)
	#nop
	j	$2
	.rdata
	.align	3
$L21:
	.word	$L15
	.word	$L16
	.word	$L17
	.word	$L18
	.word	$L19
	.word	$L20
	.text
$L15:
	.set	noreorder
	.set	nomacro
	j	$L23
	li	$2,0x00000005		# 5
	.set	macro
	.set	reorder

$L16:
	.set	noreorder
	.set	nomacro
	j	$L23
	li	$2,0x00000047		# 71
	.set	macro
	.set	reorder

$L17:
	.set	noreorder
	.set	nomacro
	j	$L23
	li	$2,0x00000013		# 19
	.set	macro
	.set	reorder

$L18:
	.set	noreorder
	.set	nomacro
	j	$L23
	li	$2,0x00000001		# 1
	.set	macro
	.set	reorder

$L19:
	.set	noreorder
	.set	nomacro
	j	$L23
	li	$2,0x00000016		# 22
	.set	macro
	.set	reorder

$L20:
	.set	noreorder
	.set	nomacro
	j	$L23
	li	$2,0x00000021		# 33
	.set	macro
	.set	reorder

$L14:
	la	$4,$LC2
	jal	g
$L23:
	lw	$31,16($sp)
	addu	$sp,$sp,24
	j	$31
	.end	f3
