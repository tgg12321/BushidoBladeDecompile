/* REJECTED (s14, 2026-09-10) - 20-cell sweep of HOISTING the second loop's invariants into
 * fresh locals, on the 22-point if-guarded do/while chassis (s8 had measured this family only
 * on the TOP-TEST chassis; this closes the do/while half).
 * AXES: bound hoist (none / full `n = D_800A35B0 + (s16)D_800A3558 + 1` / three partials) x
 * mode-test sum hoist (none / `sm = (s16)D_800A3558 + D_800A35B0`) x D_800A35BC hoist
 * (none / `md = D_800A35BC`).
 * RESULT: the incumbent (no hoist) = 22 is the unique optimum; EVERY hoist is worse.
 * sum hoist alone = 31 at 192 insns (this file); md hoist alone = 33 at 196; partial bound
 * hoists 33/35/37; full bound hoist = 35 at 192; all-three = 40-41.  Mirrors s8's top-test
 * result: a C-level local is exactly what loop.c/cse.c are free to hoist out of the loop, and
 * the target's per-iteration RELOADS cannot be spelled by a local.  tmp/.../s14/enum2.json.
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
    s32 sm;

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
    sm = (s16)D_800A3558 + D_800A35B0;
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
                if ((D_800A35BC == 2) || (sm != 0)) {
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
