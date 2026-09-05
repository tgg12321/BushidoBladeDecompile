/* candidate_objmodel_ALL2.c - func_800770B8 (src/text1b.c) - s29 object-model, 2026-09-05
 * The floor body (candidate.c, score 5 / 175 / 175) with the OBJECT MODEL corrected at the
 * DECLARATION level.  Measured this session: sandbox 5 / 175 / 175, whole-TU .text byte-
 * identical to candidate.c's object (the only object difference is the relocation spelling
 * D_8009BD20+1 vs D_8009BD21+0, identical linked bytes).
 * REQUIRES the TU declaration edits in s29-objmodel-ALL2-declaration-edits.patch
 * (unified diff against HEAD src/text1b.c):
 *   extern u8  D_8009BCE4[20];      (was: extern u8 D_8009BCE4;  - 20-slot flag table, census)
 *   extern u8  D_8009BD20[2][2];    (was: extern u8 D_8009BD20; extern u8 D_8009BD21;  - pair
 *                                    table, func_80074488 reads both bytes with one x*2 index)
 *   extern s16 D_800A35D0[2][2];    (was: extern s16 D_800A35D0;  - per-player pair; the matched
 *                                    func_80075670 argument becomes D_800A35D0[arg1], byte-neutral)
 * These are src/text1b.c edits (and would be header-canonical on completion: an integration
 * handoff naming include/*.h).  This body does NOT compile against the pristine TU.
 * Every DECLARATION PUN the driver flagged in candidate.c (325/348/350/375) is gone: the
 * D_8009BCE4 and D_8009BD20 sites are plain subscripts; the D_800A35D0 site is the array
 * decaying to a byte pointer, which is the only spelling that reproduces the target's
 * lui/addiu/addu-then-two-sh rows (direct subscript = 13/176, row decay = 10/175, row
 * pointer local = 35/175 - evidence.md [s29]).  Same single FAKE unit as candidate.c.
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
        a2 = 0;
        ptr = (u8 *)((t0 * 2) + (s32)base);
        *(s16 *)(ptr + 0x10) = 0;
        *(s16 *)(ptr + 0x8) = 0;
        *(s16 *)(ptr + 0xC) = 0;
        *(s16 *)(ptr + 0x14) = 0;
        *(s16 *)(ptr + 0x3C) = 0;
        ptr = (u8 *)D_800A35D0;
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
            D_8009BCE4[idx] = (u8)(D_8009BCE4[idx] & 0xF2);
            if ((arg2 & mask) != 0) {
                D_8009BCE4[idx] = (u8)(D_8009BCE4[idx] | 1);
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
    *(u8 *)(D_800A36A0 + 0x66) = D_8009BD20[*(u8 *)(D_800A36A0 + 0x67)][1];
    D_800A35DC = 1;
    return 1;
}
