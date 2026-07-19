/* REJECTED s1: score 2 -> 11. Initializing i before v1 rotates callee-save RA
 * (a0->$21, s4->$18, s5->$20) and cse copy-propagates the shift operand to
 * sll $3,$16 (target reads $18). Proves original source computes v1 = a0<<4
 * before i = a0. */
void func_80045294(s32 a0, s32 a1) {
    s32 sum = 0;
    s32 i = a0;
    s32 v1 = a0 << 4;
    s32 s4 = *(s32 *)((u8 *)&D_800EED14 + v1);
    s32 count = D_800A33AC;
    s32 s5 = s4 + a1;
    /* ... rest identical to candidate.c ... */
}
