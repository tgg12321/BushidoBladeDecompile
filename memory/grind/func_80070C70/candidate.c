/* candidate.c - func_80070C70 - session 3 (permuter). Honest floor 99 (was 101 at s2).
 *
 * FLOOR HISTORY: 194 (HEAD, no C body) -> 108 (de-cheated + array decls, s1) -> 101 (IconC70
 * sized 0x20, s1) -> 99 (s3: D_800A3590 declared as an array of a 2-byte RECORD instead of a
 * bare s16 array, which routes the address through expand_expr's COMPONENT_REF/get_inner_reference
 * `offset` path and kills the loop.c LICM hoist of &D_800A3590 that s1/s2 identified).
 * At 99 our build emits 194 insns against target's 194 - the insn COUNT now matches exactly.
 *
 * REQUIRES these companion edits in src/text1b.c - they are part of the measured 99:
 *   1. extern u8  D_800A3560[];   (was: extern u8  D_800A3560;)   both occurrences: l.2124, l.6499
 *   2. typedef struct RecC70 { s16 v; } RecC70;
 *      extern RecC70 D_800A3590[];   (was: extern s16 D_800A3590;)  both occurrences: l.2129, l.6500
 *   3. typedef struct IconC70 { s16 sp48; s16 sp4A; s16 sp4C; s16 sp4E; s16 sp50[12]; } IconC70;
 *      i.e. total size 0x20, not 8.
 *
 * WHY (2) IS THE WHOLE s3 MOVE - the mechanism, read out of the GCC 2.7.2 sources, not guessed:
 *   - `D_800A3590[var_s0]` on a bare `extern s16 D_800A3590[]` goes through expand_expr's
 *     ARRAY_REF case (expr.c:4589), whose nonconstant-index branch rewrites the reference to
 *     `*(&array + index*size)`. The resulting address rtx is `(plus (mult (reg) 2) (symbol_ref))`.
 *     MIPS GO_IF_LEGITIMATE_ADDRESS (config/mips/mips.h) only accepts a PLUS whose operands are a
 *     REG plus a constant/symbolic term - it swaps operands only when `code0 != REG && code1 ==
 *     REG` - so a MULT operand is rejected outright. memory_address (explow.c:385) then calls
 *     break_out_memory_refs, which force_reg's the SYMBOL_REF into a pseudo, and force_operand
 *     emits the scale AFTER it. That is s2's insn 312 / 315 / 317 triple, and the symbol pseudo's
 *     2-luid lifetime is exactly what let loop.c:1631 hoist it.
 *   - `D_800A3590[var_s0].v` on an array of a 2-byte record is a COMPONENT_REF, so expand_expr
 *     takes the get_inner_reference path instead: the byte offset is expanded FIRST as an ordinary
 *     insn (`expand_expr (offset, NULL_RTX, VOIDmode, 0)`) and then force_reg'd, giving the address
 *     `(plus (symbol_ref) (reg))`. That form IS legitimate on MIPS (the macro's swap branch fires),
 *     so no symbol pseudo survives, there is no loop-invariant movable to hoist, and the read comes
 *     out as target's fused `lui %hi / addu / lh %lo` at 80070E5C-80070E64.
 *   - Confirmed in tmp/grind/func_80070C70/dumps/text1b.loop: the "Insn 312: regno 126 (life 2)
 *     ... moved to 486" line is GONE, and the object now carries exactly one HI16/LO16 relocation
 *     pair for D_800A3590 instead of a preheader `la`.
 *
 * CAVEAT ON (2) - THIS IS THE OPEN QUESTION FOR THE NEXT SESSION AND IT BLOCKS SUBMISSION.
 * The 2-byte record SIZE is proven by the target's stride, but `struct RecC70 { s16 v; }` is a
 * codegen-motivated spelling of that size, not recovered evidence: a reviewer will fairly ask why
 * a one-member struct exists. `extern s16 D_800A3590[][1];` measures identically (99) and is
 * equally contrived. The honest fix is to recover the REAL record type from D_800A3590's other
 * consumers - func_8006F100, func_80070188, func_80070F78 (all still INCLUDE_ASM) and
 * src/text1b_b.c:271 - and, per the brief's DATA MODEL block, keep the census row as
 * `alias of g_replay_motion_shared_state_d+6`. If those consumers show a wider record with the
 * halfword at offset 0, the declaration becomes evidence-backed and the construct question closes.
 *
 * CAVEAT ON (3) IS UNCHANGED from s1/s2: the 0x20 SIZE is proven by frame arithmetic, but the
 * trailing `s16 sp50[12]` is a PLACEHOLDER whose field list must be recovered from func_80069898's
 * other callers (func_8006B120, func_8006CFBC, func_800720FC, func_80074488, func_8006F97C)
 * before this body is submitted.
 *
 * KNOWN RESIDUAL AT 99 (measured this session, see hypotheses.md F1'):
 *   The LICM hoist is dead, but loop.c strength_reduce still reduces the D_800A3590 address giv.
 *   text1b.loop now reads: `Insn 313: giv reg 127 src reg 75 benefit 2 ... mult 2 add 0`,
 *   `Insn 319: dest address src reg 75 benefit 4 ... mult 2 add (symbol_ref "D_800A3590")`,
 *   `giv at 319 combined with giv at 313`, `giv at 313 reduced to (reg:SI 159)`. That reduced giv
 *   is a 4th induction register (`move s3,zero` / `addiu s3,s3,2`) which target does not spend -
 *   target recomputes `sll $a0, $s0, 1` in-loop - and it is the whole 0x88-vs-0x80 frame delta.
 *   By contrast the scale-1 D_800A3560 address giv at insn 291 IS rejected
 *   ("giv of insn 291 not worth while, 0 vs 48"), exactly like target.
 *   The gate is loop.c:3824 `v->lifetime * threshold * benefit < insn_count` with
 *   threshold = (loop_has_call ? 1 : 2) * (3 + n_non_fixed_regs) at loop.c:3241.
 *
 * All cheat-asm from the pre-migration chassis is still GONE. Nothing here is a FAKE construct.
 */
void func_80070C70(s32 arg0) {
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
                } else {
                    prim.mode = 0x105;
                }
                prim.code = 1;
                prim.link = *(s32 *)(arg0 + 0x10);
                *(s32 *)(arg0 + 0x10) = func_8007352C((s32 *)&prim);
            }
            var_s3 += 0x16C;
            var_s0 += 1;
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