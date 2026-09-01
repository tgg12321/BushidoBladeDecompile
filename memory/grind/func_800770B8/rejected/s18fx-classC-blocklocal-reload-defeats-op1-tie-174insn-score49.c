/* REJECTED - s18 forensics, 2026-09-01.  score 49 / 174 insns (target 175).
 *
 * WHAT IT IS: the outer loop's SECOND read of D_800A36A0 is hoisted into a
 * named local `b2` that is also used for the 0x5C/0x60 stores AFTER the inner
 * loop, so the reload's pseudo crosses a basic-block boundary.
 *
 * WHY IT IS DEAD: it deletes the third read (174 insns vs the target's 175) and
 * globally allocates the reload ($8), which renames rows 60-89.
 *
 * WHY IT IS KEPT: it is the MECHANISM PROOF for class C.  It is the first form
 * in 18 sessions that reaches the target's rows 62-64 TIE TOPOLOGY (dest tied to
 * the t0*10 shift, `addu $2,$8,$2`) WITHOUT the operand-order flip.  Dump-proven:
 * tmp/grind/func_800770B8/s18/fx/p1.lreg.txt insn 186 `(set 110 (plus 86 109))`,
 * with `Register 86 used 6 times across 26 insns` and NO `in block 1` marker ->
 * reg_basic_block[86] < 0 -> local-alloc.c:472 sets reg_qty[86] = -1 ->
 * combine_regs (local-alloc.c:1824) returns 0 on operand 1 -> block_alloc's
 * operand loop (local-alloc.c:1240-1298) falls through to operand 2 (reg 109,
 * the shift, which carries the REG_DEAD note) and ties there.  `;; Register 109
 * in 2.` / `;; Register 110 in 2.` confirm the merge.
 */
s32 func_800770B8(s32 arg0, s32 arg1, s32 arg2) {
    u16 sp[2];
    s32 *p_old;
    s32 r;
    s16 t0;
    s16 a2;

    /* FAKE: empty do-while(0) wrap. Effect: it anchors a
       NOTE_INSN_LOOP_BEG/END pair at this statement position, which stops sched2
       interleaving the five reload-emitted frame-save stores with the first body
       insns; without it the prologue emits sw $s1 / addiu $s1,$s0,0x58 / lw
       D_800A374C / li 0x1008 / sw $ra where the target emits sw $ra / sw $s1 /
       li 0x1008 / lw D_800A374C / addiu $s1 (residual class A, 4 rows).
       mechanism: GCC 2.7.2 sched.c list scheduler, second pass (sched2, post-reload);
       the notes bound the scheduling region so the save stores cannot be hoisted
       across them. See evidence.md [s9] for the insn-level read-out of the
       unfenced order and [s11] for the measurement.
       lever-exhaustion: hypotheses.md classes A/B/C; s3 (12 statement orderings),
       s5 (honest-loop fence hunt, +11 insns), s9 (exhaustive 3234-atom sched_solver
       depth-1 sweep against the target emission order: 0 hits; the only reachable
       sub-goal needs atoms not expressible in C), s10 (struct-typed rederive 178
       insns), s11 (63-position single-wrap sweep + 18 nested-wrap variants). */
    do { } while (0);
    sp[0] = 0;
    sp[1] = 0;
    ClearOTagR(D_800A374C, 0x1008);
    p_old = (s32 *)(arg0 + 0x58);
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
        u8 *b2;
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
        b2 = D_800A36A0;
        {
            s16 *p_6a = (s16 *)(b2 + (t0 * 10) + 0x6A);
            s16 *p_7e = (s16 *)(b2 + (t0 * 10) + 0x7E);
            do {
                p_6a[a2] = -1;
                p_7e[a2] = 0;
                a2 = (s16)(a2 + 1);
            } while (a2 < 5);
        }
        a2 = 0;
        *(s16 *)(b2 + (t0 * 2) + 0x5C) = 0;
        *(s16 *)(b2 + (t0 * 2) + 0x60) = 5;
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
