/* REJECTED — s5 (synthesis, 2026-09-01)
 * s5-real-loop-note-anchor-insn-cost.c
 *
 * WHY IT IS DEAD: the s4 frontier's #1 hypothesis was that some ORDINARY-C
 * construct could anchor a NOTE_INSN_LOOP_BEG/END pair between the
 * reload-emitted register-save stores and the first body insn (the position
 * that the cheat `do { } while (0);` occupies in V1, worth floor 7 / floor 5).
 * Only LOOP notes survive to sched2 anchored mid-block, so the construct had
 * to be a REAL loop placed as the function's first statement.
 *
 * This form is the strongest such spelling: the two `sp[]` u16 counters —
 * which the function really does zero-initialise — written as a genuine
 * 2-iteration `for` loop at the very top of the body.  Variants measured:
 *   P1  for-loop first, then p_old, then ClearOTagR ......... 42 / 186 insns
 *   P2  for-loop first, ClearOTagR, then p_old (V10 pairing)  41 / 186 insns
 *   P3  p_old first, then the for-loop ...................... 42 / 186 insns
 *   P4  `while` spelling of the same loop ................... 42 / 186 insns
 * (honest floor of the s3/s5 candidate on the same chassis: 9 / 175 insns.)
 *
 * The MECHANISM HALF IS CONFIRMED, and that is the part worth inheriting:
 * tmp/grind/func_800770B8/s5/dumps/p1/in.i.sched2 shows
 *   (note 599 611 16 "" NOTE_INSN_PROLOGUE_END)
 *   (note  16 599 25 "" NOTE_INSN_LOOP_BEG)
 * i.e. a real first-statement loop DOES anchor its loop-note pair at exactly
 * the V1 fence position.  Ordinary C can reach that anchor.
 *
 * It cannot pay for it.  A real loop over `sp[]` forces the two `sh $zero`
 * stores onto a computed stack address (`addu $3,$sp,16` + sll/sra of the
 * index) plus loop control: +11 insns, 175 -> 186.  Our insn count already
 * EQUALS the target's 175 (closed in s3), so ANY construct that adds insns is
 * off the path to 0 by construction.  The only zero-insn loop is an empty one,
 * which is the s4 cheat.
 *
 * THE DECISIVE COROLLARY (see evidence.md s5): the TARGET has no loop at the
 * top of this function — asm/funcs/func_800770B8.s rows 1-14 are straight-line
 * prologue + ClearOTagR setup with no branch.  So the target's contiguous save
 * emission is NOT produced by a loop-note fence.  The fence is a coincidental
 * route to the same order, not the original mechanism, and hunting for an
 * honest fence is hunting for something the target did not have.
 */
s32 func_800770B8(s32 arg0, s32 arg1, s32 arg2) {
    u16 sp[2];
    s32 *p_old;
    s32 r;
    s16 t0;
    s16 a2;

    for (t0 = 0; t0 < 2; t0 = (s16)(t0 + 1)) {
        sp[t0] = 0;
    }
    p_old = (s32 *)(arg0 + 0x58);
    ClearOTagR(D_800A374C, 0x1008);
    D_800A35D8 = arg0;
    snd_StopAll();
    func_8006E950(6, p_old);
    r = func_80076FF8(p_old);
    {
        s32 *prev = p_old;
        p_old = (s32 *)func_8006E49C(r, D_800A35D8);
        D_800A36A0 = (u8 *)p_old;
        *(s32 *)((u8 *)p_old + 4) = (s32)prev;
        *(s32 *)(D_800A36A0 + 0x30) = 0;
        *(s16 *)(D_800A36A0 + 0x34) = 0;
    }
    t0 = 0;
    do {
        u8 *base = D_800A36A0;
        u8 *ptr;
        a2 = 0;
        ptr = (u8 *)((t0 * 2) + (s32)base);
        *(s16 *)(ptr + 0x10) = 0;
        *(s16 *)(ptr + 0x8) = 0;
        *(s16 *)(ptr + 0xC) = 0;
        *(s16 *)(ptr + 0x14) = 0;
        *(s16 *)(ptr + 0x3C) = 0;
        ptr = (u8 *)&D_800A35D0;
        ptr = (t0 * 4) + ptr;
        *(s16 *)(ptr + 2) = 0;
        *(s16 *)(ptr + 0) = 0;
        ptr = base + (t0 * 4);
        *(s16 *)(ptr + 0x42) = 0;
        *(s16 *)(ptr + 0x40) = 0;
        *(u8 *)(base + t0 + 0x68) = (u8)t0;
        {
            s16 *p_6a = (s16 *)(D_800A36A0 + (t0 * 10) + 0x6A);
            s16 *p_7e = (s16 *)(D_800A36A0 + (t0 * 10) + 0x7E);
            do {
                p_6a[a2] = -1;
                p_7e[a2] = 0;
                a2 = (s16)(a2 + 1);
            } while (a2 < 5);
        }
        a2 = 0;
        *(s16 *)(D_800A36A0 + (t0 * 2) + 0x5C) = 0;
        *(s16 *)(D_800A36A0 + (t0 * 2) + 0x60) = 5;
        for (a2 = 0; a2 < 0xA; a2 = (s16)(a2 + 1)) {
            s16 idx = (s16)(a2 + (t0 * 10));
            s32 mask = 1 << idx;
            (&D_8009BCE4)[idx] = (u8)((&D_8009BCE4)[idx] & 0xF2);
            if ((arg2 & mask) != 0) {
                (&D_8009BCE4)[idx] = (u8)((&D_8009BCE4)[idx] | 1);
                sp[t0] += 1;
            }
        }
        t0 = (s16)(t0 + 1);
    } while (t0 < 2);
    {
        u8 *p = D_800A36A0;
        *(s32 *)(p + 0x20) = 0;
        *(s32 *)(p + 0x1C) = 0;
        if ((s16)sp[0] < (s16)sp[1]) {
            *(s8 *)(p + 0x64) = (s8)((s16)sp[0] - 3);
        } else {
            *(s8 *)(p + 0x64) = (s8)((s16)sp[1] - 3);
        }
    }
    if (*(u8 *)(D_800A36A0 + 0x64) >= 3) {
        *(u8 *)(D_800A36A0 + 0x64) = 2;
    }
    {
        u8 *q = D_800A36A0;
        *(s32 *)q = arg1;
        *(s8 *)(q + 0x65) = 0;
    }
    *(u8 *)(D_800A36A0 + 0x67) = 1;
    *(u8 *)(D_800A36A0 + 0x66) = (&D_8009BD21)[*(u8 *)(D_800A36A0 + 0x67) * 2];
    D_800A35DC = 1;
    return 1;
}
