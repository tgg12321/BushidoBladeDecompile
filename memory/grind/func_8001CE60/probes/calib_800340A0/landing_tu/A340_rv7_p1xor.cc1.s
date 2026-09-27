func_800340A0:
	.frame	$sp,0,$31		# vars= 0, regs= 0/0, args= 0, extra= 0
	.mask	0x00000000,0
	.fmask	0x00000000,0
	lbu	$3,D_800A3874
	xor	$2,$3,$3
	andi	$7,$2,0x00ff
	lbu	$10,g_sc($7)
	lbu	$6,D_800A37F8
	andi	$5,$10,0x00ff
	.set	noreorder
	.set	nomacro
	beq	$5,$6,.L1200
	ori	$9,$2,0x0001
	.set	macro
	.set	reorder

	lbu	$8,g_sc($9)
	andi	$4,$8,0x00ff
	.set	noreorder
	.set	nomacro
	bne	$4,$6,.L1190
	sltu	$2,$4,$5
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
	sltu	$2,$5,$4
	.set	macro
	.set	reorder

.L1200:
	move	$2,$3
	sb	$0,D_800A377C($2)
	j	.L1189
.L1192:
	.set	noreorder
	.set	nomacro
	bne	$2,$0,.L1201
	li	$2,0x00000001		# 1
	.set	macro
	.set	reorder

	lbu	$5,g_tb($7)
	lbu	$4,g_tb+1
	sltu	$2,$5,$4
	.set	noreorder
	.set	nomacro
	beq	$2,$0,.L1196
	addu	$2,$10,1
	.set	macro
	.set	reorder

	sb	$2,g_sc($7)
	lbu	$2,D_800A3874
	sb	$0,D_800A377C($2)
	j	.L1189
.L1196:
	sltu	$2,$4,$5
	.set	noreorder
	.set	nomacro
	beq	$2,$0,.L1198
	addu	$2,$8,1
	.set	macro
	.set	reorder

	sb	$2,g_sc($9)
	lbu	$3,D_800A3874
	.set	noreorder
	.set	nomacro
	j	.L1201
	li	$2,0x00000001		# 1
	.set	macro
	.set	reorder

.L1198:
	li	$2,0x00000002		# 2
.L1201:
	sb	$2,D_800A377C($3)
.L1189:
	lbu	$2,D_800A3874
	xor	$3,$2,$2
	lbu	$3,g_sc($3)
	sll	$2,$2,1
	sb	$3,D_800F65F8($2)
	lbu	$3,D_800A3874
	xor	$2,$3,$3
	ori	$2,$2,0x0001
	lbu	$2,g_sc($2)
	sll	$3,$3,1
	sb	$2,D_800F65F9($3)
	lbu	$2,D_800A3874
	addu	$2,$2,1
	sb	$2,D_800A3874
	j	$31
	.end	func_800340A0
