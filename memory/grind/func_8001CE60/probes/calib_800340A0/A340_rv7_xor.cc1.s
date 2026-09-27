func_800340A0:
	.frame	$sp,0,$31		# vars= 0, regs= 0/0, args= 0, extra= 0
	.mask	0x00000000,0
	.fmask	0x00000000,0
	lbu	$2,D_800A37F8
	xor	$3,$2,$2
	andi	$5,$3,0x00ff
	lbu	$8,D_800A3898($5)
	andi	$2,$2,0x00ff
	andi	$4,$8,0x00ff
	.set	noreorder
	.set	nomacro
	beq	$4,$2,.L1200
	xori	$7,$3,0x0001
	.set	macro
	.set	reorder

	lbu	$6,D_800A3898($7)
	andi	$3,$6,0x00ff
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
	bne	$2,$0,.L1200
	sltu	$2,$4,$3
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	beq	$2,$0,.L1194
	li	$2,0x00000001		# 1
	.set	macro
	.set	reorder

	lbu	$3,D_800A3874
	j	.L1201
.L1194:
	lbu	$4,D_800A38AA($5)
	lbu	$3,D_800A38AB
	sltu	$2,$4,$3
	.set	noreorder
	.set	nomacro
	beq	$2,$0,.L1196
	addu	$2,$8,1
	.set	macro
	.set	reorder

	sb	$2,D_800A3898($5)
.L1200:
	lbu	$2,D_800A3874
	sb	$0,D_800A377C($2)
	j	.L1189
.L1196:
	sltu	$2,$3,$4
	.set	noreorder
	.set	nomacro
	beq	$2,$0,.L1198
	addu	$2,$6,1
	.set	macro
	.set	reorder

	sb	$2,D_800A3898($7)
	lbu	$3,D_800A3874
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
	lbu	$2,D_800A37F8
	lbu	$3,D_800A3874
	xor	$2,$2,$2
	lbu	$2,D_800A3898($2)
	sll	$3,$3,1
	sb	$2,D_800F65F8($3)
	lbu	$2,D_800A37F8
	lbu	$3,D_800A3874
	xor	$2,$2,$2
	xori	$2,$2,0x0001
	lbu	$2,D_800A3898($2)
	sll	$3,$3,1
	sb	$2,D_800F65F9($3)
	lbu	$2,D_800A3874
	addu	$2,$2,1
	sb	$2,D_800A3874
	j	$31
	.end	func_800340A0
