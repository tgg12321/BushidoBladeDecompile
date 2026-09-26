	.file	1 "tmp/func_80036140/p3/src/code6cac_b5.c"

 # GNU C 2.7.2.SN.1 [AL 1.1, MM 40] Sony Playstation compiled by GNU C

 # Cc1 defaults:
 # -mgas -msoft-float

 # Cc1 arguments (-G value = 8, Cpu = 3000, ISA = 1):
 # -quiet -O2 -G8 -funsigned-char -mcpu=3000 -mips1 -msoft-float -w -o

gcc2_compiled.:
__gnu_compiled_c:
 #APP
	.include "include/labels.inc"

 #NO_APP
	.text
	.align	2
	.globl	func_80036140
	.align	2
	.globl	func_80036940

	.extern	g_cd_result_plus_0x5, 1
	.extern	g_cd_result_plus_0x3, 1
	.extern	g_cd_result_plus_0x4, 1
	.extern	g_cd_result, 1
	.extern	D_80101E58, 80
	.extern	g_cd_atv, 4
	.extern	D_800A3840, 2
	.extern	D_800A36B8, 4
	.extern	D_800A3854, 2

	.text
	.text
	.ent	func_80036140
func_80036140:
	.frame	$sp,32,$31		# vars= 8, regs= 2/0, args= 16, extra= 0
	.mask	0x80010000,-4
	.fmask	0x00000000,0
	lh	$8,D_800A3854
	subu	$sp,$sp,32
	sw	$31,28($sp)
	.set	noreorder
	.set	nomacro
	blez	$8,$L2
	sw	$16,24($sp)
	.set	macro
	.set	reorder

	lbu	$2,D_800A36B8
	lh	$4,D_800A3840
	#nop
	mult	$2,$4
	lbu	$2,g_cd_atv
	mflo	$3
	#nop
	subu	$6,$8,$4
	mult	$2,$6
	mflo	$10
	#nop
	#nop
	addu	$7,$3,$10
	div	$7,$7,$8
	lbu	$2,D_800A36B8+1
	#nop
	mult	$2,$4
	lbu	$2,g_cd_atv+1
	mflo	$3
	#nop
	#nop
	mult	$2,$6
	mflo	$10
	#nop
	#nop
	addu	$5,$3,$10
	div	$5,$5,$8
	lbu	$2,D_800A36B8+2
	#nop
	mult	$2,$4
	lbu	$2,g_cd_atv+2
	mflo	$3
	#nop
	#nop
	mult	$2,$6
	mflo	$10
	#nop
	#nop
	addu	$3,$3,$10
	div	$3,$3,$8
	lbu	$2,D_800A36B8+3
	#nop
	mult	$2,$4
	lbu	$2,g_cd_atv+3
	mflo	$4
	#nop
	#nop
	mult	$2,$6
	mflo	$10
	#nop
	#nop
	addu	$2,$4,$10
	div	$2,$2,$8
	sb	$7,16($sp)
	sb	$5,17($sp)
	addu	$4,$sp,16
	sb	$3,18($sp)
	.set	noreorder
	.set	nomacro
	jal	CdMix
	sb	$2,19($sp)
	.set	macro
	.set	reorder

	lhu	$2,D_800A3840
	lh	$3,D_800A3854
	addu	$2,$2,1
	sh	$2,D_800A3840
	sll	$2,$2,16
	sra	$2,$2,16
	slt	$2,$2,$3
	bne	$2,$0,$L2
	sh	$0,D_800A3854
	la	$5,D_800A36B8
	la	$4,g_cd_atv
	lwl	$2,3($5)
	lwr	$2,0($5)
	swl	$2,3($4)
	swr	$2,0($4)
$L2:
	lhu	$2,D_80101E58+10
	#nop
	addu	$2,$2,-16
	sll	$2,$2,16
	sra	$3,$2,16
	sltu	$2,$3,15
	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L4
	sll	$2,$3,2
	.set	macro
	.set	reorder

	lw	$2,$L80($2)
	#nop
	j	$2
	.rdata
	.align	3
$L80:
	.word	$L5
	.word	$L6
	.word	$L10
	.word	$L15
	.word	$L22
	.word	$L26
	.word	$L38
	.word	$L63
	.word	$L66
	.word	$L67
	.word	$L73
	.word	$L74
	.word	$L56
	.word	$L57
	.word	$L61
	.text
$L5:
	move	$4,$0
	move	$5,$0
	move	$6,$0
	.set	noreorder
	.set	nomacro
	jal	cdrom_SetMix
	move	$7,$0
	.set	macro
	.set	reorder

	la	$5,D_80101E58+56
	.set	noreorder
	.set	nomacro
	jal	CdControlF
	li	$4,0x0000000e		# 14
	.set	macro
	.set	reorder

	sw	$0,D_80101E58+48
	sw	$0,D_80101E58+52
	sw	$0,D_80101E58+76
	.set	noreorder
	.set	nomacro
	j	$L82
	li	$2,0x00000011		# 17
	.set	macro
	.set	reorder

$L6:
	la	$5,g_cd_result
	.set	noreorder
	.set	nomacro
	jal	CdSync
	li	$4,0x00000001		# 1
	.set	macro
	.set	reorder

	move	$3,$2
	li	$2,0x00000002		# 2
	.set	noreorder
	.set	nomacro
	bne	$3,$2,$L84
	li	$2,0x00000005		# 5
	.set	macro
	.set	reorder

	la	$5,D_80101E58+20
	.set	noreorder
	.set	nomacro
	jal	CdControlF
	li	$4,0x00000002		# 2
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	j	$L82
	li	$2,0x00000012		# 18
	.set	macro
	.set	reorder

$L10:
	la	$3,D_80101E58+52
	lw	$2,0($3)
	#nop
	addu	$2,$2,1
	sw	$2,0($3)
	slt	$2,$2,3
	bne	$2,$0,$L4
	la	$5,g_cd_result
	.set	noreorder
	.set	nomacro
	jal	CdSync
	li	$4,0x00000001		# 1
	.set	macro
	.set	reorder

	move	$3,$2
	li	$2,0x00000002		# 2
	.set	noreorder
	.set	nomacro
	bne	$3,$2,$L84
	li	$2,0x00000005		# 5
	.set	macro
	.set	reorder

	li	$4,0x00000016		# 22
	.set	noreorder
	.set	nomacro
	jal	CdControlF
	move	$5,$0
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	j	$L82
	li	$2,0x00000013		# 19
	.set	macro
	.set	reorder

$L15:
	la	$5,g_cd_result
	.set	noreorder
	.set	nomacro
	jal	CdSync
	li	$4,0x00000001		# 1
	.set	macro
	.set	reorder

	move	$3,$2
	li	$2,0x00000002		# 2
	.set	noreorder
	.set	nomacro
	bne	$3,$2,$L84
	li	$2,0x00000005		# 5
	.set	macro
	.set	reorder

	lh	$2,D_80101E58+12
	#nop
	bne	$2,$0,$L4
	lw	$2,D_80101E58+60
	#nop
	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L18
	li	$4,0x00000003		# 3
	.set	macro
	.set	reorder

	li	$4,0x0000001b		# 27
$L18:
	.set	noreorder
	.set	nomacro
	jal	CdControlF
	move	$5,$0
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	j	$L82
	li	$2,0x00000014		# 20
	.set	macro
	.set	reorder

$L22:
	la	$5,g_cd_result
	.set	noreorder
	.set	nomacro
	jal	CdSync
	li	$4,0x00000001		# 1
	.set	macro
	.set	reorder

	move	$3,$2
	li	$2,0x00000002		# 2
	.set	noreorder
	.set	nomacro
	bne	$3,$2,$L84
	li	$2,0x00000005		# 5
	.set	macro
	.set	reorder

	sh	$0,D_80101E58+68
	.set	noreorder
	.set	nomacro
	j	$L82
	li	$2,0x00000015		# 21
	.set	macro
	.set	reorder

$L26:
	la	$16,D_80101E58+48
	lw	$2,0($16)
	li	$3,0x00000002		# 2
	addu	$2,$2,1
	.set	noreorder
	.set	nomacro
	bne	$2,$3,$L27
	sw	$2,0($16)
	.set	macro
	.set	reorder

	li	$4,0x000000ff		# 255
	move	$5,$0
	li	$6,0x000000ff		# 255
	.set	noreorder
	.set	nomacro
	jal	cdrom_SetMix
	move	$7,$0
	.set	macro
	.set	reorder

$L27:
	lh	$2,D_80101E58+16
	#nop
	beq	$2,$0,$L28
	lw	$2,0($16)
	#nop
	slt	$2,$2,5
	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L82
	li	$2,0x0000001c		# 28
	.set	macro
	.set	reorder

$L28:
	lw	$3,D_80101E58+60
	la	$16,D_80101E58+10
	li	$2,0x00000016		# 22
	.set	noreorder
	.set	nomacro
	beq	$3,$0,$L29
	sh	$2,0($16)
	.set	macro
	.set	reorder

	la	$5,g_cd_result
	.set	noreorder
	.set	nomacro
	jal	CdControlF
	li	$4,0x00000011		# 17
	.set	macro
	.set	reorder

	sh	$0,D_80101E58+66
	j	$L4
$L29:
	li	$4,0x00000001		# 1
	.set	noreorder
	.set	nomacro
	jal	CdControlF
	move	$5,$0
	.set	macro
	.set	reorder

	la	$5,g_cd_result
	.set	noreorder
	.set	nomacro
	jal	CdReady
	li	$4,0x00000001		# 1
	.set	macro
	.set	reorder

	move	$3,$2
	li	$2,0x00000001		# 1
	.set	noreorder
	.set	nomacro
	bne	$3,$2,$L30
	li	$2,0x00000005		# 5
	.set	macro
	.set	reorder

	lbu	$2,g_cd_result_plus_0x4
	#nop
	andi	$2,$2,0x0080
	bne	$2,$0,$L35
	la	$4,g_cd_result_plus_0x3
	sh	$0,D_80101E58+68
	jal	CdPosToInt
	lw	$3,D_80101E58+28
	#nop
	slt	$2,$2,$3
	.set	noreorder
	.set	nomacro
	bne	$2,$0,$L35
	move	$4,$0
	.set	macro
	.set	reorder

	move	$5,$0
	move	$6,$0
	.set	noreorder
	.set	nomacro
	jal	cdrom_SetMix
	move	$7,$0
	.set	macro
	.set	reorder

	lh	$2,D_80101E58+18
	#nop
	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L33
	li	$3,0x0000001c		# 28
	.set	macro
	.set	reorder

	li	$3,0x00000010		# 16
$L33:
	.set	noreorder
	.set	nomacro
	j	$L35
	sh	$3,0($16)
	.set	macro
	.set	reorder

$L30:
	.set	noreorder
	.set	nomacro
	bne	$3,$2,$L35
	li	$2,0x00000017		# 23
	.set	macro
	.set	reorder

	sh	$2,0($16)
$L35:
	la	$2,D_80101E58+68
	lhu	$3,0($2)
	#nop
	addu	$4,$3,1
	sll	$3,$3,16
	sra	$3,$3,16
	slt	$3,$3,61
	.set	noreorder
	.set	nomacro
	bne	$3,$0,$L4
	sh	$4,0($2)
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	j	$L82
	li	$2,0x00000017		# 23
	.set	macro
	.set	reorder

$L38:
	la	$3,D_80101E58+76
	lw	$2,0($3)
	#nop
	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L39
	addu	$2,$2,-4
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	bgtz	$2,$L39
	sw	$2,0($3)
	.set	macro
	.set	reorder

	move	$4,$0
	move	$5,$0
	move	$6,$0
	.set	noreorder
	.set	nomacro
	jal	cdrom_SetMix
	move	$7,$0
	.set	macro
	.set	reorder

	lh	$2,D_80101E58+18
	#nop
	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L41
	li	$3,0x0000001c		# 28
	.set	macro
	.set	reorder

	li	$3,0x00000010		# 16
$L41:
	sh	$3,D_80101E58+10
	j	$L4
$L39:
	la	$5,g_cd_result
	.set	noreorder
	.set	nomacro
	jal	CdSync
	li	$4,0x00000001		# 1
	.set	macro
	.set	reorder

	move	$3,$2
	li	$2,0x00000002		# 2
	.set	noreorder
	.set	nomacro
	bne	$3,$2,$L43
	li	$2,0x00000005		# 5
	.set	macro
	.set	reorder

	lw	$3,D_80101E58+60
	la	$16,D_80101E58+10
	li	$2,0x00000015		# 21
	.set	noreorder
	.set	nomacro
	beq	$3,$0,$L4
	sh	$2,0($16)
	.set	macro
	.set	reorder

	la	$4,g_cd_result_plus_0x5
	jal	CdPosToInt
	lw	$4,D_80101E58+28
	move	$3,$2
	slt	$2,$3,$4
	.set	noreorder
	.set	nomacro
	bne	$2,$0,$L45
	addu	$2,$4,-150
	.set	macro
	.set	reorder

	move	$4,$0
	move	$5,$0
	move	$6,$0
	.set	noreorder
	.set	nomacro
	jal	cdrom_SetMix
	move	$7,$0
	.set	macro
	.set	reorder

	lh	$2,D_80101E58+18
	#nop
	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L46
	li	$3,0x0000001c		# 28
	.set	macro
	.set	reorder

	li	$3,0x00000010		# 16
$L46:
	.set	noreorder
	.set	nomacro
	j	$L4
	sh	$3,0($16)
	.set	macro
	.set	reorder

$L45:
	slt	$2,$3,$2
	bne	$2,$0,$L4
	lw	$2,D_80101E58+76
	#nop
	.set	noreorder
	.set	nomacro
	bne	$2,$0,$L4
	subu	$2,$4,$3
	.set	macro
	.set	reorder

	sw	$2,D_80101E58+76
	j	$L4
$L43:
	.set	noreorder
	.set	nomacro
	beq	$3,$2,$L82
	li	$2,0x00000017		# 23
	.set	macro
	.set	reorder

	lw	$2,D_80101E58+60
	#nop
	beq	$2,$0,$L4
	lhu	$2,D_80101E58+66
	#nop
	addu	$2,$2,1
	sh	$2,D_80101E58+66
	sll	$2,$2,16
	sra	$2,$2,16
	slt	$2,$2,31
	bne	$2,$0,$L4
	jal	CdFlush
	.set	noreorder
	.set	nomacro
	j	$L82
	li	$2,0x00000015		# 21
	.set	macro
	.set	reorder

$L56:
	move	$4,$0
	move	$5,$0
	move	$6,$0
	.set	noreorder
	.set	nomacro
	jal	cdrom_SetMix
	move	$7,$0
	.set	macro
	.set	reorder

	li	$4,0x00000009		# 9
	.set	noreorder
	.set	nomacro
	jal	CdControlF
	move	$5,$0
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	j	$L82
	li	$2,0x0000001d		# 29
	.set	macro
	.set	reorder

$L57:
	la	$5,g_cd_result
	.set	noreorder
	.set	nomacro
	jal	CdSync
	li	$4,0x00000001		# 1
	.set	macro
	.set	reorder

	move	$3,$2
	li	$2,0x00000002		# 2
	.set	noreorder
	.set	nomacro
	bne	$3,$2,$L84
	li	$2,0x00000005		# 5
	.set	macro
	.set	reorder

	sw	$0,D_80101E58+48
	.set	noreorder
	.set	nomacro
	j	$L82
	li	$2,0x0000001e		# 30
	.set	macro
	.set	reorder

$L61:
	la	$3,D_80101E58+48
	lw	$2,0($3)
	#nop
	addu	$2,$2,1
	sw	$2,0($3)
	slt	$2,$2,5
	bne	$2,$0,$L4
	sh	$0,D_80101E58+10
	j	$L4
$L63:
	move	$4,$0
	move	$5,$0
	move	$6,$0
	.set	noreorder
	.set	nomacro
	jal	cdrom_SetMix
	move	$7,$0
	.set	macro
	.set	reorder

	lbu	$2,g_cd_result
	#nop
	andi	$2,$2,0x0010
	.set	noreorder
	.set	nomacro
	bne	$2,$0,$L82
	li	$2,0x00000018		# 24
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	j	$L82
	li	$2,0x0000001a		# 26
	.set	macro
	.set	reorder

$L66:
	li	$4,0x00000001		# 1
	.set	noreorder
	.set	nomacro
	jal	CdControlF
	move	$5,$0
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	j	$L82
	li	$2,0x00000019		# 25
	.set	macro
	.set	reorder

$L67:
	la	$5,g_cd_result
	.set	noreorder
	.set	nomacro
	jal	CdSync
	li	$4,0x00000001		# 1
	.set	macro
	.set	reorder

	move	$3,$2
	li	$2,0x00000002		# 2
	.set	noreorder
	.set	nomacro
	bne	$3,$2,$L68
	li	$2,0x00000005		# 5
	.set	macro
	.set	reorder

	lbu	$2,g_cd_result
	#nop
	andi	$2,$2,0x0010
	.set	noreorder
	.set	nomacro
	bne	$2,$0,$L82
	li	$2,0x00000018		# 24
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	j	$L82
	li	$2,0x0000001a		# 26
	.set	macro
	.set	reorder

$L68:
	.set	noreorder
	.set	nomacro
	bne	$3,$2,$L4
	li	$2,0x00000018		# 24
	.set	macro
	.set	reorder

	j	$L82
$L73:
	li	$4,0x00000013		# 19
	.set	noreorder
	.set	nomacro
	jal	CdControlF
	move	$5,$0
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	j	$L82
	li	$2,0x0000001b		# 27
	.set	macro
	.set	reorder

$L74:
	la	$5,g_cd_result
	.set	noreorder
	.set	nomacro
	jal	CdSync
	li	$4,0x00000001		# 1
	.set	macro
	.set	reorder

	move	$3,$2
	li	$2,0x00000002		# 2
	bne	$3,$2,$L75
	lh	$2,D_80101E58+16
	#nop
	sltu	$2,$2,1
	.set	noreorder
	.set	nomacro
	j	$L82
	sll	$2,$2,4
	.set	macro
	.set	reorder

$L75:
	li	$2,0x00000005		# 5
$L84:
	.set	noreorder
	.set	nomacro
	bne	$3,$2,$L4
	li	$2,0x00000017		# 23
	.set	macro
	.set	reorder

$L82:
	sh	$2,D_80101E58+10
$L4:
	lw	$31,28($sp)
	lw	$16,24($sp)
	addu	$sp,$sp,32
	j	$31
	.end	func_80036140
	.text
	.ent	func_80036940
func_80036940:
	.frame	$sp,32,$31		# vars= 8, regs= 2/0, args= 16, extra= 0
	.mask	0x80010000,-4
	.fmask	0x00000000,0
	lh	$3,D_80101E58+10
	subu	$sp,$sp,32
	sw	$31,28($sp)
	slt	$2,$3,16
	.set	noreorder
	.set	nomacro
	bne	$2,$0,$L86
	sw	$16,24($sp)
	.set	macro
	.set	reorder

	jal	func_80036140
	j	$L85
$L86:
	sltu	$2,$3,14
	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L85
	sll	$2,$3,2
	.set	macro
	.set	reorder

	lw	$2,$L136($2)
	#nop
	j	$2
	.rdata
	.align	3
$L136:
	.word	$L85
	.word	$L85
	.word	$L89
	.word	$L91
	.word	$L97
	.word	$L99
	.word	$L105
	.word	$L85
	.word	$L140
	.word	$L117
	.word	$L120
	.word	$L121
	.word	$L129
	.word	$L130
	.text
$L89:
	lh	$2,D_80101E58+16
	#nop
	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L90
	li	$4,0x0000000e		# 14
	.set	macro
	.set	reorder

$L140:
	sh	$0,D_80101E58+10
	j	$L85
$L90:
	addu	$5,$sp,16
	li	$2,0x000000a0		# 160
	.set	noreorder
	.set	nomacro
	jal	CdControlF
	sb	$2,16($sp)
	.set	macro
	.set	reorder

	sh	$0,D_80101E58+14
	sh	$0,D_80101E58+64
	.set	noreorder
	.set	nomacro
	j	$L138
	li	$2,0x00000003		# 3
	.set	macro
	.set	reorder

$L91:
	la	$5,g_cd_result
	.set	noreorder
	.set	nomacro
	jal	CdSync
	li	$4,0x00000001		# 1
	.set	macro
	.set	reorder

	move	$3,$2
	li	$2,0x00000002		# 2
	.set	noreorder
	.set	nomacro
	bne	$3,$2,$L92
	li	$2,0x00000005		# 5
	.set	macro
	.set	reorder

	li	$2,0x00000004		# 4
	sh	$2,D_80101E58+10
	sw	$0,D_80101E58+52
	j	$L85
$L92:
	.set	noreorder
	.set	nomacro
	beq	$3,$2,$L138
	li	$2,0x00000009		# 9
	.set	macro
	.set	reorder

	la	$3,D_80101E58+64
	lhu	$2,0($3)
	#nop
	addu	$2,$2,1
	sh	$2,0($3)
	sll	$2,$2,16
	sra	$2,$2,16
	slt	$2,$2,61
	.set	noreorder
	.set	nomacro
	bne	$2,$0,$L85
	li	$2,0x0000000a		# 10
	.set	macro
	.set	reorder

	j	$L138
$L97:
	la	$16,D_80101E58+52
	lw	$2,0($16)
	#nop
	addu	$2,$2,1
	sw	$2,0($16)
	slt	$2,$2,3
	.set	noreorder
	.set	nomacro
	bne	$2,$0,$L85
	addu	$16,$16,-32
	.set	macro
	.set	reorder

	lw	$2,D_80101E58+36
	lw	$3,D_80101E58+32
	sw	$2,D_80101E58+44
	sw	$3,D_80101E58+40
	.set	noreorder
	.set	nomacro
	jal	CdPosToInt
	move	$4,$16
	.set	macro
	.set	reorder

	li	$4,0x00000002		# 2
	move	$5,$16
	sw	$2,D_80101E58+72
	.set	noreorder
	.set	nomacro
	jal	CdControl
	move	$6,$0
	.set	macro
	.set	reorder

	sh	$0,D_80101E58+64
	.set	noreorder
	.set	nomacro
	j	$L138
	li	$2,0x00000005		# 5
	.set	macro
	.set	reorder

$L99:
	la	$5,g_cd_result
	.set	noreorder
	.set	nomacro
	jal	CdSync
	li	$4,0x00000001		# 1
	.set	macro
	.set	reorder

	move	$3,$2
	li	$2,0x00000002		# 2
	.set	noreorder
	.set	nomacro
	bne	$3,$2,$L100
	li	$2,0x00000005		# 5
	.set	macro
	.set	reorder

	la	$4,cdrom_ReadyCallback
	la	$16,D_80101E58+64
	.set	noreorder
	.set	nomacro
	jal	CdReadyCallback
	sh	$0,0($16)
	.set	macro
	.set	reorder

	li	$4,0x00000006		# 6
	.set	noreorder
	.set	nomacro
	jal	CdControlF
	addu	$5,$16,-44
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	j	$L138
	li	$2,0x00000006		# 6
	.set	macro
	.set	reorder

$L100:
	.set	noreorder
	.set	nomacro
	beq	$3,$2,$L138
	li	$2,0x00000009		# 9
	.set	macro
	.set	reorder

	la	$3,D_80101E58+64
	lhu	$2,0($3)
	#nop
	addu	$2,$2,1
	sh	$2,0($3)
	sll	$2,$2,16
	sra	$2,$2,16
	slt	$2,$2,61
	.set	noreorder
	.set	nomacro
	bne	$2,$0,$L85
	li	$2,0x0000000a		# 10
	.set	macro
	.set	reorder

	j	$L138
$L105:
	la	$5,g_cd_result
	.set	noreorder
	.set	nomacro
	jal	CdSync
	li	$4,0x00000001		# 1
	.set	macro
	.set	reorder

	move	$3,$2
	li	$2,0x00000002		# 2
	.set	noreorder
	.set	nomacro
	bne	$3,$2,$L106
	li	$2,0x00000005		# 5
	.set	macro
	.set	reorder

	lw	$2,D_80101E58+40
	#nop
	bne	$2,$0,$L107
	.set	noreorder
	.set	nomacro
	j	$L138
	li	$2,0x00000008		# 8
	.set	macro
	.set	reorder

$L107:
	.set	noreorder
	.set	nomacro
	bltz	$2,$L138
	li	$2,0x00000009		# 9
	.set	macro
	.set	reorder

	lhu	$2,D_80101E58+64
	#nop
	addu	$2,$2,1
	sh	$2,D_80101E58+64
	sll	$2,$2,16
	sra	$2,$2,16
	slt	$2,$2,61
	bne	$2,$0,$L85
	.set	noreorder
	.set	nomacro
	jal	CdReadyCallback
	move	$4,$0
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	j	$L138
	li	$2,0x0000000a		# 10
	.set	macro
	.set	reorder

$L106:
	bne	$3,$2,$L113
	.set	noreorder
	.set	nomacro
	jal	CdReadyCallback
	move	$4,$0
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	j	$L138
	li	$2,0x00000009		# 9
	.set	macro
	.set	reorder

$L113:
	la	$3,D_80101E58+64
	lhu	$2,0($3)
	#nop
	addu	$2,$2,1
	sh	$2,0($3)
	sll	$2,$2,16
	sra	$2,$2,16
	slt	$2,$2,61
	bne	$2,$0,$L85
	.set	noreorder
	.set	nomacro
	jal	CdReadyCallback
	move	$4,$0
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	j	$L138
	li	$2,0x0000000a		# 10
	.set	macro
	.set	reorder

$L117:
	lbu	$2,g_cd_result
	#nop
	andi	$2,$2,0x0010
	.set	noreorder
	.set	nomacro
	bne	$2,$0,$L138
	li	$2,0x0000000a		# 10
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	j	$L138
	li	$2,0x0000000c		# 12
	.set	macro
	.set	reorder

$L120:
	li	$4,0x00000001		# 1
	.set	noreorder
	.set	nomacro
	jal	CdControlF
	move	$5,$0
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	j	$L141
	li	$2,0x0000000b		# 11
	.set	macro
	.set	reorder

$L121:
	la	$5,g_cd_result
	.set	noreorder
	.set	nomacro
	jal	CdSync
	li	$4,0x00000001		# 1
	.set	macro
	.set	reorder

	move	$3,$2
	li	$2,0x00000002		# 2
	.set	noreorder
	.set	nomacro
	bne	$3,$2,$L122
	li	$2,0x00000005		# 5
	.set	macro
	.set	reorder

	lbu	$2,g_cd_result
	#nop
	andi	$2,$2,0x0010
	.set	noreorder
	.set	nomacro
	bne	$2,$0,$L138
	li	$2,0x0000000a		# 10
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	j	$L138
	li	$2,0x0000000c		# 12
	.set	macro
	.set	reorder

$L122:
	beq	$3,$2,$L139
	la	$3,D_80101E58+4
	lw	$2,0($3)
	#nop
	addu	$2,$2,1
	sw	$2,0($3)
	.set	noreorder
	.set	nomacro
	j	$L142
	slt	$2,$2,11
	.set	macro
	.set	reorder

$L129:
	li	$4,0x00000013		# 19
	.set	noreorder
	.set	nomacro
	jal	CdControlF
	move	$5,$0
	.set	macro
	.set	reorder

	li	$2,0x0000000d		# 13
$L141:
	sh	$2,D_80101E58+10
	sw	$0,D_80101E58+4
	j	$L85
$L130:
	la	$5,g_cd_result
	.set	noreorder
	.set	nomacro
	jal	CdSync
	li	$4,0x00000001		# 1
	.set	macro
	.set	reorder

	move	$3,$2
	li	$2,0x00000002		# 2
	.set	noreorder
	.set	nomacro
	bne	$3,$2,$L131
	li	$2,0x00000005		# 5
	.set	macro
	.set	reorder

	li	$2,0x00000002		# 2
	sh	$2,D_80101E58+10
	.set	noreorder
	.set	nomacro
	jal	VSync
	li	$4,0x00000004		# 4
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	jal	VSync
	li	$4,0x00000004		# 4
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	jal	VSync
	li	$4,0x00000004		# 4
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	jal	VSync
	li	$4,0x00000004		# 4
	.set	macro
	.set	reorder

	j	$L85
$L131:
	.set	noreorder
	.set	nomacro
	beq	$3,$2,$L138
	li	$2,0x00000009		# 9
	.set	macro
	.set	reorder

	la	$3,D_80101E58+4
	lw	$2,0($3)
	#nop
	addu	$2,$2,1
	sw	$2,0($3)
	slt	$2,$2,31
$L142:
	bne	$2,$0,$L85
	jal	CdFlush
$L139:
	li	$2,0x0000000a		# 10
$L138:
	sh	$2,D_80101E58+10
$L85:
	lw	$31,28($sp)
	lw	$16,24($sp)
	addu	$sp,$sp,32
	j	$31
	.end	func_80036940
