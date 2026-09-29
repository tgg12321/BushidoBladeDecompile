func_800720FC:
	.frame	$sp,152,$31		# vars= 88, regs= 10/0, args= 24, extra= 0
	.mask	0xc0ff0000,-4
	.fmask	0x00000000,0
	lw	$2,D_800A35A8
	lw	$3,D_800A35C0
	subu	$sp,$sp,152
	sw	$31,148($sp)
	sw	$fp,144($sp)
	sw	$23,140($sp)
	sw	$22,136($sp)
	sw	$21,132($sp)
	sw	$20,128($sp)
	sw	$19,124($sp)
	sw	$18,120($sp)
	sw	$17,116($sp)
	sw	$16,112($sp)
	lw	$2,128($2)
	sw	$0,40($sp)
	sb	$0,64($sp)
	sw	$2,88($sp)
	li	$2,0x00000011		# 17
	sw	$2,44($sp)
	lhu	$2,0($3)
	#nop
	sh	$2,80($sp)
	lhu	$2,2($3)
	#nop
	sh	$2,82($sp)
	lhu	$2,4($3)
	#nop
	sh	$2,84($sp)
	lhu	$2,6($3)
	move	$20,$4
	sh	$2,86($sp)
	lw	$4,28($20)
	move	$fp,$5
	move	$23,$6
	.set	noreorder
	.set	nomacro
	jal	SetDrawArea
	addu	$5,$sp,80
	.set	macro
	.set	reorder

	lw	$4,44($sp)
	lw	$2,g_gpu_ot_ptr
	lw	$5,28($20)
	sll	$4,$4,2
	.set	noreorder
	.set	nomacro
	jal	AddPrim
	addu	$4,$2,$4
	.set	macro
	.set	reorder

	lw	$2,28($20)
	lw	$3,D_800A35C0
	lw	$5,D_800A35C4
	addu	$2,$2,12
	sw	$2,28($20)
	lhu	$2,8($3)
	#nop
	sh	$2,16($5)
	lhu	$2,10($3)
	#nop
	sh	$2,18($5)
	lw	$4,32($20)
	.set	noreorder
	.set	nomacro
	jal	SetDrawOffset
	addu	$5,$5,16
	.set	macro
	.set	reorder

	lw	$4,44($sp)
	lw	$2,g_gpu_ot_ptr
	lw	$5,32($20)
	sll	$4,$4,2
	.set	noreorder
	.set	nomacro
	jal	AddPrim
	addu	$4,$2,$4
	.set	macro
	.set	reorder

	lw	$2,32($20)
	lbu	$3,D_800A3578
	addu	$2,$2,12
	.set	noreorder
	.set	nomacro
	bne	$3,$0,$L111
	sw	$2,32($20)
	.set	macro
	.set	reorder

	lw	$3,D_800A35A8
	li	$2,-10			# 0xfffffff6
	sw	$2,48($sp)
	li	$2,-5			# 0xfffffffb
	sw	$2,52($sp)
	lw	$19,116($3)
	lh	$3,D_800A359C
	lh	$4,D_800A3598
	lw	$2,16($19)
	sll	$3,$3,1
	addu	$3,$3,$4
	addu	$18,$2,24
	sw	$2,24($sp)
	li	$2,0x00000003		# 3
	.set	noreorder
	.set	nomacro
	bne	$3,$2,$L113
	li	$2,0x00000006		# 6
	.set	macro
	.set	reorder

	lw	$3,D_800A35BC
	#nop
	.set	noreorder
	.set	nomacro
	bne	$3,$2,$L113
	li	$2,0x00000002		# 2
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	beq	$23,$2,$L177
	move	$17,$0
	.set	macro
	.set	reorder

$L113:
	lh	$2,D_800A359C
	lh	$4,D_800A3598
	sll	$3,$23,1
	addu	$3,$3,$23
	sw	$18,28($sp)
	addu	$2,$2,$3
	sll	$2,$2,1
	addu	$2,$2,$4
	sll	$2,$2,3
	addu	$2,$18,$2
	sw	$2,28($sp)
	lw	$2,16($20)
	addu	$4,$sp,24
	.set	noreorder
	.set	nomacro
	jal	func_8007352C
	sw	$2,32($sp)
	.set	macro
	.set	reorder

	sw	$2,16($20)
	move	$17,$0
$L177:
	lw	$3,24($sp)
	sll	$2,$23,1
	addu	$2,$2,$23
	sll	$16,$2,1
	addu	$3,$3,12
	sw	$3,24($sp)
	li	$2,0x00000003		# 3
$L178:
	.set	noreorder
	.set	nomacro
	bne	$17,$2,$L119
	li	$2,0x00000006		# 6
	.set	macro
	.set	reorder

	lw	$3,D_800A35BC
	#nop
	.set	noreorder
	.set	nomacro
	bne	$3,$2,$L119
	li	$2,0x00000002		# 2
	.set	macro
	.set	reorder

	beq	$23,$2,$L116
$L119:
	lh	$2,D_800A359C
	lh	$3,D_800A3598
	sll	$2,$2,1
	addu	$2,$2,$3
	.set	noreorder
	.set	nomacro
	beq	$2,$17,$L116
	addu	$2,$16,$17
	.set	macro
	.set	reorder

	sll	$2,$2,3
	addu	$2,$18,$2
	sw	$2,28($sp)
	lw	$2,16($20)
	addu	$4,$sp,24
	.set	noreorder
	.set	nomacro
	jal	func_8007352C
	sw	$2,32($sp)
	.set	macro
	.set	reorder

	sw	$2,16($20)
$L116:
	addu	$17,$17,1
	slt	$2,$17,6
	.set	noreorder
	.set	nomacro
	bne	$2,$0,$L178
	li	$2,0x00000003		# 3
	.set	macro
	.set	reorder

	lw	$4,16($19)
	move	$5,$0
	.set	noreorder
	.set	nomacro
	jal	func_8006E480
	sw	$4,24($sp)
	.set	macro
	.set	reorder

	sw	$0,16($sp)
	lw	$4,24($20)
	li	$5,0x00000001		# 1
	move	$6,$0
	.set	noreorder
	.set	nomacro
	jal	SetDrawMode
	move	$7,$2
	.set	macro
	.set	reorder

	lw	$4,44($sp)
	lw	$2,g_gpu_ot_ptr
	lw	$5,24($20)
	sll	$4,$4,2
	.set	noreorder
	.set	nomacro
	jal	AddPrim
	addu	$4,$2,$4
	.set	macro
	.set	reorder

	lw	$2,24($20)
	#nop
	addu	$2,$2,12
	sw	$2,24($20)
$L111:
	li	$2,0x00000090		# 144
	sw	$2,48($sp)
	li	$2,0x00000028		# 40
	sw	$2,52($sp)
	lw	$2,0($fp)
	#nop
	addu	$18,$2,12
	sw	$2,24($sp)
	sw	$18,28($sp)
	lw	$2,16($20)
	addu	$4,$sp,24
	.set	noreorder
	.set	nomacro
	jal	func_8007352C
	sw	$2,32($sp)
	.set	macro
	.set	reorder

	lw	$6,D_800A35C0
	sll	$4,$23,2
	sw	$2,16($20)
	lhu	$3,D_8009BCC4($4)
	lhu	$2,8($6)
	lw	$5,D_800A35C4
	subu	$2,$2,$3
	sh	$2,16($5)
	lhu	$2,10($6)
	lhu	$3,D_8009BCC4+2($4)
	lbu	$4,D_800A3578
	subu	$2,$2,$3
	sh	$2,18($5)
	li	$2,0x00000001		# 1
	.set	noreorder
	.set	nomacro
	beq	$4,$2,$L122
	li	$2,0x00000003		# 3
	.set	macro
	.set	reorder

	bne	$4,$2,$L121
$L122:
	lhu	$4,D_800A3584
	#nop
	addu	$2,$4,-4
	sltu	$2,$2,3
	beq	$2,$0,$L121
	lhu	$3,D_800A3580
	#nop
	addu	$2,$3,-4
	sltu	$2,$2,3
	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L121
	move	$10,$0
	.set	macro
	.set	reorder

	la	$6,D_8009BCD0
	move	$11,$6
	la	$2,D_8009BCB4
	sll	$3,$3,16
	sra	$3,$3,14
	addu	$9,$3,$2
	sll	$3,$4,16
	sra	$3,$3,14
	addu	$8,$3,$2
$L126:
	lh	$3,0($8)
	lh	$2,0($9)
	#nop
	subu	$4,$3,$2
	sll	$2,$4,4
	subu	$5,$2,$4
	lh	$3,0($6)
	sll	$2,$5,1
	move	$4,$2
	.set	noreorder
	bgez	$4,1f
	move	$2,$4
	subu	$2,$0,$2
1:
	.set	reorder
	move	$7,$3
	bgez	$3,1f
	subu	$3,$0,$3
1:
	slt	$3,$3,$2
	.set	noreorder
	.set	nomacro
	beq	$3,$0,$L127
	li	$2,0x43250000		# 1126498304
	.set	macro
	.set	reorder

	ori	$2,$2,0xc53f
	sll	$3,$5,6
	mult	$3,$2
	sra	$3,$3,31
	mfhi	$12
	#nop
	#nop
	sra	$2,$12,7
	subu	$2,$2,$3
	addu	$2,$7,$2
	.set	noreorder
	.set	nomacro
	j	$L128
	sh	$2,0($6)
	.set	macro
	.set	reorder

$L127:
	sh	$4,0($6)
$L128:
	lhu	$4,0($6)
	li	$3,-2004353024			# 0x88880000
	ori	$3,$3,0x8889
	sll	$4,$4,16
	sra	$2,$4,16
	mult	$2,$3
	lw	$5,D_800A35C4
	addu	$6,$6,2
	addu	$9,$9,2
	addu	$8,$8,2
	addu	$5,$10,$5
	addu	$10,$10,2
	lhu	$3,16($5)
	sra	$4,$4,31
	mfhi	$12
	#nop
	#nop
	addu	$2,$12,$2
	sra	$2,$2,4
	subu	$2,$2,$4
	subu	$3,$3,$2
	addu	$2,$11,4
	slt	$2,$6,$2
	.set	noreorder
	.set	nomacro
	bne	$2,$0,$L126
	sh	$3,16($5)
	.set	macro
	.set	reorder

	j	$L130
$L121:
	sh	$0,D_8009BCD0+2
	sh	$0,D_8009BCD0
$L130:
	lw	$5,D_800A35C4
	lw	$4,32($20)
	.set	noreorder
	.set	nomacro
	jal	SetDrawOffset
	addu	$5,$5,16
	.set	macro
	.set	reorder

	lw	$4,44($sp)
	lw	$2,g_gpu_ot_ptr
	lw	$5,32($20)
	sll	$4,$4,2
	.set	noreorder
	.set	nomacro
	jal	AddPrim
	addu	$4,$2,$4
	.set	macro
	.set	reorder

	lw	$2,32($20)
	lw	$3,D_800A35C0
	addu	$2,$2,12
	sw	$2,32($20)
	lw	$2,48($sp)
	lw	$4,52($sp)
	sh	$2,80($sp)
	lhu	$3,2($3)
	li	$2,0x00000160		# 352
	sh	$2,84($sp)
	li	$2,0x0000004a		# 74
	sh	$2,86($sp)
	addu	$3,$3,$4
	sh	$3,82($sp)
	lw	$4,28($20)
	.set	noreorder
	.set	nomacro
	jal	SetDrawArea
	addu	$5,$sp,80
	.set	macro
	.set	reorder

	lw	$4,44($sp)
	lw	$2,g_gpu_ot_ptr
	lw	$5,28($20)
	sll	$4,$4,2
	.set	noreorder
	.set	nomacro
	jal	AddPrim
	addu	$4,$2,$4
	.set	macro
	.set	reorder

	lw	$2,28($20)
	#nop
	addu	$2,$2,12
	sw	$2,28($20)
	lw	$4,24($sp)
	.set	noreorder
	.set	nomacro
	jal	func_8006E480
	move	$5,$0
	.set	macro
	.set	reorder

	sw	$0,16($sp)
	lw	$4,24($20)
	li	$5,0x00000001		# 1
	move	$6,$0
	.set	noreorder
	.set	nomacro
	jal	SetDrawMode
	move	$7,$2
	.set	macro
	.set	reorder

	lw	$4,44($sp)
	lw	$2,g_gpu_ot_ptr
	lw	$5,24($20)
	sll	$4,$4,2
	.set	noreorder
	.set	nomacro
	jal	AddPrim
	addu	$4,$2,$4
	.set	macro
	.set	reorder

	lw	$2,24($20)
	#nop
	addu	$2,$2,12
	sw	$2,24($20)
	li	$2,0x00000100		# 256
	sw	$2,56($sp)
	sw	$2,60($sp)
	li	$2,0x0000000b		# 11
	sw	$0,52($sp)
	sw	$0,48($sp)
	sw	$2,44($sp)
	lw	$3,4($fp)
	li	$2,0x00000180		# 384
	sw	$2,56($sp)
	li	$2,0x00000120		# 288
	sw	$2,60($sp)
	addu	$18,$3,12
	sw	$3,24($sp)
	sw	$18,28($sp)
	lw	$2,4($20)
	addu	$4,$sp,24
	move	$5,$0
	.set	noreorder
	.set	nomacro
	jal	func_80073728
	sw	$2,36($sp)
	.set	macro
	.set	reorder

	sw	$2,4($20)
	lw	$2,24($sp)
	#nop
	lbu	$2,2($2)
	lw	$3,28($sp)
	sll	$2,$2,3
	addu	$3,$3,$2
	sw	$3,28($sp)
	lw	$2,4($20)
	addu	$4,$sp,24
	li	$5,0x00000001		# 1
	.set	noreorder
	.set	nomacro
	jal	func_80073728
	sw	$2,36($sp)
	.set	macro
	.set	reorder

	lw	$3,D_800A354C
	li	$4,-1610612736			# 0xa0000000
	ori	$4,$4,0xa000
	and	$3,$3,$4
	.set	noreorder
	.set	nomacro
	beq	$3,$0,$L131
	sw	$2,4($20)
	.set	macro
	.set	reorder

	lbu	$2,D_800A3578
	#nop
	.set	noreorder
	.set	nomacro
	bne	$2,$0,$L131
	move	$4,$0
	.set	macro
	.set	reorder

	li	$5,0x0000007f		# 127
	.set	noreorder
	.set	nomacro
	jal	func_8005C650
	li	$6,0x0000007f		# 127
	.set	macro
	.set	reorder

	lw	$3,D_800A354C
	li	$2,0x20000000		# 536870912
	ori	$2,$2,0x2000
	and	$2,$3,$2
	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L179
	li	$2,-2147483648			# 0x80000000
	.set	macro
	.set	reorder

	lh	$2,D_800A3598
	#nop
	.set	noreorder
	.set	nomacro
	bne	$2,$0,$L134
	li	$2,-2147483648			# 0x80000000
	.set	macro
	.set	reorder

$L179:
	ori	$2,$2,0x8000
	and	$2,$3,$2
	beq	$2,$0,$L133
	lh	$2,D_800A3598
	#nop
	.set	noreorder
	.set	nomacro
	bne	$2,$0,$L180
	sltu	$2,$2,1
	.set	macro
	.set	reorder

$L134:
	lh	$2,D_800A3580
	lh	$3,D_800A3598
	lw	$12,88($sp)
	sll	$2,$2,3
	addu	$2,$2,$3
	addu	$2,$2,$12
	lbu	$4,-26($2)
	li	$3,0x0000000f		# 15
	andi	$2,$4,0x000f
	andi	$2,$2,0x00ff
	sh	$2,D_800A3584
	.set	noreorder
	.set	nomacro
	beq	$2,$3,$L133
	srl	$2,$4,4
	.set	macro
	.set	reorder

	sh	$2,D_800A3578
	li	$4,0x00000006		# 6
	li	$5,0x0000007f		# 127
	.set	noreorder
	.set	nomacro
	jal	func_8005C650
	li	$6,0x0000007f		# 127
	.set	macro
	.set	reorder

$L133:
	lh	$2,D_800A3598
	#nop
	sltu	$2,$2,1
$L180:
	sh	$2,D_800A3598
$L131:
	lw	$3,D_800A354C
	li	$2,0x40000000		# 1073741824
	ori	$2,$2,0x4000
	and	$2,$3,$2
	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L137
	move	$4,$0
	.set	macro
	.set	reorder

	li	$5,0x0000007f		# 127
	.set	noreorder
	.set	nomacro
	jal	func_8005C650
	li	$6,0x0000007f		# 127
	.set	macro
	.set	reorder

	lhu	$2,D_800A359C
	.set	noreorder
	.set	nomacro
	j	$L175
	addu	$2,$2,1
	.set	macro
	.set	reorder

$L137:
	li	$2,0x10000000		# 268435456
	ori	$2,$2,0x1000
	and	$2,$3,$2
	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L138
	li	$5,0x0000007f		# 127
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	jal	func_8005C650
	li	$6,0x0000007f		# 127
	.set	macro
	.set	reorder

	lhu	$2,D_800A359C
	#nop
	addu	$2,$2,-1
$L175:
	sh	$2,D_800A359C
$L138:
	lh	$3,D_800A359C
	#nop
	slt	$2,$3,3
	bne	$2,$0,$L140
	sh	$0,D_800A359C
	j	$L141
$L140:
	.set	noreorder
	.set	nomacro
	bgez	$3,$L141
	li	$2,0x00000002		# 2
	.set	macro
	.set	reorder

	sh	$2,D_800A359C
$L141:
	lw	$2,D_800A35C4
	#nop
	lw	$4,8($2)
	#nop
	andi	$4,$4,0x001f
	sll	$4,$4,7
	.set	noreorder
	.set	nomacro
	jal	rsin
	addu	$4,$4,511
	.set	macro
	.set	reorder

	sll	$3,$2,6
	subu	$3,$3,$2
	sra	$3,$3,12
	addu	$3,$3,-64
	li	$2,0x00000100		# 256
	sw	$2,56($sp)
	sw	$2,60($sp)
	li	$2,0x00000001		# 1
	sb	$3,67($sp)
	sb	$3,66($sp)
	sb	$3,65($sp)
	sw	$0,52($sp)
	sw	$0,48($sp)
	sw	$2,44($sp)
	lw	$2,8($fp)
	move	$17,$0
	addu	$18,$2,12
	sw	$2,24($sp)
	sw	$18,28($sp)
	lw	$2,4($20)
	move	$22,$fp
	addu	$4,$sp,24
	move	$5,$0
	.set	noreorder
	.set	nomacro
	jal	func_80073728
	sw	$2,36($sp)
	.set	macro
	.set	reorder

	move	$4,$20
	addu	$5,$sp,72
	li	$6,0x00000001		# 1
	sw	$2,4($20)
	li	$2,0x00000111		# 273
	sh	$2,76($sp)
	li	$2,0x000000b7		# 183
	sh	$2,72($sp)
	li	$2,0x00000025		# 37
	sh	$2,74($sp)
	li	$2,0x00000001		# 1
	.set	noreorder
	.set	nomacro
	jal	func_80069898
	sh	$2,78($sp)
	.set	macro
	.set	reorder

	li	$2,0x0000000a		# 10
	sw	$2,44($sp)
$L146:
	move	$16,$0
	sll	$21,$17,1
	move	$19,$22
$L150:
	lh	$2,D_800A359C
	#nop
	bne	$17,$2,$L151
	lh	$2,D_800A3598
	#nop
	bne	$16,$2,$L151
	lbu	$2,D_800A3578
	#nop
	.set	noreorder
	.set	nomacro
	bne	$2,$0,$L151
	li	$2,0x00000001		# 1
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	j	$L152
	sb	$2,64($sp)
	.set	macro
	.set	reorder

$L151:
	sb	$0,64($sp)
$L152:
	addu	$3,$21,$16
	li	$2,0x00000003		# 3
	.set	noreorder
	.set	nomacro
	bne	$3,$2,$L154
	li	$2,0x00000006		# 6
	.set	macro
	.set	reorder

	lw	$3,D_800A35BC
	#nop
	.set	noreorder
	.set	nomacro
	bne	$3,$2,$L154
	li	$2,0x00000002		# 2
	.set	macro
	.set	reorder

	beq	$23,$2,$L153
$L154:
	lw	$2,12($19)
	.set	noreorder
	.set	nomacro
	j	$L181
	sw	$2,24($sp)
	.set	macro
	.set	reorder

$L153:
	lw	$2,48($fp)
	#nop
	sw	$2,24($sp)
$L181:
	lw	$2,24($sp)
	addu	$4,$sp,24
	addu	$18,$2,12
	sw	$18,28($sp)
	lw	$2,4($20)
	move	$5,$0
	addu	$19,$19,4
	addu	$16,$16,1
	.set	noreorder
	.set	nomacro
	jal	func_80073728
	sw	$2,36($sp)
	.set	macro
	.set	reorder

	sw	$2,4($20)
	slt	$2,$16,2
	bne	$2,$0,$L150
	addu	$17,$17,1
	slt	$2,$17,4
	.set	noreorder
	.set	nomacro
	bne	$2,$0,$L146
	addu	$22,$22,8
	.set	macro
	.set	reorder

	sb	$0,64($sp)
	lw	$2,44($fp)
	#nop
	addu	$18,$2,12
	sw	$2,24($sp)
	sw	$18,28($sp)
	lw	$2,4($20)
	addu	$4,$sp,24
	move	$5,$0
	.set	noreorder
	.set	nomacro
	jal	func_80073728
	sw	$2,36($sp)
	.set	macro
	.set	reorder

	lh	$3,D_800A3578
	#nop
	.set	noreorder
	.set	nomacro
	bne	$3,$0,$L169
	sw	$2,4($20)
	.set	macro
	.set	reorder

	lw	$2,D_800A35B0
	#nop
	addu	$2,$2,1
	.set	noreorder
	.set	nomacro
	blez	$2,$L160
	move	$17,$0
	.set	macro
	.set	reorder

	li	$19,0x00000010		# 16
	li	$18,0x000000ff		# 255
	la	$21,D_800A35C8
	move	$16,$0
$L162:
	lw	$3,D_800A354C
	sll	$2,$17,4
	sll	$2,$19,$2
	and	$3,$3,$2
	.set	noreorder
	.set	nomacro
	beq	$3,$0,$L161
	li	$4,0x00000002		# 2
	.set	macro
	.set	reorder

	li	$5,0x0000007f		# 127
	.set	noreorder
	.set	nomacro
	jal	func_8005C650
	li	$6,0x0000007f		# 127
	.set	macro
	.set	reorder

	lbu	$3,D_800A3560($16)
	li	$2,0x00000005		# 5
	sb	$18,D_800A3560+2($16)
	beq	$3,$2,$L165
	.set	noreorder
	.set	nomacro
	bne	$3,$19,$L164
	li	$2,0x00000001		# 1
	.set	macro
	.set	reorder

$L165:
	sltu	$2,$17,1
	subu	$2,$0,$2
	andi	$2,$2,0x0003
	sh	$0,D_800A3580
	sb	$18,D_800A3560+2($2)
	sb	$18,D_800A3560($16)
	j	$L168
$L164:
	sh	$2,D_800A3580
$L168:
	lw	$2,D_800A35C4
	li	$3,0x0000000f		# 15
	sh	$3,0($21)
	li	$3,0x00000014		# 20
	sh	$3,2($21)
	sh	$0,6($2)
	.set	noreorder
	.set	nomacro
	j	$L169
	sh	$0,4($2)
	.set	macro
	.set	reorder

$L161:
	lw	$2,D_800A35B0
	addu	$17,$17,1
	addu	$2,$2,1
	slt	$2,$17,$2
	.set	noreorder
	.set	nomacro
	bne	$2,$0,$L162
	addu	$16,$16,3
	.set	macro
	.set	reorder

$L160:
	lw	$2,D_800A354C
	li	$3,0x00400000		# 4194304
	ori	$3,$3,0x0040
	and	$2,$2,$3
	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L169
	li	$4,0x00000001		# 1
	.set	macro
	.set	reorder

	lh	$2,D_800A359C
	lh	$3,D_800A3580
	lh	$5,D_800A3598
	lw	$12,88($sp)
	li	$6,0x0000007f		# 127
	sll	$2,$2,1
	sll	$3,$3,3
	addu	$2,$2,$3
	addu	$2,$2,$5
	addu	$2,$2,$12
	lbu	$16,-32($2)
	.set	noreorder
	.set	nomacro
	jal	func_8005C650
	li	$5,0x0000007f		# 127
	.set	macro
	.set	reorder

	li	$2,0x0000000d		# 13
	andi	$16,$16,0x00ff
	.set	noreorder
	.set	nomacro
	bne	$16,$2,$L173
	li	$2,0x00000006		# 6
	.set	macro
	.set	reorder

	lw	$3,D_800A35BC
	#nop
	.set	noreorder
	.set	nomacro
	beq	$3,$2,$L172
	li	$4,-1009			# 0xfffffc0f
	.set	macro
	.set	reorder

$L173:
	lw	$4,D_800A3568
	#nop
	lw	$3,20($4)
	li	$2,-1009			# 0xfffffc0f
	and	$3,$3,$2
	andi	$2,$16,0x003f
	sll	$2,$2,4
	or	$3,$3,$2
	.set	noreorder
	.set	nomacro
	j	$L174
	sw	$3,20($4)
	.set	macro
	.set	reorder

$L172:
	lw	$2,D_800A3568
	#nop
	lw	$3,20($2)
	#nop
	and	$3,$3,$4
	ori	$3,$3,0x0250
	sw	$3,20($2)
$L174:
	li	$2,0x00000001		# 1
	sw	$2,D_800A35A0
$L169:
	.set	noreorder
	.set	nomacro
	jal	func_80072E10
	move	$4,$20
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	jal	func_80073200
	move	$4,$20
	.set	macro
	.set	reorder

	jal	func_8005C6D0
	lw	$31,148($sp)
	lw	$fp,144($sp)
	lw	$23,140($sp)
	lw	$22,136($sp)
	lw	$21,132($sp)
	lw	$20,128($sp)
	lw	$19,124($sp)
	lw	$18,120($sp)
	lw	$17,116($sp)
	lw	$16,112($sp)
	addu	$sp,$sp,152
	j	$31
	.end	func_800720FC
