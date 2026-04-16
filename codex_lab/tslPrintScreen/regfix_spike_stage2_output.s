
.file	1 "/tmp/tmpj1zz_bde.c"




.version	"01.01"
gcc2_compiled.:
.text
.align	2
.globl	tslPrintScreen
.type	 tslPrintScreen,@function
.ent	tslPrintScreen
.set	noreorder
tslPrintScreen:
.frame	$sp,72,$31		# vars= 16, regs= 9/0, args= 16, extra= 0
.mask	0x80ff0000,-8
.fmask	0x00000000,0
subu	$sp,$sp,72
sw	$20,48($sp)
addu	$20,$4,$zero
sw	$31,64($sp)
sw	$23,60($sp)
sw	$22,56($sp)
sw	$21,52($sp)
sw	$19,44($sp)
sw	$18,40($sp)
sw	$17,36($sp)
sw	$16,32($sp)
lw	$19,36($20)
nop # DEBUG: Reuse of '$19'. 'beq	$19,$0,.L1' does not use $at
beq	$19,$0,.L1
addu	$23,$5,$zero

lw	$17,40($20)
nop # DEBUG: Reuse of '$17'. 'lh	$2,0($17)' does not use $at
lh	$2,0($17)
#nop # DEBUG: 'li	$4,-3			# 0xfffffffd' does not load from $2
addiu	$3,$zero,-3
bne	$2,$3,.L4
addu	$16,$19,1128

j	.L1
sw	$0,36($20)

.L4:
sh	$0,0($19)
sh	$0,2($19)
sh	$0,4($19)
sh	$0,6($19)
lh	$2,0($17)
#nop # DEBUG: 'lhu	$3,0($17)' does not load from $2
lhu	$4,0($17)
#nop # DEBUG: 'beq	$2,$4,.L6' does not load from $3
beq	$2,$3,.L6
addu	$21,$0,$zero

addiu	$22,$zero,-2
.L7:
sll	$2,$4,16
sra	$2,$2,16
beq	$2,$22,.L27
nop  # DEBUG: branch/jump
lh	$2,0($19)
nop # DEBUG: Reuse of '$2'. 'slt	$2,$2,5' does not use $at
slt	$2,$2,5
bne	$2,$0,.L10+4
addu	$5,$17,$zero

jal	func_80052C10
nop  # DEBUG: branch/jump
.L10:
	addu	$5,$17,$zero
lh	$3,0($19)
addu	$6,$16,$zero
sll	$2,$3,1
addu	$2,$2,$3
sll	$2,$2,2
addu	$2,$2,$3
sll	$2,$2,4
addu	$2,$2,8
addu	$18,$19,$2
sll	$2,$21,2
addu	$2,$2,$20
lw	$16,6708($2)
#nop # DEBUG: 'move	$4,$18' does not load from $21
addu	$4,$18,$zero
sw	$6,24($18)
jal	func_8003FA24
sw	$16,20($18)

lh	$3,0($19)
nop # DEBUG: Reuse of '$3'. 'sll	$3,$3,4' does not use $at
sll	$3,$3,4
addu	$3,$3,$19
lw	$4,4($18)
#nop # DEBUG: 'lw	$5,8($18)' does not load from $4
lw	$5,8($18)
#nop # DEBUG: 'lw	$6,12($18)' does not load from $5
lw	$6,12($18)
#nop # DEBUG: 'lw	$7,16($18)' does not load from $6
lw	$7,16($18)
#nop # DEBUG: 'sw	$4,1048($3)' does not load from $7
sw	$4,1048($3)
sw	$5,1052($3)
sw	$6,1056($3)
sw	$7,1060($3)
addiu	$3,$zero,13
sb	$3,0($16)
lhu	$3,0($19)
nop # DEBUG: Reuse of '$3'. 'addu	$3,$3,1' does not use $at
addu	$3,$3,1
sh	$3,0($19)
lh	$3,0($17)
nop # DEBUG: Reuse of '$3'. 'bltz	$3,.L28' does not use $at
bltz	$3,.L28
addu	$16,$2,$zero

addu	$17,$17,2
.L16:
lh	$2,0($17)
nop # DEBUG: Reuse of '$2'. 'bgez	$2,.L16' does not use $at
bgez	$2,.L16
addu	$17,$17,2

addu	$17,$17,-2
lh	$3,0($17)
#nop # DEBUG: '.L28:' does not load from $3
.L28:
addiu	$2,$zero,-1
bne	$3,$2,.L27
addu	$4,$20,$zero

addu	$17,$17,2
addu	$5,$18,$zero
jal	func_8003FECC
addu	$6,$17,$zero
#nop # DEBUG: '.L27:' does not load from $3
.L27:
lh	$2,0($17)
nop  # DEBUG: branch/jump

beq	$2,$22,.L9+4
.L26:
addu	$17,$17,2
addiu	$3,$zero,-2
lh	$2,0($17)
nop  # DEBUG: branch/jump
bne	$2,$3,.L26+8
.L9:
addu	$17,$17,2
addiu	$2,$zero,-3
lh	$3,0($17)
#nop # DEBUG: 'lhu	$3,0($17)' does not load from $21
lhu	$4,0($17)
#nop # DEBUG: 'bne	$21,$2,.L7' does not load from $3
bne	$3,$2,.L7
addu	$21,$21,1

.L6:
beq	$23,$0,.L1
nop  # DEBUG: branch/jump
lw	$5,28($20)
#nop # DEBUG: 'lh	$4,4($20)' does not load from $5
lh	$4,4($20)
#nop # DEBUG: 'jal	func_80045A28' does not load from $4
jal	func_80045A28
subu	$5,$16,$5

.L1:
lw	$31,64($sp)
#nop # DEBUG: 'lw	$23,60($sp)' does not load from $31
lw	$23,60($sp)
#nop # DEBUG: 'lw	$22,56($sp)' does not load from $23
lw	$22,56($sp)
#nop # DEBUG: 'lw	$21,52($sp)' does not load from $22
lw	$21,52($sp)
#nop # DEBUG: 'lw	$20,48($sp)' does not load from $21
lw	$20,48($sp)
#nop # DEBUG: 'lw	$19,44($sp)' does not load from $20
lw	$19,44($sp)
#nop # DEBUG: 'lw	$18,40($sp)' does not load from $19
lw	$18,40($sp)
#nop # DEBUG: 'lw	$17,36($sp)' does not load from $18
lw	$17,36($sp)
#nop # DEBUG: 'lw	$16,32($sp)' does not load from $17
lw	$16,32($sp)
#nop # DEBUG: 'addu	$sp,$sp,72' does not load from $16
addu	$sp,$sp,72
j	$31
nop  # DEBUG: branch/jump
.end	tslPrintScreen
.Lfe1:
.size	 tslPrintScreen,.Lfe1-tslPrintScreen
.ident	"GCC: (GNU) 2.7.2"
