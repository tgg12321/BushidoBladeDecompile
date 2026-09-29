	.file	1 "memory/grind/func_800198D0/micro/gen2/left_end.c"

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
	lw	$11,16($sp)
	move	$9,$0
	li	$10,0x00000020		# 32
	li	$12,0x0000000c		# 12
.L5:
	slt	$2,$6,12
	.set	noreorder
	.set	nomacro
	beq	$2,$0,.L6
	subu	$3,$10,$6
	.set	macro
	.set	reorder

	srl	$3,$7,$3
	lw	$7,0($8)
	addu	$8,$8,4
	subu	$4,$12,$6
	subu	$2,$10,$4
	move	$6,$2
	sll	$3,$3,$4
	srl	$2,$7,$6
	sll	$7,$7,$4
	or	$3,$3,$2
	.set	noreorder
	.set	nomacro
	j	.L4
	sh	$3,6($5)
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

	sw	$8,0($11)
	sw	$6,4($11)
	.set	noreorder
	.set	nomacro
	j	$31
	sw	$7,8($11)
	.set	macro
	.set	reorder

	.end	f
.Lfe1:
	.size	 f,.Lfe1-f
	.ident	"GCC: (GNU) 2.7.2"
