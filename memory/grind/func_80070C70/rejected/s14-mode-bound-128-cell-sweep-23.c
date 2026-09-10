/* REJECTED (s14, 2026-09-10) - representative of a 128-cell EXHAUSTIVE sweep of the second
 * loop's bound + mode-test spelling space on the 22-point if-guarded do/while chassis.
 * AXES (all crossed): guard-bound parenthesisation/operand order (4) x tail-bound
 * parenthesisation/operand order (4) x mode-test spelling (8: {35BC-first, sum-first} x
 * {(s16)3558+35B0, 35B0+(s16)3558} x {`!= 0`, bare truthiness}).
 * HISTOGRAM: {22:4, 23:12, 24:12, 25:8, 26:12, 27:12, 28:4, 30:8, 31:16, 32:8, 33:8, 34:16, 35:8}.
 * The axes are ADDITIVE and separable (per-axis minima): guard g0 = the incumbent spelling
 * `D_800A35B0 + (s16)D_800A3558 + 1 > 0` is the unique optimum (g2 +1, g1/g3 +3..4); tail t0
 * `D_800A35B0 + ((s16)D_800A3558 + 1)` ties t2 and beats t1/t3 by 1; mode-test 35BC-first is 22
 * and sum-first (the TARGET's own branch topology) is uniformly +8.  `!= 0` vs bare is
 * BYTE-NEUTRAL.  Four cells (g0t0m0, g0t0m2, g0t2m0, g0t2m2) are byte-identical at 22/194.
 * This file is the best NON-incumbent cell (g2t0m0 = 23).  Full data: tmp/grind/func_80070C70/
 * s14/enum1.json.  Nothing in this region's spelling space is below the incumbent.
 */
void func_80070C70(s32 arg0) {
    s32 c60 = 0x60;
    PrimC70 prim;
    u16 rect[16];
    s32 ctx_or_var_s2;
    s32 var_s0;
    s32 t;
    u8 code;
    s32 g;

    prim.zero10 = 0;
    prim.mode = 0;
    prim.zero1C = 0;
    prim.width = 0x100;
    prim.height = 0x100;
    prim.byte28 = 0;
    ctx_or_var_s2 = (s32)*(s32 **)(D_800A35A8 + 0x64);
    prim.zero1C = 0;
    prim.mode = 0;
    g = *(s32 *)(ctx_or_var_s2 + 4);
    var_s0 = 0;
    t = g + 0xC;
    prim.p_geom = g;
    prim.p_static = t;
    prim.link = *(s32 *)(arg0 + 0x10);
    prim.code = 1;
    *(s32 *)(arg0 + 0x10) = func_8007352C((s32 *)&prim);
    SetDrawMode(*(s32 *)(arg0 + 0x18), 1, 0, func_8006E480(prim.p_geom, c60), 0);
    AddPrim(D_800A374C + 4, *(s32 *)(arg0 + 0x18));
    *(s32 *)(arg0 + 0x18) = *(s32 *)(arg0 + 0x18) + 0xC;
    rect[2] = 0xE7;
    rect[0] = 0xCC;
    rect[1] = 0x25;
    rect[3] = 1;
    func_80069898((GameObj *)arg0, (u16 *)rect, 1);
    g = *(s32 *)(ctx_or_var_s2);
    t = g + 0x48;
    prim.p_geom = g;
    prim.p_static = t;
    do {
        prim.mode = var_s0 << 6;
        prim.link = *(s32 *)(arg0 + 0x10);
        prim.code = 0xA;
        *(s32 *)(arg0 + 0x10) = func_8007352C((s32 *)&prim);
        var_s0 += 1;
        prim.p_geom += 0xC;
    } while (var_s0 < 6);
    prim.p_geom = *(s32 *)(ctx_or_var_s2);
    SetDrawMode(*(s32 *)(arg0 + 0x18), 1, 0, func_8006E480(prim.p_geom, c60), 0);
    AddPrim(D_800A374C + 0x28, *(s32 *)(arg0 + 0x18));
    *(s32 *)(arg0 + 0x18) = *(s32 *)(arg0 + 0x18) + 0xC;
    prim.p_geom = *(s32 *)(ctx_or_var_s2 + 8);
    if ((s16)D_800A3558 + D_800A35B0 + 1 > 0) {
        var_s0 = 0;
        do {
            s32 ctx = var_s0 * 3;
            code = D_800A3560[ctx];
            if ((code != 5) && (code != 16)) {
                g = prim.p_geom;
                t = g + 0xC;
                prim.p_static = t;
                prim.p_static = t + (D_800A3590[var_s0] << 4);
                if ((D_800A35BC == 2) || (((s16)D_800A3558 + D_800A35B0) != 0)) {
                    prim.mode = 0x50 + var_s0 * 0x16C;
                } else {
                    prim.mode = 0x105;
                }
                prim.link = *(s32 *)(arg0 + 0x10);
                prim.code = 1;
                *(s32 *)(arg0 + 0x10) = func_8007352C((s32 *)&prim);
            }
            var_s0++;
        } while (var_s0 < D_800A35B0 + ((s16)D_800A3558 + 1));
    }
    SetDrawMode(*(s32 *)(arg0 + 0x18), 1, 0, func_8006E480(prim.p_geom, c60), 0);
    AddPrim(D_800A374C + 4, *(s32 *)(arg0 + 0x18));
    *(s32 *)(arg0 + 0x18) = *(s32 *)(arg0 + 0x18) + 0xC;
    func_80070F78(arg0, (s32 *)&prim);
    func_8006ECF4(arg0);
    func_80072E10(arg0);
    func_80073200(arg0);
}
