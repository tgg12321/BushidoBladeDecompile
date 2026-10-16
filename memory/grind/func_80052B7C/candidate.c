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
 *
 *   (illustrative only -- NOT the landed chassis; the include line below is
 *   NOT inline_o.h, see s2 finding 1a)
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
/* s15 (enumerate): re-applied this exact chassis fresh, re-measured -- still 19, 4/4 hunks
 * source-level, 0/4 operand-only (tmp/grind/func_80052B7C/s15_sandbox_diff.txt). Ran
 * spelling_enum.py against the full 5-statement body both without and with the operand-swap
 * axis (tmp/grind/func_80052B7C/s15/enum, .../enum_swaps): both report "0 named locals, 0
 * assignments, 5 anchors -> 1 distinct spellings" -- zero mutation axes at either setting
 * (re-confirms s9, now covering the swap axis too). Region already spans the entire function
 * body (no neighbouring block exists to widen into per the sweep SOP). CLASS KILL: the
 * spelling-sweep search space on this chassis is provably empty; residual stays locked inside
 * gte_SetRotMatrix's / gte_stlvl's own fixed __asm__ volatile strings. src/main/3AB48.c
 * reverted to INCLUDE_ASM.
 */
/* s16 (enumerate): third consecutive enumerate-modality re-confirmation (after s9, s15).
 * Re-applied this exact chassis fresh, re-measured -- still 19, 4/4 hunks source-level,
 * 0/4 operand-only (tmp/grind/func_80052B7C/s16/sandbox_diff.txt). Ran spelling_enum.py
 * against the full 5-statement ENUM-BEGIN/END region both without (tmp/grind/func_80052B7C/
 * s16/enum_noswap) and with (tmp/grind/func_80052B7C/s16/enum) the operand-swap axis: both
 * report "0 named locals, 0 assignments, 5 anchors -> 1 distinct spellings" -- identical to
 * s9's and s15's mechanical result, byte-for-byte. The sweep SOP's step 3 widen option (move
 * to the neighbouring block) is inapplicable: the region already spans the entire function
 * body, as s15 already noted. No new axis exists to try under the current sweep tooling; this
 * is now the THIRD independent enumerate pass landing on the identical empty mutation space.
 * The residual remains exactly chunks 1 and 5 (gte_SetRotMatrix's batched lw/ctc2 order,
 * gte_stlvl's delay-slot swc2 placement), both inside each macro's own fixed __asm__ volatile
 * string (include/psxsdk/inline_c.h:297, :1086) -- outside any C-level spelling space by
 * construction. src/main/3AB48.c reverted to INCLUDE_ASM; no new src/ edit banked.
 */
/* s17 (enumerate): fourth consecutive enumerate pass, re-confirmed 19 (24/24 insns,
 * 4/4 hunks source-level). Note for future sessions: applying ONLY this body without
 * also carrying the `#include <psxsdk/inline_c.h>` line below silently mis-measures --
 * src/main/3AB48.c's own `#include <psxsdk/libgte.h>` only comments on the vendored
 * headers (libgte.h:8-9), it does not include one, so without this line the gte_*
 * names compile as real calls (jal), not inline asm, and the sandbox score is wrong
 * (worse) with no diagnostic other than the diff showing jal/addiu sp prologue insns
 * that should not exist. spelling_enum.py (both --no-swaps and swap axis,
 * tmp/grind/func_80052B7C/s17/enum_noswap, .../enum_swap): identical "0 named locals,
 * 0 assignments, 5 anchors -> 1 distinct spellings" to s9/s15/s16. See hypotheses.md s17/H14.
 */
/* s26 (enumerate): fifth consecutive enumerate pass (after s9, s15, s16, s17), re-confirmed
 * chassis at 19, 24/24 insns, 4/4 hunks source-level, 0 operand-only (tmp/grind/func_80052B7C/
 * s26/sandbox_diff.txt). spelling_enum.py re-run on the full 5-statement ENUM-BEGIN/END region
 * (scratch copy tmp/grind/func_80052B7C/s26/enum_candidate.c, candidate.c itself untouched),
 * both --no-swaps (s26/enum_noswap) and swap axis (s26/enum_swap): identical "0 named locals,
 * 0 assignments, 5 anchors -> 1 distinct spellings" to all four prior enumerate passes. CLASS
 * KILL (standing, 5th measurement): the C-level spelling-sweep search space on this chassis is
 * provably empty; the residual (chunk 1 batching order inside gte_SetRotMatrix/gte_ldtr, chunk
 * 5 delay-slot placement inside gte_stlvl) remains locked inside each macro's fixed __asm__
 * volatile string (include/psxsdk/inline_c.h:297, :1086). No src/ edit banked; src/main/3AB48.c
 * stays INCLUDE_ASM. See hypotheses.md s26.
 */
/* s27 (enumerate): sixth consecutive enumerate pass (after s9, s15, s16, s17, s26);
 * mandatory kill re-audit first: chassis re-applied fresh to src/main/3AB48.c, re-measured
 * 19 (24/24 insns, 4/4 hunks source-level, 0 operand-only, tmp/grind/func_80052B7C/s27/
 * sandbox_diff.txt), fake_ablate.py confirms "no FAKE-annotated constructs found; nothing
 * to ablate" (identical to s14/s18-s26 -- no instance kill voided by chassis/FAKE drift).
 * spelling_enum.py re-run on the exact 5-statement region (scratch copy s27/
 * enum_candidate2.c): both --no-swaps (s27/enum_noswap2) and swap axis (s27/enum_swap2)
 * report "0 named locals, 0 assignments, 5 anchors -> 1 distinct spellings" -- byte-identical
 * to all five prior passes. NOTE for future sessions: a first attempt this session widened
 * the ENUM region to also cover the `#include` and function-signature lines (s27/
 * enum_candidate.c); the swap axis then reported "16 distinct spellings" because the tool's
 * commutative-swap pass treats a pointer declarator `T * name` as a binary `*` operator and
 * swaps it into invalid syntax (`out * VECTOR`) -- this is a tool artifact of the malformed
 * region, not a real mutation axis; discard it, do not re-propose it as a finding, and keep
 * the region scoped to the 5 call statements only (matching s9/s15/s16/s17/s26). CLASS KILL
 * (standing, 6th measurement): the C-level spelling-sweep search space on this chassis is
 * provably empty. No src/ edit banked; src/main/3AB48.c stays INCLUDE_ASM. See hypotheses.md
 * s27.
 */
/* s18 (synthesis): kill re-audit -- body unchanged, re-measured 19 (4/4 source-level), no
 * FAKE to ablate (tmp/grind/func_80052B7C/s18/). New census (s18/batch_census.txt): the
 * target's batched lw/lh $t0..$t7 -> ctc2 run occurs in 16 EXE functions, all hand-written
 * (8 Sony LIBGTE asmpsx modules -- SetRotMatrix/MTX_09 is register-identical to chunk 1 --,
 * 4 STRONG/handwritten game funcs, the 3AB48 cluster); 0 compiled-C sites. Evidence for the
 * whole-body grant branch, not a C lever. See hypotheses.md s18.
 */
#include <psxsdk/inline_c.h>
void func_80052B7C(MATRIX *rot, SVECTOR *tr, SVECTOR *v0, VECTOR *out) {
    gte_SetRotMatrix(rot);
    gte_ldtr((s32)tr->vx, (s32)tr->vy, (s32)tr->vz);
    gte_ldv0(v0);
    gte_rt();
    gte_stlvl(out);
}
/* s19 (solver): chassis re-audited at 19 with no FAKE carriers. classify's RA label is an
 * artifact: the rotation registers are literal $12-$14 in the inline_c.h:298-307 template.
 * The allocated ldtr operands go to the lowest free regs ({t0,v1,v0} or {a0,v1,v0}).
 * See hypotheses.md s19. */
/* s20 (forensics): chassis re-audited at 19, no FAKE. Dumps show hunk 1 is fixed at RTL
 * expand (insn 15 template literal) plus local-alloc lowest-free for the ldtr operands, and
 * hunks 2-4 come from insn 35 (one asm_operands) before an unfilled return. The inline_o.h
 * macro set measures 31 (it adds move $12,%0). See hypotheses.md s20. */
/* s21 (forensics): chassis re-audited at 19, no FAKE. gte_nop() before gte_stlvl is stripped
 * as a scheduling barrier (engine/gtemacro.py:1095; rejected/gte-nop-hazard-barrier.c);
 * gte_rt_b() drops the target's pre-RT nops and scores 19. See hypotheses.md s21. */
/* s22 (rederive): chassis re-audited at 19, no FAKE. Mandatory sibling-transplant check on
 * func_8004A4E0/func_8004A76C (both re-dispositioned 2026-10-15) reconfirmed s12's kill: both
 * candidate.c files are comments-only (no C body), so there is nothing to transplant -- their
 * residuals (custom callee-saved ABI, trapping addi, cross-function jtbl) are a different
 * problem class from this function's RTL-template-literal residual anyway. A fresh grep of
 * docs/reference/sotn-construct-index.md (1,365 PSX entries) for setrotmatrix/ctc2/ldtr/
 * RotMatrix found zero precedent -- the Q55 SOTN-citation route stays closed. No new C-level
 * mutation axis found. See hypotheses.md s22. */
/* s23 (rederive): re-applied this exact chassis fresh, re-measured -- still 19, 4/4 hunks
 * source-level, 0/4 operand-only (tmp/grind/func_80052B7C/s23/sandbox_diff.txt). New
 * rederive-modality probe: fetched psyz's libgte.c (Xeeynamo/psyz, the PsyQ 4.0 cross-platform
 * decomp) and read its SetRotMatrix/RotMatrix/ApplyRotMatrix -- all pure software
 * reimplementations (plain field-copy / trig math) with zero cop2 opcodes, so it carries no
 * hardware instruction-order precedent for this residual. No new C-level mutation axis found.
 * src/main/3AB48.c reverted to INCLUDE_ASM; no new src/ edit banked.
 */
/* s24 (structural): chassis re-audited at 19, no FAKE. New structural probe: declaring
 * named s16 locals for tr->vx/vy/vz before gte_SetRotMatrix (vs direct member-expression
 * args to gte_ldtr) moves the 3 sub-word loads earlier in the stream but changes nothing
 * scored -- still lhu not the target's lh, still batched per-macro-string. This extends
 * s19's statement-level reorder probe to the finer-grained local-staging axis; same null
 * result. The structural lever set (var split / decl order / type narrowing / statement
 * reassociation) is now re-confirmed exhausted on this chassis for the 4th+ time (s8, s13,
 * s14, s19, s24). See hypotheses.md s24 and tmp/grind/func_80052B7C/s24/chassis_sandbox_diff.txt.
 */
/* s25 (structural): FOUND AND FIXED a ledger-hygiene bug in this file itself: a stray mid-word
 * comment-close token at old-line 231 ("RotMatrix" + close-comment + "ApplyRotMatrix") and a
 * bare #include stranding 7 lines of an illustrative written-island sketch outside its
 * comment (old-line 16) together broke this
 * candidate's C comment nesting, so `sandbox --candidate` on this file failed to build at all
 * (cascading cpp parse errors) -- this is why prior dispatches reported "chassis check:
 * measurement unavailable". Fixed both breaks (see hypotheses.md s25); re-measured clean at
 * 19, 24/24 insns, 4/4 hunks source-level, identical hunk classification to s6/s8/s13-s24.
 * Two new structural sub-axes tried and killed: (1) removing the redundant (s32) casts on
 * gte_ldtr's three args (tmp/grind/func_80052B7C/s25/variant_nocast.c) -- score unchanged, 19;
 * the cast is a no-op for the "r" asm operand. (2) block-scoped s32-typed locals (vs s24's s16
 * locals declared before SetRotMatrix) assigned from tr->vx/vy/vz right at the gte_ldtr call
 * (tmp/grind/func_80052B7C/s25/variant_s32locals.c) -- score unchanged, 19. Together with s24
 * this exhausts both the declared-width and declared-position sub-axes of local-staging; the
 * structural lever set is now re-confirmed exhausted for the 5th time (s8, s13, s14, s19, s24,
 * s25). No src/ edit banked; src/main/3AB48.c stays INCLUDE_ASM. See hypotheses.md s25.
 */
