/* ===================== S17 (structural): 7 -> 0 / 194  BYTE MATCH ======================
 * Companion declaration edits in src/text1b.c (all applied, all part of the diff):
 *   - `extern s32 D_800A3558;`  ->  `extern s16 D_800A3558;`   (target reads it lhu/lh)
 *   - `extern u8  D_800A3560;`  ->  `extern u8  D_800A3560[];` (INDEXED-ACCESS signal)
 *   - `extern s16 D_800A3590;`  ->  `extern s16 D_800A3590[];` (INDEXED-ACCESS signal)
 *   - the redundant `extern s32 func_80069898(s32, s32 *, s32);` m2c prototype is removed
 *     (the real definition at src/text1b.c:5413 takes `u16 *`)
 *   - the s15 `IconC70 { ... s16 sp50[12]; }` frame-coercion field is NOT needed and is gone.
 *
 * THE FIX - sched.c/true_dependence's MEM_IN_STRUCT_P disambiguation (the whole 7 points).
 * s16 closed the guard-block residual as "the three gp loads carry a DATA dependence on the
 * `*(arg0 + 0x18) += 0xC` store, so the scheduler cannot move them above it".  That is true
 * ONLY while the store's MEM is a scalar.  tools/gcc-2.7.2/sched.c:817 true_dependence returns
 * 0 when  MEM_IN_STRUCT_P(store) && rtx_addr_varies_p(store) && mode != QImode &&
 * !MEM_IN_STRUCT_P(load) && !rtx_addr_varies_p(load).  The store is (plus (reg) 24) - address
 * varies, SImode; the loads are (symbol_ref D_800A3558/D_800A35B0) - fixed address, not in a
 * struct.  So the ONLY missing predicate was MEM_IN_STRUCT_P on the store, which a
 * pointer-cast deref never sets and a COMPONENT_REF always does.  Spelling the 0x18 accesses
 * as `((GameObj *)arg0)->field_18` (GameObj is already the declared type of this argument -
 * func_80069898 takes GameObj *) killed the dependence and the scheduler produced the
 * target's interleave: 7 -> 2.  Measured: all 12 sites = 2, the single pre-loop store LHS
 * alone = 2, the RHS alone = 7 (the store is what must be in-struct, not the read).
 *
 * 2 -> 0: the last two insns were the long-standing prologue tie (`addiu a0,sp,24` vs
 * `move s0,zero`).  On the NEW (struct-store) chassis the second loop's induction-variable
 * initialiser is no longer inert: writing that loop as `for (var_s0 = 0; var_s0 < 6;
 * var_s0++)` - i.e. moving `var_s0 = 0` out of the entry block and into the loop header -
 * flips the tie and matches.  Every entry-block position from the first func_8007352C call
 * onward (13 of them) is also 0; the 4 positions before it stay at 2.  s16's "initialiser
 * position is inert" kill was chassis-relative, exactly as the ledger recorded it.
 *
 * `prim.p_static = t; prim.p_static = t + (D_800A3590[i] << 4);` (a real dead store) is NOT
 * needed either: the compound `prim.p_static = t; prim.p_static += D_800A3590[i] << 4;` -
 * ordinary split-init accumulation ([[split-init-accumulation-sanctioned]]) - is also 0,
 * and that is what this body carries.  Dropping the first store entirely costs 22.
 *
 * WHAT REMAINS IN THE BODY (both vetted in memory/grind/func_80070C70/self_vet.md):
 *   - `s32 c60 = 0x60;` - constant-holder local, FAKE-annotated, sanctioned family
 *     (.claude/rules/named-local-fake-exception.md).  Literal spelling = 7/191.
 *   - the second `prim.zero1C = 0; prim.mode = 0;` pair - NOT a FAKE: the target emits both
 *     `sw zero,48(sp)` / `sw zero,52(sp)` pairs, so this is the original source's own
 *     redundancy, reproduced (removing either costs 1-2 insns of distance).
 *
 * ORACLE: full build SHA1 == 62efab4f73f992798c43e8c730aa43baa10bb4fa (verify-oracle, s17).
 */
void func_80070C70(s32 arg0) {
    s32 c60 = 0x60; /* FAKE: constant-holder local, mechanism: local-alloc/global.c keeps a
                     * live-across-call pseudo in a callee-saved register (the target's
                     * `li s4,96` + three `move a1,s4`); the inline literal re-materializes
                     * `li a1,0x60` at each call site and measures 7/191 vs 0/194.
                     * lever-exhaustion: memory/grind/func_80070C70/hypotheses.md (s11-s17;
                     * literal re-measured on every chassis, 9 declaration slots inert) */
    PrimC70 prim;
    u16 rect[4];
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
    /* NOT a coercion: the target itself stores zero to both fields twice - asm/funcs/
     * func_80070C70.s emits `sw zero,48(sp)` / `sw zero,52(sp)` before the `lw s2,100(v1)`
     * context fetch AND again after it.  The original source clears mode/zero1C a second
     * time after fetching the context; both stores are in the matched 194 insns. */
    prim.zero1C = 0;
    prim.mode = 0;
    g = *(s32 *)(ctx_or_var_s2 + 4);
    t = g + 0xC;
    prim.p_geom = g;
    prim.p_static = t;
    prim.link = *(s32 *)(arg0 + 0x10);
    prim.code = 1;
    *(s32 *)(arg0 + 0x10) = func_8007352C((s32 *)&prim);
    SetDrawMode(((GameObj *)arg0)->field_18, 1, 0, func_8006E480(prim.p_geom, c60), 0);
    AddPrim(D_800A374C + 4, ((GameObj *)arg0)->field_18);
    ((GameObj *)arg0)->field_18 += 0xC;
    rect[2] = 0xE7;
    rect[0] = 0xCC;
    rect[1] = 0x25;
    rect[3] = 1;
    func_80069898((GameObj *)arg0, (u16 *)rect, 1);
    g = *(s32 *)(ctx_or_var_s2);
    t = g + 0x48;
    prim.p_geom = g;
    prim.p_static = t;
    for (var_s0 = 0; var_s0 < 6; var_s0++) {
        prim.mode = var_s0 << 6;
        prim.link = *(s32 *)(arg0 + 0x10);
        prim.code = 0xA;
        *(s32 *)(arg0 + 0x10) = func_8007352C((s32 *)&prim);
        prim.p_geom += 0xC;
    }
    prim.p_geom = *(s32 *)(ctx_or_var_s2);
    SetDrawMode(((GameObj *)arg0)->field_18, 1, 0, func_8006E480(prim.p_geom, c60), 0);
    AddPrim(D_800A374C + 0x28, ((GameObj *)arg0)->field_18);
    ((GameObj *)arg0)->field_18 += 0xC;
    prim.p_geom = *(s32 *)(ctx_or_var_s2 + 8);
    for (var_s0 = 0; var_s0 < 1 + D_800A35B0 + D_800A3558; var_s0++) {
            s32 ctx = var_s0 * 3;
            code = D_800A3560[ctx];
            if ((code != 5) && (code != 16)) {
                g = prim.p_geom;
                t = g + 0xC;
                prim.p_static = t;
                prim.p_static += D_800A3590[var_s0] << 4;
                if (((D_800A35B0 + D_800A3558) != 0) || (D_800A35BC == 2)) {
                    prim.mode = 0x50 + var_s0 * 0x16C;
                } else {
                    prim.mode = 0x105;
                }
                prim.link = *(s32 *)(arg0 + 0x10);
                prim.code = 1;
                *(s32 *)(arg0 + 0x10) = func_8007352C((s32 *)&prim);
            }
    }
    SetDrawMode(((GameObj *)arg0)->field_18, 1, 0, func_8006E480(prim.p_geom, c60), 0);
    AddPrim(D_800A374C + 4, ((GameObj *)arg0)->field_18);
    ((GameObj *)arg0)->field_18 += 0xC;
    func_80070F78(arg0, (s32 *)&prim);
    func_8006ECF4(arg0);
    func_80072E10(arg0);
    func_80073200(arg0);
}
