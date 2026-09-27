func_800340A0:
	.frame	$sp,0,$31		# vars= 0, regs= 0/0, args= 0, extra= 0
	.mask	0x00000000,0
	.fmask	0x00000000,0
	la	$7,g_match_p1_score
	lbu	$6,0($7)
	lbu	$2,D_800A37F8
	andi	$4,$6,0x00ff
	beq	$4,$2,.L1087
	lbu	$5,g_match_p1_score+1
	andi	$3,$5,0x00ff
	.set	noreorder
	.set	nomacro
	bne	$3,$2,.L1077
	sltu	$2,$3,$4
	.set	macro
	.set	reorder

	lbu	$3,D_800A3874
	.set	noreorder
	.set	nomacro
	j	.L1088
	li	$2,0x00000001		# 1
	.set	macro
	.set	reorder

.L1077:
	.set	noreorder
	.set	nomacro
	beq	$2,$0,.L1079
	sltu	$2,$4,$3
	.set	macro
	.set	reorder

.L1087:
	lbu	$2,D_800A3874
	sb	$0,D_800A377C($2)
	j	.L1076
.L1079:
	.set	noreorder
	.set	nomacro
	beq	$2,$0,.L1081
	li	$2,0x00000001		# 1
	.set	macro
	.set	reorder

	lbu	$3,D_800A3874
	j	.L1088
.L1081:
	lbu	$4,g_match_p1_tiebreaker
	lbu	$3,g_match_p1_tiebreaker+1
	sltu	$2,$4,$3
	.set	noreorder
	.set	nomacro
	beq	$2,$0,.L1083
	addu	$2,$6,1
	.set	macro
	.set	reorder

	lbu	$3,D_800A3874
	sb	$2,0($7)
	sb	$0,D_800A377C($3)
	j	.L1076
.L1083:
	sltu	$2,$3,$4
	.set	noreorder
	.set	nomacro
	beq	$2,$0,.L1085
	addu	$2,$5,1
	.set	macro
	.set	reorder

	lbu	$3,D_800A3874
	sb	$2,g_match_p1_score+1
	.set	noreorder
	.set	nomacro
	j	.L1088
	li	$2,0x00000001		# 1
	.set	macro
	.set	reorder

.L1085:
	lbu	$3,D_800A3874
	li	$2,0x00000002		# 2
.L1088:
	sb	$2,D_800A377C($3)
.L1076:
	lbu	$2,D_800A3874
	lbu	$3,g_match_p1_score
	sll	$2,$2,1
	sb	$3,D_800F65F8($2)
	lbu	$2,D_800A3874
	lbu	$3,g_match_p1_score+1
	sll	$2,$2,1
	sb	$3,D_800F65F9($2)
	lbu	$2,D_800A3874
	addu	$2,$2,1
	sb	$2,D_800A3874
	j	$31
	.end	func_800340A0
