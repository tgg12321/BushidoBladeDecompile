func_8002C61C:
	.frame	$sp,32,$31		# vars= 0, regs= 3/0, args= 16, extra= 0
	.mask	0x80030000,-8
	.fmask	0x00000000,0
	subu	$sp,$sp,32
	sw	$17,20($sp)
	la	$17,g_practice_menu_table
	sw	$16,16($sp)
	addu	$16,$17,1100
	lhu	$3,g_practice_menu_table+106
	li	$2,0x0000000f		# 15
	andi	$4,$3,0xffff
	.set	noreorder
	.set	nomacro
	beq	$4,$2,.L595
	sw	$31,24($sp)
	.set	macro
	.set	reorder

	addu	$2,$3,-28
	sltu	$2,$2,2
	.set	noreorder
	.set	nomacro
	bne	$2,$0,.L595
	addu	$2,$3,-30
	.set	macro
	.set	reorder

	sltu	$2,$2,2
	.set	noreorder
	.set	nomacro
	bne	$2,$0,.L595
	addu	$2,$3,-32
	.set	macro
	.set	reorder

	sltu	$2,$2,2
	.set	noreorder
	.set	nomacro
	beq	$2,$0,.L594
	li	$2,0x00000011		# 17
	.set	macro
	.set	reorder

.L595:
	jal	func_80026DA4
	j	.L596
.L594:
	bne	$4,$2,.L597
	jal	func_8002C0DC
	j	.L596
.L597:
	jal	func_8002872C
	jal	func_800288C8
	jal	func_80029454
	sh	$2,D_800A3824
	sll	$2,$2,16
	bltz	$2,.L600
	jal	func_8002C22C
	lh	$2,D_800A3824
	#nop
	bltz	$2,.L600
	lbu	$2,g_practice_menu_table+173
	#nop
	.set	noreorder
	.set	nomacro
	bne	$2,$0,.L629
	move	$4,$17
	.set	macro
	.set	reorder

	lbu	$2,g_practice_menu_table+1273
	#nop
	beq	$2,$0,.L600
.L629:
	li	$5,0x1f800000		# 528482304
	.set	noreorder
	.set	nomacro
	jal	func_800283D0
	ori	$5,$5,0x03f4
	.set	macro
	.set	reorder

	move	$4,$16
	li	$5,0x1f800000		# 528482304
	.set	noreorder
	.set	nomacro
	jal	func_800283D0
	ori	$5,$5,0x03f4
	.set	macro
	.set	reorder

	sb	$0,g_practice_menu_table+1273
	sb	$0,g_practice_menu_table+173
	j	.L604
.L600:
	.set	noreorder
	.set	nomacro
	jal	func_8002AB08
	move	$4,$0
	.set	macro
	.set	reorder

.L604:
	lw	$2,60($17)
	#nop
	slt	$2,$2,3
	bne	$2,$0,.L596
	lw	$2,60($16)
	#nop
	slt	$2,$2,3
	bne	$2,$0,.L596
	lh	$2,D_800A38A8
	#nop
	.set	noreorder
	.set	nomacro
	beq	$2,$0,.L596
	li	$2,-1			# 0xffffffff
	.set	macro
	.set	reorder

	lh	$3,646($17)
	#nop
	bne	$3,$2,.L596
	lh	$2,646($16)
	#nop
	.set	noreorder
	.set	nomacro
	bne	$2,$3,.L596
	li	$3,0x0000001f		# 31
	.set	macro
	.set	reorder

	lh	$2,12($17)
	#nop
	beq	$2,$3,.L596
	lh	$2,12($16)
	#nop
	beq	$2,$3,.L596
	lw	$3,248($17)
	lw	$2,248($16)
	#nop
	subu	$2,$3,$2
	bgez	$2,.L606
	subu	$2,$0,$2
.L606:
	slt	$2,$2,1000
	.set	noreorder
	.set	nomacro
	beq	$2,$0,.L596
	li	$2,0x0000000a		# 10
	.set	macro
	.set	reorder

	sh	$2,646($17)
	sh	$2,646($16)
	sw	$0,652($16)
	sw	$0,652($17)
	sh	$0,D_800A3910
	sh	$0,D_800A389C
.L596:
	lhu	$2,g_practice_menu_table+106
	li	$3,0x00000005		# 5
	.set	noreorder
	.set	nomacro
	bne	$2,$3,.L608
	li	$2,0x00000001		# 1
	.set	macro
	.set	reorder

	sb	$2,D_800A3748
	.set	noreorder
	.set	nomacro
	j	.L628
	li	$2,0x0000001c		# 28
	.set	macro
	.set	reorder

.L608:
	lhu	$2,g_practice_menu_table+1206
	#nop
	.set	noreorder
	.set	nomacro
	bne	$2,$3,.L630
	move	$9,$0
	.set	macro
	.set	reorder

	li	$2,0x0000001c		# 28
	sb	$0,D_800A3748
.L628:
	sh	$2,D_800A3834
	move	$9,$0
.L630:
	la	$2,g_practice_menu_table+528
	addu	$8,$2,1100
	move	$7,$2
	li	$6,0x1f800000		# 528482304
.L614:
	lw	$2,0($6)
	lw	$3,4($6)
	lw	$4,8($6)
	sw	$2,0($7)
	sw	$3,4($7)
	sw	$4,8($7)
	lw	$2,36($6)
	lw	$3,40($6)
	lw	$4,44($6)
	sw	$2,0($8)
	sw	$3,4($8)
	sw	$4,8($8)
	addu	$8,$8,12
	addu	$7,$7,12
	addu	$9,$9,1
	slt	$2,$9,3
	.set	noreorder
	.set	nomacro
	bne	$2,$0,.L614
	addu	$6,$6,12
	.set	macro
	.set	reorder

	move	$9,$0
	la	$2,g_practice_menu_table+564
	addu	$8,$2,1100
	move	$7,$2
	li	$6,0x1f800000		# 528482304
.L619:
	lw	$2,72($6)
	lw	$3,76($6)
	lw	$4,80($6)
	sw	$2,0($7)
	sw	$3,4($7)
	sw	$4,8($7)
	lw	$2,96($6)
	lw	$3,100($6)
	lw	$4,104($6)
	sw	$2,0($8)
	sw	$3,4($8)
	sw	$4,8($8)
	addu	$8,$8,12
	addu	$7,$7,12
	addu	$9,$9,1
	slt	$2,$9,2
	.set	noreorder
	.set	nomacro
	bne	$2,$0,.L619
	addu	$6,$6,12
	.set	macro
	.set	reorder

	move	$9,$0
	li	$7,0x55550000		# 1431633920
	ori	$7,$7,0x5556
	li	$5,0x1f800000		# 528482304
	ori	$5,$5,0x00ec
	move	$6,$0
.L624:
	lw	$2,-56($5)
	lw	$3,-44($5)
	lw	$4,-32($5)
	addu	$2,$2,$3
	addu	$2,$2,$4
	mult	$2,$7
	sra	$2,$2,31
	mfhi	$10
	#nop
	#nop
	subu	$2,$10,$2
	sw	$2,g_practice_menu_table+396($6)
	lw	$2,-52($5)
	lw	$3,-40($5)
	lw	$4,-28($5)
	addu	$2,$2,$3
	addu	$2,$2,$4
	mult	$2,$7
	sra	$2,$2,31
	mfhi	$10
	#nop
	#nop
	subu	$2,$10,$2
	sw	$2,g_practice_menu_table+400($6)
	lw	$2,-48($5)
	lw	$3,-36($5)
	lw	$4,-24($5)
	addu	$2,$2,$3
	addu	$2,$2,$4
	mult	$2,$7
	sra	$2,$2,31
	mfhi	$10
	#nop
	#nop
	subu	$2,$10,$2
	sw	$2,g_practice_menu_table+404($6)
	lw	$2,-20($5)
	lw	$3,-8($5)
	#nop
	addu	$2,$2,$3
	srl	$3,$2,31
	addu	$2,$2,$3
	sra	$2,$2,1
	sw	$2,g_practice_menu_table+372($6)
	lw	$2,-16($5)
	lw	$3,-4($5)
	addu	$9,$9,1
	addu	$2,$2,$3
	srl	$3,$2,31
	addu	$2,$2,$3
	sra	$2,$2,1
	sw	$2,g_practice_menu_table+376($6)
	lw	$2,-12($5)
	lw	$3,0($5)
	addu	$5,$5,264
	addu	$2,$2,$3
	srl	$3,$2,31
	addu	$2,$2,$3
	sra	$2,$2,1
	sw	$2,g_practice_menu_table+380($6)
	slt	$2,$9,2
	.set	noreorder
	.set	nomacro
	bne	$2,$0,.L624
	addu	$6,$6,1100
	.set	macro
	.set	reorder

	lh	$16,646($17)
	li	$2,-1			# 0xffffffff
	bne	$16,$2,.L626
	jal	func_80031B24
	lh	$2,646($17)
	#nop
	bne	$2,$16,.L626
	jal	func_80032314
.L626:
	lw	$31,24($sp)
	lw	$17,20($sp)
	lw	$16,16($sp)
	addu	$sp,$sp,32
	j	$31
	.end	func_8002C61C
