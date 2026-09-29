	.file	1 "memory/grind/func_800198D0/micro/gen/u8_need.c"

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
	lw	$8,16($sp)
	move	$3,$0
.L5:
	slt	$2,$6,12
	.set	noreorder
	.set	nomacro
	beq	$2,$0,.L6
	srl	$2,$7,20
	.set	macro
	.set	reorder

	lw	$7,0($4)
	.set	noreorder
	.set	nomacro
	j	.L4
	addu	$4,$4,4
	.set	macro
	.set	reorder

.L6:
	sh	$2,6($5)
	sll	$7,$7,12
	addu	$6,$6,-12
.L4:
	addu	$3,$3,1
	slt	$2,$3,63
	.set	noreorder
	.set	nomacro
	bne	$2,$0,.L5
	addu	$5,$5,2
	.set	macro
	.set	reorder

	sw	$4,0($8)
	sw	$6,4($8)
	.set	noreorder
	.set	nomacro
	j	$31
	sw	$7,8($8)
	.set	macro
	.set	reorder

	.end	f
.Lfe1:
	.size	 f,.Lfe1-f
	.ident	"GCC: (GNU) 2.7.2"
