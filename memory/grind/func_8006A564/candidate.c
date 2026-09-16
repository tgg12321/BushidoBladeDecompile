/* func_8006A564 -- src/text1b.c
 * Session 3 (structural). sandbox --disable all score = 60 (target_insns 199,
 * build_insns 199 -- EQUAL). Pure C, zero cheat constructs, zero FAKE
 * annotations.
 *
 * Structure: draws 3 TILE primitives (SetTile/SetSemiTrans/AddPrim) whose
 * color bytes are chosen by (D_800A34F8 & 0xF == arg2), then loads a second
 * object's field_1C->field_28 into arg1's packet, halves/sets its color
 * bytes under the same condition, advances two fields, calls
 * func_8007352C(arg1) twice (feeding two different source structs' data
 * through the same packet), and finishes with a SetDrawMode/AddPrim of
 * *(arg0+0x1C), advancing that field by 0xC.
 *
 * SESSION 2 FIXES (137 -> 68): see memory/grind/func_8006A564/hypotheses.md
 * H2/H3 and evidence.md Session 2 for the block-local var split, delay-slot
 * reorder, and two real transcription-bug fixes that brought build_insns to
 * exact parity with target (199 == 199).
 *
 * SESSION 3 FIX (68 -> 60): at all 4 `D_800A34F8 & 0xF` sites, REMOVED the
 * separately-named `v0 = D_800A34F8 & 0xF; if (v0 == arg2)` two-statement
 * form and replaced it with the mask expression written DIRECTLY in the if
 * condition: `if ((D_800A34F8 & 0xF) == arg2) { ... }`. `v0` (used only
 * inside/after the branch arms, for the tile-draw values) is now declared
 * but never assigned by the mask compute at all -- the mask's AND result is
 * an anonymous compiler temp consumed only by the comparison, never named.
 *
 * Measured via `& tools/wteng.ps1 main sandbox func_8006A564 --disable all`:
 * applying this at site 1 alone: 68 -> 66. All 4 sites: 68 -> 60.
 * Confirmed via a register-normalized objdump diff
 * (tmp/grind/func_8006A564/s3/diff.py against tmp/grind/func_8006A564/s3/build.dis.txt)
 * that the SYSTEMIC v0<->v1 register-coloring swap identified in session 2
 * (target's `lw $v0,SYM; andi $v0,$v0,0xF` in-place AND vs our build's
 * `lw v0,SYM; andi v1,v0,0xf` result-in-different-register) is STILL PRESENT
 * at all 4 sites after this change -- the greg dump
 * (tmp/grind/func_8006A564/dumps/text1b.greg, function func_8006A564 section)
 * shows the SAME pseudo->hardreg disposition as session 2 (the raw-load
 * pseudo still gets hardreg 2/v0 via local-alloc; the AND-result pseudo,
 * which crosses the if/else merge, still gets hardreg 3/v1 via global-alloc).
 * So the 8-point improvement is NOT from fixing the coloring swap -- it comes
 * from some OTHER, not-yet-isolated diff (likely a secondary effect of
 * removing the redundant one-statement-then-discard v0 write before the
 * branch; not fully attributed this session -- see frontier).
 *
 * KILLED THIS SESSION:
 * 1. Reversing the comparison's operand order (`arg2 == (D_800A34F8 & 0xF)`
 *    instead of `(D_800A34F8 & 0xF) == arg2`) at all 4 sites: MEASURED WORSE
 *    (60 -> 64). Reverted. See hypotheses.md.
 * 2. Splitting the AND into a genuinely FRESH, separately-named local
 *    (`s32 raw; raw = D_800A34F8; v0 = raw & 0xF;`) instead of reusing `v0`
 *    for both the mask compute and the arm values (distinct from session 2's
 *    H3, which reused THE SAME variable for both statements): MEASURED NO
 *    CHANGE (68 -> 68, byte-identical RTL to the pre-split form -- combine
 *    folds the extra named copy back to the exact same 2-insn shape either
 *    way). See hypotheses.md H4.
 *
 * NOT yet attempted / frontier for next session: the v0/v1 coloring swap is
 * STILL the entire visible residual pattern in the objdump diff (see
 * tmp/grind/func_8006A564/s3/diff.py output) -- confirmed unchanged by this
 * session's fix, which must be closing some other diff instead. Next session
 * should (a) isolate exactly WHICH instructions closed between the 68 and 60
 * scores (diff the build.dis.txt from session 2's chassis against this
 * session's, not just each against target) to attribute the 8-point win
 * precisely, and (b) continue attacking the coloring swap itself -- GCC 2.7.2
 * has no MIPS REG_ALLOC_ORDER override (confirmed: no such macro in
 * tools/gcc-2.7.2/config/mips/mips.h), so hard-reg search order is the
 * default ascending numeric order; local-alloc grabs the short-lived raw-load
 * pseudo for v0 (the first free int-class hardreg after a0-a3 are freed)
 * before global-alloc ever sees the cross-block AND-result pseudo, which is
 * then forced to v1. To flip this, target's compile must NOT be treating the
 * raw-load temp as a purely local (single-block) pseudo the way ours does --
 * this needs either (i) a `.lreg` dump read (not done this session, budget)
 * to see the exact QTY classification GCC gives each pseudo in target's own
 * compile (impossible directly since we don't have target's cc1 -da output,
 * but the ORIGINAL C's structure is the unknown -- try structural variants
 * that make the raw load's pseudo ALSO cross a block, e.g. hoisting the load
 * for all 4 sites into one shared local read once outside all 4 blocks, or
 * (ii) restructuring so v0 (the arm-value pseudo) is NOT itself a single
 * variable crossing the if/else -- try genuinely separate per-arm locals
 * merged via a mechanism other than one shared block-scoped `v0`.
 */
extern s32 D_800A34F8;
extern s32 D_800A374C;
extern void SetTile(void *);
extern s32 SetSemiTrans(void *, s32);
extern s32 SetDrawMode(s32, s32, s32, s32, s32);
extern s32 AddPrim(s32, void *);
extern s32 func_8007352C(s32);
extern s32 func_8006E480(s32, s32);
void func_8006A564(u8 *arg0, u8 *arg1, s32 arg2) {
    u8 *tile;
    u8 *obj2;
    s32 s4;

    tile = *(u8 **)(arg0 + 0x18);
    SetTile(tile);
    {
        s32 v0;
        if ((D_800A34F8 & 0xF) == arg2) {
            v0 = *(u8 *)(arg1 + 0x29);
            tile[5] = 0;
            tile[4] = v0;
            v0 = *(u8 *)(arg1 + 0x2B);
            s4 = 0;
        } else {
            s4 = 0x20;
            v0 = 0x50;
            tile[4] = v0;
            tile[5] = v0;
        }
        tile[6] = v0;
        v0 = 0x5F;
        *(s16 *)(tile + 8) = v0;
        v0 = *(s32 *)(arg1 + 0x1C);
        v0 = v0 + 0xF;
        *(s16 *)(tile + 0xA) = v0;
        v0 = *(s32 *)(arg1 + 0x18);
        *(s16 *)(tile + 0xE) = 1;
        v0 = v0 + 0x19;
        *(s16 *)(tile + 0xC) = v0;
    }
    SetSemiTrans(tile, *(s32 *)(arg1 + 0x10));
    AddPrim(D_800A374C + (*(s32 *)(arg1 + 0x14) << 2), tile);
    tile += 0x10;

    SetTile(tile);
    {
        s32 v0;
        if ((D_800A34F8 & 0xF) == arg2) {
            v0 = *(u8 *)(arg1 + 0x29);
            tile[5] = 0;
            tile[4] = v0;
            v0 = *(u8 *)(arg1 + 0x2B);
            tile[6] = v0;
        } else {
            v0 = 0x20;
            tile[4] = v0;
            tile[5] = v0;
            tile[6] = v0;
        }
        v0 = *(s32 *)(arg1 + 0x18);
        *(s16 *)(tile + 8) = v0;
        v0 = *(s32 *)(arg1 + 0x1C);
        *(s16 *)(tile + 0xC) = 0x78;
        *(s16 *)(tile + 0xE) = 1;
        v0 = v0 + 0xE;
        *(s16 *)(tile + 0xA) = v0;
    }
    SetSemiTrans(tile, 1);
    AddPrim(D_800A374C + (*(s32 *)(arg1 + 0x14) << 2), tile);
    tile += 0x10;

    SetTile(tile);
    {
        s32 v0;
        if ((D_800A34F8 & 0xF) == arg2) {
            v0 = *(u8 *)(arg1 + 0x29);
            tile[5] = 0;
            v0 = (u32)v0 >> 1;
            tile[4] = v0;
            v0 = *(u8 *)(arg1 + 0x2B);
            v0 = (u32)v0 >> 1;
        } else {
            v0 = 0x10;
            tile[4] = v0;
            tile[5] = v0;
        }
        tile[6] = v0;
        v0 = *(s32 *)(arg1 + 0x18);
        v0 = v0 + 0x40;
        *(s16 *)(tile + 8) = v0;
        v0 = *(s32 *)(arg1 + 0x1C);
        *(s16 *)(tile + 0xC) = 0x38;
        *(s16 *)(tile + 0xE) = 1;
        v0 = v0 + 0xD;
        *(s16 *)(tile + 0xA) = v0;
    }
    SetSemiTrans(tile, 1);
    AddPrim(D_800A374C + (*(s32 *)(arg1 + 0x14) << 2), tile);

    obj2 = *(u8 **)(arg0 + 4);
    tile = tile + 0x10;
    *(u8 **)(arg0 + 0x18) = tile;
    tile = *(u8 **)(obj2 + 0x1C);
    {
        s32 v0;
        v0 = *(s32 *)(tile + 0x28);
        *(s32 *)(arg1 + 0) = v0;

        if ((D_800A34F8 & 0xF) == arg2) {
            s32 v1;
            v0 = *(u8 *)(arg1 + 0x29);
            v1 = *(u8 *)(arg1 + 0x2B);
            *(u8 *)(arg1 + 0x2A) = 0;
            v0 = (u32)v0 >> 1;
            v1 = (u32)v1 >> 1;
            *(u8 *)(arg1 + 0x29) = v0;
            *(u8 *)(arg1 + 0x2B) = v1;
        } else {
            v0 = 0x28;
            *(u8 *)(arg1 + 0x2B) = v0;
            *(u8 *)(arg1 + 0x2A) = v0;
            *(u8 *)(arg1 + 0x29) = v0;
        }

        v0 = *(s32 *)(arg1 + 0);
        v0 = *(s32 *)(arg1 + 0x1C);
        *(s32 *)(arg1 + 0x18) = 0;
        v0 = *(s32 *)(arg1 + 0) + 0xC;
        *(s32 *)(arg1 + 0x1C) = *(s32 *)(arg1 + 0x1C) + 0xF;
        *(s32 *)(arg1 + 4) = v0;

        v0 = *(s32 *)(arg0 + 0x14);
        *(s32 *)(arg1 + 8) = v0;
        *(s32 *)(arg0 + 0x14) = func_8007352C((s32)arg1);

        v0 = *(s32 *)(tile + 0x2C);
        *(s32 *)(arg1 + 0) = v0;
        *(s32 *)(arg1 + 4) = v0 + 0xC;
        v0 = *(s32 *)(arg0 + 0x14);
        *(s32 *)(arg1 + 8) = v0;
        *(s32 *)(arg0 + 0x14) = func_8007352C((s32)arg1);
    }

    SetDrawMode(*(s32 *)(arg0 + 0x1C), 1, 0,
                func_8006E480(*(s32 *)(arg1 + 0), s4), 0);
    AddPrim(D_800A374C + (*(s32 *)(arg1 + 0x14) << 2), *(u8 **)(arg0 + 0x1C));
    *(s32 *)(arg0 + 0x1C) = *(s32 *)(arg0 + 0x1C) + 0xC;
}
