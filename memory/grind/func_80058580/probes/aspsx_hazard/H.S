	.text
	.globl	f
f:
	mflo	$3
	subu	$2,$2,$9
$L1:
	mult	$2,$2
	mflo	$8
	j	$31

	.globl	g
g:
	mflo	$3
	subu	$2,$2,$9
	mult	$2,$2
	mflo	$8
	j	$31

	.globl	h
h:
	mflo	$3
	j	$L2
$L3:
	div	$2,$3
$L2:
	mflo	$8
	j	$31
