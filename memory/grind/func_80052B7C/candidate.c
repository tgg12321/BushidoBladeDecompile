/* func_80052B7C -- STRUCTURAL LEVERS EXHAUSTED as of s3 (this session). No
 * pure-C landing form exists under the current rules; every prior frontier
 * item is now measured dead. Read hypotheses.md s2/s3/H4/H5 for the full
 * derivation; summary below.
 *
 * What's PROVEN (s2, re-verified s3, sandbox func_80052B7C --disable all
 * --diff, honest floor 16 against the INCLUDE_ASM-stub floor of 24):
 *   /* s10 (solver): classify (object mode) = FIRST DIVERGENCE RA (register-blanked multisets match).
 * RA verdict FORECLOSED: target's 8 data GPRs are t0..t7 ($8-$15) with v0/v1 never touched; GCC
 * 2.7.2 mips.h has no REG_ALLOC_ORDER, so local-alloc.c:2249 find_free_reg takes the lowest free
 * hard reg -- v0 is always free here (void, no calls). Scratch probe (8 C pseudos fed to one asm via
 * "r", rejected/ra-probe-8-operand-written-island.c) measured {v0,v1,t0..t5}, not t0..t7. No vendored
 * macro hardcodes lw $8..$11 (census 0). So the target registers come only from a fixed asm string
 * the vendored headers do not contain. See hypotheses.md s10/H9.
 */
#include <psxsdk/inline_c.h>   (NOT inline_o.h -- see s2 finding 1a)
 *   void func_80052B7C(MATRIX *rot, SVECTOR *tr, VECTOR *v0, VECTOR *out) {
 *       gte_SetRotMatrix(rot);   // chunk 1 -- WRONG instruction order (see below)
 *       <chunk 2: no vendored macro exists for TR-from-halfwords -> CR5-7>
 *       gte_ldv0(v0);            // chunk 3 -- CONFIRMED byte-identical
 *       gte_rt();                // chunk 4 -- CONFIRMED byte-identical
 *       gte_stlvl(out);          // chunk 5 -- WRONG delay-slot placement (see below)
 *   }
 *
 * What's DEAD, and why each is now a closed question rather than an open
 * frontier item:
 *   - Chunk 1 (gte_SetRotMatrix): both vendored headers interleave 2-lw+2-ctc2
 *     then 3-lw+3-ctc2; target batches all 5 lw then all 5 ctc2. No finer
 *     vendored leaf or gtemac.h composite exists to compose the batched order
 *     (s3 H2: full leaf enumeration; this session's H5: full gtemac.h visual
 *     read -- no composite names or expands to gte_SetRotMatrix at all).
 *   - Chunk 2 (TR from halfwords into CR5-7): no vendored macro of any kind
 *     performs this op (s1).
 *   - Chunk 5 (gte_stlvl): target's last swc2 sits in the jr $ra delay slot;
 *     our build emits it before jr with an explicit nop. Confirmed via .dbr
 *     RTL dump (s3 H3): gte_stlvl's 3-swc2 body is one opaque asm_operands
 *     insn reorg.c's delay-slot filler cannot split or select into.
 *   - A written __asm__ island closing any of the three (chunks 1, 2 or 5)
 *     is RULED OUT: docs/grind/decisions.md 2026-10-15 20:38 func_80052B7C
 *     ruling, citing inline-asm-policy.md:55-57/70-72 -- "the macro can't
 *     reach the bytes" is not an exception anywhere in the rule. This
 *     session's H4 confirms the ruling matches the live rule text. The
 *     previously-banked written-island candidate is now in
 *     rejected/written-island-chunks-1-2-5.c; do not resubmit it or any
 *     respelling of the same construct.
 *
 * What's LEFT, and why it is NOT a grind-session action:
 *   - A new vendored macro / DMPSX word covering chunk 2 needs its own owner
 *     ruling plus an `engine:` commit (inline-asm-policy.md item 1) -- and
 *     would still leave chunks 1 and 5 unreached (their macros already exist
 *     and still don't reach the bytes).
 *   - The whole-function inline_asm_canonical.txt:340 grant (reopened for
 *     audit by aeea8ed44, owner 2026-10-08) would hold AS-IS if
 *     scan_hand_coded reached STRONG tier (precedent: the func_8004A76C
 *     2026-10-15 CANONICAL-ASM GRANT PATH entry in docs/grind/decisions.md).
 *     Re-checked fresh this session: `scan_hand_coded.py --single
 *     func_80052B7C` = LOW 0/12, no gate-class signal
 *     (tmp/grind/func_80052B7C/s3/scan.txt). Raising this needs a scanner
 *     signal-set change (engine/tools), not a C edit.
 *
 * No src/ edit is banked here: the honest floor (16, with the now-rejected
 * written island) cannot be re-landed, and no replacement reaches a lower
 * honest floor without the banned construct. src/main/3AB48.c keeps
 * INCLUDE_ASM("asm/funcs", func_80052B7C); per asm-until-matched rule 1.
 *
 * s4 (permuter, mandated modality): decomp-permuter's standard import path
 * (tools/decomp-permuter/import.py) cannot even build a scorable workspace
 * for this function -- its default assembler (plain mips-linux-gnu-as) has
 * no cop2 GTE opcodes and fails on asm/funcs/func_80052B7C.s:22's `mvmva`.
 * No campaign was launched (nothing to launch against). See
 * hypotheses.md s4/H6 for the full measurement and tmp/grind/func_80052B7C/
 * s4/ for the run log. Even a hand-built workspace (routing target.s through
 * the project's own maspsx/aspsx pipeline, untested) would have nothing to
 * mutate: the macro-call chassis has zero free C-level variables, and the
 * unresolved chunks' divergence sits inside each macro's own opaque
 * asm_operands RTL node (s3/H3, dump-confirmed), unreachable by statement
 * reordering and not reachable without the already class-killed written-
 * island construct (s3/H4).
 */

/* s6 (synthesis): best LEGAL form measured -- pure vendored-macro calls only,
 * no __asm__ in the body (satisfies the Judge constraint's first branch).
 * sandbox func_80052B7C --disable all = 19 (stub 24; the rejected written-island
 * form was 16). Chunk 2 lands via gte_ldtr (inline_c.h:371) with C-level lh loads;
 * (s32) casts are semantically required (TRX/TRY/TRZ are 32-bit signed -- without
 * them GCC emits lhu, a zero-extension, wrong for negative translations; score
 * masks it, both 19). Residual = chunks 1 and 5 only (macro-internal order /
 * delay slot), source-level per --diff.
 * Apply by replacing INCLUDE_ASM("asm/funcs", func_80052B7C); in src/main/3AB48.c.
 */
/* s7 (synthesis): body unchanged, still 19. cc1psx calibration: SN cc1psx emits a
 * byte-identical body from this C (tmp/grind/func_80052B7C/s7/cc1_vs_cc1psx.txt).
 * Chunk 5 class-closed: reorg.c:740-744 never fills a delay slot with an asm insn and
 * mips.md has no cop2 store pattern, so no C body puts swc2 $11 in the jr slot. Only the
 * whole-body hand-written grant branch can land this function.
 */
/* s8 (structural): re-applied and re-measured fresh -- still 19, 4/4 hunks source-level,
 * 0 operand-only (tmp/grind/func_80052B7C/s8/sandbox_diff.txt). CLASS KILL: no block-local
 * var split / declaration-order / type-narrowing / statement-reassociation lever can touch
 * chunk 1 or chunk 5, because gte_SetRotMatrix and gte_stlvl (include/psxsdk/inline_c.h:297,
 * :1086, read in full this session) are each a single __asm__ volatile statement whose
 * instruction sequence is a literal fixed string -- not computed from any C-level expression
 * -- so nothing outside the macro invocation can reorder it. See hypotheses.md s8/H7.
 */
/* s9 (enumerate): re-ran spelling_enum.py fresh against this exact chassis (ENUM-BEGIN/END
 * around all 5 macro-call statements, tmp/grind/func_80052B7C/s9/enum_test.c). Mechanical
 * result BOTH with and without --no-swaps: "0 named locals (), 0 assignments, 5 anchors ->
 * 1 distinct spellings" -- the tool's own precondition scan finds zero declarations,
 * assignments or commutative binary operators anywhere in the region, so it can only emit
 * the identity spelling. This re-confirms s5's floor-16-era finding still holds on the
 * current floor-19 chassis: the residual (chunk 1 batching order inside gte_SetRotMatrix,
 * chunk 5 delay-slot placement inside gte_stlvl) lives entirely inside each macro's own
 * fixed-string __asm__ volatile body (s8, inline_c.h:297/:1086 read in full), which is not
 * C-level syntax spelling_enum.py's region scanner can see or vary. CLASS KILL for the
 * spelling-sweep search space on this chassis. See hypotheses.md s9/H8.
 */
/* s11 (forensics): body unchanged, re-measured 19. Chunk 5 is decided at the ASSEMBLER stage:
 * cc1 emits a bare reorder-mode `j $31` after the #APP block and maspsx always appends a nop
 * (tools/maspsx/maspsx/__init__.py:1484). EXE census (tmp/grind/func_80052B7C/s11/jr_census.txt):
 * a cop2 insn in the jr $ra slot occurs only in the 5-function 3AB48 GTE cluster, 0 compiled-C
 * sites; the 4 siblings are granted (inline_asm_canonical.txt:335/338/339/341). See hypotheses.md H10-H12.
 */
/* s12 (rederive): re-measured this exact chassis fresh -- still 19, 4/4 hunks source-level
 * (tmp/grind/func_80052B7C/s12/sandbox_reaudit.txt). Three rederive-specific levers tried and
 * class-killed: (1) m2c (mips_to_c, freshly pip-installed) decompile of the target asm --
 * its instruction model has zero cop2/GTE opcodes, so the entire body comes back as 14
 * M2C_ERROR stubs with no structural suggestion at all (s12/m2c_out.c). (2) decomp.me corpus
 * search (gcc2.7.2-psx bucket) -- top 5 hits score similarity 0.022-0.026, an order of
 * magnitude below a citable match, none touching cop2 (s12/decomp_me_search.txt). (3) Full
 * read of gtemac.h's composite macros (lines 260-358) -- none batches the rotation-matrix and
 * TR-vector loads together or re-expresses gte_SetRotMatrix/gte_ldtr's own fixed asm-string
 * body with a different instruction order; composites are calls-to-calls, not alternate
 * leaves. See hypotheses.md s12 for all four entries (re-audit + 3 kills).
 */
/* s13 (structural): re-applied this exact chassis fresh, measured 19 again, 4/4 hunks
 * source-level, 0/4 operand-only (tmp/grind/func_80052B7C/s13/sandbox_diff.txt). Re-confirms
 * s8's class kill: no structural lever exists because there is no operand-only bucket to act
 * on -- the whole residual sits inside gte_SetRotMatrix's and gte_stlvl's own fixed __asm__
 * volatile strings (include/psxsdk/inline_c.h:297, :1086, re-read this session, unchanged).
 * No src/ edit banked; src/main/3AB48.c reverted to INCLUDE_ASM.
 */
/* s14 (structural, kill re-audit mandated by the dispatcher): re-applied this exact
 * chassis fresh and re-measured -- still 19, 4/4 hunks source-level, 0/4 operand-only
 * (tmp/grind/func_80052B7C/s14/sandbox_diff.txt). tools/fake_ablate.py --func func_80052B7C
 * --file main/3AB48 --candidate memory/grind/func_80052B7C/candidate.c reports "no
 * FAKE-annotated constructs found ... nothing to ablate" (tmp/grind/func_80052B7C/s14/ablate.txt)
 * -- there is no FAKE carrier on this chassis to have invalidated s8/s9/s13's class kills, so
 * those re-audits stand as clean. Hunk 1 (batched lw/ctc2 order) and hunks 2-4 (gte_stlvl's
 * delay-slot placement) are unchanged from s6: both residuals sit inside gte_SetRotMatrix's and
 * gte_stlvl's own fixed __asm__ volatile strings (include/psxsdk/inline_c.h:297, :1086), with
 * zero C-level declarations, assignments or operators in the 5-statement region for any
 * structural lever (block-local split / declaration order / type narrowing / statement
 * re-association) to act on. Fourth consecutive structural-or-equivalent re-measurement
 * (s8, s9, s13, s14) at the identical floor with the identical hunk classification; no new
 * lever found. src/main/3AB48.c reverted to INCLUDE_ASM.
 */
#include <psxsdk/inline_c.h>
void func_80052B7C(MATRIX *rot, SVECTOR *tr, SVECTOR *v0, VECTOR *out) {
    gte_SetRotMatrix(rot);
    gte_ldtr((s32)tr->vx, (s32)tr->vy, (s32)tr->vz);
    gte_ldv0(v0);
    gte_rt();
    gte_stlvl(out);
}
