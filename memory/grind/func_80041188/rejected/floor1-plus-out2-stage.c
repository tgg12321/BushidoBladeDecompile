/* REJECTED (s4, sandbox 17): the s3 floor-1 chassis (out2-read re-init of the
 * reused stptr) PLUS the s4 out2 staging line in loop2. Dead because the two
 * levers are chassis-exclusive: on the out2-read chassis out2 already has its
 * 4th flow ref from the re-init, so the extra loop-weighted ref in loop2 pushes
 * out2's global.c priority ABOVE tbl's and it steals s5 (17, worse than the
 * bare floor-1 form's 1). The staging line only pays on the pa4-read chassis,
 * where it REPLACES the ref the re-init gave up (15 -> 9).
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
    stptr = (s32) out2;
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
