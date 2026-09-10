/* candidate.c - func_80070C70 - session 4 (permuter). Honest floor 56 (was 99 at s3).
 *
 * FLOOR HISTORY: 194 (HEAD, no C body) -> 108 -> 101 (s1) -> 99 (s3, RecC70 COMPONENT_REF
 * access path) -> 61 -> 56 (s4: `var_s0 += 1;` duplicated into all THREE arms of the loop body, which
 * raises loop.c's `bl->biv_count` for var_s0's biv from 1 to 3 and drives the D_800A3590 address
 * giv's benefit to 0, so strength_reduce REJECTS it instead of reducing it).
 * Two further s4 moves, both proposed by the permuter campaign on the floor-61 chassis and
 * hand-vetted before adoption, take 61 -> 58 -> 56:
 *   (i)  at the FIRST func_8007352C call site, `prim.link = *(s32 *)(arg0 + 0x10);` is written
 *        BEFORE `prim.code = 1;` (plain statement reordering, ordinary C)            61 -> 58
 *   (ii) `*(s32 *)(ctx_or_var_s2 + 8)` is read into the named intermediate `new_var` before the
 *        `*(arg0 + 0x18) += 0xC` store and assigned to prim.p_geom after it         58 -> 56
 * At 56 our build emits 190 insns against target's 194.
 *
 * REQUIRES these companion edits in src/text1b.c - they are part of the measured 61 (unchanged
 * from s3):
 *   1. extern u8  D_800A3560[];   (was: extern u8  D_800A3560;)   both occurrences: l.2124, l.6499
 *   2. typedef struct RecC70 { s16 v; } RecC70;
 *      extern RecC70 D_800A3590[];   (was: extern s16 D_800A3590;)  both occurrences: l.2129, l.6500
 *   3. typedef struct IconC70 { s16 sp48; s16 sp4A; s16 sp4C; s16 sp4E; s16 sp50[12]; } IconC70;
 *
 * WHY THE THREE-ARM `var_s0 += 1;` IS THE WHOLE s4 MOVE - mechanism, read out of the dump:
 *   - s3 (H7) pinned the residual to tools/gcc-2.7.2/loop.c:3824,
 *     `v->lifetime * threshold * benefit < insn_count` marks a giv "not worth while", where
 *     benefit is the recorded benefit MINUS `add_cost * bl->biv_count` (add_cost = 2, loop.c:307)
 *     and threshold = (loop_has_call ? 1 : 2) * (3 + n_non_fixed_regs) (loop.c:3241) = 28..31 here.
 *   - With ONE source increment of var_s0, biv_count = 1: the combined 313+319 giv has benefit
 *     6 - 2 = 4, product 112..124 >= insn_count, so it is REDUCED into a fourth induction
 *     register (`move sN,zero` / `addiu sN,sN,2`) that the target does not spend.
 *   - With THREE source increments, biv_count = 3: benefit 6 - 6 = 0, product 0 < insn_count, and
 *     tmp/grind/func_80070C70/dumps/text1b.loop now prints
 *     `giv at 337 combined with giv at 331` followed by `giv of insn 331 not worth while, 0 vs 63`
 *     - the reduction is gone and the target's in-loop `sll $a0, $s0, 1` recompute is what we emit.
 *   - TWO increment sites are NOT enough (measured): benefit 6 - 4 = 2, product 56..62 >= 48, and
 *     both 2-site spellings measure 101, i.e. WORSE than the 99 one-site chassis.
 *   - The arms chosen matter as much as the count. Duplicating the whole call tail into the two
 *     mode arms (3 sites) measures 88 at 200 insns; duplicating all three increments measures 93
 *     at 195; duplicating ONLY `var_s0 += 1` into the two mode arms plus a `continue`-style skip
 *     arm measures 65 at 195; and the form below - only `var_s0 += 1` duplicated, the skip arm
 *     spelled as a plain `else` so that `var_s3 += 0x16C;` and `ctx_or_var_s2 += 3;` stay SHARED
 *     and fall through exactly like the target's .L80070EC4 - measures 61 at 192.
 *
 * CONSTRUCT STATUS: the three `var_s0 += 1;` copies are the duplicated-statement-into-arms family
 * and `new_var` is the named-intermediate family (.claude/rules/narrow-byte-args-packed-call.md +
 * the 2026-08-17 clarification in no-new-park-categories.md)
 * (.claude/rules/duplicated-statement-into-arms.md). This body is NOT submittable yet (floor 56),
 * and when it is, that family's FAKE annotation + lever-exhaustion prerequisites apply.
 *
 * KNOWN RESIDUAL AT 61 (measured this session, tmp/grind/func_80070C70/s4/):
 *   (a) We emit 190 insns, target 194. The missing pair is the target's body read of D_800A3558:
 *       target loads `lhu $a2, %gp_rel(D_800A3558)` in the loop TAIL block and sign-extends it in
 *       the next iteration's body with `sll $v0,$a2,16 / sra $v0,$v0,16`; we emit a single `lh`
 *       in the body block, which combine folds. Re-measured on THIS chassis: u16 / s16 / uncast
 *       declarations of D_800A3558 are all still byte-neutral (s3's K4 holds at floor 61).
 *   (b) The callee-saved seat rotation is still not the target's (target: arg0=$s1, var_s0=$s0,
 *       var_s3=$s3, ctx=$s2, c60=$s4).
 *
 * CAVEATS ON THE DECLARATIONS (2) and (3) are UNCHANGED from s3 and still block submission:
 * RecC70 is a codegen-motivated spelling of a proven 2-byte stride, and IconC70's tail
 * `s16 sp50[12]` is a placeholder. Both must be recovered from the other consumers.
 *
 * All cheat-asm from the pre-migration chassis is still GONE. Nothing here is a register pin,
 * an asm block, a volatile coercion, or a dead local.
 */void func_80070C70(s32 arg0) {
    s32 c60 = 0x60;
    PrimC70 prim;
    IconC70 icon;
    s32 ctx_or_var_s2;
    s32 var_s0;
    s32 var_s3;

    prim.zero10 = 0;
    prim.mode = 0;
    prim.zero1C = 0;
    prim.width = 0x100;
    prim.height = 0x100;
    prim.byte28 = 0;
    ctx_or_var_s2 = (s32)*(s32 **)(D_800A35A8 + 0x64);
    prim.zero1C = 0;
    prim.mode = 0;
    prim.p_geom = *(s32 *)(ctx_or_var_s2 + 4);
    var_s0 = 0;
    prim.p_static = prim.p_geom + 0xC;
    prim.code = 1;
    prim.link = *(s32 *)(arg0 + 0x10);
    *(s32 *)(arg0 + 0x10) = func_8007352C((s32 *)&prim);
    SetDrawMode(*(s32 *)(arg0 + 0x18), 1, 0, func_8006E480(prim.p_geom, c60), 0);
    AddPrim(D_800A374C + 4, *(s32 *)(arg0 + 0x18));
    *(s32 *)(arg0 + 0x18) = *(s32 *)(arg0 + 0x18) + 0xC;
    icon.sp4C = 0xE7;
    icon.sp48 = 0xCC;
    icon.sp4A = 0x25;
    icon.sp4E = 1;
    func_80069898(arg0, (s32 *)&icon, 1);
    prim.p_geom = *(s32 *)(ctx_or_var_s2);
    var_s3 = 0xA;
    prim.p_static = prim.p_geom + 0x48;
    do {
        prim.mode = var_s0 << 6;
        prim.code = var_s3;
        prim.link = *(s32 *)(arg0 + 0x10);
        *(s32 *)(arg0 + 0x10) = func_8007352C((s32 *)&prim);
        var_s0 += 1;
        prim.p_geom += 0xC;
    } while (var_s0 < 6);
    prim.p_geom = *(s32 *)(ctx_or_var_s2);
    SetDrawMode(*(s32 *)(arg0 + 0x18), 1, 0, func_8006E480(prim.p_geom, c60), 0);
    AddPrim(D_800A374C + 0x28, *(s32 *)(arg0 + 0x18));
    var_s0 = 0;
    *(s32 *)(arg0 + 0x18) = *(s32 *)(arg0 + 0x18) + 0xC;
    prim.p_geom = *(s32 *)(ctx_or_var_s2 + 8);
    if ((s32)(D_800A35B0 + ((s16)D_800A3558 + 1)) > 0) {
        var_s3 = 0x50;
        ctx_or_var_s2 = 0;
        do {
            u8 code = D_800A3560[ctx_or_var_s2];
            if ((code != 5) && (code != 16)) {
                s32 t = prim.p_geom + 0xC;
                prim.p_static = t;
                prim.p_static = t + (D_800A3590[var_s0].v << 4);
                if (((D_800A35B0 + (s16)(u16)D_800A3558) != 0) || (D_800A35BC == 2)) {
                    prim.mode = var_s3;
                    var_s0 += 1;
                } else {
                    prim.mode = 0x105;
                    var_s0 += 1;
                }
                prim.code = 1;
                prim.link = *(s32 *)(arg0 + 0x10);
                *(s32 *)(arg0 + 0x10) = func_8007352C((s32 *)&prim);
            } else {
                var_s0 += 1;
            }
            var_s3 += 0x16C;
            ctx_or_var_s2 += 3;
        } while (var_s0 < (s32)(D_800A35B0 + ((s16)D_800A3558 + 1)));
    }
    SetDrawMode(*(s32 *)(arg0 + 0x18), 1, 0, func_8006E480(prim.p_geom, c60), 0);
    AddPrim(D_800A374C + 4, *(s32 *)(arg0 + 0x18));
    *(s32 *)(arg0 + 0x18) = *(s32 *)(arg0 + 0x18) + 0xC;
    func_80070F78(arg0, (s32 *)&prim);
    func_8006ECF4(arg0);
    func_80072E10(arg0);
    func_80073200(arg0);
}*/
void func_80070C70(s32 arg0) {
    s32 c60 = 0x60;
    PrimC70 prim;
    IconC70 icon;
    s32 ctx_or_var_s2;
    s32 var_s0;
    s32 new_var;
    s32 var_s3;

    prim.zero10 = 0;
    prim.mode = 0;
    prim.zero1C = 0;
    prim.width = 0x100;
    prim.height = 0x100;
    prim.byte28 = 0;
    ctx_or_var_s2 = (s32)*(s32 **)(D_800A35A8 + 0x64);
    prim.zero1C = 0;
    prim.mode = 0;
    prim.p_geom = *(s32 *)(ctx_or_var_s2 + 4);
    var_s0 = 0;
    prim.p_static = prim.p_geom + 0xC;
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
    prim.p_geom = *(s32 *)(ctx_or_var_s2);
    var_s3 = 0xA;
    prim.p_static = prim.p_geom + 0x48;
    do {
        prim.mode = var_s0 << 6;
        prim.code = var_s3;
        prim.link = *(s32 *)(arg0 + 0x10);
        *(s32 *)(arg0 + 0x10) = func_8007352C((s32 *)&prim);
        var_s0 += 1;
        prim.p_geom += 0xC;
    } while (var_s0 < 6);
    prim.p_geom = *(s32 *)(ctx_or_var_s2);
    SetDrawMode(*(s32 *)(arg0 + 0x18), 1, 0, func_8006E480(prim.p_geom, c60), 0);
    AddPrim(D_800A374C + 0x28, *(s32 *)(arg0 + 0x18));
    var_s0 = 0;
    new_var = *(s32 *)(ctx_or_var_s2 + 8);
    *(s32 *)(arg0 + 0x18) = *(s32 *)(arg0 + 0x18) + 0xC;
    prim.p_geom = new_var;
    if ((s32)(D_800A35B0 + ((s16)D_800A3558 + 1)) > 0) {
        var_s3 = 0x50;
        ctx_or_var_s2 = 0;
        do {
            u8 code = D_800A3560[ctx_or_var_s2];
            if ((code != 5) && (code != 16)) {
                s32 t = prim.p_geom + 0xC;
                prim.p_static = t;
                prim.p_static = t + (D_800A3590[var_s0].v << 4);
                if (((D_800A35B0 + (s16)(u16)D_800A3558) != 0) || (D_800A35BC == 2)) {
                    prim.mode = var_s3;
                    var_s0 += 1;
                } else {
                    prim.mode = 0x105;
                    var_s0 += 1;
                }
                prim.code = 1;
                prim.link = *(s32 *)(arg0 + 0x10);
                *(s32 *)(arg0 + 0x10) = func_8007352C((s32 *)&prim);
            } else {
                var_s0 += 1;
            }
            var_s3 += 0x16C;
            ctx_or_var_s2 += 3;
        } while (var_s0 < (s32)(D_800A35B0 + ((s16)D_800A3558 + 1)));
    }
    SetDrawMode(*(s32 *)(arg0 + 0x18), 1, 0, func_8006E480(prim.p_geom, c60), 0);
    AddPrim(D_800A374C + 4, *(s32 *)(arg0 + 0x18));
    *(s32 *)(arg0 + 0x18) = *(s32 *)(arg0 + 0x18) + 0xC;
    func_80070F78(arg0, (s32 *)&prim);
    func_8006ECF4(arg0);
    func_80072E10(arg0);
    func_80073200(arg0);
}
