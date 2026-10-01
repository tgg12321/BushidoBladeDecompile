	.sdata
es:
	.word	0
	.text
	lui	$4,%hi(es)
	lw	$4,%lo(es)($4)
