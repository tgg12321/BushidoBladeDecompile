func_80033DF4:
.frame	$sp,24,$31		# vars= 0, regs= 1/0, args= 16, extra= 0
.mask	0x80000000,-8
.fmask	0x00000000,0
subu	$sp,$sp,24
lbu	$3,D_800A38E2
#nop # DEBUG: 'li	$2,0x00000064		# 100' does not load from $3
addiu	$2,$zero,100
bne	$3,$2,.L2
sw	$31,16($sp)

lw	$2,D_800A3858
#nop # DEBUG: 'sb	$0,D_800A36F0' does not load from $2
sb	$0,D_800A36F0
sb	$0,D_800A3781
slt	$2,$2,27001
beq	$2,$0,.L3
nop  # DEBUG: branch/jump
lb	$2,D_8010277C
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
nop # DEBUG: Reuse of '$2'. 'beq	$2,$0,.L4' does not use $at
beq	$2,$0,.L4
addiu	$5,$zero,32

lui	$5,(65536 >> 16) & 0xFFFF
.L4:
la	$4,D_80106A50
lw	$3,0($4)
nop # DEBUG: Reuse of '$3'. 'and	$2,$3,$5' does not use $at
and	$2,$3,$5
sltu	$2,$2,1
or	$3,$3,$5
sb	$2,D_800A36F0
sw	$3,0($4)
.L3:
lw	$2,D_800A380C
nop # DEBUG: Reuse of '$2'. 'bne	$2,$0,.L5' does not use $at
bne	$2,$0,.L5
nop  # DEBUG: branch/jump
lb	$2,D_8010277C
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
nop # DEBUG: Reuse of '$2'. 'beq	$2,$0,.L6' does not use $at
beq	$2,$0,.L6
lui	$5,(16777216 >> 16) & 0xFFFF

lui	$5,(67108864 >> 16) & 0xFFFF
.L6:
la	$4,D_80106A50
lw	$3,0($4)
nop # DEBUG: Reuse of '$3'. 'and	$2,$3,$5' does not use $at
and	$2,$3,$5
sltu	$2,$2,1
or	$3,$3,$5
sb	$2,D_800A3781
sw	$3,0($4)
.L5:
jal	motutil_CheckKamaeReq
nop  # DEBUG: branch/jump
addiu	$2,$zero,4
sh	$2,D_800A3834
j	.L8
addu	$2,$0,$zero

.L2:
la	$5,cpu_practice_honmokuroku_data_tbl
addu	$3,$3,1
addu	$5,$5,$5
sb	$3,D_800A38E2
sb	$0,D_800A376B
lbu	$6,0($5)
#nop # DEBUG: 'lb	$2,D_8010277C' does not load from $6
lb	$2,D_8010277C
#nop # DEBUG: 'sb	$6,D_800A384C' does not load from $2
sb	$6,D_800A384C
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
#nop # DEBUG: 'la	$3,D_8008EC24' does not load from $2
la	$3,D_8008EC24
sltu	$2,$2,1
sll	$4,$2,2
addu	$4,$4,$2
addu	$3,$4,$3
addu	$3,$3,$6
lbu	$2,0($3)
sb	$2,D_800A38DE
lbu	$2,1($5)
sb	$2,D_800A38EC
lbu	$2,2($5)
sb	$2,D_800A38ED
la	$2,D_8008E908
addu	$4,$4,$2
lbu	$3,3($5)
#nop # DEBUG: 'addu	$4,$4,$6' does not load from $3
addu	$4,$4,$6
sb	$3,D_800A38EE
lbu	$3,0($4)
#nop # DEBUG: 'li	$2,0x00000001		# 1' does not load from $3
addiu	$2,$zero,1
sb	$3,D_8010277D
.L8:
lw	$31,16($sp)
#nop # DEBUG: 'addu	$sp,$sp,24' does not load from $31
addu	$sp,$sp,24
j	$31
nop  # DEBUG: branch/jump
.end	func_80033DF4
