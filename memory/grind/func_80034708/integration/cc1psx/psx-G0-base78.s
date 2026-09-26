	.file	1 "src/code6cac_b3.c"

 # GNU C 2.7.2.SN.1 [AL 1.1, MM 40] Sony Playstation compiled by GNU C

 # Cc1 defaults:
 # -mgas -msoft-float

 # Cc1 arguments (-G value = 0, Cpu = 3000, ISA = 1):
 # -quiet -O2 -G0 -funsigned-char -mcpu=3000 -mips1 -msoft-float -w -o

gcc2_compiled.:
__gnu_compiled_c:
 #APP
	.include "include/labels.inc"

 #NO_APP
	.text
	.align	2
	.globl	func_80034708
	.ent	func_80034708
func_80034708:
	.frame	$sp,64,$31		# vars= 8, regs= 10/0, args= 16, extra= 0
	.mask	0xc0ff0000,-4
	.fmask	0x00000000,0
	lw	$2,D_800A37B8
	subu	$sp,$sp,64
	sw	$31,60($sp)
	sw	$fp,56($sp)
	sw	$23,52($sp)
	sw	$22,48($sp)
	sw	$21,44($sp)
	sw	$20,40($sp)
	sw	$19,36($sp)
	sw	$18,32($sp)
	sw	$17,28($sp)
	sw	$16,24($sp)
	addu	$2,$2,1
	sw	$2,D_800A37B8
	jal	rand
	la	$17,D_800A3178
	la	$16,D_800A3174
	lh	$2,0($16)
	la	$18,D_800A3180
	la	$21,D_800A3188
	.set	noreorder
	.set	nomacro
	bne	$2,$0,$L2
	move	$5,$17
	.set	macro
	.set	reorder

	move	$5,$18
$L2:
	la	$22,D_80102778+4
	lb	$6,0($22)
	.set	noreorder
	.set	nomacro
	jal	func_8003D52C
	move	$4,$21
	.set	macro
	.set	reorder

	lh	$2,2($16)
	la	$20,D_800A3190
	.set	noreorder
	.set	nomacro
	bne	$2,$0,$L4
	move	$5,$17
	.set	macro
	.set	reorder

	move	$5,$18
$L4:
	lb	$6,D_80102778+5
	.set	noreorder
	.set	nomacro
	jal	func_8003D52C
	move	$4,$20
	.set	macro
	.set	reorder

	lh	$2,0($16)
	li	$19,0x00000001		# 1
	.set	noreorder
	.set	nomacro
	bne	$2,$19,$L6
	move	$5,$17
	.set	macro
	.set	reorder

	move	$5,$18
$L6:
	lb	$6,D_80102778+6
	.set	noreorder
	.set	nomacro
	jal	func_8003D52C
	move	$4,$21
	.set	macro
	.set	reorder

	lh	$2,2($16)
	#nop
	.set	noreorder
	.set	nomacro
	bne	$2,$19,$L8
	move	$5,$17
	.set	macro
	.set	reorder

	move	$5,$18
$L8:
	lb	$6,D_80102778+7
	.set	noreorder
	.set	nomacro
	jal	func_8003D52C
	move	$4,$20
	.set	macro
	.set	reorder

	lh	$2,0($16)
	li	$19,0x00000002		# 2
	.set	noreorder
	.set	nomacro
	bne	$2,$19,$L10
	move	$5,$17
	.set	macro
	.set	reorder

	move	$5,$18
$L10:
	lb	$6,D_80102778+8
	la	$4,D_800A3198
	jal	func_8003D52C
	lh	$2,2($16)
	#nop
	.set	noreorder
	.set	nomacro
	bne	$2,$19,$L12
	move	$5,$17
	.set	macro
	.set	reorder

	move	$5,$18
$L12:
	lb	$6,D_80102778+9
	.set	noreorder
	.set	nomacro
	jal	func_8003D52C
	move	$4,$20
	.set	macro
	.set	reorder

	lh	$2,0($16)
	li	$19,0x00000003		# 3
	.set	noreorder
	.set	nomacro
	bne	$2,$19,$L14
	move	$5,$17
	.set	macro
	.set	reorder

	move	$5,$18
$L14:
	lhu	$6,D_80102778
	la	$4,D_800A31A0
	.set	noreorder
	.set	nomacro
	jal	func_8003D52C
	srl	$6,$6,8
	.set	macro
	.set	reorder

	lh	$2,2($16)
	#nop
	.set	noreorder
	.set	nomacro
	bne	$2,$19,$L16
	move	$5,$17
	.set	macro
	.set	reorder

	move	$5,$18
$L16:
	lhu	$6,D_80102778+2
	la	$4,D_800A31A8
	.set	noreorder
	.set	nomacro
	jal	func_8003D52C
	srl	$6,$6,8
	.set	macro
	.set	reorder

	lh	$3,0($16)
	la	$19,D_800A31B0
	li	$2,0x00000004		# 4
	.set	noreorder
	.set	nomacro
	bne	$3,$2,$L18
	move	$5,$17
	.set	macro
	.set	reorder

	move	$5,$18
$L18:
	lb	$6,D_80102778+12
	.set	noreorder
	.set	nomacro
	jal	func_8003D52C
	move	$4,$19
	.set	macro
	.set	reorder

	lh	$3,0($16)
	li	$2,0x00000005		# 5
	.set	noreorder
	.set	nomacro
	bne	$3,$2,$L20
	move	$5,$17
	.set	macro
	.set	reorder

	move	$5,$18
$L20:
	lb	$6,D_80102778+13
	.set	noreorder
	.set	nomacro
	jal	func_8003D52C
	move	$4,$19
	.set	macro
	.set	reorder

	lh	$3,0($16)
	li	$2,0x00000006		# 6
	.set	noreorder
	.set	nomacro
	bne	$3,$2,$L22
	move	$5,$17
	.set	macro
	.set	reorder

	move	$5,$18
$L22:
	lb	$6,10($22)
	la	$4,D_800A31B8
	jal	func_8003D52C
	lh	$3,0($16)
	li	$2,0x00000007		# 7
	.set	noreorder
	.set	nomacro
	bne	$3,$2,$L24
	move	$5,$17
	.set	macro
	.set	reorder

	move	$5,$18
$L24:
	lb	$6,11($22)
	la	$4,D_80010834
	jal	func_8003D52C
	lh	$3,0($16)
	li	$2,0x00000008		# 8
	.set	noreorder
	.set	nomacro
	bne	$3,$2,$L26
	move	$5,$17
	.set	macro
	.set	reorder

	move	$5,$18
$L26:
	la	$19,D_80106A50+35
	lbu	$6,0($19)
	la	$4,D_80010840
	.set	noreorder
	.set	nomacro
	jal	func_8003D52C
	andi	$6,$6,0x0001
	.set	macro
	.set	reorder

	lh	$3,0($16)
	li	$2,0x00000009		# 9
	.set	noreorder
	.set	nomacro
	bne	$3,$2,$L28
	move	$5,$17
	.set	macro
	.set	reorder

	move	$5,$18
$L28:
	lbu	$6,0($19)
	la	$4,D_800A31C0
	srl	$6,$6,1
	.set	noreorder
	.set	nomacro
	jal	func_8003D52C
	andi	$6,$6,0x0001
	.set	macro
	.set	reorder

	lh	$3,0($16)
	li	$2,0x0000000a		# 10
	.set	noreorder
	.set	nomacro
	bne	$3,$2,$L30
	move	$5,$17
	.set	macro
	.set	reorder

	move	$5,$18
$L30:
	lbu	$6,D_800A36F9
	la	$4,D_800A31C8
	jal	func_8003D52C
	lh	$3,0($16)
	li	$2,0x0000000b		# 11
	.set	noreorder
	.set	nomacro
	bne	$3,$2,$L32
	move	$5,$17
	.set	macro
	.set	reorder

	move	$5,$18
$L32:
	la	$4,D_800A31D0
	lbu	$6,D_800A3690
	move	$20,$0
	la	$23,D_80102778+12
	addu	$fp,$23,2
	move	$21,$19
	addu	$19,$23,-4
	addu	$18,$23,-6
	addu	$17,$23,-8
	la	$16,D_800A3174
	.set	noreorder
	.set	nomacro
	jal	func_8003D52C
	move	$22,$0
	.set	macro
	.set	reorder

$L37:
	lw	$4,D_80102788+12
	sll	$3,$20,4
	li	$2,0x00001000		# 4096
	sll	$2,$2,$3
	and	$2,$4,$2
	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L38
	li	$5,0x0000007f		# 127
	.set	macro
	.set	reorder

	move	$4,$0
	.set	noreorder
	.set	nomacro
	jal	func_8005C650
	li	$6,0x0000007f		# 127
	.set	macro
	.set	reorder

	lh	$2,0($16)
	#nop
	.set	noreorder
	.set	nomacro
	blez	$2,$L39
	move	$3,$2
	.set	macro
	.set	reorder

	addu	$2,$3,-1
	.set	noreorder
	.set	nomacro
	j	$L43
	sh	$2,0($16)
	.set	macro
	.set	reorder

$L39:
	beq	$20,$0,$L41
	li	$2,0x00000003		# 3
	.set	noreorder
	.set	nomacro
	j	$L43
	sh	$2,0($16)
	.set	macro
	.set	reorder

$L41:
	li	$2,0x0000000b		# 11
	.set	noreorder
	.set	nomacro
	j	$L43
	sh	$2,0($16)
	.set	macro
	.set	reorder

$L38:
	li	$2,0x00004000		# 16384
	sll	$2,$2,$3
	and	$2,$4,$2
	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L44
	li	$2,0x00008000		# 32768
	.set	macro
	.set	reorder

	move	$4,$0
	.set	noreorder
	.set	nomacro
	jal	func_8005C650
	li	$6,0x0000007f		# 127
	.set	macro
	.set	reorder

	lh	$2,0($16)
	beq	$20,$0,$L46
	slt	$2,$2,3
	bne	$2,$0,$L47
	.set	noreorder
	.set	nomacro
	j	$L43
	sh	$0,0($16)
	.set	macro
	.set	reorder

$L46:
	slt	$2,$2,11
	beq	$2,$0,$L45
$L47:
	lhu	$2,0($16)
	#nop
	addu	$2,$2,1
	.set	noreorder
	.set	nomacro
	j	$L43
	sh	$2,0($16)
	.set	macro
	.set	reorder

$L45:
	.set	noreorder
	.set	nomacro
	j	$L43
	sh	$0,0($16)
	.set	macro
	.set	reorder

$L44:
	sll	$2,$2,$3
	and	$2,$4,$2
	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L50
	li	$5,0x0000003f		# 63
	.set	macro
	.set	reorder

	li	$4,0x00000004		# 4
	.set	noreorder
	.set	nomacro
	jal	func_8005C650
	li	$6,0x0000003f		# 63
	.set	macro
	.set	reorder

	lh	$3,0($16)
	#nop
	sltu	$2,$3,12
	beq	$2,$0,$L43
	sll	$2,$3,2
	lw	$2,$L64($2)
	#nop
	j	$2
	.rdata
	.align	3
$L64:
	.word	$L52
	.word	$L53
	.word	$L54
	.word	$L55
	.word	$L56
	.word	$L57
	.word	$L58
	.word	$L59
	.word	$L77
	.word	$L78
	.word	$L62
	.word	$L63
	.text
$L52:
	lbu	$2,0($17)
	#nop
	addu	$2,$2,-1
	.set	noreorder
	.set	nomacro
	j	$L43
	sb	$2,0($17)
	.set	macro
	.set	reorder

$L53:
	lbu	$2,0($18)
	#nop
	addu	$2,$2,-1
	.set	noreorder
	.set	nomacro
	j	$L43
	sb	$2,0($18)
	.set	macro
	.set	reorder

$L54:
	lbu	$2,0($19)
	#nop
	addu	$2,$2,-1
	.set	noreorder
	.set	nomacro
	j	$L43
	sb	$2,0($19)
	.set	macro
	.set	reorder

$L55:
	la	$3,D_80102778
	addu	$3,$22,$3
	lhu	$2,0($3)
	#nop
	addu	$2,$2,-128
	.set	noreorder
	.set	nomacro
	j	$L43
	sh	$2,0($3)
	.set	macro
	.set	reorder

$L56:
	lbu	$2,0($23)
	#nop
	addu	$2,$2,-1
	.set	noreorder
	.set	nomacro
	j	$L43
	sb	$2,0($23)
	.set	macro
	.set	reorder

$L57:
	lbu	$2,D_80102778+13
	#nop
	addu	$2,$2,-1
	sb	$2,D_80102778+13
	j	$L43
$L58:
	lbu	$2,0($fp)
	#nop
	addu	$2,$2,-1
	.set	noreorder
	.set	nomacro
	j	$L43
	sb	$2,0($fp)
	.set	macro
	.set	reorder

$L59:
	la	$3,D_80102778+15
	lbu	$2,0($3)
	#nop
	addu	$2,$2,-1
	.set	noreorder
	.set	nomacro
	j	$L43
	sb	$2,0($3)
	.set	macro
	.set	reorder

$L62:
	lbu	$2,D_800A36F9
	#nop
	addu	$2,$2,-1
	sb	$2,D_800A36F9
	j	$L43
$L63:
	lbu	$2,D_800A3690
	.set	noreorder
	.set	nomacro
	j	$L87
	addu	$2,$2,-1
	.set	macro
	.set	reorder

$L50:
	li	$2,0x00002000		# 8192
	sll	$2,$2,$3
	and	$2,$4,$2
	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L43
	li	$4,0x00000004		# 4
	.set	macro
	.set	reorder

	li	$5,0x0000003f		# 63
	.set	noreorder
	.set	nomacro
	jal	func_8005C650
	li	$6,0x0000003f		# 63
	.set	macro
	.set	reorder

	lh	$3,0($16)
	#nop
	sltu	$2,$3,12
	beq	$2,$0,$L43
	sll	$2,$3,2
	lw	$2,$L81($2)
	#nop
	j	$2
	.rdata
	.align	3
$L81:
	.word	$L69
	.word	$L70
	.word	$L71
	.word	$L72
	.word	$L73
	.word	$L74
	.word	$L75
	.word	$L76
	.word	$L77
	.word	$L78
	.word	$L79
	.word	$L80
	.text
$L69:
	lbu	$2,0($17)
	#nop
	addu	$2,$2,1
	.set	noreorder
	.set	nomacro
	j	$L43
	sb	$2,0($17)
	.set	macro
	.set	reorder

$L70:
	lbu	$2,0($18)
	#nop
	addu	$2,$2,1
	.set	noreorder
	.set	nomacro
	j	$L43
	sb	$2,0($18)
	.set	macro
	.set	reorder

$L71:
	lbu	$2,0($19)
	#nop
	addu	$2,$2,1
	.set	noreorder
	.set	nomacro
	j	$L43
	sb	$2,0($19)
	.set	macro
	.set	reorder

$L72:
	la	$3,D_80102778
	addu	$3,$22,$3
	lhu	$2,0($3)
	#nop
	addu	$2,$2,128
	.set	noreorder
	.set	nomacro
	j	$L43
	sh	$2,0($3)
	.set	macro
	.set	reorder

$L73:
	lbu	$2,0($23)
	#nop
	addu	$2,$2,1
	.set	noreorder
	.set	nomacro
	j	$L43
	sb	$2,0($23)
	.set	macro
	.set	reorder

$L74:
	lbu	$2,D_80102778+13
	#nop
	addu	$2,$2,1
	sb	$2,D_80102778+13
	j	$L43
$L75:
	lbu	$2,0($fp)
	#nop
	addu	$2,$2,1
	.set	noreorder
	.set	nomacro
	j	$L43
	sb	$2,0($fp)
	.set	macro
	.set	reorder

$L76:
	la	$3,D_80102778+15
	lbu	$2,0($3)
	#nop
	addu	$2,$2,1
	.set	noreorder
	.set	nomacro
	j	$L43
	sb	$2,0($3)
	.set	macro
	.set	reorder

$L77:
	lbu	$2,0($21)
	#nop
	xori	$2,$2,0x0001
	.set	noreorder
	.set	nomacro
	j	$L43
	sb	$2,0($21)
	.set	macro
	.set	reorder

$L78:
	lbu	$2,0($21)
	#nop
	xori	$2,$2,0x0002
	.set	noreorder
	.set	nomacro
	j	$L43
	sb	$2,0($21)
	.set	macro
	.set	reorder

$L79:
	lbu	$2,D_800A36F9
	#nop
	addu	$2,$2,1
	sb	$2,D_800A36F9
	j	$L43
$L80:
	lbu	$2,D_800A3690
	#nop
	addu	$2,$2,1
$L87:
	sb	$2,D_800A3690
$L43:
	lb	$4,0($17)
	li	$2,0x3e0f0000		# 1041170432
	ori	$2,$2,0x83e1
	addu	$4,$4,33
	mult	$4,$2
	sra	$2,$4,31
	mfhi	$12
	#nop
	#nop
	sra	$3,$12,3
	subu	$3,$3,$2
	sll	$2,$3,5
	addu	$2,$2,$3
	subu	$4,$4,$2
	sb	$4,0($17)
	lb	$4,0($18)
	#nop
	addu	$3,$4,8
	.set	noreorder
	.set	nomacro
	bgez	$3,$L83
	move	$2,$3
	.set	macro
	.set	reorder

	addu	$2,$4,15
$L83:
	sra	$2,$2,3
	sll	$2,$2,3
	subu	$2,$3,$2
	sb	$2,0($18)
	addu	$18,$18,1
	addu	$17,$17,1
	lb	$3,0($19)
	addu	$22,$22,2
	addu	$16,$16,2
	addu	$20,$20,1
	addu	$3,$3,2
	srl	$2,$3,31
	addu	$2,$3,$2
	sra	$2,$2,1
	sll	$2,$2,1
	subu	$3,$3,$2
	sb	$3,0($19)
	slt	$2,$20,2
	.set	noreorder
	.set	nomacro
	bne	$2,$0,$L37
	addu	$19,$19,1
	.set	macro
	.set	reorder

	la	$7,D_80102778+12
	lb	$5,0($7)
	li	$2,0x6bca0000		# 1808400384
	ori	$2,$2,0x1af3
	addu	$5,$5,38
	mult	$5,$2
	li	$6,-1840709632			# 0x92490000
	ori	$6,$6,0x2493
	lbu	$10,D_800A36F9
	lb	$4,D_80102778+13
	lbu	$3,D_80102778+15
	lbu	$2,D_80102778+14
	addu	$8,$10,4
	addu	$4,$4,7
	andi	$3,$3,0x0001
	sb	$3,D_80102778+15
	mfhi	$9
	#nop
	andi	$2,$2,0x0001
	sb	$2,D_80102778+14
	mult	$4,$6
	sra	$2,$5,31
	sra	$3,$9,4
	subu	$3,$3,$2
	sll	$2,$3,2
	addu	$2,$2,$3
	sll	$2,$2,2
	subu	$2,$2,$3
	sll	$2,$2,1
	subu	$5,$5,$2
	sra	$2,$4,31
	sb	$5,0($7)
	mfhi	$13
	#nop
	#nop
	addu	$3,$13,$4
	sra	$3,$3,2
	subu	$3,$3,$2
	sll	$2,$3,3
	subu	$2,$2,$3
	subu	$4,$4,$2
	sb	$4,D_80102778+13
	.set	noreorder
	.set	nomacro
	bgez	$8,$L85
	move	$11,$8
	.set	macro
	.set	reorder

	addu	$11,$10,7
$L85:
	li	$5,0x08000000		# 134217728
	lbu	$3,D_800A3690
	lw	$4,D_80102788+12
	ori	$5,$5,0x0800
	andi	$2,$11,0x01fc
	subu	$2,$8,$2
	sb	$2,D_800A36F9
	andi	$3,$3,0x0001
	and	$4,$4,$5
	sb	$3,D_800A3690
	.set	noreorder
	.set	nomacro
	beq	$4,$0,$L86
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

	jal	func_800344B4
$L86:
	lw	$31,60($sp)
	lw	$fp,56($sp)
	lw	$23,52($sp)
	lw	$22,48($sp)
	lw	$21,44($sp)
	lw	$20,40($sp)
	lw	$19,36($sp)
	lw	$18,32($sp)
	lw	$17,28($sp)
	lw	$16,24($sp)
	addu	$sp,$sp,64
	j	$31
	.end	func_80034708
