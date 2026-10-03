# CD_getsector2 volatile dummy local - evidence (2026-10-02)

Target: the only access to the local is `sw $v0,0x0($sp)` at 0x80081EF8 inside an 8-byte frame (addiu sp,-8 at 0x80081EB8 / +8 at 0x80081F00).
Ablations (round-3 layer-2, cheat-reviewer a7a86845): a non-volatile `s32 tmp` (n1) and a bare `*g_cd_dma_ctrl;` (n3) both drop the store and the frame. Route B (Q48) applies; no SOTN source exists for CD_getsector2.

n1 vs target, CD_getsector2 region:
```
56,61c56
< bnez	v0,1e44 <CD_getsector2+0xb4>
< addiu	sp,sp,-8
< lbu	v0,0(v1)
< nop
< andi	v0,v0,0x40
< beqz	v0,1e30 <CD_getsector2+0xa0>
---
> beqz	v0,1e18 <CD_getsector2+0x88>
65c60
< 	1e48: R_MIPS_HI16	g_cd_dma_ctrl
---
> 	1e34: R_MIPS_HI16	g_cd_dma_ctrl
67c62
< 	1e4c: R_MIPS_LO16	g_cd_dma_ctrl
---
> 	1e38: R_MIPS_LO16	g_cd_dma_ctrl
71c66
< 	1e58: R_MIPS_HI16	g_cd_dma_ctrl
---
> 	1e44: R_MIPS_HI16	g_cd_dma_ctrl
73c68
< 	1e5c: R_MIPS_LO16	g_cd_dma_ctrl
---
> 	1e48: R_MIPS_LO16	g_cd_dma_ctrl
76,79d70
< nop
< sw	v0,0(sp)
< move	v0,zero
< addiu	sp,sp,8
```
