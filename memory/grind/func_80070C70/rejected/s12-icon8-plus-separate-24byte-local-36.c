/* [s12] IconC70 shrunk to its evidence-backed 8 bytes plus a separate unreferenced `s32 sp50[6]`: 36/194 and the frame FALLS to 104 - GCC 2.7.2 drops the unreferenced local outright, so the real 24-byte local must be referenced. */
//@sub     s16 sp50[12];|||
void func_80070C70(s32 arg0) {
    s32 c60 = 0x60;
    PrimC70 prim;
    IconC70 icon;
    s32 sp50[6];
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
    icon.sp4C = 0xE7;
    icon.sp48 = 0xCC;
    icon.sp4A = 0x25;
    icon.sp4E = 1;
    func_80069898(arg0, (s32 *)&icon, 1);
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
    if (D_800A35B0 + (s16)D_800A3558 + 1 > 0) {
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
