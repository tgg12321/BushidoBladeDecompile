func_8002AB08:
	.frame	$sp,192,$31		# vars= 120, regs= 10/0, args= 32, extra= 0
	.mask	0xc0ff0000,-4
	.fmask	0x00000000,0
	subu	$sp,$sp,192
	sw	$18,160($sp)
	li	$18,0x1f800000		# 528482304
	ori	$18,$18,0x02b8
	sw	$22,176($sp)
	move	$22,$0
	li	$11,0x1f800000		# 528482304
	ori	$11,$11,0x02d0
	li	$12,0x1f800000		# 528482304
	ori	$12,$12,0x02c4
	sw	$31,188($sp)
	sw	$fp,184($sp)
	sw	$23,180($sp)
	sw	$21,172($sp)
	sw	$20,168($sp)
	sw	$19,164($sp)
	sw	$17,156($sp)
	sw	$16,152($sp)
	sw	$4,40($sp)
	sw	$11,104($sp)
	sw	$12,112($sp)
	sw	$0,136($sp)
.L5:
	lw	$11,40($sp)
	li	$12,0x00000001		# 1
	bne	$11,$12,.L6
	lh	$2,D_800A38AE
	#nop
	beq	$2,$22,.L4
.L6:
	la	$2,g_practice_menu_table
	move	$19,$2
	sll	$2,$22,4
	addu	$2,$2,$22
	sll	$2,$2,2
	addu	$2,$2,$22
	sll	$2,$2,2
	subu	$2,$2,$22
	sll	$2,$2,2
	addu	$2,$2,$19
	.set	noreorder
	.set	nomacro
	bne	$22,$0,.L7
	sw	$2,48($sp)
	.set	macro
	.set	reorder

	addu	$19,$19,1100
.L7:
	sw	$0,56($sp)
	sw	$0,64($sp)
	sw	$0,72($sp)
	lh	$2,150($19)
	move	$9,$0
	sw	$0,32($sp)
	.set	noreorder
	.set	nomacro
	bne	$2,$0,.L8
	sw	$0,36($sp)
	.set	macro
	.set	reorder

	lh	$2,146($19)
	#nop
	beq	$2,$0,.L8
	lw	$2,60($19)
	#nop
	beq	$2,$0,.L8
	lh	$2,12($19)
	#nop
	xori	$2,$2,0x001f
	sltu	$9,$0,$2
.L8:
	lh	$2,140($19)
	#nop
	beq	$2,$0,.L9
	addu	$9,$9,1
.L9:
	beq	$9,$0,.L10
	lhu	$2,14($19)
	#nop
	addu	$2,$2,-4
	sltu	$2,$2,2
	beq	$2,$0,.L10
	addu	$9,$9,1
.L10:
	lhu	$2,14($19)
	#nop
	addu	$2,$2,-6
	sltu	$2,$2,2
	.set	noreorder
	.set	nomacro
	beq	$2,$0,.L11
	li	$11,0x00000002		# 2
	.set	macro
	.set	reorder

	lhu	$3,106($19)
	#nop
	.set	noreorder
	.set	nomacro
	beq	$3,$11,.L12
	li	$2,0x0000001b		# 27
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	beq	$3,$2,.L12
	li	$2,0x00000028		# 40
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	beq	$3,$2,.L12
	li	$2,0x00000026		# 38
	.set	macro
	.set	reorder

	bne	$3,$2,.L11
.L12:
	lh	$3,64($19)
	lbu	$2,161($19)
	#nop
	bne	$3,$2,.L11
	lbu	$2,842($19)
	#nop
	.set	noreorder
	.set	nomacro
	beq	$2,$0,.L13
	addu	$2,$2,-1
	.set	macro
	.set	reorder

	sb	$2,842($19)
	sltu	$2,$22,1
	subu	$2,$0,$2
	andi	$2,$2,0x0024
	lw	$3,528482316($2)
	lw	$4,528482320($2)
	lw	$5,528482324($2)
	sw	$3,0($18)
	sw	$4,4($18)
	sw	$5,8($18)
	lw	$3,528482304($2)
	lw	$4,528482308($2)
	lw	$5,528482312($2)
	sw	$3,12($18)
	sw	$4,16($18)
	sw	$5,20($18)
	addu	$5,$sp,32
	addu	$6,$sp,36
	lw	$4,48($sp)
	move	$7,$0
	.set	noreorder
	.set	nomacro
	jal	func_8002A458
	sw	$9,144($sp)
	.set	macro
	.set	reorder

	lw	$2,32($sp)
	lw	$12,56($sp)
	lw	$9,144($sp)
	or	$12,$12,$2
	.set	noreorder
	.set	nomacro
	j	.L11
	sw	$12,56($sp)
	.set	macro
	.set	reorder

.L13:
	li	$6,0x1f800000		# 528482304
	ori	$6,$6,0x000c
	.set	noreorder
	.set	nomacro
	bne	$22,$0,.L19
	sltu	$4,$22,1
	.set	macro
	.set	reorder

	li	$6,0x1f800000		# 528482304
	ori	$6,$6,0x0030
.L19:
	li	$5,0x00000032		# 50
	.set	noreorder
	.set	nomacro
	jal	func_80032854
	move	$7,$0
	.set	macro
	.set	reorder

	move	$9,$0
.L11:
	lh	$3,12($19)
	li	$2,0x0000001d		# 29
	.set	noreorder
	.set	nomacro
	beq	$3,$2,.L22
	li	$2,0x0000000e		# 14
	.set	macro
	.set	reorder

	bne	$3,$2,.L21
.L22:
	lh	$2,620($19)
	#nop
	.set	noreorder
	.set	nomacro
	beq	$2,$0,.L21
	li	$11,0x00000002		# 2
	.set	macro
	.set	reorder

	lhu	$3,106($19)
	#nop
	.set	noreorder
	.set	nomacro
	beq	$3,$11,.L23
	li	$2,0x0000001b		# 27
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	beq	$3,$2,.L23
	li	$2,0x00000028		# 40
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	beq	$3,$2,.L23
	li	$2,0x00000026		# 38
	.set	macro
	.set	reorder

	bne	$3,$2,.L21
.L23:
	lh	$3,64($19)
	lbu	$2,162($19)
	#nop
	bne	$3,$2,.L21
	lbu	$2,842($19)
	#nop
	.set	noreorder
	.set	nomacro
	beq	$2,$0,.L21
	addu	$2,$2,-1
	.set	macro
	.set	reorder

	sltu	$16,$22,1
	sb	$2,842($19)
	subu	$2,$0,$16
	andi	$2,$2,0x0018
	lw	$3,528482388($2)
	lw	$4,528482392($2)
	lw	$5,528482396($2)
	sw	$3,0($18)
	sw	$4,4($18)
	sw	$5,8($18)
	lw	$3,528482376($2)
	lw	$4,528482380($2)
	lw	$5,528482384($2)
	sw	$3,12($18)
	sw	$4,16($18)
	sw	$5,20($18)
	addu	$5,$sp,32
	lw	$4,48($sp)
	lh	$7,12($19)
	addu	$6,$sp,36
	sw	$9,144($sp)
	xori	$7,$7,0x000e
	.set	noreorder
	.set	nomacro
	jal	func_8002A458
	sltu	$7,$7,1
	.set	macro
	.set	reorder

	li	$6,0x1f800000		# 528482304
	lw	$9,144($sp)
	.set	noreorder
	.set	nomacro
	bne	$22,$0,.L28
	ori	$6,$6,0x000c
	.set	macro
	.set	reorder

	li	$6,0x1f800000		# 528482304
	ori	$6,$6,0x0030
.L28:
	move	$4,$16
	li	$5,0x0000002a		# 42
	move	$7,$0
	.set	noreorder
	.set	nomacro
	jal	func_80032854
	sw	$9,144($sp)
	.set	macro
	.set	reorder

	lw	$2,32($sp)
	lw	$12,56($sp)
	#nop
	or	$12,$12,$2
	sw	$12,56($sp)
	lw	$9,144($sp)
.L21:
	.set	noreorder
	.set	nomacro
	blez	$9,.L31
	move	$23,$0
	.set	macro
	.set	reorder

.L33:
	bne	$23,$0,.L34
	.set	noreorder
	.set	nomacro
	j	.L183
	move	$fp,$0
	.set	macro
	.set	reorder

.L34:
	lh	$2,140($19)
	#nop
	beq	$2,$0,.L36
	li	$fp,0x00000001		# 1
.L183:
	li	$11,0x00000001		# 1
	li	$12,0x00000001		# 1
	sw	$0,80($sp)
	sw	$11,88($sp)
	.set	noreorder
	.set	nomacro
	j	.L35
	sw	$12,96($sp)
	.set	macro
	.set	reorder

.L36:
	lhu	$2,14($19)
	#nop
	addu	$2,$2,-4
	sltu	$2,$2,2
	beq	$2,$0,.L35
	move	$fp,$0
	li	$11,0x00000001		# 1
	li	$12,0x00000002		# 2
	sw	$11,80($sp)
	sw	$12,88($sp)
	sw	$0,96($sp)
.L35:
	bne	$fp,$0,.L39
	.set	noreorder
	.set	nomacro
	bne	$22,$0,.L40
	li	$3,0x1f800000		# 528482304
	.set	macro
	.set	reorder

	ori	$3,$3,0x0024
.L40:
	lw	$11,80($sp)
	#nop
	sll	$2,$11,1
	addu	$2,$2,$11
	sll	$8,$2,2
	addu	$2,$8,$3
	lw	$3,0($2)
	lw	$4,4($2)
	lw	$5,8($2)
	sw	$3,0($18)
	sw	$4,4($18)
	sw	$5,8($18)
	.set	noreorder
	.set	nomacro
	bne	$22,$0,.L41
	li	$3,0x1f800000		# 528482304
	.set	macro
	.set	reorder

	ori	$3,$3,0x0024
.L41:
	lw	$12,88($sp)
	#nop
	sll	$2,$12,1
	addu	$2,$2,$12
	sll	$2,$2,2
	addu	$3,$2,$3
	lw	$4,0($3)
	lw	$5,4($3)
	lw	$6,8($3)
	sw	$4,12($18)
	sw	$5,16($18)
	sw	$6,20($18)
	addu	$3,$8,$19
	lw	$4,528($3)
	lw	$5,532($3)
	lw	$6,536($3)
	sw	$4,24($18)
	sw	$5,28($18)
	sw	$6,32($18)
	addu	$2,$2,$19
	lw	$3,528($2)
	lw	$4,532($2)
	lw	$5,536($2)
	sw	$3,36($18)
	sw	$4,40($18)
	sw	$5,44($18)
	.set	noreorder
	.set	nomacro
	j	.L186
	li	$6,0x005f0000		# 6225920
	.set	macro
	.set	reorder

.L39:
	li	$3,0x1f800000		# 528482304
	.set	noreorder
	.set	nomacro
	bne	$22,$0,.L43
	ori	$3,$3,0x0048
	.set	macro
	.set	reorder

	li	$3,0x1f800000		# 528482304
	ori	$3,$3,0x0060
.L43:
	lw	$11,80($sp)
	#nop
	sll	$2,$11,1
	addu	$2,$2,$11
	sll	$8,$2,2
	addu	$2,$8,$3
	lw	$3,0($2)
	lw	$4,4($2)
	lw	$5,8($2)
	sw	$3,0($18)
	sw	$4,4($18)
	sw	$5,8($18)
	li	$3,0x1f800000		# 528482304
	.set	noreorder
	.set	nomacro
	bne	$22,$0,.L44
	ori	$3,$3,0x0048
	.set	macro
	.set	reorder

	li	$3,0x1f800000		# 528482304
	ori	$3,$3,0x0060
.L44:
	lw	$12,88($sp)
	#nop
	sll	$2,$12,1
	addu	$2,$2,$12
	sll	$2,$2,2
	addu	$3,$2,$3
	lw	$4,0($3)
	lw	$5,4($3)
	lw	$6,8($3)
	sw	$4,12($18)
	sw	$5,16($18)
	sw	$6,20($18)
	addu	$3,$8,$19
	lw	$4,564($3)
	lw	$5,568($3)
	lw	$6,572($3)
	sw	$4,24($18)
	sw	$5,28($18)
	sw	$6,32($18)
	addu	$2,$2,$19
	lw	$3,564($2)
	lw	$4,568($2)
	lw	$5,572($2)
	sw	$3,36($18)
	sw	$4,40($18)
	sw	$5,44($18)
	li	$6,0x005f0000		# 6225920
.L186:
	ori	$6,$6,0x5e0f
	lw	$5,12($18)
	lw	$3,36($18)
	lw	$4,16($18)
	lw	$2,40($18)
	subu	$16,$5,$3
	subu	$7,$4,$2
	mult	$16,$16
	lw	$4,0($18)
	lw	$5,20($18)
	lw	$2,44($18)
	lw	$3,24($18)
	subu	$17,$5,$2
	addu	$4,$4,$3
	srl	$2,$4,31
	addu	$4,$4,$2
	lw	$3,4($18)
	lw	$2,28($18)
	sra	$4,$4,1
	sw	$4,48($18)
	lw	$4,8($18)
	addu	$3,$3,$2
	srl	$2,$3,31
	addu	$3,$3,$2
	lw	$2,32($18)
	sra	$3,$3,1
	sw	$3,52($18)
	lw	$3,12($18)
	mflo	$8
	#nop
	addu	$4,$4,$2
	srl	$2,$4,31
	mult	$7,$7
	addu	$4,$4,$2
	lw	$2,36($18)
	sra	$4,$4,1
	sw	$4,56($18)
	lw	$4,16($18)
	addu	$3,$3,$2
	srl	$2,$3,31
	addu	$3,$3,$2
	lw	$2,40($18)
	sra	$3,$3,1
	sw	$3,60($18)
	mflo	$5
	#nop
	lw	$3,44($18)
	addu	$4,$4,$2
	mult	$17,$17
	srl	$2,$4,31
	addu	$4,$4,$2
	lw	$2,20($18)
	sra	$4,$4,1
	sw	$4,64($18)
	addu	$2,$2,$3
	srl	$3,$2,31
	addu	$2,$2,$3
	sra	$2,$2,1
	sw	$2,68($18)
	addu	$2,$8,$5
	mflo	$10
	#nop
	#nop
	addu	$5,$2,$10
	slt	$6,$6,$5
	.set	noreorder
	.set	nomacro
	beq	$6,$0,.L45
	li	$20,0x00000002		# 2
	.set	macro
	.set	reorder

	li	$20,0x00000004		# 4
.L45:
	.set	noreorder
	.set	nomacro
	beq	$20,$0,.L32
	move	$21,$0
	.set	macro
	.set	reorder

	addu	$17,$18,60
	addu	$16,$18,48
.L49:
	li	$11,0x00000002		# 2
	bne	$20,$11,.L50
	.set	noreorder
	.set	nomacro
	bne	$21,$0,.L51
	addu	$2,$18,36
	.set	macro
	.set	reorder

	lw	$12,104($sp)
	#nop
	sw	$12,96($18)
	sw	$2,100($18)
	lw	$11,112($sp)
	.set	noreorder
	.set	nomacro
	j	.L53
	sw	$11,104($18)
	.set	macro
	.set	reorder

.L51:
	sw	$18,96($18)
	lw	$12,112($sp)
	.set	noreorder
	.set	nomacro
	j	.L184
	sw	$12,100($18)
	.set	macro
	.set	reorder

.L50:
	li	$12,0x00000001		# 1
	.set	noreorder
	.set	nomacro
	beq	$21,$12,.L56
	slt	$2,$21,2
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	beq	$2,$0,.L61
	li	$11,0x00000002		# 2
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	beq	$21,$0,.L55
	addu	$2,$18,36
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	j	.L187
	addu	$7,$18,4
	.set	macro
	.set	reorder

.L61:
	.set	noreorder
	.set	nomacro
	beq	$21,$11,.L57
	li	$2,0x00000003		# 3
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	beq	$21,$2,.L58
	addu	$7,$18,4
	.set	macro
	.set	reorder

	j	.L187
.L55:
	sw	$2,96($18)
	lw	$12,104($sp)
	sw	$17,104($18)
	.set	noreorder
	.set	nomacro
	j	.L53
	sw	$12,100($18)
	.set	macro
	.set	reorder

.L56:
	sw	$17,96($18)
	sw	$16,100($18)
.L184:
	lw	$11,104($sp)
	.set	noreorder
	.set	nomacro
	j	.L53
	sw	$11,104($18)
	.set	macro
	.set	reorder

.L57:
	sw	$17,96($18)
	sw	$16,100($18)
	lw	$12,112($sp)
	.set	noreorder
	.set	nomacro
	j	.L53
	sw	$12,104($18)
	.set	macro
	.set	reorder

.L58:
	lw	$11,112($sp)
	sw	$18,100($18)
	sw	$16,104($18)
	sw	$11,96($18)
.L53:
	addu	$7,$18,4
.L187:
	lw	$2,96($18)
	addu	$8,$18,12
	lw	$3,0($2)
	lw	$4,4($2)
	lw	$5,8($2)
	sw	$3,132($18)
	sw	$4,136($18)
	sw	$5,140($18)
	lw	$2,132($18)
	lw	$3,136($18)
	lw	$4,140($18)
	sw	$2,120($18)
	sw	$3,124($18)
	sw	$4,128($18)
.L65:
	lw	$2,96($7)
	#nop
	lw	$3,0($2)
	lw	$2,120($18)
	#nop
	slt	$2,$3,$2
	beq	$2,$0,.L66
	.set	noreorder
	.set	nomacro
	j	.L67
	sw	$3,120($18)
	.set	macro
	.set	reorder

.L66:
	lw	$2,132($18)
	#nop
	slt	$2,$2,$3
	beq	$2,$0,.L67
	sw	$3,132($18)
.L67:
	lw	$2,96($7)
	#nop
	lw	$3,4($2)
	lw	$2,124($18)
	#nop
	slt	$2,$3,$2
	beq	$2,$0,.L69
	.set	noreorder
	.set	nomacro
	j	.L70
	sw	$3,124($18)
	.set	macro
	.set	reorder

.L69:
	lw	$2,136($18)
	#nop
	slt	$2,$2,$3
	beq	$2,$0,.L70
	sw	$3,136($18)
.L70:
	lw	$2,96($7)
	#nop
	lw	$3,8($2)
	lw	$2,128($18)
	#nop
	slt	$2,$3,$2
	beq	$2,$0,.L72
	.set	noreorder
	.set	nomacro
	j	.L64
	sw	$3,128($18)
	.set	macro
	.set	reorder

.L72:
	lw	$2,140($18)
	#nop
	slt	$2,$2,$3
	beq	$2,$0,.L64
	sw	$3,140($18)
.L64:
	addu	$7,$7,4
	slt	$2,$7,$8
	.set	noreorder
	.set	nomacro
	bne	$2,$0,.L65
	move	$4,$18
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	jal	func_8002CD58
	sw	$9,144($sp)
	.set	macro
	.set	reorder

	lw	$4,48($sp)
	lw	$6,96($sp)
	.set	noreorder
	.set	nomacro
	jal	func_8002CA8C
	move	$5,$2
	.set	macro
	.set	reorder

	lw	$2,32($sp)
	lw	$4,180($18)
	lw	$3,36($sp)
	lw	$5,196($18)
	lw	$9,144($sp)
	or	$2,$2,$4
	or	$3,$3,$5
	sw	$2,32($sp)
	.set	noreorder
	.set	nomacro
	beq	$fp,$0,.L76
	sw	$3,36($sp)
	.set	macro
	.set	reorder

	lw	$2,180($18)
	lw	$12,56($sp)
	#nop
	or	$12,$12,$2
	sw	$12,56($sp)
.L76:
	li	$11,0x00000001		# 1
	bne	$23,$11,.L77
	bne	$fp,$0,.L77
	lw	$2,180($18)
	lw	$12,64($sp)
	#nop
	or	$12,$12,$2
	.set	noreorder
	.set	nomacro
	j	.L48
	sw	$12,64($sp)
	.set	macro
	.set	reorder

.L77:
	lw	$2,180($18)
	lw	$11,72($sp)
	#nop
	or	$11,$11,$2
	sw	$11,72($sp)
.L48:
	addu	$21,$21,1
	slt	$2,$21,$20
	bne	$2,$0,.L49
.L32:
	addu	$23,$23,1
	slt	$2,$23,$9
	bne	$2,$0,.L33
.L31:
	lw	$4,32($sp)
	#nop
	.set	noreorder
	.set	nomacro
	beq	$4,$0,.L4
	sll	$2,$22,3
	.set	macro
	.set	reorder

	subu	$2,$2,$22
	sll	$2,$2,3
	subu	$2,$2,$22
	sll	$2,$2,3
	la	$3,D_800F5F68
	addu	$8,$2,$3
	li	$9,0x7fff0000		# 2147418112
	lw	$2,540($19)
	ori	$9,$9,0xffff
	sw	$2,200($18)
	lw	$2,544($19)
	move	$10,$4
	sw	$2,204($18)
	lw	$2,548($19)
	lw	$4,136($sp)
	move	$20,$0
	sw	$2,208($18)
.L85:
	li	$12,0x00000001		# 1
	sll	$2,$12,$20
	and	$2,$10,$2
	beq	$2,$0,.L84
	lw	$3,528482472($4)
	lw	$2,200($18)
	#nop
	subu	$16,$3,$2
	mult	$16,$16
	lw	$3,528482476($4)
	lw	$2,204($18)
	mflo	$6
	#nop
	subu	$7,$3,$2
	mult	$7,$7
	lw	$3,528482480($4)
	lw	$2,208($18)
	mflo	$5
	#nop
	subu	$17,$3,$2
	mult	$17,$17
	lhu	$3,14($8)
	addu	$2,$6,$5
	mflo	$13
	#nop
	#nop
	addu	$2,$2,$13
	subu	$5,$2,$3
	slt	$2,$5,$9
	beq	$2,$0,.L84
	move	$9,$5
	move	$21,$20
.L84:
	addu	$4,$4,12
	addu	$20,$20,1
	slt	$2,$20,22
	.set	noreorder
	.set	nomacro
	bne	$2,$0,.L85
	addu	$8,$8,20
	.set	macro
	.set	reorder

	lw	$11,40($sp)
	li	$12,0x00000001		# 1
	bne	$11,$12,.L89
	la	$5,D_800A37E8
	.set	noreorder
	.set	nomacro
	jal	func_800274BC
	addu	$4,$19,276
	.set	macro
	.set	reorder

	li	$2,0x1f800000		# 528482304
	ori	$2,$2,0x00a8
	move	$4,$22
	li	$5,0x00000004		# 4
	sll	$6,$21,1
	addu	$6,$6,$21
	sll	$6,$6,2
	addu	$6,$6,$2
	lw	$11,136($sp)
	la	$7,D_800A37E8
	.set	noreorder
	.set	nomacro
	jal	func_80032854
	addu	$6,$11,$6
	.set	macro
	.set	reorder

	j	.L1
.L89:
	lbu	$4,161($19)
	lh	$3,64($19)
	addu	$2,$4,-3
	slt	$2,$3,$2
	.set	noreorder
	.set	nomacro
	bne	$2,$0,.L92
	move	$5,$0
	.set	macro
	.set	reorder

	slt	$2,$3,$4
	bne	$2,$0,.L91
.L92:
	lbu	$4,162($19)
	#nop
	addu	$2,$4,-3
	slt	$2,$3,$2
	.set	noreorder
	.set	nomacro
	bne	$2,$0,.L90
	slt	$2,$3,$4
	.set	macro
	.set	reorder

	beq	$2,$0,.L90
.L91:
	li	$5,0x00000001		# 1
.L90:
	lhu	$2,14($19)
	#nop
	addu	$2,$2,-6
	sltu	$2,$2,2
	.set	noreorder
	.set	nomacro
	bne	$2,$0,.L94
	move	$4,$0
	.set	macro
	.set	reorder

	lh	$3,12($19)
	li	$2,0x0000001d		# 29
	.set	noreorder
	.set	nomacro
	beq	$3,$2,.L94
	li	$2,0x0000000e		# 14
	.set	macro
	.set	reorder

	bne	$3,$2,.L93
.L94:
	li	$12,0x00000001		# 1
	lw	$11,56($sp)
	sll	$2,$12,$21
	and	$2,$11,$2
	beq	$2,$0,.L93
	li	$4,0x00000001		# 1
.L93:
	.set	noreorder
	.set	nomacro
	bne	$4,$0,.L95
	li	$12,0x00000002		# 2
	.set	macro
	.set	reorder

	lhu	$3,106($19)
	#nop
	.set	noreorder
	.set	nomacro
	beq	$3,$12,.L97
	li	$2,0x0000001b		# 27
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	beq	$3,$2,.L97
	li	$2,0x00000028		# 40
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	beq	$3,$2,.L97
	li	$2,0x00000026		# 38
	.set	macro
	.set	reorder

	bne	$3,$2,.L96
.L97:
	bne	$5,$0,.L95
.L96:
	li	$11,0x00000001		# 1
	lw	$12,56($sp)
	sll	$2,$11,$21
	and	$20,$12,$2
	sltu	$20,$0,$20
	beq	$20,$0,.L98
	sltu	$2,$22,1
	subu	$2,$0,$2
	andi	$2,$2,0x0018
	lw	$7,528482376($2)
	lw	$5,244($19)
	#nop
	subu	$16,$7,$5
	mult	$16,$16
	lw	$6,528482384($2)
	lw	$4,252($19)
	mflo	$9
	#nop
	subu	$17,$6,$4
	mult	$17,$17
	lw	$3,528482388($2)
	mflo	$8
	#nop
	subu	$16,$3,$5
	lw	$2,528482396($2)
	.set	noreorder
	.set	nomacro
	j	.L185
	mult	$16,$16
	.set	macro
	.set	reorder

.L98:
	lw	$11,64($sp)
	#nop
	and	$2,$11,$2
	.set	noreorder
	.set	nomacro
	beq	$2,$0,.L116
	sltu	$2,$22,1
	.set	macro
	.set	reorder

	subu	$2,$0,$2
	andi	$2,$2,0x0024
	lw	$6,528482328($2)
	lw	$5,244($19)
	#nop
	subu	$16,$6,$5
	mult	$16,$16
	lw	$9,528482336($2)
	lw	$4,252($19)
	mflo	$10
	#nop
	subu	$17,$9,$4
	mult	$17,$17
	lw	$3,528482316($2)
	mflo	$8
	#nop
	subu	$16,$3,$5
	mult	$16,$16
	lw	$5,528482324($2)
	mflo	$7
	#nop
	subu	$17,$5,$4
	mult	$17,$17
	subu	$2,$3,$6
	addu	$10,$10,$8
	sw	$10,80($sp)
	mflo	$13
	#nop
	#nop
	addu	$7,$7,$13
	.set	noreorder
	.set	nomacro
	bgez	$2,.L129
	sw	$7,88($sp)
	.set	macro
	.set	reorder

	addu	$2,$2,3
.L129:
	sra	$16,$2,2
	subu	$2,$5,$9
	.set	noreorder
	.set	nomacro
	bgez	$2,.L115
	sra	$17,$2,2
	.set	macro
	.set	reorder

	addu	$2,$2,3
	.set	noreorder
	.set	nomacro
	j	.L115
	sra	$17,$2,2
	.set	macro
	.set	reorder

.L116:
	subu	$2,$0,$2
	andi	$2,$2,0x0024
	lw	$7,528482304($2)
	lw	$5,244($19)
	#nop
	subu	$16,$7,$5
	mult	$16,$16
	lw	$6,528482312($2)
	lw	$4,252($19)
	mflo	$9
	#nop
	subu	$17,$6,$4
	mult	$17,$17
	lw	$3,528482316($2)
	mflo	$8
	#nop
	subu	$16,$3,$5
	mult	$16,$16
	lw	$2,528482324($2)
.L185:
	mflo	$5
	#nop
	subu	$17,$2,$4
	mult	$17,$17
	subu	$16,$3,$7
	addu	$9,$9,$8
	sw	$9,80($sp)
	subu	$17,$2,$6
	mflo	$10
	#nop
	#nop
	addu	$5,$5,$10
	sw	$5,88($sp)
.L115:
	lw	$11,88($sp)
	lw	$12,80($sp)
	#nop
	slt	$2,$11,$12
	.set	noreorder
	.set	nomacro
	beq	$2,$0,.L188
	move	$4,$16
	.set	macro
	.set	reorder

	subu	$16,$0,$16
	subu	$17,$0,$17
	move	$4,$16
.L188:
	.set	noreorder
	.set	nomacro
	jal	ratan2
	move	$5,$17
	.set	macro
	.set	reorder

	lh	$3,472($19)
	#nop
	subu	$3,$3,$2
	andi	$3,$3,0x0fff
	slt	$2,$3,2048
	.set	noreorder
	.set	nomacro
	bne	$2,$0,.L189
	slt	$2,$3,1024
	.set	macro
	.set	reorder

	li	$2,0x00001000		# 4096
	subu	$3,$2,$3
	slt	$2,$3,1024
.L189:
	.set	noreorder
	.set	nomacro
	beq	$2,$0,.L95
	sll	$2,$fp,4
	.set	macro
	.set	reorder

	addu	$2,$19,$2
	lw	$4,276($2)
	lw	$5,284($2)
	jal	ratan2
	lw	$11,48($sp)
	#nop
	lh	$3,472($11)
	#nop
	addu	$3,$3,-2048
	subu	$2,$2,$3
	andi	$5,$2,0x0fff
	slt	$2,$5,2048
	.set	noreorder
	.set	nomacro
	bne	$2,$0,.L190
	li	$2,0x00000400		# 1024
	.set	macro
	.set	reorder

	li	$2,0x00001000		# 4096
	subu	$5,$2,$5
	li	$2,0x00000400		# 1024
.L190:
	subu	$5,$2,$5
	bgez	$5,.L156
	move	$5,$0
.L156:
	.set	noreorder
	.set	nomacro
	bgez	$16,.L157
	move	$4,$16
	.set	macro
	.set	reorder

	addu	$4,$16,3
.L157:
	sll	$2,$20,4
	addu	$7,$19,$2
	lw	$2,276($7)
	#nop
	mult	$2,$5
	move	$6,$17
	sra	$4,$4,2
	mflo	$2
	#nop
	#nop
	srl	$3,$2,31
	addu	$2,$2,$3
	sra	$2,$2,12
	.set	noreorder
	.set	nomacro
	bgez	$17,.L158
	addu	$16,$4,$2
	.set	macro
	.set	reorder

	addu	$6,$17,3
.L158:
	lw	$2,284($7)
	#nop
	mult	$2,$5
	sra	$4,$6,2
	mflo	$2
	#nop
	#nop
	srl	$3,$2,31
	addu	$2,$2,$3
	sra	$2,$2,12
	addu	$17,$4,$2
	lhu	$3,106($19)
	li	$2,0x00000003		# 3
	.set	noreorder
	.set	nomacro
	beq	$3,$2,.L160
	li	$2,0x00000007		# 7
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	beq	$3,$2,.L160
	li	$2,0x0000000d		# 13
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	beq	$3,$2,.L160
	li	$2,0x0000002c		# 44
	.set	macro
	.set	reorder

	bne	$3,$2,.L159
.L160:
	srl	$2,$16,31
	lw	$11,48($sp)
	addu	$2,$16,$2
	lw	$3,308($11)
	sra	$2,$2,1
	addu	$3,$3,$2
	srl	$2,$17,31
	addu	$2,$17,$2
	sw	$3,308($11)
	lw	$3,316($11)
	sra	$2,$2,1
	addu	$3,$3,$2
	.set	noreorder
	.set	nomacro
	j	.L161
	sw	$3,316($11)
	.set	macro
	.set	reorder

.L159:
	lw	$12,48($sp)
	#nop
	lw	$2,308($12)
	lw	$3,316($12)
	addu	$2,$2,$16
	addu	$3,$3,$17
	sw	$2,308($12)
	sw	$3,316($12)
.L161:
	lw	$11,48($sp)
	#nop
	lhu	$3,106($11)
	li	$2,0x00000013		# 19
	.set	noreorder
	.set	nomacro
	beq	$3,$2,.L163
	li	$2,0x0000001b		# 27
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	beq	$3,$2,.L163
	li	$2,0x00000030		# 48
	.set	macro
	.set	reorder

	bne	$3,$2,.L95
.L163:
	lhu	$3,106($19)
	li	$2,0x00000015		# 21
	bne	$3,$2,.L95
	lw	$12,48($sp)
	#nop
	lhu	$2,14($12)
	#nop
	addu	$2,$2,-6
	sltu	$2,$2,2
	.set	noreorder
	.set	nomacro
	bne	$2,$0,.L191
	li	$2,0x00000005		# 5
	.set	macro
	.set	reorder

	lhu	$2,14($19)
	#nop
	addu	$2,$2,-6
	sltu	$2,$2,2
	.set	noreorder
	.set	nomacro
	beq	$2,$0,.L164
	li	$2,0x00000005		# 5
	.set	macro
	.set	reorder

.L191:
	sh	$2,646($19)
	la	$6,g_practice_menu_table+244
	.set	noreorder
	.set	nomacro
	bne	$22,$0,.L166
	sltu	$16,$22,1
	.set	macro
	.set	reorder

	addu	$6,$6,1100
.L166:
	move	$4,$16
	li	$5,0x00000021		# 33
	.set	noreorder
	.set	nomacro
	jal	func_80032854
	move	$7,$0
	.set	macro
	.set	reorder

	la	$6,g_practice_menu_table+244
	.set	noreorder
	.set	nomacro
	bne	$22,$0,.L192
	move	$4,$16
	.set	macro
	.set	reorder

	addu	$6,$6,1100
.L192:
	li	$5,0x0000002d		# 45
	.set	noreorder
	.set	nomacro
	jal	func_80032854
	move	$7,$0
	.set	macro
	.set	reorder

	j	.L95
.L164:
	li	$11,0x00000001		# 1
	sh	$11,D_800A38A8
	sh	$22,D_800A3876
.L95:
	lw	$12,48($sp)
	lh	$3,458($19)
	lh	$2,458($12)
	#nop
	subu	$2,$2,$3
	andi	$16,$2,0x0fff
	slt	$2,$16,2048
	.set	noreorder
	.set	nomacro
	bne	$2,$0,.L193
	li	$11,0x00000001		# 1
	.set	macro
	.set	reorder

	li	$2,0x00001000		# 4096
	subu	$16,$2,$16
.L193:
	lw	$12,56($sp)
	sll	$2,$11,$21
	and	$fp,$12,$2
	lbu	$2,174($19)
	#nop
	.set	noreorder
	.set	nomacro
	beq	$2,$0,.L4
	sltu	$fp,$0,$fp
	.set	macro
	.set	reorder

	lh	$2,140($19)
	#nop
	.set	noreorder
	.set	nomacro
	beq	$2,$0,.L173
	addu	$4,$19,$fp
	.set	macro
	.set	reorder

	lh	$3,64($19)
	lbu	$2,161($4)
	#nop
	slt	$2,$3,$2
	bne	$2,$0,.L4
	lbu	$2,163($4)
	#nop
	slt	$2,$2,$3
	bne	$2,$0,.L4
.L173:
	lhu	$2,14($19)
	#nop
	addu	$2,$2,-6
	sltu	$2,$2,2
	.set	noreorder
	.set	nomacro
	bne	$2,$0,.L175
	move	$17,$0
	.set	macro
	.set	reorder

	lh	$3,12($19)
	li	$2,0x0000001d		# 29
	.set	noreorder
	.set	nomacro
	beq	$3,$2,.L177
	li	$2,0x0000000e		# 14
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	bne	$3,$2,.L194
	sll	$4,$fp,4
	.set	macro
	.set	reorder

.L177:
	.set	noreorder
	.set	nomacro
	beq	$fp,$0,.L176
	sll	$4,$fp,4
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	j	.L175
	li	$17,0x00000001		# 1
	.set	macro
	.set	reorder

.L176:
.L194:
	addu	$4,$4,276
	la	$5,D_800A37E8
	.set	noreorder
	.set	nomacro
	jal	func_800274BC
	addu	$4,$19,$4
	.set	macro
	.set	reorder

.L175:
	lw	$2,D_800A3140
	#nop
	.set	noreorder
	.set	nomacro
	bne	$2,$0,.L179
	li	$8,0x00000001		# 1
	.set	macro
	.set	reorder

	lw	$3,36($sp)
	li	$11,0x00000001		# 1
	sll	$2,$11,$21
	and	$8,$3,$2
.L179:
	li	$12,0x00000001		# 1
	sll	$2,$12,$21
	lw	$11,64($sp)
	lw	$12,72($sp)
	and	$2,$11,$2
	nor	$3,$0,$12
	and	$2,$2,$3
	.set	noreorder
	.set	nomacro
	beq	$2,$0,.L195
	move	$4,$0
	.set	macro
	.set	reorder

	move	$8,$0
.L195:
	move	$6,$21
	lw	$5,48($sp)
	move	$7,$16
	sw	$8,16($sp)
	sw	$fp,20($sp)
	sw	$17,24($sp)
	.set	noreorder
	.set	nomacro
	jal	func_80027AD8
	sw	$0,28($sp)
	.set	macro
	.set	reorder

	sb	$0,173($19)
.L4:
	addu	$22,$22,1
	lw	$11,136($sp)
	slt	$2,$22,2
	addu	$11,$11,264
	.set	noreorder
	.set	nomacro
	bne	$2,$0,.L5
	sw	$11,136($sp)
	.set	macro
	.set	reorder

.L1:
	lw	$31,188($sp)
	lw	$fp,184($sp)
	lw	$23,180($sp)
	lw	$22,176($sp)
	lw	$21,172($sp)
	lw	$20,168($sp)
	lw	$19,164($sp)
	lw	$18,160($sp)
	lw	$17,156($sp)
	lw	$16,152($sp)
	addu	$sp,$sp,192
	j	$31
	.end	func_8002AB08
