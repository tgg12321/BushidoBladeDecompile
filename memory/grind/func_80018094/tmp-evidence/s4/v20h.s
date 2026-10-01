func_80018094:
	.frame	$sp,48,$31		# vars= 16, regs= 3/0, args= 16, extra= 0
	.mask	0x80030000,-8
	.fmask	0x00000000,0
	subu	$sp,$sp,48
	sw	$16,32($sp)
	move	$16,$4
	sw	$17,36($sp)
	move	$17,$5
	sw	$31,40($sp)
	lw	$6,4($16)
 #APP
	move   $12, $6
lw     $13, 0($12)
lw     $14, 4($12)
ctc2   $13, $0
ctc2   $14, $1
lw     $13, 8($12)
lw     $14, 12($12)
lw     $15, 16($12)
ctc2   $13, $2
ctc2   $14, $3
ctc2   $15, $4

 #NO_APP
	#nop
	lw	$6,4($16)
 #APP
	move   $12, $6
lw     $13, 20($12)
lw     $14, 24($12)
ctc2   $13, $5
lw     $15, 28($12)
ctc2   $14, $6
ctc2   $15, $7

 #NO_APP
	jal	func_80017FA0
	lw	$2,4($16)
	lw	$3,40($17)
	lw	$2,20($2)
	#nop
	subu	$2,$2,$3
	mult	$2,$2
	sw	$2,528482340
	lw	$2,4($16)
	lw	$3,44($17)
	lw	$2,24($2)
	mflo	$5
	#nop
	subu	$2,$2,$3
	mult	$2,$2
	sw	$2,528482344
	lw	$2,4($16)
	lw	$3,48($17)
	lw	$2,28($2)
	mflo	$4
	#nop
	subu	$2,$2,$3
	mult	$2,$2
	li	$3,0x00030000		# 196608
	ori	$3,$3,0xd090
	sw	$2,528482348
	addu	$2,$5,$4
	mflo	$7
	#nop
	#nop
	addu	$4,$2,$7
	slt	$3,$3,$4
	.set	noreorder
	.set	nomacro
	bne	$3,$0,.L3
	li	$3,0x00000100		# 256
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	bgez	$4,.L4
	slt	$2,$4,1024
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	j	.L3
	move	$3,$0
	.set	macro
	.set	reorder

.L4:
	beq	$2,$0,.L6
	lbu	$2,D_8008D118($4)
	.set	noreorder
	.set	nomacro
	j	.L7
	srl	$3,$2,3
	.set	macro
	.set	reorder

.L6:
 #APP
	addu   $t4, $4, $zero
mtc2   $t4, $30
nop
nop
addiu  $v0, $sp, 0x10
addu   $t4, $v0, $zero
swc2   $31, 0($t4)

 #NO_APP
	lw	$3,16($sp)
	li	$2,-2			# 0xfffffffe
	and	$2,$3,$2
	li	$3,0x00000016		# 22
	subu	$3,$3,$2
	sra	$2,$4,$3
	sra	$3,$3,1
	lbu	$4,D_8008D118($2)
	li	$2,0x00000013		# 19
	subu	$2,$2,$3
	sll	$4,$4,16
	sra	$3,$4,$2
.L7:
	li	$2,0x10620000		# 274857984
	ori	$2,$2,0x4dd3
	sll	$3,$3,6
	mult	$3,$2
	sra	$3,$3,31
	mfhi	$6
	#nop
	#nop
	sra	$2,$6,5
	subu	$2,$2,$3
	addu	$3,$2,192
.L3:
	lw	$2,528482340
	#nop
	mult	$2,$3
	mflo	$7
	#nop
	lw	$2,528482344
	#nop
	mult	$2,$3
	mflo	$4
	#nop
	lw	$2,528482348
	#nop
	mult	$2,$3
	sra	$2,$7,1
	sw	$2,528482340
	sra	$2,$4,1
	sw	$2,528482344
	mflo	$3
	#nop
	#nop
	sra	$2,$3,1
	sw	$2,528482348
	lw	$2,4($16)
	move	$4,$17
	lw	$3,0($2)
	lw	$5,4($2)
	lw	$6,8($2)
	lw	$7,12($2)
	sw	$3,20($4)
	sw	$5,24($4)
	sw	$6,28($4)
	sw	$7,32($4)
	lw	$3,16($2)
	lw	$5,20($2)
	lw	$6,24($2)
	lw	$7,28($2)
	sw	$3,36($4)
	sw	$5,40($4)
	sw	$6,44($4)
	sw	$7,48($4)
	jal	func_80018300
	lw	$31,40($sp)
	lw	$17,36($sp)
	lw	$16,32($sp)
	addu	$sp,$sp,48
	j	$31
