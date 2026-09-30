	.text
	.globl	k
k:
	mflo	$3
	subu	$2,$2,$9
$L7:
	mult	$2,$2
	mflo	$3
	j	$L8
$L9:
	mult	$2,$3
$L8:
	mflo	$3
	b	$L10
$L11:
	divu	$2,$3
$L10:
	mfhi	$3
	jr	$31
$L12:
	mult	$2,$3
	nop
