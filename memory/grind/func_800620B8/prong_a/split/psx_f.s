	.frame	$sp,72,$31		# vars= 16, regs= 10/0, args= 16, extra= 0
	.mask	0xc0ff0000,-4
	.fmask	0x00000000,0
	subu	$sp,$sp,72
	sw	$31,68($sp)
	sw	$fp,64($sp)
	sw	$23,60($sp)
	sw	$22,56($sp)
	sw	$21,52($sp)
	sw	$20,48($sp)
	sw	$19,44($sp)
	sw	$18,40($sp)
	sw	$17,36($sp)
	.set	noreorder
	.set	nomacro
	jal	func_80060E38
	sw	$16,32($sp)
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
	#nop
	lhu	$2,2($2)
	#nop
	sh	$2,2($4)
	lw	$2,4($3)
	#nop
	lhu	$2,4($2)
	#nop
	sh	$2,4($4)
	lw	$2,8($3)
	lw	$5,D_800A3470
	lw	$17,D_800A37D4
	lw	$6,D_800A3474
	lw	$2,0($2)
	#nop
	sw	$2,0($5)
	lw	$2,8($3)
	#nop
	lw	$2,4($2)
	#nop
	sw	$2,4($5)
	lw	$3,8($3)
	lw	$16,D_800A34EC
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

	lw	$3,D_800A34B0
	sll	$2,$2,8
	addu	$19,$16,16
	addu	$18,$16,18
	sw	$2,0($3)
	lw	$3,D_800A3490
	addu	$23,$16,20
	addu	$20,$16,36
	li	$2,0x0000002f		# 47
	sw	$2,0($3)
	lw	$2,D_800F1198
	addu	$8,$16,52
	addu	$fp,$16,60
	addu	$22,$16,68
	andi	$2,$2,0x0001
	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L947
	sw	$8,16($sp)
	.set	macro
	.set	reorder

	addu	$16,$17,37
$L949:
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
	beq	$3,$2,$L957
	slt	$2,$3,2
	.set	macro
	.set	reorder

	beq	$2,$0,$L961
	.set	noreorder
	.set	nomacro
	beq	$3,$0,$L953
	li	$2,0x000001c2		# 450
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	j	$L975
	sll	$3,$21,16
	.set	macro
	.set	reorder

$L961:
	li	$2,0x00000002		# 2
	.set	noreorder
	.set	nomacro
	beq	$3,$2,$L955
	li	$2,0x00000050		# 80
	.set	macro
	.set	reorder

	li	$2,0x00000003		# 3
	.set	noreorder
	.set	nomacro
	bne	$3,$2,$L975
	sll	$3,$21,16
	.set	macro
	.set	reorder

	lw	$3,D_800A34A8
	li	$2,0x00000151		# 337
	sh	$2,0($3)
	lw	$3,D_800A34AC
	.set	noreorder
	.set	nomacro
	j	$L972
	li	$2,0x000000a8		# 168
	.set	macro
	.set	reorder

$L953:
	lw	$3,D_800A34A8
	#nop
	sh	$2,0($3)
	lw	$3,D_800A34AC
	li	$2,0x000000e1		# 225
$L972:
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
	lw	$3,D_8009BD44
	sll	$4,$4,3
	la	$2,D_8009BA00
	addu	$4,$4,$2
	sw	$4,D_800A3488
	sw	$4,D_800A348C
	andi	$3,$3,0x0001
	beq	$3,$0,$L954
	la	$2,D_8009BA50
	sw	$2,D_800A348C
$L954:
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
	j	$L973
	addu	$2,$2,31
	.set	macro
	.set	reorder

$L955:
	lw	$3,D_800A34A8
	#nop
	sh	$2,0($3)
	lw	$3,D_800A34AC
	.set	noreorder
	.set	nomacro
	j	$L974
	li	$2,0x0000003c		# 60
	.set	macro
	.set	reorder

$L957:
	lw	$3,D_800A34A8
	li	$2,0x00000064		# 100
	sh	$2,0($3)
	lw	$3,D_800A34AC
	li	$2,0x00000078		# 120
$L974:
	sh	$2,0($3)
	lw	$2,D_800A32B8
	lw	$4,D_8009BD44
	la	$3,D_8009BA30
	andi	$2,$2,0x0003
	sll	$2,$2,3
	addu	$3,$2,$3
	andi	$4,$4,0x0001
	sw	$3,D_800A3488
	sw	$3,D_800A348C
	beq	$4,$0,$L958
	la	$2,D_8009BA58
	sw	$2,D_800A348C
$L958:
	lhu	$2,4($3)
	lw	$3,D_800A349C
	addu	$2,$2,15
	sh	$2,0($3)
	lw	$2,D_800A3488
	#nop
	lhu	$2,6($2)
	lw	$3,D_800A34A4
	addu	$2,$2,19
$L973:
	sh	$2,0($3)
	sll	$3,$21,16
$L975:
	sra	$3,$3,16
	sll	$2,$3,1
	addu	$2,$2,$3
	sll	$5,$2,2
	lw	$6,D_800A3470
	lw	$2,D_800F1198($5)
	lw	$4,0($6)
	srl	$3,$2,31
	addu	$2,$2,$3
	sra	$2,$2,1
	subu	$2,$2,$4
	sw	$2,0($20)
	lw	$2,D_800F1198+4($5)
	#nop
	bgez	$2,$L962
	addu	$2,$2,7
$L962:
	lw	$3,4($6)
	sra	$2,$2,3
	subu	$2,$2,$3
	sw	$2,4($20)
	lw	$2,D_800F1198+8($5)
	lw	$3,8($6)
	move	$4,$20
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

	lw	$5,D_800A34B8
	lw	$7,D_800A34CC
	lw	$4,16($sp)
	.set	noreorder
	.set	nomacro
	jal	RotTransPers
	move	$6,$fp
	.set	macro
	.set	reorder

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
	beq	$2,$0,$L948
	sw	$3,0($22)
	.set	macro
	.set	reorder

	lw	$2,D_800A34B0
	#nop
	lw	$2,0($2)
	#nop
	bgez	$2,$L964
	addu	$2,$2,255
$L964:
	sra	$2,$2,12
	sltu	$2,$2,$3
	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L976
	addu	$4,$21,1
	.set	macro
	.set	reorder

	lw	$3,D_800A348C
	#nop
	lhu	$2,0($3)
	lhu	$3,2($3)
	lw	$4,D_800A3494
	srl	$2,$2,4
	andi	$2,$2,0x003f
	sll	$3,$3,6
	addu	$2,$2,$3
	andi	$2,$2,0xffff
	sw	$2,0($4)
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
	bne	$2,$0,$L965
	li	$2,0x00000001		# 1
$L965:
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
	bne	$2,$0,$L966
	li	$2,0x00000002		# 2
	.set	macro
	.set	reorder

	srl	$2,$4,8
$L966:
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
	bne	$2,$0,$L968
	li	$2,0x00000002		# 2
	.set	macro
	.set	reorder

	srl	$2,$4,8
$L968:
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
	#nop
	lhu	$2,0($2)
	li	$5,0x00000001		# 1
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
	beq	$2,$0,$L976
	addu	$4,$21,1
	.set	macro
	.set	reorder

	lw	$2,0($22)
	lw	$3,g_gpu_ot_ptr
	li	$6,0x00ff0000		# 16711680
	sw	$17,D_800A34E8
	sll	$2,$2,2
	addu	$3,$3,$2
	sw	$3,D_800A34E4
	lw	$2,0($17)
	lw	$3,0($3)
	ori	$6,$6,0xffff
	li	$5,-16777216			# 0xff000000
	and	$2,$2,$5
	and	$3,$3,$6
	or	$2,$2,$3
	sw	$2,0($17)
	lw	$4,D_800A34E4
	lw	$2,D_800A34E8
	lw	$3,0($4)
	addu	$16,$16,40
	addu	$17,$17,40
	and	$2,$2,$6
	and	$3,$3,$5
	or	$2,$2,$3
	sw	$2,0($4)
$L948:
	addu	$4,$21,1
$L976:
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
	bne	$2,$0,$L949
	move	$21,$4
	.set	macro
	.set	reorder

$L947:
	sw	$17,D_800A37D4
	lw	$31,68($sp)
	lw	$fp,64($sp)
	lw	$23,60($sp)
	lw	$22,56($sp)
	lw	$21,52($sp)
	lw	$20,48($sp)
	lw	$19,44($sp)
	lw	$18,40($sp)
	lw	$17,36($sp)
	lw	$16,32($sp)
	addu	$sp,$sp,72
	j	$31