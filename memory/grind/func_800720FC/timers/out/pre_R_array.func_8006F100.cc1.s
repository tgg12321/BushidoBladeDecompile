func_8006F100:
	.frame	$sp,128,$31		# vars= 72, regs= 9/0, args= 16, extra= 0
	.mask	0x80ff0000,-8
	.fmask	0x00000000,0
	lh	$2,D_800A3550
	subu	$sp,$sp,128
	sw	$18,96($sp)
	move	$18,$4
	sw	$31,120($sp)
	sw	$23,116($sp)
	sw	$22,112($sp)
	sw	$21,108($sp)
	sw	$20,104($sp)
	sw	$19,100($sp)
	sw	$17,92($sp)
	sw	$16,88($sp)
	.set	noreorder
	.set	nomacro
	beq	$2,$0,.L2108
	move	$3,$2
	.set	macro
	.set	reorder

	addu	$2,$3,16
	sh	$2,D_800A3550
	sll	$2,$2,16
	sra	$2,$2,16
	slt	$2,$2,256
	.set	noreorder
	.set	nomacro
	bne	$2,$0,.L2109
	li	$2,0x000000ff		# 255
	.set	macro
	.set	reorder

	sh	$2,D_800A3550
	jal	func_8006F038
	sh	$0,D_800A3550
	j	.L2110
.L2109:
	.set	noreorder
	.set	nomacro
	jal	func_8006F038
	move	$4,$18
	.set	macro
	.set	reorder

.L2110:
	.set	noreorder
	.set	nomacro
	jal	func_8006ECF4
	move	$4,$18
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	jal	func_80072E10
	move	$4,$18
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	jal	func_80073200
	move	$4,$18
	.set	macro
	.set	reorder

	j	.L2107
.L2108:
	lh	$2,D_800A355C
	#nop
	slt	$2,$2,121
	.set	noreorder
	.set	nomacro
	bne	$2,$0,.L2111
	li	$2,0x00000003		# 3
	.set	macro
	.set	reorder

	sh	$2,D_800A3584
.L2111:
	lh	$3,D_800A3558
	li	$2,0x00000100		# 256
	sw	$2,48($sp)
	sw	$2,52($sp)
	li	$2,0x00000013		# 19
	sw	$2,36($sp)
	lw	$2,D_800A35B0
	lw	$4,D_800A35A8
	sw	$0,32($sp)
	sb	$0,56($sp)
	addu	$3,$3,1
	addu	$2,$2,$3
	lw	$23,88($4)
	.set	noreorder
	.set	nomacro
	blez	$2,.L2113
	move	$17,$0
	.set	macro
	.set	reorder

	li	$22,0x66660000		# 1717960704
	ori	$22,$22,0x6667
	la	$16,D_800A35C8
	move	$21,$0
	move	$20,$0
	move	$19,$0
.L2115:
	li	$4,0x00000002		# 2
	lbu	$2,D_800A3560+2($19)
	lw	$3,D_800A35BC
	sll	$2,$2,2
	addu	$2,$2,$23
	lw	$7,0($2)
	.set	noreorder
	.set	nomacro
	bne	$3,$4,.L2116
	li	$5,0x00000001		# 1
	.set	macro
	.set	reorder

	lw	$2,D_800A3568
	#nop
	lw	$2,20($2)
	li	$3,0x00020000		# 131072
	and	$2,$2,$3
	bne	$2,$0,.L2116
	bne	$17,$5,.L2116
	lbu	$2,D_800A3561
	#nop
	lbu	$2,D_8009BC7C($2)
	#nop
	andi	$2,$2,0x0002
	.set	noreorder
	.set	nomacro
	j	.L2119
	sltu	$5,$0,$2
	.set	macro
	.set	reorder

.L2116:
	lbu	$2,D_800A3560+1($19)
	#nop
	lbu	$2,D_8009BC7C($2)
	#nop
	andi	$2,$2,0x0002
	.set	noreorder
	.set	nomacro
	beq	$2,$0,.L2131
	sll	$2,$5,1
	.set	macro
	.set	reorder

	move	$5,$0
.L2119:
	sll	$2,$5,1
.L2131:
	addu	$2,$2,$5
	sll	$2,$2,2
	addu	$3,$7,$2
	.set	noreorder
	.set	nomacro
	beq	$17,$0,.L2121
	sw	$3,16($sp)
	.set	macro
	.set	reorder

	li	$2,0x00000040		# 64
	.set	noreorder
	.set	nomacro
	j	.L2122
	sh	$2,8($3)
	.set	macro
	.set	reorder

.L2121:
	sh	$0,8($3)
.L2122:
	addu	$6,$7,24
	sw	$6,20($sp)
	lh	$5,0($16)
	#nop
	sll	$2,$5,1
	addu	$2,$2,$5
	sll	$2,$2,3
	addu	$2,$2,$5
	sll	$2,$2,5
	subu	$2,$0,$2
	mult	$2,$22
	lw	$3,16($sp)
	sra	$2,$2,31
	mfhi	$11
	#nop
	#nop
	sra	$4,$11,3
	subu	$8,$4,$2
	move	$9,$8
	sll	$2,$5,2
	addu	$2,$2,$5
	sll	$2,$2,2
	addu	$2,$2,$5
	sll	$2,$2,2
	subu	$2,$2,$5
	sll	$2,$2,3
	subu	$2,$0,$2
	lbu	$4,2($3)
	mult	$2,$22
	sra	$2,$2,31
	addu	$4,$4,-1
	sll	$4,$4,3
	addu	$4,$4,$6
	lbu	$6,6($4)
	lhu	$3,0($4)
	lhu	$5,24($7)
	addu	$3,$3,$6
	subu	$6,$3,$5
	lbu	$5,7($4)
	lhu	$3,2($4)
	lhu	$4,26($7)
	addu	$3,$3,$5
	subu	$7,$3,$4
	mfhi	$11
	#nop
	#nop
	sra	$3,$11,3
	.set	noreorder
	.set	nomacro
	beq	$17,$0,.L2123
	subu	$10,$3,$2
	.set	macro
	.set	reorder

	subu	$9,$0,$8
.L2123:
	sll	$4,$6,16
	sra	$4,$4,16
	lw	$2,48($sp)
	la	$6,D_800A3590
	mult	$4,$2
	addu	$6,$21,$6
	lh	$3,0($6)
	#nop
	sll	$3,$3,2
	addu	$3,$3,$20
	sll	$2,$9,16
	sra	$2,$2,16
	addu	$2,$2,320
	lh	$5,D_8009BC94($3)
	lw	$3,52($sp)
	addu	$5,$5,$2
	mflo	$4
	#nop
	#nop
	sra	$2,$4,8
	srl	$4,$4,31
	addu	$2,$2,$4
	sll	$4,$7,16
	sra	$4,$4,16
	mult	$4,$3
	sra	$2,$2,1
	subu	$5,$5,$2
	sll	$2,$10,16
	sw	$5,40($sp)
	lh	$3,0($6)
	sra	$2,$2,16
	sll	$3,$3,2
	addu	$3,$3,$20
	lh	$3,D_8009BC94+2($3)
	addu	$2,$2,157
	addu	$3,$3,$2
	mflo	$4
	#nop
	#nop
	sra	$2,$4,8
	srl	$4,$4,31
	addu	$2,$2,$4
	sra	$2,$2,1
	subu	$3,$3,$2
	sw	$3,44($sp)
	lw	$2,4($18)
	.set	noreorder
	.set	nomacro
	beq	$17,$0,.L2124
	sw	$2,28($sp)
	.set	macro
	.set	reorder

	addu	$4,$sp,16
	li	$5,0x000001c0		# 448
	.set	noreorder
	.set	nomacro
	j	.L2130
	li	$6,0x00000001		# 1
	.set	macro
	.set	reorder

.L2124:
	addu	$4,$sp,16
	li	$5,0x00000e40		# 3648
	move	$6,$0
.L2130:
	jal	func_80073C78
	sw	$2,4($18)
	lh	$2,0($16)
	#nop
	.set	noreorder
	.set	nomacro
	blez	$2,.L2126
	move	$3,$2
	.set	macro
	.set	reorder

	addu	$2,$3,-1
	sh	$2,0($16)
.L2126:
	lh	$3,0($16)
	li	$2,0x0000000a		# 10
	.set	noreorder
	.set	nomacro
	bne	$3,$2,.L2127
	li	$4,0x00000007		# 7
	.set	macro
	.set	reorder

	li	$5,0x0000007f		# 127
	.set	noreorder
	.set	nomacro
	jal	func_8005C650
	li	$6,0x0000007f		# 127
	.set	macro
	.set	reorder

.L2127:
	lh	$2,0($16)
	#nop
	.set	noreorder
	.set	nomacro
	bgez	$2,.L2114
	li	$4,0x00000008		# 8
	.set	macro
	.set	reorder

	sh	$0,0($16)
	li	$5,0x0000007f		# 127
	.set	noreorder
	.set	nomacro
	jal	func_8005C650
	li	$6,0x0000007f		# 127
	.set	macro
	.set	reorder

.L2114:
	addu	$16,$16,2
	addu	$21,$21,2
	addu	$20,$20,24
	addu	$17,$17,1
	lh	$3,D_800A3558
	lw	$2,D_800A35B0
	addu	$3,$3,1
	addu	$2,$2,$3
	slt	$2,$17,$2
	.set	noreorder
	.set	nomacro
	bne	$2,$0,.L2115
	addu	$19,$19,3
	.set	macro
	.set	reorder

.L2113:
	lhu	$2,D_800A355C
	#nop
	addu	$2,$2,1
	sh	$2,D_800A355C
.L2107:
	lw	$31,120($sp)
	lw	$23,116($sp)
	lw	$22,112($sp)
	lw	$21,108($sp)
	lw	$20,104($sp)
	lw	$19,100($sp)
	lw	$18,96($sp)
	lw	$17,92($sp)
	lw	$16,88($sp)
	addu	$sp,$sp,128
	j	$31
	.end	func_8006F100
