func_8006F100:
	.frame	$sp,120,$31		# vars= 64, regs= 9/0, args= 16, extra= 0
	.mask	0x80ff0000,-8
	.fmask	0x00000000,0
	lh	$2,D_800A3550
	subu	$sp,$sp,120
	sw	$18,88($sp)
	move	$18,$4
	sw	$31,112($sp)
	sw	$23,108($sp)
	sw	$22,104($sp)
	sw	$21,100($sp)
	sw	$20,96($sp)
	sw	$19,92($sp)
	sw	$17,84($sp)
	sw	$16,80($sp)
	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L2096
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
	bne	$2,$0,$L2097
	li	$2,0x000000ff		# 255
	.set	macro
	.set	reorder

	sh	$2,D_800A3550
	jal	func_8006F038
	sh	$0,D_800A3550
	j	$L2098
$L2097:
	.set	noreorder
	.set	nomacro
	jal	func_8006F038
	move	$4,$18
	.set	macro
	.set	reorder

$L2098:
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

	j	$L2095
$L2096:
	lh	$2,D_800A355C
	#nop
	slt	$2,$2,121
	.set	noreorder
	.set	nomacro
	bne	$2,$0,$L2099
	li	$2,0x00000003		# 3
	.set	macro
	.set	reorder

	sh	$2,D_800A3584
$L2099:
	lw	$5,D_800A35A8
	lh	$3,D_800A3558
	lw	$4,D_800A35B0
	li	$2,0x00000100		# 256
	sw	$2,48($sp)
	sw	$2,52($sp)
	li	$2,0x00000013		# 19
	sw	$2,36($sp)
	sw	$0,32($sp)
	sb	$0,56($sp)
	lw	$23,88($5)
	addu	$3,$3,1
	addu	$4,$4,$3
	.set	noreorder
	.set	nomacro
	blez	$4,$L2101
	move	$17,$0
	.set	macro
	.set	reorder

	li	$22,0x66660000		# 1717960704
	ori	$22,$22,0x6667
	move	$20,$0
	move	$19,$0
	move	$21,$0
$L2103:
	lbu	$2,D_800A3560+2($19)
	lw	$3,D_800A35BC
	sll	$2,$2,2
	addu	$2,$2,$23
	lw	$9,0($2)
	li	$2,0x00000002		# 2
	.set	noreorder
	.set	nomacro
	bne	$3,$2,$L2104
	li	$4,0x00000001		# 1
	.set	macro
	.set	reorder

	lw	$2,D_800A3568
	#nop
	lw	$2,20($2)
	li	$3,0x00020000		# 131072
	and	$2,$2,$3
	bne	$2,$0,$L2104
	bne	$17,$4,$L2104
	lbu	$2,D_800A3561
	#nop
	lbu	$2,D_8009BC7C($2)
	#nop
	andi	$2,$2,0x0002
	.set	noreorder
	.set	nomacro
	j	$L2107
	sltu	$4,$0,$2
	.set	macro
	.set	reorder

$L2104:
	lbu	$2,D_800A3560+1($19)
	#nop
	lbu	$2,D_8009BC7C($2)
	#nop
	andi	$2,$2,0x0002
	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L2121
	sll	$2,$4,1
	.set	macro
	.set	reorder

	move	$4,$0
$L2107:
	sll	$2,$4,1
$L2121:
	addu	$2,$2,$4
	sll	$2,$2,2
	addu	$3,$9,$2
	.set	noreorder
	.set	nomacro
	beq	$17,$0,$L2109
	sw	$3,16($sp)
	.set	macro
	.set	reorder

	li	$2,0x00000040		# 64
	.set	noreorder
	.set	nomacro
	j	$L2110
	sh	$2,8($3)
	.set	macro
	.set	reorder

$L2109:
	sh	$0,8($3)
$L2110:
	addu	$6,$9,24
	la	$16,D_800A35C8
	.set	noreorder
	.set	nomacro
	beq	$17,$0,$L2111
	sw	$6,20($sp)
	.set	macro
	.set	reorder

	la	$16,D_800A35CA
$L2111:
	lh	$4,0($16)
	lw	$3,16($sp)
	sll	$2,$4,1
	addu	$2,$2,$4
	sll	$2,$2,3
	addu	$2,$2,$4
	sll	$2,$2,5
	subu	$2,$0,$2
	mult	$2,$22
	lbu	$3,2($3)
	sra	$2,$2,31
	addu	$3,$3,-1
	sll	$3,$3,3
	addu	$3,$3,$6
	lbu	$8,6($3)
	mfhi	$12
	#nop
	#nop
	sra	$5,$12,3
	subu	$10,$5,$2
	sll	$2,$4,2
	addu	$2,$2,$4
	sll	$2,$2,2
	addu	$2,$2,$4
	sll	$2,$2,2
	subu	$2,$2,$4
	sll	$2,$2,3
	subu	$2,$0,$2
	mult	$2,$22
	lhu	$6,24($9)
	lbu	$7,7($3)
	move	$11,$10
	lhu	$4,0($3)
	lhu	$3,2($3)
	lhu	$5,26($9)
	sra	$2,$2,31
	addu	$4,$4,$8
	subu	$4,$4,$6
	addu	$3,$3,$7
	subu	$7,$3,$5
	mfhi	$12
	#nop
	#nop
	sra	$3,$12,3
	.set	noreorder
	.set	nomacro
	beq	$17,$0,$L2113
	subu	$8,$3,$2
	.set	macro
	.set	reorder

	subu	$11,$0,$10
$L2113:
	la	$5,D_800A3590
	lw	$2,48($sp)
	addu	$5,$21,$5
	sll	$3,$4,16
	sra	$3,$3,16
	mult	$3,$2
	lh	$2,0($5)
	lw	$6,52($sp)
	sll	$2,$2,2
	addu	$2,$2,$20
	lh	$4,D_8009BC94($2)
	sll	$2,$11,16
	sra	$2,$2,16
	addu	$2,$2,320
	addu	$4,$4,$2
	mflo	$3
	#nop
	#nop
	sra	$2,$3,8
	srl	$3,$3,31
	addu	$2,$2,$3
	sra	$2,$2,1
	subu	$4,$4,$2
	sll	$3,$7,16
	sra	$3,$3,16
	sw	$4,40($sp)
	mult	$3,$6
	lh	$2,0($5)
	#nop
	sll	$2,$2,2
	addu	$2,$2,$20
	lh	$4,D_8009BC94+2($2)
	sll	$2,$8,16
	sra	$2,$2,16
	addu	$2,$2,157
	addu	$4,$4,$2
	mflo	$3
	#nop
	#nop
	sra	$2,$3,8
	srl	$3,$3,31
	addu	$2,$2,$3
	sra	$2,$2,1
	subu	$4,$4,$2
	sw	$4,44($sp)
	lw	$2,4($18)
	.set	noreorder
	.set	nomacro
	beq	$17,$0,$L2114
	sw	$2,28($sp)
	.set	macro
	.set	reorder

	addu	$4,$sp,16
	li	$5,0x000001c0		# 448
	.set	noreorder
	.set	nomacro
	j	$L2120
	li	$6,0x00000001		# 1
	.set	macro
	.set	reorder

$L2114:
	addu	$4,$sp,16
	li	$5,0x00000e40		# 3648
	move	$6,$0
$L2120:
	jal	func_80073C78
	sw	$2,4($18)
	lh	$2,0($16)
	#nop
	blez	$2,$L2116
	addu	$16,$16,-2
$L2116:
	lh	$3,0($16)
	li	$2,0x0000000a		# 10
	.set	noreorder
	.set	nomacro
	bne	$3,$2,$L2117
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

$L2117:
	lh	$2,0($16)
	#nop
	.set	noreorder
	.set	nomacro
	bgez	$2,$L2102
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

$L2102:
	addu	$20,$20,24
	addu	$19,$19,3
	lh	$3,D_800A3558
	lw	$2,D_800A35B0
	addu	$17,$17,1
	addu	$3,$3,1
	addu	$2,$2,$3
	slt	$2,$17,$2
	.set	noreorder
	.set	nomacro
	bne	$2,$0,$L2103
	addu	$21,$21,2
	.set	macro
	.set	reorder

$L2101:
	lhu	$2,D_800A355C
	#nop
	addu	$2,$2,1
	sh	$2,D_800A355C
$L2095:
	lw	$31,112($sp)
	lw	$23,108($sp)
	lw	$22,104($sp)
	lw	$21,100($sp)
	lw	$20,96($sp)
	lw	$19,92($sp)
	lw	$18,88($sp)
	lw	$17,84($sp)
	lw	$16,80($sp)
	addu	$sp,$sp,120
	j	$31
	.end	func_8006F100
