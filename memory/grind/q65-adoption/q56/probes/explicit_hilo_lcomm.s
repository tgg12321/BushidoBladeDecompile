	.lcomm	el,4
	.text
	lui	$4,%hi(el)
	lw	$4,%lo(el)($4)
