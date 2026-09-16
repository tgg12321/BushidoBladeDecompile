/* _exeque — s6 (structural) rejected form.
 * Swapping the && operand order in the final callback guard from
 * `if (*p != 0 && D_8009BE80 != 0)` to `if (D_8009BE80 != 0 && *p != 0)`
 * on the s4/s5 floor-2/187 do-while(0)-wrapped chassis (candidate.c
 * unchanged otherwise).
 *
 * Measured: sandbox _exeque --disable all score 2 -> 8 (WORSE). Reordering
 * which value is loaded first changes the RTL insn immediately preceding
 * the jalr call and disturbs more than just the delay-slot fill; the whole
 * final block's register allocation regresses. Reverted; not adopted.
 * kill_scope: instance — this exact operand-order swap, on the s4 chassis,
 * both do-while(0) FAKE wraps present, no other construct changed.
 */
s32 _exeque(void) {
    s32 mask;

    if (*D_8009BF54 & 0x01000000) {
        return 1;
    }
    mask = SetIntrMask(0);
    D_8009BF84 = mask;
    if (D_8009BF78 != D_8009BF7C && !(*D_8009BF54 & 0x01000000)) {
        do {
            if (((D_8009BF7C + 1) & 0x3F) == D_8009BF78 && D_8009BE80 == 0) {
                DMACallback(2, 0);
            }
            while (!(*D_8009BF48 & 0x04000000)) {
            }
            do {
                _que[D_8009BF7C].func(_que[D_8009BF7C].arg, _que[D_8009BF7C].count);
                do {
                    D_8009BF68[0] = (s32)_que[D_8009BF7C].func;
                    D_8009BF6C = (s32)_que[D_8009BF7C].arg;
                } while (0);
                D_8009BF70 = _que[D_8009BF7C].count;
            } while (0);
            D_8009BF7C = (D_8009BF7C + 1) & 0x3F;
        } while (D_8009BF78 != D_8009BF7C && !(*D_8009BF54 & 0x01000000));
    }
    SetIntrMask(D_8009BF84);
    if (D_8009BF78 == D_8009BF7C && !(*D_8009BF54 & 0x01000000)) {
        s32 *p = &D_8009BE7C;
        if (D_8009BE80 != 0 && *p != 0) {           /* <-- swapped here */
            *p = 0;
            ((s32 (*)(void))D_8009BE80)();
        }
    }
    return (D_8009BF78 - D_8009BF7C) & 0x3F;
}
