	.file	1 "T.C"




gcc2_compiled.:
__gnu_compiled_c:
	.sdata
	.align	2
si:
	.word	5
	.align	2
sc:
	.byte	1
	.byte	2
	.byte	3
	.align	2
sd:
	.byte	4
	.byte	5
	.byte	6
	.align	2
sz:
	.word	0
	.globl	gi
	.align	2
gi:
	.word	7
	.data
	.align	2
big:
	.word	1
	.word	2
	.word	3
	.word	4
	.sdata
	.align	2
s8:
	.ascii	"abcdefg\000"
	.align	2
s4:
	.ascii	"abc\000"
	.align	2
q2:
	.ascii	"\001\000"
	.align	1
hs:
	.half	3
	.align	2
$LC0:
	.ascii	"xy\000"
	.align	2
sp8:
	.word	$LC0
	.text
	.align	2
	.globl	f

	.text
	.text
	.ent	f
f:
	.frame	$sp,0,$31		# vars= 0, regs= 0/0, args= 0, extra= 0
	.mask	0x00000000,0
	.fmask	0x00000000,0
	lbu	$4,sc+1
	lw	$2,si
	lbu	$3,sd+1
	addu	$2,$2,$4
	addu	$2,$2,$3
	lw	$3,sz
	lw	$4,gi
	lw	$5,big+8
	lbu	$6,s8+3
	addu	$2,$2,$3
	addu	$2,$2,$4
	addu	$2,$2,$5
	addu	$2,$2,$6
	lw	$3,sp8
	lbu	$4,s4+1
	lbu	$5,q2
	lh	$6,hs
	lbu	$3,0($3)
	addu	$2,$2,$4
	addu	$2,$2,$5
	addu	$2,$2,$6
	.set	noreorder
	.set	nomacro
	j	$31
	addu	$2,$2,$3
	.set	macro
	.set	reorder

	.end	f
