/* func_80041188 / hirahira_w_ctrl - s15 (solver) BASE FORM "P1".
 * sandbox --disable all == 15, 132/132 insns.  ORDINARY C: no FAKE construct,
 * no chain-extender, no variable reuse, no do-while(0), no pin.
 *
 * P1 = alt_fakefree_floor3_s14.c with `i = 0x12;` moved from FIRST to THIRD
 * statement of the between-loops block (immediately before `stptr2 = ...`).
 * That single statement move is the UNIQUE scheduler vector that turns
 * alt_fakefree's +2 emission-order residual into target's order - proven by
 * tools/sched_solver (block 2, 5 insns, 30 single atoms + all pairs, exactly
 * one vector: `luid_move 152 -> immediately before 161`).  s15 spelled it and
 * measured it: emission order becomes target's, and requirement (A) breaks,
 * because the same source position sets reg_live_length(i) = 97 instead of 99
 * (i 8 refs/97 = 2474 > stptr 5 refs/41 = 2439, so `i` takes $s3 and every
 * seat below it shifts: score 15).
 *
 * WHY THIS FORM IS THE RIGHT BASE ANYWAY.  Forward-replaying global.c on P1's
 * own extracted model (tools/ra_solver/simulate.py, tmp/grind/func_80041188/
 * s15/probe_ra.py) over every +/-1..4 live-length and +/-1..2 reference
 * perturbation of all ten contested pseudos: P1 reaches target's COMPLETE
 * callee-saved disposition ($s0 stptr2, $s3 stptr+out3, $s4 i, $s5 tbl,
 * $s6 out2, $s7 pa4, $fp a3) under a SINGLE-ATOM perturbation, and only these:
 *     stptr live 41 -> 40 (or 39, 38, 37)      [pri 2439 -> 2500]
 *     stptr refs  5 -> 6  (or 7)               [pri 2439 -> 2926]
 *     i    live 97 -> 99                       [pri 2474 -> 2424; = the s14
 *                                               alt route, costs +2 emission]
 * Nothing else in the model reaches it.  P1's residual is therefore ONE unit
 * of stptr live-range or ONE honest stptr reference away from an all-target,
 * FAKE-free chassis whose only remaining diff would be the long-standing
 * slot-71 `out3 = out2` vs `addiu $s3,$s7,0x20` shape (floor 1, honest).
 *
 * This is the same requirement candidate.c buys with the F1 chain-extender
 * `stptr = base; stptr += 0xFC;` (refs 5 -> 7), which s11 proved is an
 * un-annotated FAKE and therefore not committable.  The honest question the
 * next session inherits is exactly: what ordinary C gives `stptr` a sixth
 * flow-counted reference, or ends its live range one insn earlier?
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
    out2 = (s32 *) (((u8 *) pa4) + 0x20);
    stptr = base + 0xFC;
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
