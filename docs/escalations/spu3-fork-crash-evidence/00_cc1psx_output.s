	.file	1 "T.C"

 # GNU C 2.7.2.SN.1 [AL 1.1, MM 40] Sony Playstation compiled by GNU C

 # Cc1 defaults:
 # -mgas -msoft-float

 # Cc1 arguments (-G value = 0, Cpu = 3000, ISA = 1):
 # -quiet -O2 -G0 -funsigned-char -mcpu=3000 -mips1 -w -o

gcc2_compiled.:
__gnu_compiled_c:
	.text
	.align	2
	.ent	_spu_FiDMA
_spu_FiDMA:
	.frame	$sp,24,$31		# vars= 0, regs= 1/0, args= 16, extra= 0
	.mask	0x80000000,-8
	.fmask	0x00000000,0
	lw	$2,D_800A2D2C
	subu	$sp,$sp,24
	.set	noreorder
	.set	nomacro
	bne	$2,$0,$L2
	sw	$31,16($sp)
	.set	macro
	.set	reorder

	jal	spu_WriteReg16
$L2:
	lw	$4,D_800A2CDC
	#nop
	#.set	volatile
	lhu	$2,426($4)
	#.set	novolatile
	#nop
	andi	$2,$2,0xffcf
	#.set	volatile
	sh	$2,426($4)
	#.set	novolatile
	#.set	volatile
	lhu	$2,426($4)
	#.set	novolatile
	#nop
	andi	$2,$2,0x0030
	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L4
	move	$3,$0
	.set	macro
	.set	reorder

	addu	$3,$3,1
$L10:
	sltu	$2,$3,3841
	beq	$2,$0,$L4
	#.set	volatile
	lhu	$2,426($4)
	#.set	novolatile
	#nop
	andi	$2,$2,0x0030
	.set	noreorder
	.set	nomacro
	bne	$2,$0,$L10
	addu	$3,$3,1
	.set	macro
	.set	reorder

	addu	$3,$3,-1
$L4:
	#.set	volatile
	lw	$2,D_800A2D14
	#.set	novolatile
	#nop
	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L8
	li	$4,-268435456			# 0xf0000000
	.set	macro
	.set	reorder

	#.set	volatile
	lw	$2,D_800A2D14
	#.set	novolatile
	#nop
	jal	$31,$2
	j	$L9
$L8:
	ori	$4,$4,0x0009
	.set	noreorder
	.set	nomacro
	jal	bios_DeliverEvent
	li	$5,0x00000020		# 32
	.set	macro
	.set	reorder

$L9:
	lw	$31,16($sp)
	addu	$sp,$sp,24
	j	$31
	.end	_spu_FiDMA
	.align	2
	.ent	_spu_Fr_
_spu_Fr_:
	.frame	$sp,32,$31		# vars= 0, regs= 3/0, args= 16, extra= 0
	.mask	0x80030000,-8
	.fmask	0x00000000,0
	lw	$2,D_800A2CDC
	subu	$sp,$sp,32
	sw	$17,20($sp)
	move	$17,$4
	sw	$16,16($sp)
	sw	$31,24($sp)
	#.set	volatile
	sh	$5,422($2)
	#.set	novolatile
	.set	noreorder
	.set	nomacro
	jal	spu_WriteReg16
	move	$16,$6
	.set	macro
	.set	reorder

	lw	$3,D_800A2CDC
	#nop
	#.set	volatile
	lhu	$2,426($3)
	#.set	novolatile
	#nop
	ori	$2,$2,0x0030
	#.set	volatile
	sh	$2,426($3)
	#.set	novolatile
	.set	noreorder
	.set	nomacro
	jal	spu_WriteReg16
	sll	$16,$16,16
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	jal	spu_ReadReg
	ori	$16,$16,0x0010
	.set	macro
	.set	reorder

	lw	$2,D_800A2CE0
	#nop
	sw	$17,0($2)
	lw	$2,D_800A2CE4
	#nop
	sw	$16,0($2)
	lw	$4,D_800A2CE8
	li	$3,0x01000000		# 16777216
	ori	$3,$3,0x0200
	li	$2,0x00000001		# 1
	sw	$2,D_800A2D2C
	sw	$3,0($4)
	lw	$31,24($sp)
	lw	$17,20($sp)
	lw	$16,16($sp)
	addu	$sp,$sp,32
	j	$31
	.end	_spu_Fr_
	.align	2
	.globl	anchor
	.ent	anchor
anchor:
	.frame	$sp,24,$31		# vars= 0, regs= 1/0, args= 16, extra= 0
	.mask	0x80000000,-8
	.fmask	0x00000000,0
	subu	$sp,$sp,24
	sw	$31,16($sp)
	jal	_spu_FiDMA
	move	$4,$0
	move	$5,$0
	.set	noreorder
	.set	nomacro
	jal	_spu_Fr_
	move	$6,$0
	.set	macro
	.set	reorder

	lw	$31,16($sp)
	addu	$sp,$sp,24
	j	$31
	.end	anchor
