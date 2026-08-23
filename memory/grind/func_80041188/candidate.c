/* func_80041188 / hirahira_w_ctrl - s4 candidate. sandbox --disable all == 0
 * (prev best 1). 132/132 insns, frame 72 == target 0x48. Pure C: no register
 * pin, no inline asm, no volatile, no dead code, no unused local.
 *
 * s4 (permuter modality) closed the last insn. The s3 floor-1 form had a
 * single residual at slot 71 (`move s3,s6` vs target `addiu s3,s7,32`): the
 * out2-read re-init of the reused stptr bought out2's 4th flow ref (keeping
 * s6) but emitted the wrong bytes, while the pa4-read re-init emitted target's
 * exact addiu and lost the ref (sandbox 15). s4's permuter campaign on the
 * pa4-read chassis (tmp/grind/func_80041188/s4/perm2) produced the two pieces
 * that decouple the ref from the re-init:
 *   (a) output-63-1: stage the loop2 out-pointer through the dead `out2`
 *       local right before func_800523E0 - a real, immediately-consumed
 *       value that restores out2's loop-weighted ref count.  15 -> 9.
 *   (b) output-50-1: wrap loop1's leading half (up to the second
 *       func_8004A348) in do { ... } while (0), with the `loop1:` label
 *       inside and the back-goto entering from outside.  9 -> 0.
 * Both are FAKE-annotated in place. Measured decomposition, all this session:
 *   pa4-read re-init alone .......... 15
 *   + (a) out2 staging .............. 9
 *   + (b) loop1 wrap ................ 0
 *   out2-read re-init + (a) ......... 17   (rejected/floor1-plus-out2-stage.c)
 *   out2-read re-init + (b) ......... 1    (no change from s3 floor)
 * i.e. neither piece works on the s3 chassis; both are required on the
 * pa4-read chassis, which is the one that emits target's addiu s3,s7,32.
 */
void func_80041188(s32 a0, u8 *a1, u8 *a2, s32 a3, s32 *a4)
{
    s32 *pa4 = a4;
    s32 i = 1;
    s32 *tbl = D_80094CFC;
    s32 base = D_800A9A10[a0];
    s16 buf[3];
    s32 saved;
    s32 *out2;
    s32 stptr;
    s32 offset;
    u16 *p;
    s32 stptr2;
    saved = base + 0x94;
    out2 = (s32 *) (((u8 *) pa4) + 0x20);
    stptr = base;
    stptr += 0xFC;
    /* FAKE: do-while(0) wrap of loop1's leading half (the `loop1:` label sits
       inside it; the back-goto enters from outside), mechanism: the loop note
       the wrap emits re-weights flow.c's ref counts feeding global.c allocno
       priority, which seats the callee-saved set exactly as target (out2 s6,
       pa4-carrier s7, a3 fp); measured effect: identical form without the wrap
       scores 9, with it 0. lever-exhaustion: memory/grind/func_80041188/
       hypotheses.md + evidence.md (s1-s4 lever ladder; s4 permuter campaign
       tmp/grind/func_80041188/s4/perm2/output-50-1) */
    do { loop1: offset = (*tbl) * 6; p = (u16 *) (offset + (s32) a1); buf[0] = p[0]; buf[1] = -p[1]; buf[2] = -p[2]; func_8004A348(buf, pa4); tbl++; offset = offset + (s32) a2; p = (u16 *) offset; i++; buf[0] = p[0]; buf[1] = -p[1]; buf[2] = -p[2]; func_8004A348(buf, out2); } while (0);
    func_800523E0(pa4, out2, a3, stptr + 0x38);
    *((s16 *) (stptr + 6)) = 2;
    stptr += 0x68;
    if (i < 0x12) {
        goto loop1;
    }
    a1 += 0x6C;
    a2 += 0x6C;
    i = 0x12;
    stptr2 = saved + 0x750;
    /* FAKE: loop2's output pointer reuses the (dead-after-loop1) `stptr`
       walker local instead of a fresh local, mechanism: GCC 2.7.2 global.c
       gives the merged multi-ref pseudo priority 3409 so it allocates 3rd and
       lands s3 == target (a fresh low-ref local loses the s3 ordering race).
       Liveness: stptr's loop1 value is dead here - it is overwritten before any
       later read. lever-exhaustion: memory/grind/func_80041188/evidence.md s3 */
    stptr = (s32) (((u8 *) pa4) + 0x20);
    loop2:
    func_80044DE4((s16 *) a1, (s16 *) a2, a3, stptr2 + 0x4C);
    a1 += 6;
    a2 += 6;
    buf[0] = *((u16 *) a1);
    a1 += 2;
    buf[1] = -(*((u16 *) a1));
    a1 += 2;
    buf[2] = -(*((u16 *) a1));
    a1 += 2;
    func_8004A348(buf, pa4);
    buf[0] = *((u16 *) a2);
    a2 += 2;
    buf[1] = -(*((u16 *) a2));
    a2 += 2;
    buf[2] = -(*((u16 *) a2));
    a2 += 2;
    func_8004A348(buf, (s32 *) stptr);
    /* FAKE: the loop2 out-pointer is staged through the (dead-after-loop1)
       `out2` local, whose value is real and consumed by the very next call,
       mechanism: GCC 2.7.2 flow.c counts the extra loop-weighted ref so out2's
       global.c allocno priority clears the pa4-carrier's and out2 keeps s6 ==
       target. Liveness: out2's loop1 value is dead here; the staged value is
       not needed after. lever-exhaustion: memory/grind/func_80041188/
       evidence.md s3-s4 (pa4-read re-init alone scores 15, +this ref 9,
       +the loop1 wrap 0) */
    out2 = (s32 *) stptr;
    func_800523E0(pa4, out2, a3, stptr2 + 0x38);
    *((s16 *) (stptr2 + 6)) = 1;
    stptr2 += 0x68;
    i++;
    if (i < 0x14) {
        goto loop2;
    }
}
