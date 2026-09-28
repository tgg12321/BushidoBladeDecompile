func_800340A0:
	.frame	$sp,0,$31		# vars= 0, regs= 0/0, args= 0, extra= 0
	.mask	0x00000000,0
	.fmask	0x00000000,0
	lbu	$7,g_sc
	lbu	$3,D_800A37F8
	andi	$4,$7,0x00ff
	beq	$4,$3,$L1183
	lbu	$6,g_sc+1
	andi	$2,$6,0x00ff
	.set	noreorder
	.set	nomacro
	beq	$2,$3,$L1182
	sltu	$2,$2,$4
	.set	macro
	.set	reorder

	beq	$2,$0,$L1182
$L1183:
	lbu	$2,D_800A3874
	sb	$0,D_800A377C($2)
	j	$L1184
$L1182:
	lbu	$2,D_800A37F8
	andi	$4,$6,0x00ff
	.set	noreorder
	.set	nomacro
	bne	$4,$2,$L1185
	sltu	$2,$7,$4
	.set	macro
	.set	reorder

	lbu	$3,D_800A3874
	.set	noreorder
	.set	nomacro
	j	$L1193
	li	$2,0x00000001		# 1
	.set	macro
	.set	reorder

$L1185:
	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L1187
	li	$2,0x00000001		# 1
	.set	macro
	.set	reorder

	lbu	$3,D_800A3874
	j	$L1193
$L1187:
	lbu	$5,g_tb
	lbu	$4,g_tb+1
	sltu	$2,$5,$4
	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L1189
	addu	$2,$7,1
	.set	macro
	.set	reorder

	lbu	$3,D_800A3874
	sb	$2,g_sc
	sb	$0,D_800A377C($3)
	j	$L1184
$L1189:
	sltu	$2,$4,$5
	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L1191
	addu	$3,$6,1
	.set	macro
	.set	reorder

	lbu	$2,D_800A3874
	sb	$3,g_sc+1
	li	$3,0x00000001		# 1
	sb	$3,D_800A377C($2)
	j	$L1184
$L1191:
	lbu	$3,D_800A3874
	li	$2,0x00000002		# 2
$L1193:
	sb	$2,D_800A377C($3)
$L1184:
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
