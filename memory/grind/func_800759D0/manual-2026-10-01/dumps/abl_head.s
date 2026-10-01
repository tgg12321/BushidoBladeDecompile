func_800759D0:
	.frame	$sp,120,$31		# vars= 56, regs= 10/0, args= 24, extra= 0
	.mask	0xc0ff0000,-4
	.fmask	0x00000000,0
	subu	$sp,$sp,120
	sw	$16,80($sp)
	move	$16,$4
	sw	$19,92($sp)
	move	$19,$5
	sw	$18,88($sp)
	move	$18,$7
	sll	$2,$18,4
	subu	$2,$2,$18
	sw	$31,116($sp)
	sw	$fp,112($sp)
	sw	$23,108($sp)
	sw	$22,104($sp)
	sw	$21,100($sp)
	sw	$20,96($sp)
	sw	$17,84($sp)
	sw	$6,72($sp)
	sw	$0,40($sp)
	sb	$0,64($sp)
	lw	$3,0($16)
	sll	$2,$2,4
	lw	$22,20($3)
	addu	$2,$2,136
	lw	$3,0($22)
	move	$fp,$0
	sw	$2,48($sp)
	li	$2,0x00000033		# 51
	sw	$2,52($sp)
	addu	$5,$3,12
	sw	$3,24($sp)
	.set	noreorder
	.set	nomacro
	beq	$18,$0,.L141
	sw	$5,28($sp)
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	j	.L172
	li	$2,0x00000016		# 22
	.set	macro
	.set	reorder

.L141:
	li	$2,0x0000000c		# 12
.L172:
	.set	noreorder
	.set	nomacro
	beq	$19,$0,.L143
	sw	$2,44($sp)
	.set	macro
	.set	reorder

	lw	$2,24($sp)
	#nop
	lbu	$2,2($2)
	lw	$3,28($sp)
	sll	$2,$2,3
	addu	$3,$3,$2
	sw	$3,28($sp)
.L143:
	lw	$2,16($16)
	addu	$4,$sp,24
	.set	noreorder
	.set	nomacro
	jal	func_8007352C
	sw	$2,32($sp)
	.set	macro
	.set	reorder

	sw	$2,16($16)
	lw	$4,24($sp)
	.set	noreorder
	.set	nomacro
	jal	func_8006E480
	move	$5,$fp
	.set	macro
	.set	reorder

	li	$5,0x00000001		# 1
	move	$6,$0
	sw	$0,16($sp)
	lw	$4,24($16)
	.set	noreorder
	.set	nomacro
	jal	SetDrawMode
	move	$7,$2
	.set	macro
	.set	reorder

	lw	$5,24($16)
	lw	$4,44($sp)
	lw	$2,g_gpu_ot_ptr
	sll	$4,$4,2
	.set	noreorder
	.set	nomacro
	jal	AddPrim
	addu	$4,$2,$4
	.set	macro
	.set	reorder

	lw	$2,24($16)
	lw	$3,D_800A36A0
	addu	$2,$2,12
	sw	$2,24($16)
	lhu	$4,52($3)
	#nop
	andi	$4,$4,0x001f
	sll	$4,$4,7
	.set	noreorder
	.set	nomacro
	jal	rsin
	addu	$4,$4,511
	.set	macro
	.set	reorder

	sll	$2,$2,5
	sra	$2,$2,12
	addu	$2,$2,-80
	sb	$2,67($sp)
	sb	$2,66($sp)
	.set	noreorder
	.set	nomacro
	beq	$18,$0,.L144
	sb	$2,65($sp)
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	j	.L173
	li	$2,0x00000014		# 20
	.set	macro
	.set	reorder

.L144:
	li	$2,0x0000000a		# 10
.L173:
	sw	$2,44($sp)
	sll	$2,$18,4
	subu	$2,$2,$18
	sll	$2,$2,4
	sw	$2,48($sp)
	sll	$7,$19,2
	addu	$2,$7,$19
	sll	$3,$2,1
	move	$17,$3
	sll	$2,$2,17
	sra	$2,$2,16
	addu	$3,$3,10
	slt	$2,$2,$3
	beq	$2,$0,.L147
	la	$21,D_8009BCF8
	sll	$20,$18,1
	addu	$2,$20,$18
	sll	$2,$2,2
	addu	$23,$2,12
	sll	$2,$17,16
.L177:
	sra	$5,$2,16
	sll	$2,$5,1
	addu	$8,$2,$21
	lbu	$3,0($8)
	#nop
	lbu	$2,D_8009BCE4($3)
	#nop
	andi	$2,$2,0x0001
	.set	noreorder
	.set	nomacro
	beq	$2,$0,.L150
	sll	$2,$3,2
	.set	macro
	.set	reorder

	addu	$2,$2,$22
	lw	$6,4($2)
	lw	$2,D_800A36A0
	addu	$5,$6,36
	addu	$2,$20,$2
	sw	$6,24($sp)
	sw	$5,28($sp)
	lh	$4,28($2)
	lh	$2,32($2)
	sll	$3,$4,2
	addu	$3,$3,$4
	addu	$3,$3,$2
	sll	$3,$3,1
	addu	$2,$7,$19
	sll	$2,$2,2
	addu	$3,$3,$2
	lbu	$3,D_8009BCF8($3)
	lbu	$2,0($8)
	#nop
	.set	noreorder
	.set	nomacro
	bne	$3,$2,.L151
	li	$2,0x00000001		# 1
	.set	macro
	.set	reorder

	sb	$2,64($sp)
	addu	$2,$6,$23
	.set	noreorder
	.set	nomacro
	j	.L152
	sw	$2,24($sp)
	.set	macro
	.set	reorder

.L151:
	sb	$0,64($sp)
.L152:
	sw	$0,52($sp)
	lw	$2,16($16)
	addu	$4,$sp,24
	.set	noreorder
	.set	nomacro
	jal	func_8007352C
	sw	$2,32($sp)
	.set	macro
	.set	reorder

	sw	$2,16($16)
	sll	$2,$17,16
	sra	$5,$2,16
	sll	$2,$5,1
	addu	$2,$2,$21
	lbu	$3,0($2)
	li	$2,0x00000004		# 4
	lbu	$3,D_8009BCE4($3)
	sll	$2,$2,$18
	and	$3,$3,$2
	.set	noreorder
	.set	nomacro
	beq	$3,$0,.L176
	addu	$3,$17,1
	.set	macro
	.set	reorder

	move	$4,$16
	move	$6,$18
	.set	noreorder
	.set	nomacro
	j	.L174
	li	$7,0x00000001		# 1
	.set	macro
	.set	reorder

.L150:
	move	$4,$16
	move	$6,$18
	move	$7,$0
.L174:
	jal	func_80075830
	addu	$3,$17,1
.L176:
	move	$17,$3
	sll	$7,$19,2
	sll	$3,$3,16
	sra	$3,$3,16
	addu	$2,$7,$19
	sll	$2,$2,1
	addu	$2,$2,10
	slt	$3,$3,$2
	.set	noreorder
	.set	nomacro
	bne	$3,$0,.L177
	sll	$2,$17,16
	.set	macro
	.set	reorder

.L147:
	lw	$5,D_800A36A0
	sll	$4,$18,1
	addu	$2,$4,$5
	lh	$2,60($2)
	#nop
	addu	$2,$2,1
	.set	noreorder
	.set	nomacro
	blez	$2,.L179
	move	$17,$0
	.set	macro
	.set	reorder

	sll	$2,$17,16
.L178:
	sra	$6,$2,16
	lw	$9,72($sp)
	sll	$2,$6,1
	addu	$2,$2,$9
	lh	$2,0($2)
	#nop
	.set	noreorder
	.set	nomacro
	bltz	$2,.L158
	sll	$2,$2,2
	.set	macro
	.set	reorder

	addu	$2,$2,$22
	lw	$3,4($2)
	addu	$2,$4,$5
	sw	$3,24($sp)
	lh	$2,60($2)
	#nop
	.set	noreorder
	.set	nomacro
	beq	$6,$2,.L161
	addu	$5,$3,36
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	j	.L162
	sb	$0,64($sp)
	.set	macro
	.set	reorder

.L161:
	li	$2,0x00000001		# 1
	sb	$2,64($sp)
	addu	$2,$4,$18
	sll	$2,$2,2
	addu	$2,$2,12
	addu	$2,$3,$2
	sw	$2,24($sp)
.L162:
	sll	$3,$17,16
	sra	$3,$3,16
	sll	$2,$3,4
	lw	$4,24($sp)
	addu	$2,$2,$3
	sw	$2,52($sp)
	sw	$5,28($sp)
	lbu	$2,2($4)
	#nop
	sll	$2,$2,3
	addu	$2,$5,$2
	sw	$2,28($sp)
	lw	$2,16($16)
	addu	$4,$sp,24
	.set	noreorder
	.set	nomacro
	jal	func_8007352C
	sw	$2,32($sp)
	.set	macro
	.set	reorder

	sw	$2,16($16)
.L158:
	addu	$2,$17,1
	move	$17,$2
	sll	$4,$18,1
	lw	$5,D_800A36A0
	sll	$2,$2,16
	addu	$3,$4,$5
	lh	$3,60($3)
	sra	$2,$2,16
	addu	$3,$3,1
	slt	$2,$2,$3
	.set	noreorder
	.set	nomacro
	bne	$2,$0,.L178
	sll	$2,$17,16
	.set	macro
	.set	reorder

	lw	$5,D_800A36A0
.L179:
	sb	$0,64($sp)
	lbu	$2,101($5)
	#nop
	addu	$2,$2,3
	.set	noreorder
	.set	nomacro
	beq	$2,$0,.L165
	move	$17,$0
	.set	macro
	.set	reorder

	sll	$19,$18,1
	addu	$2,$19,$18
	sll	$2,$2,2
	addu	$21,$2,12
	sll	$2,$18,4
	subu	$2,$2,$18
	sll	$20,$2,4
.L167:
	lbu	$2,101($5)
	lw	$3,0($16)
	sll	$2,$2,2
	addu	$2,$2,$3
	lw	$3,32($2)
	sll	$2,$17,16
	sra	$4,$2,16
	sll	$2,$4,2
	addu	$2,$2,$3
	lw	$3,0($2)
	addu	$2,$19,$5
	sw	$3,24($sp)
	lh	$2,60($2)
	#nop
	.set	noreorder
	.set	nomacro
	bne	$4,$2,.L168
	addu	$5,$3,36
	.set	macro
	.set	reorder

	addu	$2,$3,$21
	sw	$2,24($sp)
.L168:
	sll	$2,$4,4
	addu	$2,$2,$4
	sw	$5,28($sp)
	sw	$20,48($sp)
	.set	noreorder
	.set	nomacro
	beq	$18,$0,.L169
	sw	$2,52($sp)
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	j	.L175
	li	$2,0x00000014		# 20
	.set	macro
	.set	reorder

.L169:
	li	$2,0x0000000a		# 10
.L175:
	sw	$2,44($sp)
	lw	$2,16($16)
	addu	$4,$sp,24
	.set	noreorder
	.set	nomacro
	jal	func_8007352C
	sw	$2,32($sp)
	.set	macro
	.set	reorder

	addu	$3,$17,1
	move	$17,$3
	lw	$5,D_800A36A0
	sll	$3,$3,16
	sw	$2,16($16)
	lbu	$2,101($5)
	sra	$3,$3,16
	addu	$2,$2,3
	slt	$3,$3,$2
	bne	$3,$0,.L167
.L165:
	lw	$2,0($16)
	#nop
	lw	$3,20($2)
	#nop
	lw	$4,4($3)
	move	$5,$fp
	.set	noreorder
	.set	nomacro
	jal	func_8006E480
	sw	$4,24($sp)
	.set	macro
	.set	reorder

	li	$5,0x00000001		# 1
	move	$6,$0
	sw	$0,16($sp)
	lw	$4,24($16)
	.set	noreorder
	.set	nomacro
	jal	SetDrawMode
	move	$7,$2
	.set	macro
	.set	reorder

	lw	$5,24($16)
	lw	$4,44($sp)
	lw	$2,g_gpu_ot_ptr
	sll	$4,$4,2
	.set	noreorder
	.set	nomacro
	jal	AddPrim
	addu	$4,$2,$4
	.set	macro
	.set	reorder

	lw	$2,24($16)
	#nop
	addu	$2,$2,12
	sw	$2,24($16)
	lw	$4,24($sp)
	.set	noreorder
	.set	nomacro
	jal	func_8006E480
	move	$5,$fp
	.set	macro
	.set	reorder

	li	$5,0x00000001		# 1
	move	$6,$0
	sw	$0,16($sp)
	lw	$4,24($16)
	.set	noreorder
	.set	nomacro
	jal	SetDrawMode
	move	$7,$2
	.set	macro
	.set	reorder

	lw	$5,24($16)
	lw	$2,44($sp)
	lw	$4,g_gpu_ot_ptr
	sll	$2,$2,2
	addu	$4,$4,$2
	.set	noreorder
	.set	nomacro
	jal	AddPrim
	addu	$4,$4,-4
	.set	macro
	.set	reorder

	lw	$2,24($16)
	#nop
	addu	$2,$2,12
	sw	$2,24($16)
	lw	$31,116($sp)
	lw	$fp,112($sp)
	lw	$23,108($sp)
	lw	$22,104($sp)
	lw	$21,100($sp)
	lw	$20,96($sp)
	lw	$19,92($sp)
	lw	$18,88($sp)
	lw	$17,84($sp)
	lw	$16,80($sp)
	addu	$sp,$sp,120
	j	$31
	.end	func_800759D0
