	.file	1 "memory/grind/func_800198D0/micro/gen/bits_minus_12_plus.c"

 # GNU C 2.7.2 [AL 1.1, MM 40] GNU MIPS/ELF compiled by GNU C

 # Cc1 defaults:
 # -mgas

 # Cc1 arguments (-G value = 0, Cpu = 3000, ISA = 1):
 # -O2 -G0 -funsigned-char -quiet -mcpu=3000 -mips1 -mno-abicalls -fno-builtin
 # -w -mel -msoft-float -o

	.version	"01.01"
gcc2_compiled.:
	.text
	.align	2
	.globl	f
	.type	 f,@function
	.ent	f
f:
	.frame	$sp,0,$31		# vars= 0, regs= 0/0, args= 0, extra= 0
	.mask	0x00000000,0
	.fmask	0x00000000,0
	move	$8,$4
	lw	$10,16($sp)
	move	$9,$0
	li	$12,0x00000020		# 32
	li	$11,0x0000000c		# 12
.L5:
	slt	$2,$6,12
	.set	noreorder
	.set	nomacro
	beq	$2,$0,.L6
	subu	$2,$12,$6
	.set	macro
	.set	reorder

	srl	$2,$7,$2
	lw	$7,0($8)
	addu	$8,$8,4
	subu	$3,$11,$6
	addu	$6,$6,20
	sll	$2,$2,$3
	srl	$4,$7,$6
	sll	$7,$7,$3
	or	$2,$2,$4
	.set	noreorder
	.set	nomacro
	j	.L4
	sh	$2,6($5)
	.set	macro
	.set	reorder

.L6:
	srl	$2,$7,20
	sh	$2,6($5)
	sll	$7,$7,12
	addu	$6,$6,-12
.L4:
	addu	$9,$9,1
	slt	$2,$9,63
	.set	noreorder
	.set	nomacro
	bne	$2,$0,.L5
	addu	$5,$5,2
	.set	macro
	.set	reorder

	sw	$8,0($10)
	sw	$6,4($10)
	.set	noreorder
	.set	nomacro
	j	$31
	sw	$7,8($10)
	.set	macro
	.set	reorder

	.end	f
.Lfe1:
	.size	 f,.Lfe1-f
	.ident	"GCC: (GNU) 2.7.2"
