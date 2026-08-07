/* hirahira_w_ctrl - WIP candidate. sandbox --disable all == 5 (HEAD == 27,
 * previous candidate == 23). 132/132 insns, frame 72 == target 0x48.
 * Pure C: no register pin, no __asm__, no volatile, no dead code.
 *
 * Every remaining difference is FIVE instructions in loop 2 (see notes.md);
 * loop 1, the prologue, the schedule and the epilogue match target exactly.
 *
 * The five levers that took 27 -> 5, in the order they were found:
 *  1. `offset = offset + (s32) a2; p = (u16 *) offset;`  (destination reuse)
 *  2. initialise `i` BEFORE `tbl` -- target emits the s4 init pair before the
 *     s5 pair; same registers, different emission order.            27->23->19
 *  3. `tbl++` AFTER the first func_8004A348 call, not mid-buffer -- sched1 runs
 *     before register allocation, so this also feeds the allocno data.  19->17
 *  4. loop 2's func_800523E0 takes the PARAMETER a4, not the local carrier:
 *     drops the carrier's allocno from nrefs 7 to 5 (pri 1473 -> 1052) so out2
 *     (1333) outranks it and the s6/s7 pair lands target's way.        17->11
 *  5. loop2 stptr as its own local + SPLIT-INIT on the loop1 stptr
 *     (`stptr = base; stptr += 0xFC;`): the split-init raises stptr's nrefs
 *     5 -> 7 (pri 2439 -> 3414) without changing one emitted instruction,
 *     which is what lets the loop2 split keep s3/s4 in target's order. 11->6
 *  6. `(s32)` cast on the first add's pointer operand.                  6->5
 *
 * The lone `a4` at the loop2 func_800523E0 is lever 4 and is load-bearing:
 * spelling it `pa4` there scores 11.
 */
void hirahira_w_ctrl(s32 a0, u8 *a1, u8 *a2, s32 a3, s32 *a4)
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
    loop1:
    offset = (*tbl) * 6;
    p = (u16 *) (offset + (s32) a1);
    buf[0] = p[0];
    buf[1] = -p[1];
    buf[2] = -p[2];
    func_8004A348(buf, pa4);
    tbl++;
    offset = offset + (s32) a2;
    p = (u16 *) offset;
    i++;
    buf[0] = p[0];
    buf[1] = -p[1];
    buf[2] = -p[2];
    func_8004A348(buf, out2);
    func_800523E0(pa4, out2, a3, stptr + 0x38);
    *((s16 *) (stptr + 6)) = 2;
    stptr += 0x68;
    if (i < 0x12) {
        goto loop1;
    }
    a1 += 0x6C;
    a2 += 0x6C;
    i = 0x12;
    out2 = (s32 *) (((u8 *) pa4) + 0x20);
    stptr2 = saved + 0x750;
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
    func_8004A348(buf, out2);
    func_800523E0(a4, out2, a3, stptr2 + 0x38);
    *((s16 *) (stptr2 + 6)) = 1;
    stptr2 += 0x68;
    i++;
    if (i < 0x14) {
        goto loop2;
    }
}
