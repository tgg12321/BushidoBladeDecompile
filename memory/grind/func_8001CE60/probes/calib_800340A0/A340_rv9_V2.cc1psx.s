func_800340A0:
	.frame	$sp,0,$31		# vars= 0, regs= 0/0, args= 0, extra= 0
	.mask	0x00000000,0
	.fmask	0x00000000,0
	lbu	$6,g_sc
	lbu	$2,D_800A37F8
	andi	$4,$6,0x00ff
	beq	$4,$2,$L1194
	lbu	$5,g_sc+1
	andi	$3,$5,0x00ff
	.set	noreorder
	.set	nomacro
	beq	$3,$2,$L1195
	li	$7,0x00000001		# 1
	.set	macro
	.set	reorder

	sltu	$2,$3,$4
	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L1186
	sltu	$2,$4,$3
	.set	macro
	.set	reorder

$L1194:
	lbu	$2,D_800A3874
	sb	$0,D_800A377C($2)
	j	$L1183
$L1186:
	beq	$2,$0,$L1188
$L1195:
	lbu	$3,D_800A3874
	li	$2,0x00000001		# 1
	sb	$2,D_800A377C($3)
	j	$L1183
$L1188:
	lbu	$4,g_tb
	lbu	$3,g_tb+1
	sltu	$2,$4,$3
	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L1190
	addu	$2,$6,1
	.set	macro
	.set	reorder

	lbu	$3,D_800A3874
	sb	$2,g_sc
	sb	$0,D_800A377C($3)
	j	$L1183
$L1190:
	sltu	$2,$3,$4
	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L1192
	addu	$2,$5,1
	.set	macro
	.set	reorder

	lbu	$3,D_800A3874
	sb	$2,g_sc+1
	.set	noreorder
	.set	nomacro
	j	$L1196
	andi	$2,$3,0x00ff
	.set	macro
	.set	reorder

$L1192:
	li	$7,0x00000002		# 2
	lbu	$3,D_800A3874
	andi	$2,$3,0x00ff
$L1196:
	sb	$7,D_800A377C($2)
$L1183:
	lbu	$2,D_800A3874
	lbu	$3,g_sc
	sll	$2,$2,1
	sb	$3,D_800F65F8($2)
	lbu	$2,D_800A3874
	lbu	$3,g_sc+1
	sll	$2,$2,1
	sb	$3,D_800F65F9($2)
	lbu	$2,D_800A3874
	addu	$2,$2,1
	sb	$2,D_800A3874
	j	$31
	.end	func_800340A0
