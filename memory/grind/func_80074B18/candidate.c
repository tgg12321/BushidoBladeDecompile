/* func_80074B18 — candidate (s1, 2026-09-10). Honest sandbox --disable all = 1 on the
 * HEAD chassis (-mel -msoft-float). The single residual is the ASPSX load-delay nop at
 * 0x80074CF0 (`lw $a3,0x10($sp)` / `.L80074CF0:` / `nop` / `sw $s3,0x14($a3)`): a
 * maspsx `.L`-label blind spot retired by the per-function opt-in in
 * maspsx_label_nop_funcs.txt (fidelity gate, operator surface). With that entry the
 * object is byte-identical and the full relink SHA1 == oracle
 * (tmp/grind/func_80074B18/s1/gated_sha1.txt). Zero constructs: no FAKE, no volatile,
 * no asm, no pins. `s16 n` is load-bearing: it keeps GCC from folding the hoisted
 * inner-loop entry test (see evidence.md). Apply with
 *   python3 tmp/grind/func_80074B18/s1/splice.py memory/grind/func_80074B18/candidate.c
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
