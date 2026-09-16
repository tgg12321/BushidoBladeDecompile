/* _exeque candidate — session 1 (recon), score 15/187 (down from 187 raw
 * INCLUDE_ASM floor). Apply this body to src/display.c in place of the
 * `INCLUDE_ASM("asm/funcs", _exeque);` line, plus the forward-declaration
 * fixups noted below.
 *
 * Forward-declaration fixups needed (already applied in src/display.c):
 *   - line ~871: `void _exeque();` -> `extern s32 _exeque(void);`
 *     (the function returns s32 — the old `void` forward decl conflicted
 *     with the real definition once it stopped being INCLUDE_ASM).
 *   - line ~976 (before _sync): delete the stray `extern void _exeque();`
 *     redeclaration (also wrong return type, and redundant once the
 *     forward decl above is fixed).
 *   - ADD (was completely missing — a real bug, not a style choice):
 *     `extern s32 D_8009BF84;` near the other D_8009BE7C/D_8009BE80/etc.
 *     externs. This variable was used (read AND written) in _exeque with
 *     NO declaration anywhere in the TU before this session — an
 *     undeclared-identifier bug that GCC 2.7.2 silently accepted (implicit
 *     extern) but which produced wrong codegen for the `SetIntrMask(mask)`
 *     unlock call (see s1 evidence: score 24 -> 15 after adding the decl).
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
            _que[D_8009BF7C].func(_que[D_8009BF7C].arg, _que[D_8009BF7C].count);
            D_8009BF68[0] = (s32)_que[D_8009BF7C].func;
            D_8009BF6C = (s32)_que[D_8009BF7C].arg;
            D_8009BF70 = _que[D_8009BF7C].count;
            D_8009BF7C = (D_8009BF7C + 1) & 0x3F;
        } while (D_8009BF78 != D_8009BF7C && !(*D_8009BF54 & 0x01000000));
    }
    SetIntrMask(D_8009BF84);
    if (D_8009BF78 == D_8009BF7C && !(*D_8009BF54 & 0x01000000) &&
        D_8009BE7C != 0 && D_8009BE80 != 0) {
        D_8009BE7C = 0;
        ((s32 (*)(void))D_8009BE80)();
    }
    return (D_8009BF78 - D_8009BF7C) & 0x3F;
}
