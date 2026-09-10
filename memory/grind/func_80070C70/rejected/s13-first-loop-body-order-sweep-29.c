/* [s13] First-loop body statement order sweep: prim.link before prim.mode (BAC) is 29/193 - the func_8007352C call collapses one insn. All six A/B/C orders measured: ABC=22 (optimum, the candidate), ACB=24, CAB=24, BAC=29, BCA=30, CBA=30; the two trailing increments (var_s0+=1 / prim.p_geom+=0xC) are byte-neutral in both orders. Statement-order axis of the first loop body is EXHAUSTIVELY swept and dead. */
/* candidate.c - func_80070C70 - session 12 (rederive). Honest floor 22 (unchanged from s11),
 * but with ONE FEWER constant-holder local than the s11 body: `s32 var_s3 = 0xA;` is GONE,
 * replaced by the literal `prim.code = 0xA;` at byte-identical cost (s12 measured 22/194 both
 * ways).  That removes a construct that would have needed a named-local FAKE vet at submission.
 *
 * FLOOR HISTORY: 194 -> 101 (s1) -> 99 (s3) -> 56 (s4) -> 53 (s5) -> 39 (s6) -> 22 (s11) -> 22 (s12).
 * Chassis: if-guarded do/while, 194 insns == target, frame exact
 * (.frame $sp,128 # vars= 80, regs= 6/0, args= 24).
 *
 * WHAT S12 SETTLED (all measured, see hypotheses.md):
 *  1. THE CHASSIS QUESTION IS CLOSED WITH A NUMBER.  s11's frontier asked whether L2/L3/L4
 *     port to the shape-exact TOP-TEST for chassis and beat 22.  They port (49 -> 42 with L3,
 *     -> 36 with L2, -> 31 with L4) but 31 > 22: the top-test chassis' 24-byte orphan frame
 *     penalty is still NOT affordable.  Bodies tmp/grind/func_80070C70/s12/a/a0..a6.c.
 *  2. EVERY frame-buying lever re-measured ON THAT IMPROVED (31) top-test chassis is still
 *     net-negative: bound-in-a-local 51/187, bare-u16 bound 34, `!=` bound 62.  (s10 measured
 *     these on the 49 body; the CURRENT-SCOPE rule required re-measuring them on the new one.)
 *  3. THE CARRIED-LOCALS FAMILY, re-measured on the 22 chassis (s11 measured it on 39 and got
 *     55-59): six spellings of "h = D_800A3558; nb = D_800A35B0; ... reload at the bottom" all
 *     score 42 at 195 insns, byte-identical to each other.  Carrying ONLY D_800A35B0 in a local
 *     (`nb`) is the best of the family at 27/194 - still 5 worse than plain re-reads.
 *  4. `var_s0 = 0;` hoisted ABOVE the `if` guard (the target emits `move s0,zero` before the
 *     guard's loads) is 54/193.  Dead.
 *  5. A pointer local `s32 *pp = (s32 *)&prim;` used at the three call sites is 38/197 at both
 *     tested declaration positions.  Dead.
 *
 * IconC70 CORRECTNESS - THE PLACEHOLDER IS NOW DISPROVEN, DO NOT SHIP IT.  s12 read the callee:
 * func_80069898 dereferences its $a1 argument at EXACTLY four offsets - lhu 0x0/0x2/0x4/0x6 -
 * and feeds them to a TILE prim (sh to +0x8/0xA/0xC/0xE, three times).  And the sibling caller
 * func_8006B120 has `.frame` 0x68 with $s0 saved at 0x50 and args 0x18, i.e. vars = 0x38 = 56 =
 * prim(48 at 0x18) + icon(8 at 0x48) EXACTLY.  So the icon record is EIGHT bytes (four 16-bit
 * fields), not 0x20.  The 24 bytes at sp+0x50..0x67 in func_80070C70 belong to a DIFFERENT,
 * still-unidentified local - and it must be a REFERENCED one: s12 measured that an unreferenced
 * `s32 sp50[6]` is dropped entirely by GCC 2.7.2 here (frame falls 128 -> 104, score 36).
 * The `s16 sp50[12];` member this body still relies on is therefore a KNOWN-WRONG placeholder
 * that happens to reproduce the frame; identifying the real 24-byte local is now the #1
 * correctness prerequisite for any submission.  (func_800720FC has the same shape with 32
 * spare bytes: frame 0x98, $s0 at 0x70, icon at 0x48 - so the extra local is a TU-wide idiom.)
 *
 * RESIDUAL AT 22 (objdump diff, tmp/grind/func_80070C70/s12/{ours,tgt}.txt): four hunks, and
 * three of them are the same cluster - the target keeps D_800A3558 in $a2 (as a raw `lhu`) and
 * D_800A35B0 in $a1 across the second loop's back edge, loading both in the guard block and
 * again in the tail, while we re-read both inside the body.  That is cse.c:7909
 * cse_set_around_loop, gated on REG_LOOP_TEST_P (cse.c:7933), which only jump.c:2253
 * duplicate_loop_exit_test sets - and that needs a top-test loop.  The fourth hunk is the
 * prologue tie (`addiu $a0,$sp,24` vs `addu $s0,$zero,$zero`), measured dead six ways in s11.
 *
 * CONSTRUCTS STILL NEEDING A FAMILY VET BEFORE ANY SUBMISSION:
 *   - `s32 c60 = 0x60;` - a constant-holder local, and it is LOAD-BEARING: the literal spelling
 *     is 29/191 (s12 f1.c).  Named-local FAKE family, .claude/rules/named-local-fake-exception.md.
 *   - `s32 g;` / `s32 t;` written+read at three sites each (every write a real consumed value).
 *   - `s32 ctx = var_s0 * 3;` - the byte-offset giv.
 *   - `IconC70.sp50[12]` - see above, now known wrong.
 *
 * COMPANION EDITS in src/text1b.c that are part of the measured 22 (unchanged):
 *   extern u8 D_800A3560[];  extern s16 D_800A3590[];  IconC70 gains `s16 sp50[12];`
 */
void func_80070C70(s32 arg0) {
    s32 c60 = 0x60;
    PrimC70 prim;
    IconC70 icon;
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
        prim.link = *(s32 *)(arg0 + 0x10);
        prim.mode = var_s0 << 6;
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
