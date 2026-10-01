func_80049718:
	.frame	$sp,56,$31		# vars= 8, regs= 8/0, args= 16, extra= 0
	.mask	0x807f0000,-4
	.fmask	0x00000000,0
	subu	$sp,$sp,56
	sw	$22,48($sp)
	move	$22,$4
	sw	$19,36($sp)
	move	$19,$5
	sw	$20,40($sp)
	move	$20,$6
	sll	$2,$22,1
	la	$3,D_800EF980
	sw	$16,24($sp)
	addu	$16,$2,$3
	sw	$31,52($sp)
	sw	$21,44($sp)
	sw	$18,32($sp)
	sw	$17,28($sp)
	lh	$2,0($16)
	#nop
	.set	noreorder
	.set	nomacro
	bgez	$2,.L162
	move	$17,$7
	.set	macro
	.set	reorder

	jal	func_80052C10
.L162:
	lw	$18,D_800A38B4
	move	$21,$0
	sb	$0,0($18)
	sb	$0,1($18)
	lh	$2,0($16)
	li	$3,0x00000006		# 6
	sh	$3,4($18)
	li	$3,0x00000004		# 4
	sh	$0,8($18)
	sw	$0,12($18)
	sh	$3,10($18)
	sll	$2,$2,1
	.set	noreorder
	.set	nomacro
	beq	$19,$0,.L163
	sh	$2,2($18)
	.set	macro
	.set	reorder

	li	$2,0x00000001		# 1
	bne	$19,$2,.L164
	lhu	$2,0($17)
	#nop
	sh	$2,16($18)
	lhu	$2,2($17)
	#nop
	sh	$2,18($18)
	lhu	$2,4($17)
	addu	$4,$18,16
	sh	$2,20($18)
	lw	$2,g_anim_func_table
	#nop
	.set	noreorder
	.set	nomacro
	jal	$31,$2
	addu	$5,$18,24
	.set	macro
	.set	reorder

	lw	$2,0($20)
	#nop
	sw	$2,44($18)
	lw	$2,4($20)
	#nop
	sw	$2,48($18)
	lw	$2,8($20)
	.set	noreorder
	.set	nomacro
	j	.L165
	sw	$2,52($18)
	.set	macro
	.set	reorder

.L164:
	andi	$19,$19,0x7fff
	.set	noreorder
	.set	nomacro
	jal	func_8004153C
	srl	$4,$19,1
	.set	macro
	.set	reorder

	andi	$3,$19,0x0001
	move	$17,$2
	sll	$16,$3,1
	addu	$16,$16,$3
	sll	$16,$16,2
	addu	$16,$16,$3
	sll	$16,$16,3
	addu	$16,$16,2020
	addu	$16,$17,$16
	lh	$3,18($17)
	lw	$2,76($16)
	#nop
	mult	$2,$3
	mflo	$8
	#nop
	#nop
	sra	$2,$8,12
	sw	$2,76($16)
	lh	$3,18($17)
	lw	$2,80($16)
	#nop
	mult	$2,$3
	mflo	$8
	#nop
	#nop
	sra	$2,$8,12
	sw	$2,80($16)
	lh	$3,18($17)
	lw	$2,84($16)
	#nop
	mult	$2,$3
	addu	$4,$17,68
	addu	$5,$16,56
	addu	$6,$18,24
	mflo	$8
	#nop
	#nop
	sra	$2,$8,12
	.set	noreorder
	.set	nomacro
	jal	MulMatrix0
	sw	$2,84($16)
	.set	macro
	.set	reorder

	lw	$2,76($16)
	#nop
	sh	$2,16($sp)
	lw	$2,80($16)
	#nop
	sh	$2,18($sp)
	lw	$2,84($16)
	addu	$5,$sp,16
	sh	$2,20($sp)
	lw	$4,12($16)
	addu	$6,$18,44
	.set	noreorder
	.set	nomacro
	jal	ApplyMatrix
	addu	$4,$4,24
	.set	macro
	.set	reorder

	lw	$3,12($16)
	lw	$2,44($18)
	lw	$3,44($3)
	#nop
	addu	$2,$2,$3
	sw	$2,44($18)
	lw	$3,12($16)
	lw	$2,48($18)
	lw	$3,48($3)
	#nop
	addu	$2,$2,$3
	sw	$2,48($18)
	lw	$3,12($16)
	lw	$2,52($18)
	lw	$3,52($3)
	ori	$19,$19,0x8000
	addu	$2,$2,$3
	sw	$2,52($18)
	lw	$2,24($18)
	lw	$3,28($18)
	lw	$4,32($18)
	lw	$5,36($18)
	sw	$2,24($16)
	sw	$3,28($16)
	sw	$4,32($16)
	sw	$5,36($16)
	lw	$2,40($18)
	lw	$3,44($18)
	lw	$4,48($18)
	lw	$5,52($18)
	sw	$2,40($16)
	sw	$3,44($16)
	sw	$4,48($16)
	sw	$5,52($16)
	lh	$21,6788($17)
.L165:
	lw	$2,D_800A3820
	#nop
	addu	$3,$2,4
	sw	$3,D_800A3820
	sw	$18,0($2)
	li	$2,0x00000001		# 1
	.set	noreorder
	.set	nomacro
	beq	$19,$2,.L166
	addu	$18,$18,104
	.set	macro
	.set	reorder

	sll	$2,$22,1
	lh	$3,D_800EF980($2)
	li	$2,0x00000003		# 3
	sb	$2,0($18)
	sb	$0,1($18)
	sw	$21,88($18)
	lw	$4,D_800A3820
	addu	$2,$18,-104
	sw	$2,12($18)
	li	$2,0x00000001		# 1
	sh	$2,6($18)
	li	$2,0x00000006		# 6
	sh	$0,8($18)
	sh	$0,10($18)
	sh	$2,4($18)
	sll	$3,$3,1
	addu	$3,$3,1
	addu	$2,$4,4
	sh	$3,2($18)
	sw	$2,D_800A3820
	sw	$18,0($4)
	addu	$18,$18,104
.L166:
	sw	$18,D_800A38B4
.L163:
	lw	$31,52($sp)
	lw	$22,48($sp)
	lw	$21,44($sp)
	lw	$20,40($sp)
	lw	$19,36($sp)
	lw	$18,32($sp)
	lw	$17,28($sp)
	lw	$16,24($sp)
	addu	$sp,$sp,56
	j	$31
	.end	func_80049718
