func_800340A0:
	.frame	$sp,0,$31		# vars= 0, regs= 0/0, args= 0, extra= 0
	.mask	0x00000000,0
	.fmask	0x00000000,0
	lbu	$3,D_800A3874
	xor	$2,$3,$3
	andi	$7,$2,0x00ff
	lbu	$10,D_800A3898($7)
	lbu	$6,D_800A37F8
	andi	$5,$10,0x00ff
	.set	noreorder
	.set	nomacro
	beq	$5,$6,$L1194
	ori	$9,$2,0x0001
	.set	macro
	.set	reorder

	lbu	$8,D_800A3898($9)
	andi	$4,$8,0x00ff
	.set	noreorder
	.set	nomacro
	bne	$4,$6,$L1184
	sltu	$2,$4,$5
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	j	$L1195
	li	$2,0x00000001		# 1
	.set	macro
	.set	reorder

$L1184:
	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L1186
	sltu	$2,$5,$4
	.set	macro
	.set	reorder

$L1194:
	move	$2,$3
	sb	$0,D_800A377C($2)
	j	$L1183
$L1186:
	.set	noreorder
	.set	nomacro
	bne	$2,$0,$L1195
	li	$2,0x00000001		# 1
	.set	macro
	.set	reorder

	lbu	$5,D_800A38AA($7)
	lbu	$4,D_800A38AB
	sltu	$2,$5,$4
	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L1190
	addu	$2,$10,1
	.set	macro
	.set	reorder

	sb	$2,D_800A3898($7)
	lbu	$2,D_800A3874
	sb	$0,D_800A377C($2)
	j	$L1183
$L1190:
	sltu	$2,$4,$5
	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L1192
	addu	$2,$8,1
	.set	macro
	.set	reorder

	sb	$2,D_800A3898($9)
	lbu	$3,D_800A3874
	.set	noreorder
	.set	nomacro
	j	$L1195
	li	$2,0x00000001		# 1
	.set	macro
	.set	reorder

$L1192:
	li	$2,0x00000002		# 2
$L1195:
	sb	$2,D_800A377C($3)
$L1183:
	lbu	$2,D_800A3874
	xor	$3,$2,$2
	lbu	$3,D_800A3898($3)
	sll	$2,$2,1
	sb	$3,D_800F65F8($2)
	lbu	$3,D_800A3874
	xor	$2,$3,$3
	ori	$2,$2,0x0001
	lbu	$2,D_800A3898($2)
	sll	$3,$3,1
	sb	$2,D_800F65F9($3)
	lbu	$2,D_800A3874
	addu	$2,$2,1
	sb	$2,D_800A3874
	j	$31
	.end	func_800340A0
