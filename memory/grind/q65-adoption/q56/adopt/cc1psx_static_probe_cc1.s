	.file	1 "stdin"




	.version	"01.01"
gcc2_compiled.:
	.data
	.align	2
	.type	 si,@object
	.size	 si,4
si:
	.word	5
	.align	2
	.type	 sc,@object
	.size	 sc,3
sc:
	.byte	1
	.byte	2
	.byte	3
	.align	2
	.type	 sd,@object
	.size	 sd,3
sd:
	.byte	4
	.byte	5
	.byte	6
	.align	2
	.type	 sz,@object
	.size	 sz,4
sz:
	.word	0
	.globl	gi
	.align	2
	.type	 gi,@object
	.size	 gi,4
gi:
	.word	7
	.align	2
	.type	 big,@object
	.size	 big,16
big:
	.word	1
	.word	2
	.word	3
	.word	4
	.align	2
	.type	 s8,@object
	.size	 s8,8
s8:
	.string	"abcdefg"
	.align	2
	.type	 s4,@object
	.size	 s4,4
s4:
	.string	"abc"
	.align	2
	.type	 q2,@object
	.size	 q2,2
q2:
	.string	"\001"
	.align	1
	.type	 hs,@object
	.size	 hs,2
hs:
	.half	3
.section	.rodata
	.align	2
.LC0:
	.string	"xy"
	.data
	.align	2
	.type	 sp8,@object
	.size	 sp8,4
sp8:
	.word	.LC0
	.text
	.align	2
	.globl	f
	.type	 f,@function
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
	addu	$2,$2,$3
	addu	$2,$2,$4
	lw	$3,big+8
	lbu	$4,s8+3
	addu	$2,$2,$3
	addu	$2,$2,$4
	lbu	$3,s4+1
	lbu	$4,q2
	addu	$2,$2,$3
	addu	$2,$2,$4
	lw	$3,sp8
	lh	$4,hs
	lbu	$3,0($3)
	addu	$2,$2,$4
	.set	noreorder
	.set	nomacro
	j	$31
	addu	$2,$2,$3
	.set	macro
	.set	reorder

	.end	f
.Lfe1:
	.size	 f,.Lfe1-f
	.ident	"GCC: (GNU) 2.7.2"
