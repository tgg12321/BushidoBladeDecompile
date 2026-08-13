/* REJECTED FAMILY — 18 semantics-preserving structural forms of func_80083794.
 * Session 2 (structural modality, 2026-08-13). ALL measured; NONE flips the
 * $s0/$s1 roles, NONE changes the $v0 temp to $t0, NONE changes the frame.
 *
 * WHY THEY ARE DEAD (exact, from the cc1 -da dumps in
 * tmp/grind/func_80083794/s2/dump/ings2.i.lreg + .greg):
 *
 *   pseudo 72 = `count`  — "used 8 times across 8 insns; crosses 1 call"  -> $s0 (16)
 *   pseudo 73 = `p`      — "used 7 times across 7 insns; crosses 1 call"  -> $s1 (17)
 *
 * global.c allocno_compare ranks by
 *     floor_log2(n_refs) * n_refs / live_length * 10000 * size
 * so with n_refs 8 vs 7 the numerators are 3*8=24 vs 2*7=14 — `count` wins $s0
 * by a wide margin, and live_length cannot close a 1.7x gap (both pseudos span
 * essentially the same range).
 *
 * The 8-vs-7 split is FORCED BY THE ALGORITHM, not by spelling. Loop-weighted:
 *   count = 2 out-of-loop refs (la, initial beqz guard) + 2 in-loop (addiu -1, bnez)
 *   p     = 1 out-of-loop ref  (la)                     + 2 in-loop (lw base, addiu +4)
 * Every semantics-preserving rewrite below leaves that partition untouched;
 * the only way to add a reference to `p` is to add an instruction, which the
 * 28-insn target does not have room for.
 *
 * MEASURED FORMS (all score/reg columns from `sandbox --disable all`;
 * raw JSON: tmp/grind/func_80083794/s2/sweep_out.json + sweep2_out.json):
 *
 *   form                        score insns  p_reg temp frame
 *   v0 baseline (count then p)     18   28    s1    v0   -32   <- session floor
 *   v1 hoist load into local f     18   28    s1    v0   -32
 *   v2 for(;count;count--) header  18   28    s1    v0   -32
 *   v3 if + do{}while(--count)     18   28    s1    v0   -32
 *   v4 count initialised outside if 21  28    s1    v0   -32
 *   v5 p initialised outside if    22   28    s1    v0   -32
 *   v6 while (count--)             21   29    -     -    -32
 *   v7 end-pointer walk (no count) 22   30    -     -    -32
 *   v8 p assigned before count     22   28    s1    v0   -32
 *   v9 p[0]() + p=p+1 + count=count-1 18 28   s1    v0   -32
 *   w1 block-local q = p split     22   29    -     -    -32
 *   w2 flag store after both la's  22   28    s1    v0   -32
 *   w3 early-return guard          18   28    s1    v0   -32
 *   w4 early-return + p first      22   28    s1    v0   -32
 *   w5 u32 count (type narrowing)  18   28    s1    v0   -32
 *   w6 (*p)(); p++; count--;       18   28    s1    v0   -32
 *   w7 p != base+count guard       23   30    -     -    -32
 *   w8 named flag temp + flag+1    18   28    s1    v0   -32
 *
 * The score-22 forms differ from 18 only in the emission ORDER of the two masked
 * `la` pairs (see session-1 finding C1) — they are strictly worse and carry no
 * new information. No form ever produced p in $s0.
 *
 * Representative member of the family (v1, the "hoist the load" form that the
 * session-1 ledger named as H2's most promising probe):
 */
extern s32 D_800A2668;
extern void (*D_8008D070)(void);
extern s32 D_00000000;

void func_80083794(void) {
    s32 count;
    void (**p)(void);

    if (D_800A2668 == 0) {
        D_800A2668 = 1;
        count = (s32)&D_00000000;
        p = &D_8008D070;
        while (count != 0) {
            void (*f)(void) = *p;
            p++;
            count--;
            f();
        }
    }
}
