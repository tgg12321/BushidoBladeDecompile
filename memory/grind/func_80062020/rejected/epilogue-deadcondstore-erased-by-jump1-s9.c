/* s9 MECHANISM CONTROL — NOT A PROPOSAL.
 * `if (i) { d = 1; }` with `d` an otherwise-unused local is the forbidden
 * dead-conditional-store family (.claude/agents/cheat-reviewer.md catalog).
 * It is compiled here ONLY to decide whether a jump+label pair that survives
 * jump1 / the post-loop jump pass / cse2 and is then erased by flow's DCE
 * (toplev.c:2983) + jump2 (toplev.c:3142) reproduces the target epilogue.
 * Whatever it measures, it is banked under rejected/, never submitted.
 */
void func_80062020(s32 *arg0) {
    s32 i;
    s32 ofs;
    s32 t;
    s32 d;
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
    if (i) { d = 1; }
    p[0] = 0;
}
