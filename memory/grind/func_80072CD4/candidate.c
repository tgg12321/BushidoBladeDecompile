/* func_80072CD4 - BYTE-MATCHING FORM. `sandbox func_80072CD4 --disable all` = 0,
 * build_insns 79 == target_insns 79, rules_dropped 0. Measured by grind session s5b
 * (2026-08-20) and re-verified unchanged on the current chassis by session s6
 * (artifact tmp/grind/func_80072CD4/s6/sandbox_s6_reverify.json).
 *
 * arg1 is a PSX libgpu POLY_G4 primitive; the byte offsets are its four vertex-colour
 * triples - rgb0 = 0x04/0x05/0x06, rgb1 = 0x0C/0x0D/0x0E, rgb2 = 0x14/0x15/0x16,
 * rgb3 = 0x1C/0x1D/0x1E (the setRGB0..setRGB3 field groups). Each branch assigns its own
 * complete rgb0 and rgb1 triple in ascending field order, then the two unconditional
 * rgb2/rgb3 triples follow. The COMPLETED-C sibling func_80072BC4 in this same file
 * (src/text1b.c:5822) is written in exactly this field-order house style, and itself carries
 * an identical hoistable-but-not-hoisted cross-arm duplicate store
 * `*(u8 *)((s32)(arg1) + 0x1D) = 0xC3;` at src/text1b.c:5840 and src/text1b.c:5843.
 *
 * The body declares no variable of any kind: no local, no holder, no volatile, no __asm__,
 * no register pin, no barrier, no do-while, no dead store, no annotation. Every statement is
 * a live field write executed on its own control-flow path with the value the game displays.
 * The red component of rgb0/rgb1 is 0xFC on both inner branches, so the arms happen to share
 * a common tail that GCC's jump2 cross-jumps at the join label - a compiler tail-merge of two
 * live statements on mutually exclusive paths, not a duplicated-into-arms lever.
 */
s32 func_80072CD4(s32 arg0, GameObj *arg1) {
    SetPolyG4(arg1);
    SetSemiTrans(arg1, 0);
    if (arg0 < 4) {
        if (*(s32 *)((s32)(D_800A35C4) + 8) & 4) {
            *(u8 *)((s32)(arg1) + 4) = 0xFC;
            *(u8 *)((s32)(arg1) + 5) = 0xC3;
            *(u8 *)((s32)(arg1) + 6) = 0x1E;
            *(u8 *)((s32)(arg1) + 0xC) = 0xFC;
            *(u8 *)((s32)(arg1) + 0xD) = 0xC8;
            *(u8 *)((s32)(arg1) + 0xE) = 0x32;
        } else {
            *(u8 *)((s32)(arg1) + 4) = 0xFC;
            *(u8 *)((s32)(arg1) + 5) = 0xC3;
            *(u8 *)((s32)(arg1) + 6) = 0x50;
            *(u8 *)((s32)(arg1) + 0xC) = 0xFC;
            *(u8 *)((s32)(arg1) + 0xD) = 0xDC;
            *(u8 *)((s32)(arg1) + 0xE) = 0x46;
        }
        *(u8 *)((s32)(arg1) + 0x14) = 0xFC;
        *(u8 *)((s32)(arg1) + 0x15) = 0x82;
        *(u8 *)((s32)(arg1) + 0x16) = 0;
        *(u8 *)((s32)(arg1) + 0x1C) = 0x32;
        *(u8 *)((s32)(arg1) + 0x1D) = 0x28;
        *(u8 *)((s32)(arg1) + 0x1E) = 0xA;
    } else {
        *(u8 *)((s32)(arg1) + 4) = 0x10;
        *(u8 *)((s32)(arg1) + 5) = 0x30;
        *(u8 *)((s32)(arg1) + 6) = 0x60;
        *(u8 *)((s32)(arg1) + 0xC) = 0x18;
        *(u8 *)((s32)(arg1) + 0xD) = 0;
        *(u8 *)((s32)(arg1) + 0xE) = 0x40;
        *(u8 *)((s32)(arg1) + 0x14) = 0x30;
        *(u8 *)((s32)(arg1) + 0x15) = 0;
        *(u8 *)((s32)(arg1) + 0x16) = 0x60;
        *(u8 *)((s32)(arg1) + 0x1C) = 0;
        *(u8 *)((s32)(arg1) + 0x1D) = 0;
        *(u8 *)((s32)(arg1) + 0x1E) = 0;
    }
    AddPrim(D_800A374C + 0x60, arg1);
    return (s32)((u8 *)arg1 + 0x24);
}
