func_8002E6B0:
addiu	$sp,$sp,-24
lui	$8,21845
ori	$8,$8,21846
sw	$21,20($sp)
sw	$20,16($sp)
sw	$19,12($sp)
sw	$18,8($sp)
sw	$17,4($sp)
sw	$16,0($sp)
lw	$10,0($4)
lw	$12,0($5)
lw	$14,0($6)
addu	$2,$10,$12
addu	$2,$2,$14
mult	$2,$8
lw	$9,8($4)
lw	$11,8($5)
lw	$13,8($6)
mfhi	$15
addu	$3,$9,$11
addu	$3,$3,$13
mult	$3,$8
sra	$2,$2,31
subu	$17,$15,$2
mfhi	$5
subu	$2,$11,$9
subu	$15,$17,$10
mult	$2,$15
lw	$19,0($7)
mflo	$25
subu	$24,$19,$10
nop	
mult	$2,$24
lw	$18,8($7)
mflo	$6
subu	$8,$18,$9
subu	$2,$12,$10
mult	$2,$8
sra	$3,$3,31
mflo	$4
subu	$16,$5,$3
subu	$5,$16,$9
mult	$2,$5
subu	$3,$6,$4
mflo	$21
subu	$4,$25,$21
xor	$3,$4,$3
bltz	$3,.L8002E814
addu	$2,$zero,$zero
subu	$4,$13,$9
mult	$4,$15
mflo	$7
subu	$3,$14,$10
nop	
mult	$3,$5
mflo	$5
nop	
nop	
mult	$4,$24
mflo	$6
nop	
nop	
mult	$3,$8
subu	$4,$7,$5
mflo	$3
subu	$3,$6,$3
xor	$3,$4,$3
bltz	$3,.L8002E814
subu	$4,$13,$11
subu	$2,$17,$12
mult	$4,$2
mflo	$7
subu	$3,$14,$12
subu	$2,$16,$11
mult	$3,$2
mflo	$6
subu	$2,$19,$12
nop	
mult	$4,$2
mflo	$5
subu	$2,$18,$11
nop	
mult	$3,$2
subu	$4,$7,$6
mflo	$3
subu	$3,$5,$3
xor	$2,$4,$3
nor	$2,$zero,$2
srl	$2,$2,31
.L8002E814:
lw	$21,20($sp)
lw	$20,16($sp)
lw	$19,12($sp)
lw	$18,8($sp)
lw	$17,4($sp)
lw	$16,0($sp)
addiu	$sp,$sp,24
jr	$31
nop	
.end	func_8002E6B0
