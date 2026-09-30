
func_80071C4C:
	.frame	$sp,120,$31		# vars= 72, regs= 7/0, args= 16, extra= 0
	.mask	0x803f0000,-8
	.fmask	0x00000000,0
	subu	$sp,$sp,120
	sw	$20,104($sp)
	move	$20,$4
	lh	$2,D_800A3558
	lw	$3,D_800A35B0
	lw	$4,D_800A35A8
	sw	$16,88($sp)
	sw	$31,112($sp)
	sw	$21,108($sp)
	sw	$19,100($sp)
	sw	$18,96($sp)
	sw	$17,92($sp)
	addu	$2,$2,1
	addu	$3,$3,$2
	lw	$21,88($4)
	.set	noreorder
	.set	nomacro
	blez	$3,.L382
	move	$16,$0
	.set	macro
	.set	reorder

	move	$18,$0
	move	$17,$0
	move	$19,$0
.L384:
	lbu	$3,D_800A3560($17)
	li	$2,0x00000005		# 5
	.set	noreorder
	.set	nomacro
	beq	$3,$2,.L383
	li	$2,0x00000010		# 16
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	beq	$3,$2,.L383
	li	$2,0x00000001		# 1
	.set	macro
	.set	reorder

	sw	$2,36($sp)
	li	$2,0x00000100		# 256
	li	$4,0x00000002		# 2
	sw	$0,32($sp)
	sw	$2,48($sp)
	sw	$2,52($sp)
	sb	$0,56($sp)
	lbu	$2,D_800A3560+2($17)
	lw	$3,D_800A35BC
	sll	$2,$2,2
	addu	$2,$2,$21
	lw	$6,0($2)
	.set	noreorder
	.set	nomacro
	bne	$3,$4,.L386
	li	$5,0x00000001		# 1
	.set	macro
	.set	reorder

	lw	$2,D_800A3568
	#nop
	lw	$2,20($2)
	li	$3,0x00020000		# 131072
	and	$2,$2,$3
	bne	$2,$0,.L386
	bne	$16,$5,.L386
	lbu	$2,D_800A3560+1
	#nop
	lbu	$2,D_8009BC7C($2)
	#nop
	andi	$2,$2,0x0002
	.set	noreorder
	.set	nomacro
	j	.L389
	sltu	$5,$0,$2
	.set	macro
	.set	reorder

.L386:
	lbu	$2,D_800A3560+1($17)
	#nop
	lbu	$2,D_8009BC7C($2)
	#nop
	andi	$2,$2,0x0002
	.set	noreorder
	.set	nomacro
	beq	$2,$0,.L415
	sll	$2,$5,1
	.set	macro
	.set	reorder

	move	$5,$0
.L389:
	sll	$2,$5,1
.L415:
	addu	$2,$2,$5
	sll	$2,$2,2
	addu	$3,$6,$2
	.set	noreorder
	.set	nomacro
	beq	$16,$0,.L391
	sw	$3,16($sp)
	.set	macro
	.set	reorder

	li	$2,0x00000040		# 64
	.set	noreorder
	.set	nomacro
	j	.L392
	sh	$2,8($3)
	.set	macro
	.set	reorder

.L391:
	sh	$0,8($3)
.L392:
	addu	$3,$6,24
	lw	$2,16($sp)
	la	$7,D_800A3590
	sw	$3,20($sp)
	lbu	$2,2($2)
	addu	$7,$19,$7
	addu	$2,$2,-1
	sll	$2,$2,3
	addu	$2,$2,$3
	lbu	$5,6($2)
	lhu	$4,0($2)
	lhu	$3,24($6)
	addu	$4,$4,$5
	subu	$4,$4,$3
	sll	$4,$4,16
	lw	$3,48($sp)
	sra	$4,$4,16
	mult	$4,$3
	lhu	$5,2($2)
	lbu	$3,7($2)
	lhu	$2,26($6)
	lh	$6,0($7)
	addu	$5,$5,$3
	subu	$5,$5,$2
	sll	$6,$6,2
	addu	$6,$6,$18
	sll	$5,$5,16
	sra	$5,$5,16
	lw	$2,52($sp)
	mflo	$4
	#nop
	#nop
	sra	$3,$4,8
	srl	$4,$4,31
	mult	$5,$2
	addu	$3,$3,$4
	sra	$3,$3,1
	lh	$2,D_8009BC94($6)
	addu	$3,$3,-320
	subu	$2,$2,$3
	sw	$2,40($sp)
	lh	$3,0($7)
	#nop
	sll	$3,$3,2
	addu	$3,$3,$18
	lh	$3,D_8009BC94+2($3)
	mflo	$5
	#nop
	#nop
	sra	$2,$5,8
	srl	$5,$5,31
	addu	$2,$2,$5
	sra	$2,$2,1
	addu	$2,$2,-157
	subu	$3,$3,$2
	sw	$3,44($sp)
	lw	$2,4($20)
	.set	noreorder
	.set	nomacro
	beq	$16,$0,.L393
	sw	$2,28($sp)
	.set	macro
	.set	reorder

	addu	$4,$sp,16
	li	$5,0x000001c0		# 448
	.set	noreorder
	.set	nomacro
	j	.L413
	li	$6,0x00000001		# 1
	.set	macro
	.set	reorder

.L393:
	addu	$4,$sp,16
	li	$5,0x00000e40		# 3648
	move	$6,$0
.L413:
	jal	func_80073C78
	sw	$2,4($20)
.L383:
	addu	$18,$18,24
	addu	$17,$17,3
	addu	$16,$16,1
	lh	$3,D_800A3558
	lw	$2,D_800A35B0
	addu	$3,$3,1
	addu	$2,$2,$3
	slt	$2,$16,$2
	.set	noreorder
	.set	nomacro
	bne	$2,$0,.L384
	addu	$19,$19,2
	.set	macro
	.set	reorder

.L382:
	lhu	$2,D_800A3550
	#nop
	addu	$2,$2,8
	sh	$2,D_800A3550
	sll	$2,$2,16
	sra	$2,$2,16
	slt	$2,$2,255
	.set	noreorder
	.set	nomacro
	bne	$2,$0,.L396
	li	$4,0x00000006		# 6
	.set	macro
	.set	reorder

	li	$5,0x0000007f		# 127
	li	$2,0x000000ff		# 255
	li	$16,0x00000001		# 1
	sh	$2,D_800A3550
	sh	$16,D_800A3578
	.set	noreorder
	.set	nomacro
	jal	func_8005C650
	li	$6,0x0000007f		# 127
	.set	macro
	.set	reorder

	lw	$3,D_800A35BC
	#nop
	sltu	$2,$3,2
	.set	noreorder
	.set	nomacro
	beq	$2,$0,.L397
	li	$2,0x00000004		# 4
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	jal	func_80071C20
	move	$16,$0
	.set	macro
	.set	reorder

	li	$3,0x00000001		# 1
	li	$4,-1009			# 0xfffffc0f
	lw	$5,D_800A3568
	andi	$2,$2,0x003f
	sw	$3,D_800A35A0
	lw	$3,20($5)
	sll	$2,$2,4
	and	$3,$3,$4
	or	$3,$3,$2
	.set	noreorder
	.set	nomacro
	j	.L414
	sw	$3,20($5)
	.set	macro
	.set	reorder

.L397:
	.set	noreorder
	.set	nomacro
	bne	$3,$2,.L399
	li	$2,0x00000006		# 6
	.set	macro
	.set	reorder

	li	$2,0x00000005		# 5
	sh	$2,D_800A3584
	.set	noreorder
	.set	nomacro
	j	.L414
	move	$16,$0
	.set	macro
	.set	reorder

.L399:
	.set	noreorder
	.set	nomacro
	bne	$3,$2,.L401
	li	$2,0x00000001		# 1
	.set	macro
	.set	reorder

	li	$2,0x00000006		# 6
	sh	$2,D_800A3584
	sh	$16,D_800A359C
	sh	$16,D_800A3598
	.set	noreorder
	.set	nomacro
	j	.L414
	move	$16,$0
	.set	macro
	.set	reorder

.L401:
	sw	$2,D_800A35A0
	move	$16,$0
.L414:
	lh	$2,D_800A3554
	lw	$3,D_800A35B0
	addu	$2,$2,1
	addu	$3,$3,$2
	blez	$3,.L404
	move	$5,$0
	move	$4,$0
.L406:
	lbu	$3,D_800A3560($5)
	addu	$5,$5,3
	lw	$2,D_800A3568
	addu	$16,$16,1
	addu	$2,$4,$2
	sb	$3,0($2)
	lh	$3,D_800A3554
	lw	$2,D_800A35B0
	addu	$3,$3,1
	addu	$2,$2,$3
	slt	$2,$16,$2
	.set	noreorder
	.set	nomacro
	bne	$2,$0,.L406
	addu	$4,$4,10
	.set	macro
	.set	reorder

.L404:
	lh	$2,D_800A3558
	lw	$3,D_800A35B0
	addu	$2,$2,1
	addu	$3,$3,$2
	.set	noreorder
	.set	nomacro
	blez	$3,.L396
	move	$16,$0
	.set	macro
	.set	reorder

	move	$5,$0
	move	$4,$0
.L411:
	lbu	$3,D_800A3560+2($5)
	addu	$5,$5,3
	lw	$2,D_800A3568
	addu	$16,$16,1
	addu	$2,$2,$4
	sb	$3,1($2)
	lh	$3,D_800A3558
	lw	$2,D_800A35B0
	addu	$3,$3,1
	addu	$2,$2,$3
	slt	$2,$16,$2
	.set	noreorder
	.set	nomacro
	bne	$2,$0,.L411
	addu	$4,$4,10
	.set	macro
	.set	reorder

.L396:
	.set	noreorder
	.set	nomacro
	jal	func_8006F038
	move	$4,$20
	.set	macro
	.set	reorder

	lw	$31,112($sp)
	lw	$21,108($sp)
	lw	$20,104($sp)
	lw	$19,100($sp)
	lw	$18,96($sp)
	lw	$17,92($sp)
	lw	$16,88($sp)
	addu	$sp,$sp,120
	j	$31
	