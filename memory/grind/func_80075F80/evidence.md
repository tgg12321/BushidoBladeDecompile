# func_80075F80 -- evidence

## 2026-09-29 -- REOPENED (retro-audit FAIL, Q37 class C, 803d0fea1)

The 4335d8dfb landing FAILed the 2026-09-29 retro-audit: per-use byte pointer pun `((u8 *)D_8009BCF8)[index]` over the merged Unk8009BCF8Record[20] with the 2-byte stride as magic `*2`/`*20` (aggregate-merge prongs (d)/(b)); load-bearing (record form scores 3); unannotated MenuWork per-use cast. Per owner Q37 class C the body went back to `INCLUDE_ASM("asm/funcs", func_80075F80);` and the function is back in the queue. Landed text banked verbatim in `rejected/retro-audit-2026-09-29.c`. The preceding `extern u8 D_8009BCE4;` stays. Unk8009BCF8Record (include/game.h) stays; its other consumer is func_80076D74.
