/* Rejected: s5 split-init (`s5 = s4; s5 += a1;`) rose score 2 -> 11.
 * Mechanism: the extra copy insn from split-init breaks the addu $21
 * delay-slot fill and cascades a RA rotation identical in shape to the
 * H1 kill (i-before-v1). The user-sanctioned split-init lever is
 * neutral or helpful only when the target RTL already contains an
 * extra copy insn; here it introduces one that has no target counterpart.
 */
void func_80045294(s32 a0, s32 a1) {
    s32 sum = 0;
    s32 v1 = a0 << 4;
    s32 s4 = *(s32 *)((u8 *)&D_800EED14 + v1);
    s32 i = a0;
    s32 count = D_800A33AC;
    s32 s5 = s4;
    s5 += a1;
    /* ... body unchanged ... */
}
