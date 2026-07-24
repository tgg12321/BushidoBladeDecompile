/* func_80062020 (text1b.c) — candidate, honest pure-C floor = 4 (sandbox --disable all)
 *
 * Clean pure C: 0 register-asm pins, 0 rules, 0 dead vars, no dual-spelling.
 * LOOP BODY matches target 100% (25/25 insns). Epilogue register allocation now
 * matches target 100% (index in v1, base &D_800F1198 in v0, cols b,c via 4/8(v0)).
 *
 * KEY LEVER 1 (s1): read source via FIXED-base indexed form
 *   *(s32*)((u8*)arg0 + ofs + K)   [NOT the walking a0[K] form]
 * so GCC strength-reduces source into ONE walking giv (0/4/8(a0), a0+=12).
 *
 * KEY LEVER 2 (s2): REUSE `ofs` (the loop's byte-offset biv, allocated to v1)
 * to hold the terminator index (12*count is also a byte offset -> semantically
 * the same value). This biases RA to keep the terminator index in v1 (target),
 * instead of v0. Dropped floor 10 -> 4. Everything now matches EXCEPT col a's
 * addressing: mine emits `sw zero,0(v0)` (reuses base pointer v0); target
 * recomputes `lui at,%hi(1198); addu at,at,v1; sw zero,%lo(1198)(at)` (keeps the
 * raw index v1 live). See hypotheses.md — the residual is the col-a partial-CSE
 * addressing-mode split.
 *
 * s3 (structural): the CSE-defeat lever is KILLED. Store-order permutations
 * (c,b,a=4, c,a,b=5, a,b,c=5) all fold col a onto the base pointer v0 — the
 * base-pointer CSE is store-order-invariant. Type/width distinction is
 * unavailable (all 3 are `sw` of 0; a differently-typed view of the same lvalue
 * is the banked dual-spelling). No intervening dependency exists in a 3-word
 * constant-zero terminator. Structural axis exhausted (s1/s2/s3); floor flat at
 * 4. Frontier -> permuter (confirm no non-cheat byte-0 form), then
 * endgame-lock-disposition OWNER-ESCALATION.
 */
void func_80062020(s32 *arg0) {
    s32 i;
    s32 ofs;
    s32 t;
    s32 *p;
    t = *(s32 *)((u8 *)arg0 + 0);
    D_800A32B8 = 0;
    i = 0;
    if ((t & 1) == 0) goto end;
    ofs = 0;
    do {
        t = *(s32 *)((u8 *)arg0 + ofs + 0);
        *(s32 *)((u8 *)&D_800F1198 + ofs) = t;
        t = *(s32 *)((u8 *)arg0 + ofs + 4);
        i = i + 1;
        *(s32 *)((u8 *)&D_800F119C + ofs) = t;
        t = *(s32 *)((u8 *)arg0 + ofs + 8);
        *(s32 *)((u8 *)&D_800F11A0 + ofs) = t;
        ofs = ofs + 12;
        t = *(s32 *)((u8 *)arg0 + ofs + 0);
    } while ((t & 1) != 0);
end:
    ofs = i + i;
    ofs = ofs + i;
    ofs = ofs << 2;
    p = (s32 *)((u8 *)&D_800F1198 + ofs);
    p[2] = 0;
    p[1] = 0;
    p[0] = 0;
}
