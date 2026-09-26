	.file	1 "tmp/func_80036140/p3/src/code6cac_b4.c"

 # GNU C 2.7.2 [AL 1.1, MM 40] GNU MIPS/ELF compiled by GNU C

 # Cc1 defaults:
 # -mgas

 # Cc1 arguments (-G value = 8, Cpu = 3000, ISA = 1):
 # -O2 -G8 -funsigned-char -quiet -mcpu=3000 -mips1 -mno-abicalls -fno-builtin
 # -w -mel -msoft-float

gcc2_compiled.:
	.text
	.align	2
	.globl	cdrom_SetMix
.Lfe1:
	.align	2
	.globl	func_80035F78
.Lfe2:
	.comm	D_800A36B8,4
	.comm	g_cd_atv,4
	.comm	D_800A3854,2
	.comm	D_800A3840,2

	.text
	.ent	cdrom_SetMix
cdrom_SetMix:
	.frame	$sp,24,$31		# vars= 0, regs= 1/0, args= 16, extra= 0
	.mask	0x80000000,-8
	.fmask	0x00000000,0
	subu	$sp,$sp,24
	sb	$4,g_cd_atv
	la	$4,g_cd_atv
	sw	$31,16($sp)
	sb	$5,g_cd_atv+1
	sb	$6,g_cd_atv+2
	sb	$7,g_cd_atv+3
	jal	CdMix
	sh	$0,D_800A3854
	lw	$31,16($sp)
	addu	$sp,$sp,24
	j	$31
	.end	cdrom_SetMix
	.ent	func_80035F78
func_80035F78:
	.frame	$sp,0,$31		# vars= 0, regs= 0/0, args= 0, extra= 0
	.mask	0x00000000,0
	.fmask	0x00000000,0
	lw	$2,16($sp)
	sb	$5,D_800A36B8
	sb	$6,D_800A36B8+1
	sb	$7,D_800A36B8+2
	sh	$4,D_800A3854
	sh	$0,D_800A3840
	sb	$2,D_800A36B8+3
	j	$31
	.end	func_80035F78
