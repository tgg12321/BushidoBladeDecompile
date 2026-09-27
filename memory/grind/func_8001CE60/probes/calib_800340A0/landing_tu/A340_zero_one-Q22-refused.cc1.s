func_800340A0:
	.frame	$sp,0,$31		# vars= 0, regs= 0/0, args= 0, extra= 0
	.mask	0x00000000,0
	.fmask	0x00000000,0
	lbu	$6,g_sc
	lbu	$2,D_800A37F8
	andi	$4,$6,0x00ff
	beq	$4,$2,.L1200
	lbu	$5,g_sc+1
	andi	$3,$5,0x00ff
	.set	noreorder
	.set	nomacro
	bne	$3,$2,.L1190
	sltu	$2,$3,$4
	.set	macro
	.set	reorder

	lbu	$3,D_800A3874
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
	lbu	$2,D_800A3874
	sb	$0,D_800A377C($2)
	j	.L1189
.L1192:
	.set	noreorder
	.set	nomacro
	beq	$2,$0,.L1194
	li	$2,0x00000001		# 1
	.set	macro
	.set	reorder

	lbu	$3,D_800A3874
	j	.L1201
.L1194:
	lbu	$4,g_tb
	lbu	$3,g_tb+1
	sltu	$2,$4,$3
	.set	noreorder
	.set	nomacro
	beq	$2,$0,.L1196
	addu	$2,$6,1
	.set	macro
	.set	reorder

	lbu	$3,D_800A3874
	sb	$2,g_sc
	sb	$0,D_800A377C($3)
	j	.L1189
.L1196:
	sltu	$2,$3,$4
	.set	noreorder
	.set	nomacro
	beq	$2,$0,.L1198
	addu	$2,$5,1
	.set	macro
	.set	reorder

	lbu	$3,D_800A3874
	sb	$2,g_sc+1
	.set	noreorder
	.set	nomacro
	j	.L1201
	li	$2,0x00000001		# 1
	.set	macro
	.set	reorder

.L1198:
	lbu	$3,D_800A3874
	li	$2,0x00000002		# 2
.L1201:
	sb	$2,D_800A377C($3)
.L1189:
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
