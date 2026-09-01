/* candidate.c — func_800770B8 (src/text1b.c) — s3 structural, 2026-09-01
 * Honest floor THIS form, measured s3: sandbox --disable all = 9 (175/175 insns).
 * (s2 form re-measured 10 on today's chassis at the start of s3; s3 dropped it 10 -> 9
 * and closed the last insn-COUNT gap: ours is now 175 insns, exactly the target's.)
 * 100% ordinary C, zero cheats, zero annotations, no FAKE-annotated constructs.
 *
 * s3 addition over the s2 form (one edit, ordinary C, measured):
 *   Inner loop 2 (the arg2 bit-scan) is a `for (a2 = 0; a2 < 0xA; a2 = (s16)(a2 + 1))`
 *   instead of a preheader `a2 = 0;` plus a do-while with a trailing increment.
 *   This closes residual class D (the reorg.c delay-slot fill choice, PROVEN in s2):
 *   with the `for` shape reorg no longer steals the `ori` from the fall-through
 *   thread; it fills the beqz from the branch-target thread by copying
 *   `addiu $v0,$a2,1` and redirecting past it — exactly the target's rows 103-105
 *   and 113. Insn count 174 -> 175, score 10 -> 9.
 *   NOTE the a2 initialiser must stay a `for`-header init here; s2's H6 (a2 = 0 as the
 *   FIRST statement of the outer do-body) is still what fixes rows 30-34/37, and the
 *   `for` header supplies exactly that RTL position for the second inner loop.
 *
 * NOTE: applying this body also requires the two caller-side edits (see
 * tmp/grind/func_800770B8/s3/try.py): the prototype near the caller becomes
 * `s32 func_800770B8(s32, s32, s32);` and the call site passes
 * `(s32)&D_8009BD24` (was `(s32 *)&D_8009BD24`). Byte-neutral for the caller.
 *
 * Residual at floor 9 (3 classes; class D is CLOSED):
 *   A. prologue rows 7-12 (4 diffs): sched2 (post-RA!) ordering of
 *      {sw $ra, sw $s1, addiu $a1 0x1008, lui/lw D_800A374C, addiu $s1,$s0,0x58}.
 *      s3 correction: this is a SCHED2 question, not sched1 — the register-save
 *      insns do not exist until after reload. 12 statement orderings measured, all
 *      neutral-or-worse. See evidence.md s3.
 *   B. rows 35-36 (2 diffs): the 0x30/0x34 stores go through $s1 (p_old) in ours,
 *      through $v0 (the raw call-result pseudo) in target. Second-handle spellings
 *      KILLED three times (K1 s1, K4 s2, and every s3 restatement is byte-neutral).
 *   C. rows 62-64 (3 diffs): the p_6a/p_7e base `addu` ties its dest to the lw pseudo
 *      ($v0) in ours and to the sll pseudo ($v1) in target — a local-alloc
 *      dest-coalesce decision. Six address spellings measured; all neutral or worse.
 */
s32 func_800770B8(s32 arg0, s32 arg1, s32 arg2) {
    u16 sp[2];
    s32 *p_old;
    s32 r;
    s16 t0;
    s16 a2;

    do { } while (0);
    p_old = (s32 *)(arg0 + 0x58);
    sp[0] = 0;
    sp[1] = 0;
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
