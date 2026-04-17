func_80033DF4:
.frame	$sp,24,$31		# vars= 0, regs= 1/0, args= 16, extra= 0
.mask	0x80000000,-8
.fmask	0x00000000,0
subu	$sp,$sp,24
lbu	$3,D_800A38E2
#nop # DEBUG: 'li	$2,0x00000064		# 100' does not load from $3
addiu	$2,$zero,100
andi	$4,$3,0x00ff
bne	$4,$2,.L2
sw	$31,16($sp)

lw	$2,D_800A3858
#nop # DEBUG: 'sb	$0,D_800A36F0' does not load from $2
sb	$0,D_800A36F0
sb	$0,D_800A3781
slt	$2,$2,27001
beq	$2,$0,.L3
nop  # DEBUG: branch/jump
lb	$5,D_8010277C
# EXPAND_AT START
.set	noat
lui	$at,%hi(D_8008D538)
addu	$at,$at,$5
lbu	$5,%lo(D_8008D538)($at)
.set	at
# EXPAND_AT END
# EXPAND_AT START
.set	noat
lui	$at,%hi(D_8008D9EC)
addu	$at,$at,$5
lbu	$2,%lo(D_8008D9EC)($at)
.set	at
# EXPAND_AT END
nop # DEBUG: Reuse of '$2'. 'beq	$2,$0,.L4' does not use $at
beq	$2,$0,.L4
addiu	$3,$zero,32

lui	$3,(65536 >> 16) & 0xFFFF
.L4:
la	$5,D_80106A50
lw	$4,0($5)
nop # DEBUG: Reuse of '$4'. 'and	$2,$4,$3' does not use $at
and	$2,$4,$3
slt	$2,$2,1
sb	$2,D_800A36F0
or	$2,$4,$3
sw	$2,0($5)
.L3:
lw	$2,D_800A380C
nop # DEBUG: Reuse of '$2'. 'bne	$2,$0,.L5' does not use $at
bne	$2,$0,.L5
nop  # DEBUG: branch/jump
lb	$5,D_8010277C
# EXPAND_AT START
.set	noat
lui	$at,%hi(D_8008D538)
addu	$at,$at,$5
lbu	$5,%lo(D_8008D538)($at)
.set	at
# EXPAND_AT END
# EXPAND_AT START
.set	noat
lui	$at,%hi(D_8008D9EC)
addu	$at,$at,$5
lbu	$2,%lo(D_8008D9EC)($at)
.set	at
# EXPAND_AT END
nop # DEBUG: Reuse of '$2'. 'beq	$2,$0,.L6' does not use $at
beq	$2,$0,.L6
lui	$3,(16777216 >> 16) & 0xFFFF

lui	$3,(67108864 >> 16) & 0xFFFF
.L6:
la	$5,D_80106A50
lw	$4,0($5)
nop # DEBUG: Reuse of '$4'. 'and	$2,$4,$3' does not use $at
and	$2,$4,$3
slt	$2,$2,1
sb	$2,D_800A3781
or	$2,$4,$3
sw	$2,0($5)
.L5:
jal	motutil_CheckKamaeReq
nop  # DEBUG: branch/jump
addiu	$2,$zero,4
sh	$2,D_800A3834
j	.L7
addu	$2,$0,$zero

.L2:
addu	$2,$3,1
sll	$4,$4,2
sb	$2,D_800A38E2
la	$2,cpu_practice_honmokuroku_data_tbl
addu	$4,$4,$2
sb	$0,D_800A376B
lbu	$5,0($4)
#nop # DEBUG: 'lb	$2,D_8010277C' does not load from $5
lb	$2,D_8010277C
#nop # DEBUG: 'sb	$5,D_800A384C' does not load from $2
sb	$5,D_800A384C
# EXPAND_AT START
.set	noat
lui	$at,%hi(D_8008D538)
addu	$at,$at,$2
lbu	$2,%lo(D_8008D538)($at)
.set	at
# EXPAND_AT END
# EXPAND_AT START
.set	noat
lui	$at,%hi(D_8008D9EC)
addu	$at,$at,$2
lbu	$2,%lo(D_8008D9EC)($at)
.set	at
# EXPAND_AT END
#nop # DEBUG: 'andi	$5,$5,0x00ff' does not load from $2
andi	$5,$5,0x00ff
slt	$2,$2,1
sll	$3,$2,2
addu	$2,$2,$3
addu	$2,$5,$2
# EXPAND_AT START
.set	noat
lui	$at,%hi(D_8008EC24)
addu	$at,$at,$2
lbu	$3,%lo(D_8008EC24)($at)
.set	at
# EXPAND_AT END
sb	$3,D_800A38DE
lbu	$3,1($4)
sb	$3,D_800A38EC
lbu	$3,2($4)
sb	$3,D_800A38ED
lbu	$3,3($4)
sb	$3,D_800A38EE
# EXPAND_AT START
.set	noat
lui	$at,%hi(D_8008E908)
addu	$at,$at,$2
lbu	$3,%lo(D_8008E908)($at)
.set	at
# EXPAND_AT END
#nop # DEBUG: 'li	$2,0x00000001		# 1' does not load from $3
addiu	$2,$zero,1
sb	$3,D_8010277D
.L7:
lw	$31,16($sp)
#nop # DEBUG: 'addu	$sp,$sp,24' does not load from $31
addu	$sp,$sp,24
j	$31
nop  # DEBUG: branch/jump
.end	func_80033DF4
