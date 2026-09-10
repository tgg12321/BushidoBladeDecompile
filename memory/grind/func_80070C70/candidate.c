//@sub     s16 sp50[12];
|||
//@sub extern s32 func_80069898(s32 a0, s32 *p, s32 mode);|||
/* candidate.c - func_80070C70 - session 13 (structural). Honest floor 22 (unchanged from
 * s11/s12), but the body is BYTE-IDENTICAL to the s12 candidate while replacing its single
 * biggest correctness defect: the invented `IconC70` struct with the known-wrong `s16 sp50[12]`
 * placeholder tail is GONE, replaced by `u16 rect[16];` - the idiom this TU already uses.
 *
 * FLOOR HISTORY: 194 -> 101 (s1) -> 99 (s3) -> 56 (s4) -> 53 (s5) -> 39 (s6) -> 22 (s11)
 * -> 22 (s12) -> 22 (s13).  Chassis unchanged: if-guarded do/while, 194 insns == target,
 * frame exact (.frame $sp,128 # vars= 80, regs= 6/0, args= 24).
 *
 * WHY rect[16] (s13's main result - this closes s12's frontier item #1):
 *   func_80069898 is ALREADY DECOMPILED in this same TU (src/text1b.c:5413) and its prototype
 *   is `void func_80069898(GameObj *arg0, u16 *arg1, s32 arg2)` - arg1 is a u16 POINTER, and
 *   the body reads arg1[0..3].  Both existing callers on main pass a plain local ARRAY, not a
 *   struct: func_8006BB68 (src/text1b.c:5822) declares `u16 rect[4];` and writes it in the
 *   order rect[2], rect[0], rect[1], rect[3] - the EXACT store order func_80070C70's target
 *   asm uses at sp+0x4C, +0x48, +0x4A, +0x4E - and func_8006DD94 (src/text1b.c:6065) does the
 *   same.  So the object at sp+0x48 in func_80070C70 is a `u16 rect[]`, and the 24 "mystery"
 *   bytes at sp+0x50..0x67 are simply its UNWRITTEN TAIL: rect[16] spans 0x48..0x67 exactly.
 *   That is the same OVERSIZED-LOCALS shape already accepted on main for the two exemplars
 *   (src/text1b.c:6032 func_8006DD94 `EnvB s` + `u16 rect[4]`; src/text1a_post.c:404
 *   func_80041BF4 `s16 rect[8]`), and it needs no invented type and no dead local: the array's
 *   ADDRESS is passed to func_80069898, so it is the LIVE object being extended (prong 2).
 *   MEASURED s13: rect[16] = 22/194 and BYTE-IDENTICAL (194/194 opcode-column diff empty) to
 *   the s12 IconC70+sp50[12] body; `s16 rect[16]` is also 22; `u16 rect[15]` is also 22 (the
 *   declared length is a RANGE [15,16], exactly as the func_8006DD94 record documents, because
 *   stmt.c:3419 8-aligns a BLKmode automatic); `u16 rect[4]` is 36 - so the 24 bytes are
 *   load-bearing and s12's measurement that an UNREFERENCED `s32 sp50[6]` is dropped is
 *   consistent: this tail is referenced because the whole array is address-taken.
 *   The sibling func_800720FC has the same idiom with 32 spare bytes (rect[20]).
 *
 * COMPANION EDITS in src/text1b.c that are part of the measured 22:
 *   extern u8 D_800A3560[];   extern s16 D_800A3590[];
 *   the `typedef struct IconC70` block is NO LONGER USED (delete it at submission), and the
 *   local re-declaration `extern s32 func_80069898(s32 a0, s32 *p, s32 mode);` must be DROPPED
 *   - the real prototype at line 5413 is in scope and is the one that types `rect`.
 *   (Both are expressed as the //@sub directives at the top of this file, which is why they
 *   must stay on lines 1-2: tmp/grind/func_80070C70/s10/install.py only consumes //@sub at the
 *   very start of the file - see the s10 hygiene note.)
 *
 * CONSTRUCTS STILL NEEDING A FAMILY VET BEFORE ANY SUBMISSION:
 *   - `s32 c60 = 0x60;` - constant-holder local, LOAD-BEARING (literal spelling is 29/191,
 *     s12 f1.c).  Family: .claude/rules/named-local-fake-exception.md, FAKE annotation needed.
 *   - `u16 rect[16];` - oversized LIVE address-taken array; family
 *     .claude/rules/dead-vars-local-array.md OVERSIZED-LOCALS carve-out, in-TU precedent
 *     src/text1b.c:6032 and src/text1a_post.c:404.  FAKE annotation needed.
 *   - `s32 g;` / `s32 t;` (real consumed values at three sites each) and
 *     `s32 ctx = var_s0 * 3;` (the byte-offset giv) are ordinary C.
 *
 * RESIDUAL AT 22 - unchanged and re-confirmed by s13's objdump diff
 * (tmp/grind/func_80070C70/s13/{ours,tgt}.txt): 2 points are the prologue tie
 * (`addiu $a0,$sp,24` scheduled before vs after `move $s0,$zero`) and the other ~20 are the
 * single cse_set_around_loop cluster - the target keeps D_800A3558 in $a2 as a raw `lhu` and
 * D_800A35B0 in $a1 across the second loop's back edge (reloading BOTH in the tail after the
 * call, since $a1/$a2 do not survive `jal`), loading them in the guard block too, while we
 * re-read both inside the body.  cse.c:7933 gates that on REG_LOOP_TEST_P, set only by
 * jump.c:2253 duplicate_loop_exit_test, which needs a top-test loop; the top-test chassis costs
 * 24 frame bytes of combine-orphaned spill pseudos and prices out at 31 (s12).
 *
 * S13 SWEPT AND KILLED (all on this chassis, see hypotheses.md):
 *   - first-loop body statement order: all 6 orders x 2 increment orders; ABC (this one) is the
 *     unique optimum at 22, the other five orders are 24-30, and the two increments are
 *     byte-neutral.
 *   - pre-guard statement position of `prim.p_geom = *(s32 *)(ctx + 8)`: all 3 legal positions;
 *     the current one is optimal (others 25, 26).
 *   - declaration order of the four body scalars (var_s0, t, code, g): ALL 24 permutations are
 *     byte-identical at 22/194.  The scalar declaration axis is completely inert here.
 *   - defeating LICM on the bound by reusing an existing body-written local
 *     (.claude/rules/defeat-licm-hoist-var-reuse.md) is 61/69 on the top-test chassis and
 *     byte-neutral/1-worse on this one.
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
