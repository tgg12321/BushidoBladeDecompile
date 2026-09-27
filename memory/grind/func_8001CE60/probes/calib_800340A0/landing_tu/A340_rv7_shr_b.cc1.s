func_800340A0:
	.frame	$sp,0,$31		# vars= 0, regs= 0/0, args= 0, extra= 0
	.mask	0x00000000,0
	.fmask	0x00000000,0
	lbu	$6,D_800A3874
	sra	$5,$6,8
	lbu	$9,g_sc($5)
	lbu	$2,D_800A37F8
	andi	$4,$9,0x00ff
	.set	noreorder
	.set	nomacro
	beq	$4,$2,.L1200
	ori	$8,$5,0x0001
	.set	macro
	.set	reorder

	lbu	$7,g_sc($8)
	andi	$3,$7,0x00ff
	.set	noreorder
	.set	nomacro
	bne	$3,$2,.L1190
	sltu	$2,$3,$4
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	j	.L1201
	li	$2,0x00000001		# 1
	.set	macro
	.set	reorder

.L1190:
	.set	noreorder
	.set	nomacro
	beq	$2,$0,.L1192
	sltu	$2,$4,$3
	.set	macro
	.set	reorder

.L1200:
	sb	$0,D_800A377C($6)
	j	.L1189
.L1192:
	.set	noreorder
	.set	nomacro
	bne	$2,$0,.L1201
	li	$2,0x00000001		# 1
	.set	macro
	.set	reorder

	lbu	$2,g_tb($5)
	lbu	$4,g_tb+1
	andi	$3,$2,0x00ff
	sltu	$2,$3,$4
	.set	noreorder
	.set	nomacro
	beq	$2,$0,.L1196
	addu	$2,$9,1
	.set	macro
	.set	reorder

	sb	$2,g_sc($5)
	lbu	$2,D_800A3874
	sb	$0,D_800A377C($2)
	j	.L1189
.L1196:
	sltu	$2,$4,$3
	.set	noreorder
	.set	nomacro
	beq	$2,$0,.L1198
	addu	$2,$7,1
	.set	macro
	.set	reorder

	sb	$2,g_sc($8)
	lbu	$3,D_800A3874
	li	$2,0x00000001		# 1
	sb	$2,D_800A377C($3)
	j	.L1189
.L1198:
	li	$2,0x00000002		# 2
.L1201:
	sb	$2,D_800A377C($6)
.L1189:
	lbu	$2,D_800A3874
	sra	$3,$2,8
	lbu	$3,g_sc($3)
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
