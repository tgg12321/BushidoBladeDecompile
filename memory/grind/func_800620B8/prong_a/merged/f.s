	.frame	$sp,80,$31		# vars= 24, regs= 10/0, args= 16, extra= 0
	.mask	0xc0ff0000,-4
	.fmask	0x00000000,0
	subu	$sp,$sp,80
	sw	$31,76($sp)
	sw	$fp,72($sp)
	sw	$23,68($sp)
	sw	$22,64($sp)
	sw	$21,60($sp)
	sw	$20,56($sp)
	sw	$19,52($sp)
	sw	$18,48($sp)
	sw	$17,44($sp)
	.set	noreorder
	.set	nomacro
	jal	func_80060E38
	sw	$16,40($sp)
	.set	macro
	.set	reorder

	lw	$3,D_800A3468
	#nop
	lw	$2,4($3)
	lw	$4,D_800A346C
	lhu	$2,0($2)
	#nop
	sh	$2,0($4)
	lw	$2,4($3)
	lw	$17,D_800A37D4
	lhu	$2,2($2)
	#nop
	sh	$2,2($4)
	lw	$2,4($3)
	lw	$5,D_800A3470
	lhu	$2,4($2)
	#nop
	sh	$2,4($4)
	lw	$2,8($3)
	lw	$6,D_800A3474
	lw	$2,0($2)
	#nop
	sw	$2,0($5)
	lw	$2,8($3)
	lw	$16,D_800A34EC
	lw	$2,4($2)
	#nop
	sw	$2,4($5)
	lw	$3,8($3)
	lw	$2,D_800A32B8
	lw	$3,8($3)
	addu	$2,$2,1
	sw	$2,D_800A32B8
	.set	noreorder
	.set	nomacro
	jal	func_80061FAC
	sw	$3,8($5)
	.set	macro
	.set	reorder

	lw	$4,D_800A3474
	.set	noreorder
	.set	nomacro
	jal	SetRotMatrix
	move	$21,$0
	.set	macro
	.set	reorder

	sh	$0,56($16)
	sh	$0,54($16)
	.set	noreorder
	.set	nomacro
	jal	ReadGeomScreen
	sh	$0,52($16)
	.set	macro
	.set	reorder

	sll	$2,$2,8
	addu	$19,$16,16
	addu	$18,$16,18
	addu	$23,$16,20
	addu	$20,$16,36
	lw	$3,D_800A34B0
	addu	$8,$16,52
	sw	$2,0($3)
	lw	$3,D_800A3490
	li	$2,0x0000002f		# 47
	sw	$2,0($3)
	lw	$2,D_800F1198
	addu	$22,$16,68
	sw	$8,16($sp)
	addu	$8,$16,60
	andi	$2,$2,0x0001
	.set	noreorder
	.set	nomacro
	beq	$2,$0,.L949
	sw	$8,24($sp)
	.set	macro
	.set	reorder

	la	$fp,D_8009BA00
	addu	$16,$17,37
.L951:
	sll	$3,$21,16
	sra	$3,$3,16
	sll	$2,$3,1
	addu	$2,$2,$3
	sll	$2,$2,2
	lw	$2,D_800F1198+4($2)
	#nop
	andi	$3,$2,0x0007
	li	$2,0x00000001		# 1
	.set	noreorder
	.set	nomacro
	beq	$3,$2,.L959
	slt	$2,$3,2
	.set	macro
	.set	reorder

	beq	$2,$0,.L963
	.set	noreorder
	.set	nomacro
	beq	$3,$0,.L955
	li	$2,0x000001c2		# 450
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	j	.L977
	sll	$3,$21,16
	.set	macro
	.set	reorder

.L963:
	li	$2,0x00000002		# 2
	.set	noreorder
	.set	nomacro
	beq	$3,$2,.L957
	li	$2,0x00000050		# 80
	.set	macro
	.set	reorder

	li	$2,0x00000003		# 3
	.set	noreorder
	.set	nomacro
	bne	$3,$2,.L977
	sll	$3,$21,16
	.set	macro
	.set	reorder

	lw	$3,D_800A34A8
	li	$2,0x00000151		# 337
	sh	$2,0($3)
	lw	$3,D_800A34AC
	.set	noreorder
	.set	nomacro
	j	.L974
	li	$2,0x000000a8		# 168
	.set	macro
	.set	reorder

.L955:
	lw	$3,D_800A34A8
	#nop
	sh	$2,0($3)
	lw	$3,D_800A34AC
	li	$2,0x000000e1		# 225
.L974:
	sh	$2,0($3)
	lw	$4,D_800A32B8
	li	$2,-1431655765			# 0xaaaaaaab
	multu	$4,$2
	mfhi	$8
	#nop
	#nop
	srl	$3,$8,2
	sll	$2,$3,1
	addu	$2,$2,$3
	sll	$2,$2,1
	subu	$4,$4,$2
	sll	$4,$4,3
	lw	$2,D_8009BD44
	addu	$4,$4,$fp
	sw	$4,D_800A3488
	sw	$4,D_800A348C
	andi	$2,$2,0x0001
	beq	$2,$0,.L956
	la	$8,D_8009BA00+80
	sw	$8,D_800A348C
.L956:
	lhu	$2,4($4)
	lw	$3,D_800A349C
	addu	$2,$2,31
	sh	$2,0($3)
	lw	$2,D_800A3488
	#nop
	lhu	$2,6($2)
	lw	$3,D_800A34A4
	.set	noreorder
	.set	nomacro
	j	.L975
	addu	$2,$2,31
	.set	macro
	.set	reorder

.L957:
	lw	$3,D_800A34A8
	#nop
	sh	$2,0($3)
	lw	$3,D_800A34AC
	.set	noreorder
	.set	nomacro
	j	.L976
	li	$2,0x0000003c		# 60
	.set	macro
	.set	reorder

.L959:
	lw	$3,D_800A34A8
	li	$2,0x00000064		# 100
	sh	$2,0($3)
	lw	$3,D_800A34AC
	li	$2,0x00000078		# 120
.L976:
	sh	$2,0($3)
	la	$8,D_8009BA00+48
	lw	$2,D_800A32B8
	lw	$3,D_8009BD44
	andi	$2,$2,0x0003
	sll	$2,$2,3
	addu	$2,$2,$8
	andi	$3,$3,0x0001
	sw	$2,D_800A3488
	sw	$2,D_800A348C
	beq	$3,$0,.L960
	la	$8,D_8009BA00+88
	sw	$8,D_800A348C
.L960:
	lhu	$2,4($2)
	lw	$3,D_800A349C
	addu	$2,$2,15
	sh	$2,0($3)
	lw	$2,D_800A3488
	#nop
	lhu	$2,6($2)
	lw	$3,D_800A34A4
	addu	$2,$2,19
.L975:
	sh	$2,0($3)
	sll	$3,$21,16
.L977:
	sra	$3,$3,16
	sll	$2,$3,1
	addu	$2,$2,$3
	sll	$6,$2,2
	lw	$2,D_800F1198($6)
	lw	$5,D_800A3470
	srl	$3,$2,31
	addu	$2,$2,$3
	lw	$3,0($5)
	sra	$2,$2,1
	subu	$2,$2,$3
	sw	$2,0($20)
	lw	$2,D_800F1198+4($6)
	#nop
	bgez	$2,.L964
	addu	$2,$2,7
.L964:
	move	$4,$20
	lw	$3,4($5)
	sra	$2,$2,3
	subu	$2,$2,$3
	sw	$2,4($20)
	lw	$2,D_800F1198+8($6)
	lw	$3,8($5)
	move	$5,$23
	subu	$2,$2,$3
	.set	noreorder
	.set	nomacro
	jal	ApplyRotMatrixLV
	sw	$2,8($20)
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	jal	SetTransMatrix
	addu	$4,$23,-20
	.set	macro
	.set	reorder

	lw	$4,16($sp)
	lw	$5,D_800A34B8
	lw	$7,D_800A34CC
	lw	$6,24($sp)
	jal	RotTransPers
	lw	$8,D_800A34D0
 #APP
	swc2   $19, 0($8)

 #NO_APP
	lw	$2,D_800A34D0
	#nop
	lw	$4,0($2)
	.set	noreorder
	.set	nomacro
	jal	func_80052C28
	move	$5,$0
	.set	macro
	.set	reorder

	move	$3,$2
	sltu	$2,$3,4101
	.set	noreorder
	.set	nomacro
	beq	$2,$0,.L950
	sw	$3,0($22)
	.set	macro
	.set	reorder

	lw	$2,D_800A34B0
	#nop
	lw	$2,0($2)
	#nop
	bgez	$2,.L966
	addu	$2,$2,255
.L966:
	sra	$2,$2,12
	sltu	$2,$2,$3
	.set	noreorder
	.set	nomacro
	beq	$2,$0,.L978
	addu	$4,$21,1
	.set	macro
	.set	reorder

	lw	$3,D_800A348C
	#nop
	lhu	$2,0($3)
	lhu	$3,2($3)
	srl	$2,$2,4
	andi	$2,$2,0x003f
	sll	$3,$3,6
	addu	$2,$2,$3
	lw	$3,D_800A3494
	andi	$2,$2,0xffff
	sw	$2,0($3)
	lw	$2,D_800A3488
	lw	$3,D_800A3498
	lhu	$2,4($2)
	#nop
	sh	$2,0($3)
	lw	$2,D_800A3488
	lw	$3,D_800A34A0
	lhu	$2,6($2)
	#nop
	sh	$2,0($3)
	lw	$3,D_800A34D0
	#nop
	lw	$2,0($3)
	#nop
	bne	$2,$0,.L967
	li	$2,0x00000001		# 1
.L967:
	sw	$2,0($3)
	lw	$2,D_800A34B0
	lw	$4,D_800A34D0
	lw	$3,0($2)
	lw	$2,0($4)
	#nop
	div	$3,$3,$2
	lw	$2,D_800A34B4
	#nop
	sw	$3,0($2)
	lw	$2,D_800A34A8
	lw	$3,D_800A34B4
	lh	$4,0($2)
	lw	$2,0($3)
	#nop
	mult	$4,$2
	mflo	$4
	#nop
	#nop
	slt	$2,$4,513
	.set	noreorder
	.set	nomacro
	bne	$2,$0,.L968
	li	$2,0x00000002		# 2
	.set	macro
	.set	reorder

	srl	$2,$4,8
.L968:
	sll	$2,$2,16
	sra	$3,$2,16
	srl	$2,$2,31
	addu	$3,$3,$2
	sra	$3,$3,1
	sh	$3,0($19)
	lw	$2,D_800A34AC
	lw	$3,D_800A34B4
	lh	$4,0($2)
	lw	$2,0($3)
	#nop
	mult	$4,$2
	mflo	$4
	#nop
	#nop
	slt	$2,$4,513
	.set	noreorder
	.set	nomacro
	bne	$2,$0,.L970
	li	$2,0x00000002		# 2
	.set	macro
	.set	reorder

	srl	$2,$4,8
.L970:
	.set	noreorder
	.set	nomacro
	jal	rand
	sh	$2,0($18)
	.set	macro
	.set	reorder

	lh	$5,0($18)
	#nop
	mult	$5,$2
	mflo	$2
	#nop
	li	$3,0x66660000		# 1717960704
	ori	$3,$3,0x6667
	mult	$2,$3
	move	$4,$17
	sra	$2,$2,31
	mfhi	$8
	#nop
	#nop
	sra	$3,$8,2
	subu	$3,$3,$2
	sra	$3,$3,14
	addu	$5,$5,$3
	.set	noreorder
	.set	nomacro
	jal	SetPolyFT4
	sh	$5,0($18)
	.set	macro
	.set	reorder

	lw	$2,D_800A3490
	lw	$3,D_800A3494
	lw	$2,0($2)
	#nop
	sh	$2,-15($16)
	lw	$3,0($3)
	li	$2,0x000000ff		# 255
	sb	$2,-33($16)
	li	$2,0x00000080		# 128
	sb	$2,-32($16)
	sb	$2,-31($16)
	lw	$4,D_800A34B8
	sh	$3,-23($16)
	lw	$2,0($4)
	lhu	$3,0($19)
	#nop
	subu	$2,$2,$3
	sh	$2,-29($16)
	lh	$2,2($4)
	lhu	$3,0($18)
	#nop
	subu	$2,$2,$3
	sh	$2,-27($16)
	lw	$3,0($4)
	lhu	$2,0($19)
	#nop
	addu	$2,$2,$3
	sh	$2,-21($16)
	lh	$2,2($4)
	lhu	$3,0($18)
	#nop
	subu	$2,$2,$3
	sh	$2,-19($16)
	lw	$2,0($4)
	lhu	$3,0($19)
	#nop
	subu	$2,$2,$3
	sh	$2,-13($16)
	lh	$2,2($4)
	#nop
	sh	$2,-11($16)
	lw	$3,0($4)
	lhu	$2,0($19)
	#nop
	addu	$2,$2,$3
	sh	$2,-5($16)
	lh	$2,2($4)
	lw	$3,D_800A3498
	sh	$2,-3($16)
	lhu	$2,0($3)
	#nop
	sb	$2,-25($16)
	lw	$2,D_800A34A0
	#nop
	lhu	$2,0($2)
	#nop
	sb	$2,-24($16)
	lw	$2,D_800A349C
	#nop
	lhu	$2,0($2)
	#nop
	sb	$2,-17($16)
	lw	$2,D_800A34A0
	#nop
	lhu	$2,0($2)
	#nop
	sb	$2,-16($16)
	lw	$2,D_800A3498
	#nop
	lhu	$2,0($2)
	#nop
	sb	$2,-9($16)
	lw	$2,D_800A34A4
	#nop
	lhu	$2,0($2)
	#nop
	sb	$2,-8($16)
	lw	$2,D_800A349C
	#nop
	lhu	$2,0($2)
	#nop
	sb	$2,-1($16)
	lw	$2,D_800A34A4
	li	$5,0x00000001		# 1
	lhu	$2,0($2)
	move	$4,$17
	.set	noreorder
	.set	nomacro
	jal	SetShadeTex
	sb	$2,0($16)
	.set	macro
	.set	reorder

	move	$4,$17
	.set	noreorder
	.set	nomacro
	jal	SetSemiTrans
	li	$5,0x00000001		# 1
	.set	macro
	.set	reorder

	lw	$3,D_800A3720
	#nop
	subu	$3,$17,$3
	sll	$2,$3,1
	addu	$2,$2,$3
	sll	$3,$2,4
	addu	$2,$2,$3
	sll	$3,$2,8
	addu	$2,$2,$3
	sll	$3,$2,16
	addu	$2,$2,$3
	subu	$2,$0,$2
	sra	$2,$2,3
	slt	$2,$2,449
	.set	noreorder
	.set	nomacro
	beq	$2,$0,.L978
	addu	$4,$21,1
	.set	macro
	.set	reorder

	li	$6,0x00ff0000		# 16711680
	ori	$6,$6,0xffff
	addu	$16,$16,40
	lw	$3,0($22)
	lw	$2,g_gpu_ot_ptr
	li	$5,-16777216			# 0xff000000
	sw	$17,D_800A34E8
	sll	$3,$3,2
	addu	$2,$2,$3
	sw	$2,D_800A34E4
	lw	$3,0($17)
	lw	$2,0($2)
	and	$3,$3,$5
	and	$2,$2,$6
	or	$3,$3,$2
	sw	$3,0($17)
	addu	$17,$17,40
	lw	$4,D_800A34E4
	lw	$3,D_800A34E8
	lw	$2,0($4)
	and	$3,$3,$6
	and	$2,$2,$5
	or	$3,$3,$2
	sw	$3,0($4)
.L950:
	addu	$4,$21,1
.L978:
	sll	$3,$4,16
	sra	$3,$3,16
	sll	$2,$3,1
	addu	$2,$2,$3
	sll	$2,$2,2
	lw	$2,D_800F1198($2)
	#nop
	andi	$2,$2,0x0001
	.set	noreorder
	.set	nomacro
	bne	$2,$0,.L951
	move	$21,$4
	.set	macro
	.set	reorder

.L949:
	sw	$17,D_800A37D4
	lw	$31,76($sp)
	lw	$fp,72($sp)
	lw	$23,68($sp)
	lw	$22,64($sp)
	lw	$21,60($sp)
	lw	$20,56($sp)
	lw	$19,52($sp)
	lw	$18,48($sp)
	lw	$17,44($sp)
	lw	$16,40($sp)
	addu	$sp,$sp,80
	j	$31