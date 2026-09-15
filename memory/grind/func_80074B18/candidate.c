/* func_80074B18 — candidate (s2, 2026-09-14). MEASURED THIS SESSION: honest
 * `sandbox func_80074B18 --disable all` = 0 (133/133 insns), and a full clean-driver
 * `engine build` relink produced SHA1 62efab4f73f992798c43e8c730aa43baa10bb4fa == oracle.
 *
 * History: on the s1 chassis this body scored 1. The single residual was the ASPSX
 * load-delay nop at 0x80074CF0 (`lw $a3,0x10($sp)` / `.L80074CF0:` / `nop` /
 * `sw $s3,0x14($a3)`) — a maspsx `.L`-label blind spot that then required a
 * per-function opt-in in maspsx_label_nop_funcs.txt (operator-only surface), which is
 * why s1 ended as an INTEGRATION HANDOFF. That gate was RETIRED 2026-09-14
 * (.claude/rules/maspsx-label-nop-gate.md): the label load-delay nop is now emitted
 * GLOBALLY with the same $at/$gp expansion guards the non-label path always had, and
 * maspsx_label_nop_funcs.txt / --label-nop-funcs are DELETED. The toolchain fingerprint
 * change (2046c403e76d0fe5 -> a04b7a59487234ab) is exactly that retirement, and it drops
 * this body from 1 to 0 with NO change to the C.
 *
 * Zero constructs: no FAKE, no volatile, no asm, no pins, no dead locals. `s16 n` is
 * load-bearing: it keeps GCC from folding the hoisted inner-loop entry test (evidence.md).
 */
void func_80074B18(s32 *arg0, s32 arg1, s32 arg2) {
    u8 *p;
    u8 *t;
    s16 i;
    s16 j;
    s16 n;
    s32 ot;

    n = 5;
    if (arg2 != 0) {
        n = 8;
    }
    p = (u8 *)arg0[5];
    for (i = 0; i < *(u8 *)(D_800A36A0 + 0x65) + 3; i++) {
        t = *(u8 **)(*(u8 **)(D_800A36A0 + 4) + 0x3C);
        for (j = 0; j < n; j++) {
            SetTile((GameObj *)p);
            *(u8 *)(p + 4) = *(u8 *)(t + 8);
            *(u8 *)(p + 5) = *(u8 *)(t + 9);
            *(u8 *)(p + 6) = *(u8 *)(t + 0xA);
            *(u16 *)(p + 0xC) = *(u16 *)(t + 4);
            *(u16 *)(p + 0xE) = *(u16 *)(t + 6);
            SetSemiTrans((GameObj *)p, 0);
            if (arg2 != 0) {
                *(s16 *)(p + 8) = *(u16 *)(t + 0) + arg1 * 240;
                *(s16 *)(p + 0xA) = *(u16 *)(t + 2) + i * 34 + 0x2B;
            } else {
                *(s16 *)(p + 8) = *(u16 *)(t + 0) + arg1 * 240;
                *(s16 *)(p + 0xA) = *(u16 *)(t + 2) + i * 17 + 0x7C;
            }
            ot = 0xB;
            if (arg1 != 0) {
                ot = 0x15;
            }
            AddPrim(D_800A374C + ot * 4, (GameObj *)p);
            p += 0x10;
            t += 0xC;
        }
    }
    arg0[5] = (s32)p;
}
