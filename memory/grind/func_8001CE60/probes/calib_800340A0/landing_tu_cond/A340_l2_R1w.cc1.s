func_800340A0:
	.frame	$sp,0,$31		# vars= 0, regs= 0/0, args= 0, extra= 0
	.mask	0x00000000,0
	.fmask	0x00000000,0
	lbu	$6,g_sc
	lbu	$7,D_800A37F8
	andi	$4,$6,0x00ff
	andi	$3,$7,0x00ff
	beq	$4,$3,.L1189
	lbu	$5,g_sc+1
	andi	$2,$5,0x00ff
	.set	noreorder
	.set	nomacro
	beq	$2,$3,.L1188
	sltu	$2,$2,$4
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	beq	$2,$0,.L1199
	andi	$2,$5,0x00ff
	.set	macro
	.set	reorder

.L1189:
	lbu	$2,D_800A3874
	sb	$0,D_800A377C($2)
	j	.L1190
.L1188:
	andi	$2,$5,0x00ff
.L1199:
	.set	noreorder
	.set	nomacro
	beq	$2,$7,.L1192
	sltu	$2,$6,$2
	.set	macro
	.set	reorder

	beq	$2,$0,.L1191
.L1192:
	lbu	$3,D_800A3874
	.set	noreorder
	.set	nomacro
	j	.L1198
	li	$2,0x00000001		# 1
	.set	macro
	.set	reorder

.L1191:
	lbu	$4,g_tb
	lbu	$3,g_tb+1
	sltu	$2,$4,$3
	.set	noreorder
	.set	nomacro
	beq	$2,$0,.L1194
	addu	$2,$6,1
	.set	macro
	.set	reorder

	lbu	$3,D_800A3874
	sb	$2,g_sc
	sb	$0,D_800A377C($3)
	j	.L1190
.L1194:
	sltu	$2,$3,$4
	.set	noreorder
	.set	nomacro
	beq	$2,$0,.L1196
	addu	$2,$5,1
	.set	macro
	.set	reorder

	lbu	$3,D_800A3874
	sb	$2,g_sc+1
	.set	noreorder
	.set	nomacro
	j	.L1198
	li	$2,0x00000001		# 1
	.set	macro
	.set	reorder

.L1196:
	lbu	$3,D_800A3874
	li	$2,0x00000002		# 2
.L1198:
	sb	$2,D_800A377C($3)
.L1190:
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
