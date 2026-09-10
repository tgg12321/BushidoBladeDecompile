/* REJECTED (s2) - sandbox 3; this is session 1's candidate. `if (num > 0) { b =
   _spu_RXX; for (i = 0; ...) }` produces TWO `blez $s0` branches (the source guard
   plus the loop-entry test that jump.c duplicates) and the second one carries a `nop`
   delay slot, i.e. exactly the 2 extra instructions (117 vs 115). The source-level
   guard is redundant with the loop-entry test GCC emits for the `for`, so deleting it
   is both simpler C and closer to the target. Superseded by candidate.c (score 0). */
/* CANDIDATE - _spu_FwriteByIO (src/main.c:1754, replaces the INCLUDE_ASM line)
 * session 1 (recon, 2026-09-10).  sandbox --disable all = 3  (from 115 at HEAD).
 * Frame is CORRECT (.frame $sp,48 vars=8 regs=5 args=16 == target 0x30) and 113
 * of 115 instructions are byte-identical.  RESIDUAL (2 extra insns, score 3):
 * the fifo-loop preheader.  Target emits
 *   blez $s0,.L... / addu $v1,$zero,$zero (delay) / lui $a0,%hi(_spu_RXX) / lw %lo
 * i.e. the _spu_RXX load lands AFTER the guard branch and the i=0 fills the
 * delay slot.  We emit  lui/lw/nop/blez/move $v1,zero - the load is hoisted
 * above the branch, so the load-delay nop is required and the delay slot is
 * empty.  Everything else (both timeout loops, both printf arms, the epilogue)
 * matches exactly.
 *
 * Also applies to src/main.c: delete the now-redundant forward declaration
 * `extern void _spu_FwriteByIO(s32, s32);` (was at main.c:1898) - the two call
 * sites (main.c:1719 and main.c:1905) pass (s32) values and compile unchanged.
 *
 * Provenance: PsyQ LIBSPU spu.c `_spu_FwriteByIO` (static), verbatim-linked
 * Sony object.  C refs: Xeeynamo/psyz decomp/src/libspu/spu.c:111 and
 * sotn-decomp psxsdk/libspu/spu.c (_spu_writeByIO).  NO FAKE constructs, no
 * dead locals, no volatile pads - this is ordinary C.
 */
/* PsyQ 4.0 LIBSPU spu.c: _spu_FwriteByIO (static) — verbatim-linked Sony
   object (census 2026-07-09); C ref: Xeeynamo/psyz decomp/src/libspu/spu.c:111
   and sotn-decomp psxsdk/libspu/spu.c (_spu_writeByIO). */
void _spu_FwriteByIO(u8 *addr, u32 size) {
    u16 spustat;
    s32 num;
    u16 *cur;
    s32 i;
    u32 j;
    u16 cnt;
    s32 b;

    cur = (u16 *)addr;
    spustat = *(volatile u16 *)(_spu_RXX + 0x1AE) & 0x7FF;
    *(volatile u16 *)(_spu_RXX + 0x1A6) = _spu_tsa;
    _spu_Fw1ts();
    while (size != 0) {
        num = (size > 0x40) ? 0x40 : size;
        if (num > 0) {
            b = _spu_RXX;
            for (i = 0; i < num; i += 2) {
                *(volatile u16 *)(b + 0x1A8) = *cur++;
            }
        }
        cnt = *(volatile u16 *)(_spu_RXX + 0x1AA);
        cnt &= ~0x30;
        cnt |= 0x10;
        *(volatile u16 *)(_spu_RXX + 0x1AA) = cnt;
        _spu_Fw1ts();
        if (*(volatile u16 *)(_spu_RXX + 0x1AE) & 0x400) {
            j = 0;
            do {
                if (++j > 0xF00) {
                    printf(&D_800163D8, &D_800163F8);
                    break;
                }
            } while (*(volatile u16 *)(_spu_RXX + 0x1AE) & 0x400);
        }
        _spu_Fw1ts();
        _spu_Fw1ts();
        size -= num;
    }
    cnt = *(volatile u16 *)(_spu_RXX + 0x1AA);
    j = 0;
    cnt &= ~0x30;
    *(volatile u16 *)(_spu_RXX + 0x1AA) = cnt;
    if ((*(volatile u16 *)(_spu_RXX + 0x1AE) & 0x7FF) != spustat) {
        do {
            if (++j > 0xF00) {
                printf(&D_800163D8, &D_8001640C);
                break;
            }
        } while ((*(volatile u16 *)(_spu_RXX + 0x1AE) & 0x7FF) != spustat);
    }
}
