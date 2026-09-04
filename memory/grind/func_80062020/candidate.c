/* func_80062020 (src/text1b.c) - MATCHING FORM.  Body unchanged since grind s15;
 * header rewritten in s16 (forensics) to state the mechanism accurately.
 *
 * STATUS: BYTES PROVEN, re-measured on the live chassis in s16 (2026-09-03):
 *   python3 memory/grind/func_80062020/apply_s15.py apply
 *   verify-oracle --rebuild --allow-dirty ; verify-oracle --allow-dirty
 *       -> ok true, build_matches true,
 *          build_sha1 62efab4f73f992798c43e8c730aa43baa10bb4fa == original_sha1_locked
 *   sandbox func_80062020 --disable all
 *       -> score 0, target_insns 38, build_insns 38, scorable true, rules_dropped 0
 * (Pre-rebuild the sandbox reads a false 2: the aggregate merge relocates the two
 * in-loop stores HI16/LO16 against D_800F1198 with in-field addends 4 and 8 where the
 * INCLUDE_ASM reference names D_800F119C / D_800F11A0 at addend 0.  S+A is identical;
 * engine/score.py deliberately does not mask named-symbol addends, engine/score.py:8-11,
 * :61-63.  Rebuild the reference with the diff in place, then score.)
 *
 * WHAT THE BODY IS.  Two things and nothing else:
 *   1. The storage at 0x800F1198 is declared as what the original code treats it as -
 *      an array of 3-word records (include/game.h).  Object-model evidence, independent
 *      of symbol adjacency (see [[splat-symbol-names-are-not-evidence]]): the original
 *      loop walks the table with a 12-byte-stride induction register
 *      (asm/funcs/func_80062020.s .L80062038, `addiu $v1, $v1, 0xC`) and the original
 *      epilogue addresses members through one base register at displacements 0x8 and 0x4
 *      (0x8006209C, 0x800620A0).  asm/funcs/func_800620B8.s reads the same table with the
 *      same 12-byte stride.  Frozen family: aggregate merge of per-word splat scalars,
 *      .claude/rules/no-new-park-categories.md:238.
 *   2. Ordinary C: a loop that copies 3-word records until a terminator bit clears, then
 *      one chained assignment clearing all three columns of the terminator row.
 * Every one of the four row writes uses the single spelling `D_800F1198[i].unkN`.  There
 * is no pointer local, no dead store, no duplicated address materialisation, no volatile,
 * no FAKE construct and no dual spelling anywhere in this body.
 *
 * ON THE EPILOGUE'S ADDRESSING MIX (measured in s16; full write-up in
 * tmp/grind/func_80062020/s16/forensics_s16.md and the s16 block of evidence.md).
 * The target stores the terminator row's +8 and +4 columns off a shared base register and
 * the +0 column through the inline-symbolic form.  That split is NOT authored: it is what
 * GCC 2.7.2 does with ANY chained assignment to >=3 members of an extern struct-array
 * element, reproduced in a neutral TU with unrelated names and no loop (N1/N7 in
 * tmp/grind/func_80062020/s16/).  RTL EXPAND stabilises the address of an assignment
 * whose value is consumed (expr.c:3457 in store_field) and folds the member offset into
 * the symbol when it is not.  C's right-to-left chain semantics decide which store is
 * last.  The author writes the members in ascending order, which is the ordinary way to
 * clear a row; the compiler does the rest.
 *
 * DIFF (three files, reproduce with memory/grind/func_80062020/apply_s15.py apply):
 *   include/game.h  - the record typedef + `extern Unk800F1198Record D_800F1198[];`
 *                     (prong (d): canonical shared header, never TU-local)
 *   src/text1b.c    - both stale `extern s32 D_800F1198/119C/11A0;` triples removed
 *                     (neither had a use site) and the INCLUDE_ASM replaced by the body
 *   src/text1b_b.c  - the third stale triple removed (also unused)
 * After the merge there is exactly ONE C handle for the storage.  undefined_syms_auto.txt
 * keeps D_800F119C / D_800F11A0 while asm/funcs/func_800620B8.s is still INCLUDE_ASM and
 * references them (disclosed; the sanctioned precedents func_800861BC and e788983a did
 * the same).
 *
 * SUBMISSION STATE: this exact body is listed in state.json banned_constructs (entries 3
 * and 4, added after the 2026-09-03 20:23 layer-1 FAIL), so a candidate-ready that
 * re-declares it is discarded before review.  s16 returned `ruling-request` asking whether
 * the ban survives the measurement above.  Do not respell it to evade the ban.
 */
void func_80062020(s32 *arg0) {
    s32 i;
    s32 ofs;
    s32 t;
    t = *(s32 *)((u8 *)arg0 + 0);
    D_800A32B8 = 0;
    i = 0;
    if ((t & 1) == 0) goto end;
    ofs = 0;
    do {
        D_800F1198[i].unk0 = *(s32 *)((u8 *)arg0 + ofs + 0);
        D_800F1198[i].unk4 = *(s32 *)((u8 *)arg0 + ofs + 4);
        D_800F1198[i].unk8 = *(s32 *)((u8 *)arg0 + ofs + 8);
        i = i + 1;
        ofs = ofs + 12;
        t = *(s32 *)((u8 *)arg0 + ofs + 0);
    } while ((t & 1) != 0);
end:
    D_800F1198[i].unk0 = D_800F1198[i].unk4 = D_800F1198[i].unk8 = 0;
}
