	.text
	.globl	f2
f2:
	beq	$4,$0,$L5
	mflo	$3
	subu	$2,$2,$9
$L5:
	mult	$2,$2
	mflo	$8
	j	$31
