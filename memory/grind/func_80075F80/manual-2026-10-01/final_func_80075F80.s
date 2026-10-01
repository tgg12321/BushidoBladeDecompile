func_80075F80:
	.frame	$sp,72,$31		# vars= 24, regs= 7/0, args= 16, extra= 0
	.mask	0x803f0000,-8
	.fmask	0x00000000,0
	subu	$sp,$sp,72
	sw	$4,72($sp)
	lw	$3,D_800A36A0
	sw	$21,60($sp)
	move	$21,$5
	sw	$16,40($sp)
	move	$16,$7
	sw	$17,44($sp)
	sll	$17,$16,1
	sw	$31,64($sp)
	sw	$20,56($sp)
	sw	$19,52($sp)
	sw	$18,48($sp)
	addu	$2,$17,$3
	lh	$2,16($2)
	#nop
	.set	noreorder
	.set	nomacro
	bne	$2,$0,.L168
	move	$20,$6
	.set	macro
	.set	reorder

	sll	$18,$16,4
	li	$2,0x00000010		# 16
	sll	$2,$2,$18
	and	$2,$4,$2
	.set	noreorder
	.set	nomacro
	beq	$2,$0,.L170
	li	$4,0x00000002		# 2
	.set	macro
	.set	reorder

	li	$5,0x0000007f		# 127
	.set	noreorder
	.set	nomacro
	jal	func_8005C650
	li	$6,0x0000007f		# 127
	.set	macro
	.set	reorder

	lw	$4,D_800A36A0
	#nop
	addu	$5,$17,$4
	lh	$2,60($5)
	#nop
	.set	noreorder
	.set	nomacro
	beq	$2,$0,.L171
	sll	$2,$2,1
	.set	macro
	.set	reorder

	addu	$2,$2,$20
	li	$3,-1			# 0xffffffff
	sh	$3,0($2)
	lhu	$2,60($5)
	li	$3,0x00000004		# 4
	addu	$2,$2,-1
	sh	$2,60($5)
	sll	$2,$2,16
	sra	$2,$2,15
	addu	$2,$2,$20
	lh	$4,0($2)
	sll	$3,$3,$16
	lbu	$2,D_8009BCE4($4)
	nor	$3,$0,$3
	and	$2,$2,$3
	sb	$2,D_8009BCE4($4)
	j	.L168
.L171:
	lw	$2,60($4)
	#nop
	bne	$2,$0,.L168
	.set	noreorder
	.set	nomacro
	bne	$16,$0,.L174
	addu	$2,$4,20
	.set	macro
	.set	reorder

	addu	$2,$4,22
.L174:
	lh	$3,0($2)
	li	$2,0x00000002		# 2
	.set	noreorder
	.set	nomacro
	bne	$3,$2,.L168
	li	$2,0x00000003		# 3
	.set	macro
	.set	reorder

	sh	$2,18($4)
	sh	$2,16($4)
	li	$2,0x00000001		# 1
	sh	$2,26($4)
	.set	noreorder
	.set	nomacro
	j	.L168
	sh	$2,24($4)
	.set	macro
	.set	reorder

.L170:
	addu	$4,$sp,72
	move	$5,$16
	sll	$2,$16,2
	addu	$6,$2,64
	addu	$6,$3,$6
	la	$7,D_800A35D0
	.set	noreorder
	.set	nomacro
	jal	func_800692C0
	addu	$7,$2,$7
	.set	macro
	.set	reorder

	move	$19,$2
	li	$2,0x0000f000		# 61440
	lw	$3,72($sp)
	sll	$2,$2,$18
	and	$3,$3,$2
	.set	noreorder
	.set	nomacro
	beq	$3,$0,.L175
	move	$4,$0
	.set	macro
	.set	reorder

	li	$5,0x0000007f		# 127
	.set	noreorder
	.set	nomacro
	jal	func_8005C650
	li	$6,0x0000007f		# 127
	.set	macro
	.set	reorder

.L175:
	andi	$3,$19,0x00ff
	slt	$2,$3,3
	beq	$2,$0,.L176
	.set	noreorder
	.set	nomacro
	beq	$3,$0,.L200
	sra	$3,$19,16
	.set	macro
	.set	reorder

	lw	$3,D_800A36A0
	#nop
	addu	$3,$17,$3
	lhu	$2,28($3)
	#nop
	addu	$2,$2,1
	andi	$2,$2,0x0001
	sh	$2,28($3)
.L176:
	sra	$3,$19,16
.L200:
	li	$2,0x00000001		# 1
	.set	noreorder
	.set	nomacro
	beq	$3,$2,.L178
	li	$2,0x00000002		# 2
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	beq	$3,$2,.L181
	sll	$3,$16,1
	.set	macro
	.set	reorder

	j	.L177
.L178:
	sll	$2,$16,1
	lw	$3,D_800A36A0
	#nop
	addu	$4,$2,$3
	lh	$3,32($4)
	li	$2,0x00000004		# 4
	.set	noreorder
	.set	nomacro
	bne	$3,$2,.L179
	move	$5,$3
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	j	.L177
	sh	$0,32($4)
	.set	macro
	.set	reorder

.L179:
	addu	$2,$5,1
	.set	noreorder
	.set	nomacro
	j	.L177
	sh	$2,32($4)
	.set	macro
	.set	reorder

.L181:
	lw	$2,D_800A36A0
	#nop
	addu	$3,$3,$2
	lh	$2,32($3)
	#nop
	.set	noreorder
	.set	nomacro
	bne	$2,$0,.L182
	move	$4,$2
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	j	.L199
	li	$2,0x00000004		# 4
	.set	macro
	.set	reorder

.L182:
	addu	$2,$4,-1
.L199:
	sh	$2,32($3)
.L177:
	lw	$2,D_800A36A0
	sll	$19,$16,1
	addu	$5,$19,$2
	lh	$2,28($5)
	lh	$4,32($5)
	sll	$3,$2,2
	addu	$3,$3,$2
	addu	$3,$3,$4
	sll	$3,$3,1
	sll	$2,$21,2
	addu	$2,$2,$21
	sll	$2,$2,2
	addu	$3,$3,$2
	lbu	$4,D_8009BCF8($3)
	la	$2,D_8009BCE4
	addu	$17,$4,$2
	lbu	$3,0($17)
	#nop
	andi	$2,$3,0x0001
	.set	noreorder
	.set	nomacro
	beq	$2,$0,.L186
	li	$2,0x00000004		# 4
	.set	macro
	.set	reorder

	sll	$18,$2,$16
	and	$2,$3,$18
	.set	noreorder
	.set	nomacro
	bne	$2,$0,.L201
	li	$3,0x00000014		# 20
	.set	macro
	.set	reorder

	lh	$2,60($5)
	#nop
	sll	$2,$2,1
	addu	$2,$2,$20
	sh	$4,0($2)
	sll	$4,$16,4
	li	$2,0x00000040		# 64
	lw	$3,72($sp)
	sll	$2,$2,$4
	and	$3,$3,$2
	.set	noreorder
	.set	nomacro
	beq	$3,$0,.L168
	li	$4,0x00000001		# 1
	.set	macro
	.set	reorder

	li	$5,0x0000007f		# 127
	.set	noreorder
	.set	nomacro
	jal	func_8005C650
	li	$6,0x0000007f		# 127
	.set	macro
	.set	reorder

	lbu	$2,0($17)
	#nop
	or	$2,$2,$18
	sb	$2,0($17)
	lw	$6,D_800A36A0
	#nop
	addu	$5,$19,$6
	lhu	$7,60($5)
	#nop
	addu	$3,$7,1
	sh	$3,60($5)
	sll	$3,$3,16
	lbu	$2,101($6)
	sra	$3,$3,16
	addu	$2,$2,3
	.set	noreorder
	.set	nomacro
	bne	$3,$2,.L168
	move	$4,$0
	.set	macro
	.set	reorder

	lh	$3,96($5)
	li	$2,0x00000001		# 1
	sh	$2,16($5)
	li	$2,0x00000003		# 3
	sh	$7,60($5)
	sh	$0,56($5)
	.set	noreorder
	.set	nomacro
	blez	$3,.L191
	sh	$2,24($5)
	.set	macro
	.set	reorder

	sll	$2,$16,2
	addu	$2,$2,$16
	sll	$2,$2,1
	addu	$2,$2,$6
	addu	$3,$2,72
.L193:
	sh	$4,0($3)
	lh	$2,96($5)
	addu	$4,$4,1
	slt	$2,$4,$2
	.set	noreorder
	.set	nomacro
	bne	$2,$0,.L193
	addu	$3,$3,2
	.set	macro
	.set	reorder

.L191:
	.set	noreorder
	.set	nomacro
	beq	$21,$0,.L168
	sll	$2,$16,2
	.set	macro
	.set	reorder

	addu	$2,$2,$16
	lw	$3,D_800A36A0
	sll	$2,$2,1
	addu	$3,$3,$2
	li	$2,0x00000005		# 5
	.set	noreorder
	.set	nomacro
	j	.L168
	sh	$2,80($3)
	.set	macro
	.set	reorder

.L186:
	li	$3,0x00000014		# 20
.L201:
	lh	$2,60($5)
	sll	$4,$16,4
	sll	$2,$2,1
	addu	$2,$2,$20
	sh	$3,0($2)
	li	$2,0x00000040		# 64
	lw	$3,72($sp)
	sll	$2,$2,$4
	and	$3,$3,$2
	.set	noreorder
	.set	nomacro
	beq	$3,$0,.L168
	li	$4,0x00000004		# 4
	.set	macro
	.set	reorder

	li	$5,0x0000007f		# 127
	.set	noreorder
	.set	nomacro
	jal	func_8005C650
	li	$6,0x0000007f		# 127
	.set	macro
	.set	reorder

.L168:
	lw	$31,64($sp)
	lw	$21,60($sp)
	lw	$20,56($sp)
	lw	$19,52($sp)
	lw	$18,48($sp)
	lw	$17,44($sp)
	lw	$16,40($sp)
	addu	$sp,$sp,72
	j	$31
	.end	func_80075F80
