/* func_80033550 — LANDED AND BYTES-PROVEN (s18, 2026-09-03, rederive modality;
 * form authored by s17, whose session was discarded on a self_vet.md regex
 * false positive — a prose line beginning with the word "family:" was counted
 * by tools/grinder/grindlib.py's `^\s*FAMILY\s*:` block matcher as a second
 * unquoted family claim. The wording is rewrapped; the form is unchanged).
 *
 * RE-MEASURED IN FULL THIS SESSION, with the four surface edits below
 * re-applied in-tree from tmp/grind/func_80033550/s17/full_diff.patch:
 *   `sandbox func_80033550 --disable all` = score 0, target_insns 34,
 *   build_insns 34, rules_dropped 0   (tmp/grind/func_80033550/s17/sandbox_s18.json)
 *   `verify-oracle` = ok true, build_sha1 62efab4f73f992798c43e8c730aa43baa10bb4fa
 *   == oracle, build_matches true     (tmp/grind/func_80033550/s17/verify_oracle_s18.txt)
 * Full diff of all four files: tmp/grind/func_80033550/s17/full_diff.patch.
 *
 * WHAT CHANGED FROM s16 (the only delta — the s16 body is otherwise verbatim).
 * s16's form was bytes-proven but layer-1 FAILed on a SECOND, unfixed instance
 * of the same anti-pattern its LeafPos merge had just fixed: the slot-state
 * flag byte was still read and written as `*(&D_800A3918 + i)`, a per-use
 * pointer pun on a symbol declared `extern u8 D_800A3918;` (a scalar) that the
 * diff's own evidence proved is a 6-byte array. That construct is now a
 * `banned_constructs` entry. This session fixes it at the declaration, exactly
 * as the reviewer prescribed: `extern u8 D_800A3918[6];` in include/code6cac.h,
 * and an ordinary `D_800A3918[i]` at both use sites. No pun, no cast, no
 * address arithmetic in the body.
 *
 * D_800A391E is NOT folded into the array. Per named_syms.txt:1555 it is the
 * iteration end marker (`base + 6`), i.e. the one-past-the-end bound, not an
 * element; it keeps its own declaration and its own splat symbol.
 *
 * D_800A391D (named_syms.txt:824, `g_motion_select_byte`) occupies base+5 and
 * is therefore an alias of D_800A3918[5]. It is left untouched: it is a
 * distinct pre-existing splat symbol with its own consumers OUTSIDE this
 * function (src/code6cac_b.c:2865 func_80033510, plus dead externs in six
 * other TUs), so retiring it is a project-wide symbol change well beyond this
 * function's granted surface. Prong (c) completeness is satisfied for the
 * merge this diff performs — the D_800A3918 declaration is the ONLY C
 * declaration of that object anywhere (grep, this session), and there are no
 * per-word sibling symbols for 0x800A3918 in undefined_syms_auto.txt or
 * named_syms.txt to delete. Retiring D_800A391D in favour of D_800A3918[5] is
 * banked as a follow-on, not a prerequisite.
 *
 * THE FINDING (s16, re-measured here). Fifteen sessions hunted a byte-free
 * register occupant to grow hard_reg_conflicts[72] so find_reg's pass-0 scan
 * (global.c:996-1001) would skip $a1/$a2 and seat the arg0 pointer in $a3. The
 * premise was wrong, not the arithmetic: the tail is not three scalar
 * load/store pairs, it is a 12-byte AGGREGATE COPY. GCC 2.7.2's MIPS
 * block-move expansion emits all three loads before all three stores and keeps
 * the source address register live across the whole pattern, so pseudo 72
 * stops dying at the third load:
 *   scalar-triple tail : conflicts 2 3 4 29     -> pass0 first free 5 = $a1
 *   struct-copy   tail : conflicts 2 3 4 5 6 29 -> pass0 first free 7 = $a3
 * The conflict set arrives for free, at exactly 34 instructions, with no FAKE
 * construct, no dead local, no wrap, no preference route.
 *
 * PRONG (a) EVIDENCE — both merges rest on committed census rows that PREDATE
 * this grind (named_syms.txt row for 0x80107850 dates to commit e44dcd95,
 * 2026-05-17; the grind opened 2026-07-21), corroborated in the ORIGINAL
 * BINARY:
 *  - named_syms.txt:1556  g_leaf_position_table = 0x80107850;
 *      "12-byte stride per leaf, 6 entries = 72-byte position array"
 *  - named_syms.txt:1554  g_leaf_slot_state = 0x800A3918;
 *      "6-byte slot state table (per-leaf counter byte)"
 *  - named_syms.txt:1555  g_leaf_slot_state_end = 0x800A391E;
 *      "end marker for slot iteration (6 bytes after base)"
 *  - asm/funcs/func_800335D8.s (a DIFFERENT function, still INCLUDE_ASM) loads
 *    &D_80107850 into $s2 (0x800335EC/F0) and walks the table with
 *    `addiu $s2, $s2, 0xC` (0x80033704), six iterations, in lockstep with the
 *    D_800A3918 flag array walked at stride 1 and bounded by D_800A391E.
 *  - src/code6cac_b.c:2856-2870 (func_80033510, already COMPLETED-C) clears
 *    exactly six bytes downward from base+5 — the array's own initializer.
 *  - This function itself computes i*12 in the target asm (sll 1; addu; sll 2,
 *    0x80033594-9C) as the index into D_80107850, and indexes D_800A3918 at
 *    stride 1 with the same i.
 *
 * PRONG (c)/(e): both per-word siblings D_80107854/D_80107858 are deleted from
 * include/code6cac.h, undefined_syms_auto.txt:995-996 and named_syms.txt:
 * 2562-2563; no C consumer of either exists (grep, this session); the only
 * other reader of the table, func_800335D8, is still INCLUDE_ASM and is
 * unaffected; full verify-oracle SHA1 == oracle with the complete merge in
 * place.
 *
 * SURFACE (all four edits, inside the 2026-09-03 pipeline scope grant
 * `func_80033550 include/code6cac.h undefined_syms_auto.txt named_syms.txt`):
 *   1. include/code6cac.h:240   — extern u8 D_800A3918;  ->  extern u8 D_800A3918[6];
 *   2. include/code6cac.h:478-480 — the three per-word externs -> LeafPos + array
 *   3. undefined_syms_auto.txt:995-996 — delete D_80107854 / D_80107858
 *   4. named_syms.txt:2562-2563 — delete the _plus_4 / _plus_8 rows
 *   5. src/code6cac_b.c:2873 — INCLUDE_ASM replaced by the body below
 */

/* ---- include/code6cac.h:240, replacing `extern u8 D_800A3918;` ---- */
/* 6-byte per-leaf slot state table (named_syms.txt: g_leaf_slot_state,
   "6-byte slot state table (per-leaf counter byte)"; D_800A391E is the
   separate end marker recorded at named_syms.txt:1555, not an element). */
extern u8 D_800A3918[6];

/* ---- include/code6cac.h:478-480, replacing the three per-word externs ---- */
/* 12-byte per-leaf record table (named_syms.txt: g_leaf_position_table,
   "12-byte stride per leaf, 6 entries = 72-byte position array"). */
typedef struct {
    s32 x;
    s32 y;
    s32 z;
} LeafPos;

extern LeafPos D_80107850[6];

/* ---- src/code6cac_b.c:2873 ---- */
void func_80033550(LeafPos *arg0) {
    s32 i;

    for (i = 0; i < 6; i++) {
        if (D_800A3918[i] == 0) {
            break;
        }
    }
    if (i == 6) {
        return;
    }
    D_800A3918[i] = 1;
    D_80107850[i] = *arg0;
}
