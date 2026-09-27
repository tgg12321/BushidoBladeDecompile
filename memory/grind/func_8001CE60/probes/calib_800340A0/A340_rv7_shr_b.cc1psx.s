func_800340A0:
	.frame	$sp,0,$31		# vars= 0, regs= 0/0, args= 0, extra= 0
	.mask	0x00000000,0
	.fmask	0x00000000,0
	lbu	$6,D_800A3874
	sra	$5,$6,8
	lbu	$9,D_800A3898($5)
	lbu	$2,D_800A37F8
	andi	$4,$9,0x00ff
	.set	noreorder
	.set	nomacro
	beq	$4,$2,$L1194
	ori	$8,$5,0x0001
	.set	macro
	.set	reorder

	lbu	$7,D_800A3898($8)
	andi	$3,$7,0x00ff
	.set	noreorder
	.set	nomacro
	bne	$3,$2,$L1184
	sltu	$2,$3,$4
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
	sltu	$2,$4,$3
	.set	macro
	.set	reorder

$L1194:
	sb	$0,D_800A377C($6)
	j	$L1183
$L1186:
	.set	noreorder
	.set	nomacro
	bne	$2,$0,$L1195
	li	$2,0x00000001		# 1
	.set	macro
	.set	reorder

	lbu	$2,D_800A38AA($5)
	lbu	$4,D_800A38AB
	andi	$3,$2,0x00ff
	sltu	$2,$3,$4
	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L1190
	addu	$2,$9,1
	.set	macro
	.set	reorder

	sb	$2,D_800A3898($5)
	lbu	$2,D_800A3874
	sb	$0,D_800A377C($2)
	j	$L1183
$L1190:
	sltu	$2,$4,$3
	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L1192
	addu	$2,$7,1
	.set	macro
	.set	reorder

	sb	$2,D_800A3898($8)
	lbu	$3,D_800A3874
	li	$2,0x00000001		# 1
	sb	$2,D_800A377C($3)
	j	$L1183
$L1192:
	li	$2,0x00000002		# 2
$L1195:
	sb	$2,D_800A377C($6)
$L1183:
	lbu	$2,D_800A3874
	sra	$3,$2,8
	lbu	$3,D_800A3898($3)
	sll	$2,$2,1
	sb	$3,D_800F65F8($2)
	lbu	$2,D_800A3874
	lbu	$3,D_800A3899
	sll	$2,$2,1
	sb	$3,D_800F65F9($2)
	lbu	$2,D_800A3874
	addu	$2,$2,1
	sb	$2,D_800A3874
	j	$31
	.end	func_800340A0
