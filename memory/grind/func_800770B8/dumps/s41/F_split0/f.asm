func_800770B8:
	.frame	$sp,56,$31		# vars= 24, regs= 4/0, args= 16, extra= 0
	.mask	0x80070000,-4
	.fmask	0x00000000,0
	subu	$sp,$sp,56
	sw	$16,40($sp)
	move	$16,$4
	sw	$18,48($sp)
	move	$18,$5
	sw	$17,44($sp)
	move	$17,$6
	sw	$31,52($sp)
	lw	$4,g_gpu_ot_ptr
	li	$5,0x00001008		# 4104
	sh	$0,16($sp)
	.set	noreorder
	.set	nomacro
	jal	ClearOTagR
	sh	$0,18($sp)
	.set	macro
	.set	reorder

	sw	$16,D_800A35D8
	.set	noreorder
	.set	nomacro
	jal	snd_StopAll
	addu	$16,$16,88
	.set	macro
	.set	reorder

	li	$4,0x00000006		# 6
	.set	noreorder
	.set	nomacro
	jal	func_8006E950
	move	$5,$16
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	jal	func_80076FF8
	move	$4,$16
	.set	macro
	.set	reorder

	lw	$5,D_800A35D8
	.set	noreorder
	.set	nomacro
	jal	func_8006E49C
	move	$4,$2
	.set	macro
	.set	reorder

	move	$8,$0
	li	$10,-1			# 0xffffffff
	li	$9,0x00000001		# 1
	addu	$11,$sp,16
	sw	$2,D_800A36A0
	sw	$16,4($2)
	sw	$0,48($2)
	sh	$0,52($2)
	move	$6,$0
.L338:
	sll	$5,$8,16
	sra	$5,$5,16
	sll	$2,$5,1
	lw	$4,D_800A36A0
	sll	$3,$5,2
	addu	$2,$2,$4
	sh	$0,16($2)
	sh	$0,8($2)
	sh	$0,12($2)
	sh	$0,20($2)
	sh	$0,60($2)
	la	$2,D_800A35D0
	addu	$2,$3,$2
	sh	$0,2($2)
	sh	$0,0($2)
	addu	$2,$4,$3
	addu	$4,$4,$5
	addu	$3,$3,$5
	sh	$0,66($2)
	sh	$0,64($2)
	sb	$8,104($4)
	lw	$2,D_800A36A0
	sll	$3,$3,1
	addu	$3,$3,$2
	addu	$7,$3,106
	addu	$5,$3,126
.L321:
	sll	$2,$6,16
	addu	$4,$6,1
	move	$6,$4
	sra	$2,$2,15
	addu	$3,$2,$7
	addu	$2,$2,$5
	sll	$4,$4,16
	sra	$4,$4,16
	slt	$4,$4,5
	sh	$10,0($3)
	.set	noreorder
	.set	nomacro
	bne	$4,$0,.L321
	sh	$0,0($2)
	.set	macro
	.set	reorder

	move	$6,$0
	sll	$3,$8,16
	sra	$3,$3,16
	sll	$2,$3,2
	addu	$2,$2,$3
	sll	$7,$2,1
	sll	$3,$3,1
	lw	$2,D_800A36A0
	addu	$5,$3,$11
	addu	$3,$3,$2
	li	$2,0x00000005		# 5
	sh	$0,92($3)
	sh	$2,96($3)
	addu	$2,$6,$7
.L337:
	sll	$2,$2,16
	sra	$3,$2,16
	lbu	$2,D_8009BCE4($3)
	#nop
	andi	$4,$2,0x00f2
	sll	$2,$9,$3
	and	$2,$17,$2
	sb	$4,D_8009BCE4($3)
	.set	noreorder
	.set	nomacro
	beq	$2,$0,.L336
	addu	$2,$6,1
	.set	macro
	.set	reorder

	ori	$2,$4,0x0001
	sb	$2,D_8009BCE4($3)
	lhu	$2,0($5)
	#nop
	addu	$2,$2,1
	sh	$2,0($5)
	addu	$2,$6,1
.L336:
	move	$6,$2
	sll	$2,$2,16
	sra	$2,$2,16
	slt	$2,$2,10
	.set	noreorder
	.set	nomacro
	bne	$2,$0,.L337
	addu	$2,$6,$7
	.set	macro
	.set	reorder

	addu	$2,$8,1
	move	$8,$2
	sll	$2,$2,16
	sra	$2,$2,16
	slt	$2,$2,2
	.set	noreorder
	.set	nomacro
	bne	$2,$0,.L338
	move	$6,$0
	.set	macro
	.set	reorder

	lw	$4,D_800A36A0
	#nop
	sw	$0,32($4)
	sw	$0,28($4)
	lh	$2,16($sp)
	lh	$3,18($sp)
	move	$6,$2
	slt	$2,$2,$3
	.set	noreorder
	.set	nomacro
	beq	$2,$0,.L332
	move	$5,$3
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	j	.L335
	addu	$2,$6,-3
	.set	macro
	.set	reorder

.L332:
	addu	$2,$5,-3
.L335:
	sb	$2,100($4)
	lw	$3,D_800A36A0
	#nop
	lbu	$2,100($3)
	#nop
	sltu	$2,$2,3
	.set	noreorder
	.set	nomacro
	bne	$2,$0,.L334
	li	$2,0x00000002		# 2
	.set	macro
	.set	reorder

	sb	$2,100($3)
.L334:
	lw	$2,D_800A36A0
	#nop
	sw	$18,0($2)
	sb	$0,101($2)
	lw	$2,D_800A36A0
	li	$5,0x00000001		# 1
	sb	$5,103($2)
	lw	$4,D_800A36A0
	#nop
	lbu	$2,103($4)
	#nop
	sll	$2,$2,1
	lbu	$3,D_8009BD20+1($2)
	li	$2,0x00000001		# 1
	sb	$3,102($4)
	sb	$5,D_800A35DC
	lw	$31,52($sp)
	lw	$18,48($sp)
	lw	$17,44($sp)
	lw	$16,40($sp)
	addu	$sp,$sp,56
	j	$31
	.end	func_800770B8
