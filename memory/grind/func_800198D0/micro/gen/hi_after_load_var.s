	.file	1 "memory/grind/func_800198D0/micro/gen/hi_after_load_var.c"

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
	move	$9,$4
	lw	$12,16($sp)
	move	$10,$0
	li	$13,0x0000000c		# 12
	li	$11,0x00000020		# 32
	move	$8,$5
.L5:
	slt	$2,$6,12
	.set	noreorder
	.set	nomacro
	beq	$2,$0,.L6
	move	$2,$7
	.set	macro
	.set	reorder

	lw	$7,0($9)
	addu	$9,$9,4
	subu	$3,$13,$6
	subu	$4,$11,$6
	subu	$6,$11,$3
	srl	$2,$2,$4
	sll	$2,$2,$3
	srl	$5,$7,$6
	sll	$7,$7,$3
	or	$2,$2,$5
	.set	noreorder
	.set	nomacro
	j	.L4
	sh	$2,6($8)
	.set	macro
	.set	reorder

.L6:
	srl	$2,$7,20
	sh	$2,6($8)
	sll	$7,$7,12
	addu	$6,$6,-12
.L4:
	addu	$10,$10,1
	slt	$2,$10,63
	.set	noreorder
	.set	nomacro
	bne	$2,$0,.L5
	addu	$8,$8,2
	.set	macro
	.set	reorder

	sw	$9,0($12)
	sw	$6,4($12)
	.set	noreorder
	.set	nomacro
	j	$31
	sw	$7,8($12)
	.set	macro
	.set	reorder

	.end	f
.Lfe1:
	.size	 f,.Lfe1-f
	.ident	"GCC: (GNU) 2.7.2"
