	.comm	ex,4
	.text
	lui	$4,%hi(ex)
	lw	$4,%lo(ex)($4)
