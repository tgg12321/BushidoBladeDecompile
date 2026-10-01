	.comm	s2,4
	.text
	.ent	fa
fa:
	lw	$2,s2
	jr	$31
	nop
	.end	fa
	.ent	fb
fb:
	sw	$3,s2
	jr	$31
	nop
	.end	fb
