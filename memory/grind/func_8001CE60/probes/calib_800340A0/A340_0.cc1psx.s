func_800340A0:
	.frame	$sp,0,$31		# vars= 0, regs= 0/0, args= 0, extra= 0
	.mask	0x00000000,0
	.fmask	0x00000000,0
	lbu	$6,D_800A3898
	lbu	$2,D_800A37F8
	andi	$5,$6,0x00ff
	beq	$5,$2,$L1083
	lbu	$3,D_800A3899
	andi	$4,$3,0x00ff
	.set	noreorder
	.set	nomacro
	bne	$4,$2,$L1073
	sltu	$2,$4,$5
	.set	macro
	.set	reorder

	lbu	$3,D_800A3874
	.set	noreorder
	.set	nomacro
	j	$L1084
	li	$2,0x00000001		# 1
	.set	macro
	.set	reorder

$L1073:
	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L1075
	sltu	$2,$5,$4
	.set	macro
	.set	reorder

$L1083:
	lbu	$2,D_800A3874
	sb	$0,D_800A377C($2)
	j	$L1072
$L1075:
	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L1077
	li	$2,0x00000001		# 1
	.set	macro
	.set	reorder

	lbu	$3,D_800A3874
	j	$L1084
$L1077:
	lbu	$5,D_800A38AA
	lbu	$4,D_800A38AB
	sltu	$2,$5,$4
	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L1079
	addu	$2,$6,1
	.set	macro
	.set	reorder

	lbu	$3,D_800A3874
	sb	$2,D_800A3898
	sb	$0,D_800A377C($3)
	j	$L1072
$L1079:
	sltu	$2,$4,$5
	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L1081
	addu	$3,$3,1
	.set	macro
	.set	reorder

	lbu	$2,D_800A3874
	sb	$3,D_800A3899
	li	$3,0x00000001		# 1
	sb	$3,D_800A377C($2)
	j	$L1072
$L1081:
	lbu	$3,D_800A3874
	li	$2,0x00000002		# 2
$L1084:
	sb	$2,D_800A377C($3)
$L1072:
	lbu	$2,D_800A3874
	lbu	$3,D_800A3898
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
