/* s20 (structural). Y1 with the ledger's `out3 = out2;` block-2 spelling instead of
the Z0 one, i.e. the seat-correct wrap form on the goto chassis.
MEASURED: sandbox 8 at 132 build insns with ALL-TARGET callee-saved seats --
stptr 6/41=2926 $s3, stptr2 6/48=2500 $s0, i 8/97=2474 $s4, tbl 5/47=2127 $s5,
out2 5/47=2127 $s6 (exact tie with tbl, broken the right way by allocno number
79 < 86), pa4 7/95=1473 $s7, a3 808 $fp, out3 $s3.
WHY DEAD: with identical seats candidate.c scores 1 and this scores 8. objdump
attributes every extra diff to the LOOP_BEG/END notes' effect on sched1's
emission order, NOT to allocation: `tbl++` (`addiu $s5,$s5,0x4`) is displaced
past the `addiu $a0,$sp,0x10` / `addu $a1,$s7,$zero` argument setup,
`addu $s0,$s0,$s2` moves with it, and `sw $t0,0x18($sp)` is displaced in block 0.
So on the GOTO chassis a loop1 do-while(0) wrap is insn-count-neutral but NOT
emission-order-neutral -- it is strictly worse than candidate.c's F1
chain-extender even when it produces the same seats.
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
    s32 *out3;
    s32 stptr;
    s32 offset;
    u16 *p;
    s32 stptr2;
    saved = base + 0x94;
    do { out2 = (s32 *) (((u8 *) pa4) + 0x20); } while (0); /* FAKE: loop-note ref weighting, +1 flow reference on out2 */
    stptr = base + 0xFC;
        loop1:
    do { offset = (*tbl) * 6; } while (0); /* FAKE: loop-note ref weighting, +1 flow reference on tbl (4 -> 5) */
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
    do { *((s16 *) (stptr + 6)) = 2; } while (0); /* FAKE: loop-note ref weighting, +1 flow reference on stptr (5 -> 6) */
    stptr += 0x68;
    if (i < 0x12) {
        goto loop1;
    }
    a1 += 0x6C;
    a2 += 0x6C;
    i = 0x12;
    stptr2 = saved + 0x750;
    out3 = out2;
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
    func_8004A348(buf, out3);
    func_800523E0(pa4, out3, a3, stptr2 + 0x38);
    *((s16 *) (stptr2 + 6)) = 1;
    stptr2 += 0x68;
    i++;
    if (i < 0x14) {
        goto loop2;
    }
}
