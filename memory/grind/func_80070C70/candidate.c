/* candidate.c - func_80070C70 - session 11 (rederive). Honest floor 22 (was 39 at s6-s10).
 *
 * FLOOR HISTORY: 194 -> 101 (s1) -> 99 (s3) -> 56 (s4) -> 53 (s5) -> 39 (s6) -> 22 (s11).
 * Instruction count is 194 == the target's 194 and the frame is still exact
 * (.frame $sp,128 # vars= 80, regs= 6/0, args= 24).  Chassis is UNCHANGED - still the
 * if-guarded do/while.  s11 did NOT change the chassis; it changed four ORDINARY-C
 * spellings inside it, each measured independently, each worth points:
 *
 *   L1  TAIL BOUND PARENTHESISATION  39 -> 38.  The second loop's tail test is now
 *       `var_s0 < D_800A35B0 + ((s16)D_800A3558 + 1)`.  The target adds 1 to the
 *       SIGN-EXTENDED halfword and then adds D_800A35B0 (`lh v0; addiu v0,v0,1;
 *       addu v0,a1,v0`); the unparenthesised form `D_800A35B0 + (s16)D_800A3558 + 1`
 *       parses as `(D_800A35B0 + sext) + 1` and emits `addu` then `addiu` on the other
 *       operand.  NOTE: the GUARD copy of the bound must stay UNparenthesised - moving
 *       the parens there as well is 41, and doing both is 41 at 196 insns.
 *   L2  A NAMED GEOM/STATIC TEMP PAIR AT THE TWO PRE-LOOP SITES  38 -> 22 (with L3).
 *       `g = *(s32 *)(ctx + N); t = g + K; prim.p_geom = g; prim.p_static = t;`
 *       The target computes the +0xC / +0x48 into a SECOND live register before either
 *       store (`addiu v1,v0,12` / `sw v0,24(sp)` / `sw v1,28(sp)`).  Reading the value
 *       back out of the struct member (`prim.p_static = prim.p_geom + 0xC;`) lets the
 *       add be scheduled after the store, so the pseudo dies at the add and gets the
 *       SAME hard register.  Ordering the ADD as its own statement before both stores
 *       is what keeps two pseudos live across it.  Both sites together are worth 7
 *       points (25 -> 22 for the second site alone).  s8 measured the REVERSED store
 *       order (`prim.p_static = g + 0xC; prim.p_geom = g;`) and it is still worse: 40.
 *   L3  `link` READ BEFORE `code` STORE AT BOTH LOOP CALL SITES  36 -> 31 -> 29.
 *       In both the first and the second loop the target emits `lw v1,0x10(s1)` before
 *       `sw v0,0x2C(sp)`, i.e. the source reads prim.link's new value before storing
 *       prim.code.  Swapping those two statements in the second loop is worth 5 points
 *       and in the first loop another 2.
 *   L4  `g = prim.p_geom; t = g + 0xC;` INSIDE THE SECOND LOOP BODY  38 -> 36.
 *       Same two-live-register effect as L2 at the in-loop site, where the target has
 *       `lw v0,24(sp) / addiu v1,v0,12 / ... / addu v0,v1,v0`.
 *
 * RESIDUAL AT 22 - now essentially ONE cluster plus one scheduler tie:
 *   (i) THE LOOP-CARRIED $a1/$a2 (about 18 of the 22).  The target loads
 *       `lhu $a2, D_800A3558` and `lw $a1, D_800A35B0` in the second loop's GUARD block
 *       and again in its TAIL, and the loop BODY consumes the registers
 *       (`sll $v0,$a2,16 / sra $v0,$v0,16 / addu $v0,$a1,$v0`) instead of re-loading.
 *       We re-load both inside the body every iteration.  This is cse_set_around_loop
 *       and it is the unchanged frontier from s7-s10.  s11 re-measured the direct C
 *       spelling of it (carry the two values in locals, re-assign them at the bottom of
 *       the body): 55-59 at 197 insns on THIS chassis - see rejected/.
 *  (ii) `addiu $a0,$sp,24` and `addu $s0,$zero,$zero` are swapped in the prologue.
 *       s11 measured five source positions for `var_s0 = 0;` (before `g =`, after `g =`,
 *       after `t =`, after `prim.p_geom =`, after `prim.p_static =`, and immediately
 *       before the do-loop): ALL SIX score 22 with identical bytes.  Statement order does
 *       not reach this tie.
 * (iii) The `||` operand order.  The target tests the SUM first
 *       (`bnez` on the sum, then D_800A35BC), we test D_800A35BC first.  Spelling it in
 *       the target's order costs 8 points here (22 -> 30 at 190 insns) because it also
 *       removes four insns; do not "fix" it by inspection.
 *
 * THE CONSTRUCTS IN THIS BODY THAT STILL NEED A FAMILY VET BEFORE ANY SUBMISSION:
 *   - `s32 g;` and `s32 t;` are each written and read at three sites (variable reuse).
 *     Every write is a real consumed value (the geometry pointer / the static pointer),
 *     so this reads as ordinary C, but see .claude/rules/defeat-licm-hoist-var-reuse.md
 *     and the named-intermediate 6 prongs before claiming a family.
 *   - `s32 ctx = var_s0 * 3;` (unchanged from s6) - the byte-offset giv.
 *   - `IconC70.sp50[12]` is still a PLACEHOLDER member list for the trailing 24 bytes of
 *     the 0x20-byte icon record; the size is proven, the members are not.
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
    s32 var_s3;
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
