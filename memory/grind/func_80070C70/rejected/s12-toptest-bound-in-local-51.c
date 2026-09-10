/* [s12] Bound hoisted into a local on the 31-point top-test chassis: 51/187 (7th callee-saved reg). s10 measured the same lever on the 49 body; re-measured on the improved chassis it is still net-negative. */
//@sub extern s32 D_800A3558;|||extern s16 D_800A3558;
/* [s10 forensics 2026-09-10] HYGIENE FIX: the //@sub directive MUST be line 1.
 * install.py only consumes `//@sub` lines at the START of the file, so while it sat
 * below this header comment the substitution was silently skipped and the file
 * installed as `extern s32 D_800A3558;` and measured 56, not 49.  s10 re-measured
 * 49 with the directive hoisted.  Do not move it back below the comment.
 * s10 frame census: this body spends 3 orphan spill slots (vars=104 vs the
 * target 80).  EVERY do/while declaration variant spends ZERO (vars=80 exactly).
 */
/* [s9 solver 2026-09-10] TOP-TEST CHASSIS, SHAPE-EXACT-EXCEPT-FRAME.  Score 49, 194 insns.
 * This body scores ONE POINT WORSE than chassis-toptest-cse-48.c but is structurally
 * CLOSER to the target, and score is the misleading gradient here.  Measured with
 * tools/ra_solver/goal_from_tgt.py classify (object-level, both streams 194 insns):
 *   - chassis-toptest-cse-48.c (extern s32 D_800A3558 + an (s16) cast at both sites)
 *     has 13 frame-offset shapes in one stream only PLUS `ours only: lw x2` against
 *     `target only: lhu x2`.
 *   - THIS body (extern s16 D_800A3558, read BARE in the loop bound and with an (s16)
 *     cast in the loop body) has NO load-shape difference at all: its entire residual
 *     multiset is the 13 prologue/epilogue insns that differ only because our frame is
 *     152 (vars=104) and the target's is 128 (vars=80), plus one `addiu #,#,12` against
 *     one `nop`.  Every lhu/lh/sll/sra in the target is present here in the same count.
 * The 24 bytes are the three orphaned `(use (reg))` pseudos p116/p168/p173, identical to
 * every other spelling.  Kill those and this chassis is 2 instruction-shapes from the
 * target -- see the s9 frontier in hypotheses.md (combine.c:10834 CODE_LABEL topology).
 * The equivalent `extern u16 D_800A3558;` + (s16) cast at BOTH sites (s9/d/tt_u16_c_c.c)
 * is the same 49 with the same shape residual and the same three orphans.
 */
void func_80070C70(s32 arg0) {
    s32 c60 = 0x60;
    PrimC70 prim;
    IconC70 icon;
    s32 ctx_or_var_s2;
    s32 var_s0;
    s32 var_s3;
    s32 t;
    u8 code;
    s32 g;
    s32 bound;

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
    var_s3 = 0xA;
    t = g + 0x48;
    prim.p_geom = g;
    prim.p_static = t;
    do {
        prim.mode = var_s0 << 6;
        prim.link = *(s32 *)(arg0 + 0x10);
        prim.code = var_s3;
        *(s32 *)(arg0 + 0x10) = func_8007352C((s32 *)&prim);
        var_s0 += 1;
        prim.p_geom += 0xC;
    } while (var_s0 < 6);
    prim.p_geom = *(s32 *)(ctx_or_var_s2);
    SetDrawMode(*(s32 *)(arg0 + 0x18), 1, 0, func_8006E480(prim.p_geom, c60), 0);
    AddPrim(D_800A374C + 0x28, *(s32 *)(arg0 + 0x18));
    *(s32 *)(arg0 + 0x18) = *(s32 *)(arg0 + 0x18) + 0xC;
    prim.p_geom = *(s32 *)(ctx_or_var_s2 + 8);
    bound = D_800A35B0 + D_800A3558 + 1;
    for (var_s0 = 0; var_s0 < bound; var_s0++) {
            s32 ctx = var_s0 * 3;
            code = D_800A3560[ctx];
            if ((code != 5) && (code != 16)) {
                g = prim.p_geom;
                t = g + 0xC;
                prim.p_static = t;
                prim.p_static = t + (D_800A3590[var_s0] << 4);
                if ((((s16)D_800A3558 + D_800A35B0) != 0) || (D_800A35BC == 2)) {
                    prim.mode = 0x50 + var_s0 * 0x16C;
                } else {
                    prim.mode = 0x105;
                }
                prim.link = *(s32 *)(arg0 + 0x10);
                prim.code = 1;
                *(s32 *)(arg0 + 0x10) = func_8007352C((s32 *)&prim);
            }
    }
    SetDrawMode(*(s32 *)(arg0 + 0x18), 1, 0, func_8006E480(prim.p_geom, c60), 0);
    AddPrim(D_800A374C + 4, *(s32 *)(arg0 + 0x18));
    *(s32 *)(arg0 + 0x18) = *(s32 *)(arg0 + 0x18) + 0xC;
    func_80070F78(arg0, (s32 *)&prim);
    func_8006ECF4(arg0);
    func_80072E10(arg0);
    func_80073200(arg0);
}
