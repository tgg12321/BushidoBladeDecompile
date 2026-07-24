/* func_80062020 (text1b.c) — candidate, honest pure-C floor = 10 (sandbox --disable all)
 *
 * Clean pure C: 0 register-asm pins, 0 rules, 0 dead vars, no dual-spelling.
 * LOOP BODY matches target 100% (25/25 insns). All 10 residual diffs are in
 * the epilogue (3-store terminator). See hypotheses.md / evidence.md.
 *
 * KEY LEVER (found s1): read source via FIXED-base indexed form
 *   *(s32*)((u8*)arg0 + ofs + K)   [NOT the walking a0[K] form]
 * so GCC strength-reduces source into ONE walking giv (0/4/8(a0), a0+=12)
 * while keeping `ofs` as the dest-index biv (v1). The abandoned pinned src
 * used walking a0[K], which made GCC create TWO induction pointers (floor 20).
 */
void func_80062020(s32 *arg0) {
    s32 i;
    s32 ofs;
    s32 t;
    s32 *p;
    s32 i12;
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
    i12 = i + i;
    i12 = i12 + i;
    i12 = i12 << 2;
    p = (s32 *)((u8 *)&D_800F1198 + i12);
    p[2] = 0;
    p[1] = 0;
    p[0] = 0;
}
