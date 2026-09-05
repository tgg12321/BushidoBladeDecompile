/*
 * s28 FRONTIER BODY -- NOT a dead form.  score 7 / 175 insns / 175 target insns
 * on the HEAD 08b2924a chassis (floor F = 5).  Banked here because it did not
 * beat the floor, but it is the closest the flipped-cursor family has ever come
 * (previous best 25) and it is the FIRST body in 28 sessions to emit the entire
 * class-C block -- rows 60-64, including `addu $v1,$v1,$v0` and both inheriting
 * `addiu` -- byte-exact with no LICM displacement, no $a2/$a3 counter rename and
 * no collateral outside a 5-row window.
 *
 * WHAT IT IS: the flipped cursor (`(t0*10) + (s32)D_800A36A0 + 0x6A`, which is
 * what makes local-alloc tie the cursor add's destination to the SHIFT rather
 * than to the reload), the C-group pointer in its own local `pc`, and the 0x68
 * byte store moved ahead of the C pair -- store order A, D, S, C.
 *
 * WHY IT WORKS (measured, tmp/grind/func_800770B8/s28/h3.qty): moving the S
 * store earlier gives its address temp a 2-ref local quantity over [36,38].
 * qty_compare_1 (local-alloc.c:1660-1683) prices it floor_log2(4)*4/2 = 4.0,
 * above the merged 22-ref chain+sum quantity's [28,56] = 4*22/28 = 3.1428, so
 * it is allocated first, holds $2 across the chain's interval, and dies at 38 --
 * before the cursor reload's birth at 48.  The chain is therefore forced to $3
 * ($v1, the target's seat) and the reload [48,52] finds $2 ($v0).  With the S
 * store last (cX_c) the same slot is occupied by `pc` at 6 refs over [36,40] =
 * 3.0, just BELOW 3.1428, and the seats do not flip (score 25).
 *
 * WHAT IS LEFT: 7 differing rows = the two long-known baseline rows 35/36 (the
 * `$s1` vs `$v0` store base, present in the floor too) plus five rows that are
 * purely the S-before-C emission order (54, 55, 57, 58, 59).  Getting the same
 * blocking quantity while the S store stays LAST is the open question; naming
 * the S address (k1/k2/k5) is byte-inert on every base, and dropping `pc`
 * (m1) loses the blocker entirely (29).
 *
 * fake_ablate: one FAKE unit (the empty do-while(0) prologue fence) and it is
 * load-bearing -- 7 with it, 12 without.
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
        u8 *pc;
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
        *(u8 *)(base + t0 + 0x68) = (u8)t0;
        pc = base + (t0 * 4);
        *(s16 *)(pc + 0x42) = 0;
        *(s16 *)(pc + 0x40) = 0;
        {
            s16 *p_6a = (s16 *)((t0 * 10) + (s32)D_800A36A0 + 0x6A);
            s16 *p_7e = (s16 *)((t0 * 10) + (s32)D_800A36A0 + 0x7E);
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
