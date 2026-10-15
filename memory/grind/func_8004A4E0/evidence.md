# Evidence bank — func_8004A4E0

## s1 (recon) — canonical gate verdict + OBJECT MODEL

`canonical func_8004A4E0` returns verdict **ASM-WHOLE** (`"reasons": ["whole-function
canonical-asm grant in inline_asm_canonical.txt"]`), citing `inline_asm_canonical.txt:199`.
That entry (text corrected 2026-09-29 per owner ruling Q40, commit 803d0fea1) is backed by a
full decisive audit at `docs/audits/func_8004A4E0-canonical-2026-09-29.md`: a custom
callee-saved-register ABI across `jal func_8004A940`/`func_8004A76C`/`func_8004A808` —
$s4/$s5/$s6/$fp/$s0 handed across calls in ways GCC 2.7.2 cannot emit from C (dead-looking
pre-call stores the callee actually reads; `3-$s5` the callee itself writes so C would fold
it to a constant; a ghost `$s3` save/restore the callee clobbers unsaved; a callee-advanced
`$s0` read back after return, which GCC assumes a callee-save can't do) — plus 2 trapping
`addi` (0x8004A5A0, 0x8004A67C, non-overflow-safe in C) and 4 `ctc2` (no C form). The audit's
own conclusion: C islands would have to cover every handoff/`jal` site with forbidden GPR
asm, so the remainder is the whole body. `scan_hand_coded.py --single` tier is LOW (0/8 — the
S7 heuristic checks callee-side unsaved callee-saves; this function is the *caller* and saves
everything), so the grant rests on the ABI evidence, not scanner tier.

`sandbox func_8004A4E0 --disable all` this session: `{"score": 163, "no_c_body": true,
"build_insns": 0, "cheat_asm_stripped": 71}` — current `src/main/3AB48.c` still carries this
function as raw `INCLUDE_ASM`; there is no C body to diff (`--diff` prints "no diff: function
is still whole-body INCLUDE_ASM").

OBJECT MODEL: all 7 flagged DATA MODEL symbols are real operands inside
`asm/funcs/func_8004A4E0.s` (grep-confirmed: `D_80095328` @107-108,152-153; `D_80095488`
@169-170; `D_80095508` @136-137; `D_800FF610` @20-21; `g_draw_queue` @15-16; `D_80103608`
@39-40; `jtbl_8001541C` @29-31; `jtbl_8001545C` @55-57). Every one is
**MISMATCH-unmeasured (why): no C statement in this function references any of these symbols
— the entire body is `INCLUDE_ASM`, so no header declaration/type change for any of them can
alter this function's emitted bytes.** This is confirmed mechanically by the sandbox
`no_c_body: true` above, not inferred. The declaration-fix hypothesis (brief's recon
default #1) is therefore inapplicable to *this* function as long as it stays whole-body asm;
it would only become live if a future session carved a C island out of the body (which the
2026-09-29 audit argues is impossible without forbidden GPR asm — see hypothesis H1 below).

- [s1] canonical func_8004A4E0 returns verdict ASM-WHOLE, reason 'whole-function canonical-asm grant in inline_asm_canonical.txt' (entry at inline_asm_canonical.txt:199).

- [s1] sandbox func_8004A4E0 --disable all this session: {"score": 163, "no_c_body": true, "build_insns": 0, "cheat_asm_stripped": 71}; --diff reports no diff is possible since the function is still whole-body INCLUDE_ASM.

- [s1] scan_hand_coded.py --single tier for this function is LOW (0/8) per the audit, because its S7 heuristic checks callee-side unsaved callee-saves and this function is the caller, saving everything — the grant rests on the cross-call ABI evidence in the 2026-09-29 audit, not scanner tier.

- [s1] All 7 flagged DATA MODEL symbols (D_80095328, D_80095488, D_80095508, D_800FF610, g_draw_queue, D_80103608, jtbl_8001541C, jtbl_8001545C) are grep-confirmed as real lui/addiu/lw operands inside asm/funcs/func_8004A4E0.s; none is reachable from a C statement in this function since the whole body is INCLUDE_ASM, so no header-declaration fix can change this function's score. OBJECT MODEL bank entry written in full to evidence.md with per-symbol line citations.

- [s1] The inline_asm_canonical.txt:199 entry text was corrected 2026-09-29 under owner ruling Q40 (commit 803d0fea1), post-dating the 2026-08-18 judge-sole-gate ruling, so the grant has already been owner-reviewed under current policy.

- [s2] canonical func_8004A4E0 this session returns verdict ASM-WHOLE, reason 'whole-function canonical-asm grant in inline_asm_canonical.txt', asm_insns=7, total=163 — unchanged from the ledger's prior recorded state.

- [s2] No docs/grind/decisions.md entry exists yet for func_8004A4E0 (checked via grep); not filed this session because my mandated modality is structural, not escalation, and the owner's standing auto-ruling (A) reserves the exhaustion determination to the driver's modality assignment.

- [s2] candidate.c and evidence.md already accurately describe the no-C-body state from s1 recon; no edits needed there this session beyond the hypotheses.md s2 re-confirmation entry.

- [s3] canonical func_8004A4E0 this session: verdict ASM-WHOLE, reason 'whole-function canonical-asm grant in inline_asm_canonical.txt', asm_insns 7, total 163 (tmp/grind/func_8004A4E0/s3/canonical.txt).

- [s3] sandbox --disable all this session: score 163, no_c_body: true, build_insns 0 — confirms no C body exists to score or permute (tmp/grind/func_8004A4E0/s3/sandbox.txt).

- [s3] This matches the s1 recon class kill (H1, memory/grind/func_8004A4E0/hypotheses.md) and the s2 structural-modality re-confirmation: the custom callee-saved-register ABI across jal with func_8004A940/func_8004A76C/func_8004A808 plus trapping addi/ctc2 have no C form, per docs/audits/func_8004A4E0-canonical-2026-09-29.md.

- [s3] This function's open item is operator/landing-side (authorize-bucket, Judge-gated canonical-grant path per judge-sole-gate.md), not a grind-lane hypothesis — permuter, like structural, has no surface to act on.

- [s4] canonical func_8004A4E0 verdict this session: ASM-WHOLE, asm_insns=7, total=163, reason 'whole-function canonical-asm grant in inline_asm_canonical.txt', regions [[48,48],[48,48],[103,103],[117,118],[123,124]] — byte-identical to s1/s2/s3's recorded verdict.

- [s4] sandbox func_8004A4E0 --disable all this session: score=163, no_c_body=true, build_insns=0, cheat_asm_stripped=71 — identical to s1-s3; src/main/3AB48.c:29 is still INCLUDE_ASM("asm/funcs", func_8004A4E0) unchanged.

- [s4] candidate.c (unchanged this session) is itself the INCLUDE_ASM stub annotated with the s1 recon finding: a custom callee-saved-register ABI ($s4/$s5/$s6/$fp/$s0 handoffs) shared across jal to func_8004A940/func_8004A76C/func_8004A808 plus 2 trapping addi and 4 ctc2 with no C form.

- [s4] This is the 4th consecutive modality (recon, structural, permuter, enumerate) to independently re-derive the same ASM-WHOLE/no-C-body fact with zero new ground — the mandated-modality ladder is producing repeat confirmations, not new information, on this function.

## s5 (synthesis) — kill re-audit + new hand-coded corpus signal

- [s5] KILL RE-AUDIT: the only instance kill (s3, permuter) was measured on the INCLUDE_ASM stub with zero FAKE constructs. Re-measured this session on the same (current) chassis: `sandbox func_8004A4E0 --disable all` = score 163, no_c_body true, build_insns 0 (tmp/grind/func_8004A4E0/s5/sandbox.txt); `tools/fake_ablate.py --candidate memory/grind/func_8004A4E0/candidate.c` = "No FAKE-annotated constructs found ... nothing to ablate" (tmp/grind/func_8004A4E0/s5/fake_ablate.txt). No chassis drift, no FAKE carrier — the kill's conditions still hold; nothing was masked by a carrier. The three class kills cite docs/audits/func_8004A4E0-canonical-2026-09-29.md:31 (the audit's decisive-evidence table), which this session re-verified line-by-line against asm/funcs/func_8004A4E0.s (below), so no foreclosure in this ledger is the "weakest unverified" one the contradiction rule targets.
- [s5] canonical func_8004A4E0 unchanged: ASM-WHOLE, reason "whole-function canonical-asm grant in inline_asm_canonical.txt" (inline_asm_canonical.txt:199) (tmp/grind/func_8004A4E0/s5/canonical.txt). BUT engine/queue.json carries this item as verdict 'C', status active, origin 'regression', reopen_reason "owner 2026-10-08: re-audit of whole-body INCLUDE_ASM canonical entries. Decide afresh whether the original was hand-written asm (scan_hand_coded STRONG evidence + Judge) or compiled C that must be decompiled". So the live question for this function is not a C spelling but the 2026-10-08 hand-written-vs-compiled re-audit — and its gate is scan_hand_coded STRONG + Judge.
- [s5] scan_hand_coded.py --single func_8004A4E0 this session: tier=LOW, score 0/8, all of S1..S8 unset (tmp/grind/func_8004A4E0/s5/scan_hand_coded.txt). S7 misses because this function is the CALLER of the custom-ABI callees and saves everything it touches.
- [s5] NEW CORPUS SIGNAL (not in the scanner, not in the 2026-09-29 audit): CODE AFTER THE SINGLE EPILOGUE. func_8004A4E0 has exactly one `jr $ra` (0x8004A60C, delay `addiu $sp,$sp,0x38`) and 69 non-nop instructions AFTER it — the five jump-table case bodies (.L8004A614/644/694/6F0/730), each ending `j .L8004A5C0` back into the loop. Corpus measurement (tmp/grind/func_8004A4E0/s5/midret.py → midret.txt): of 1225 asm/funcs/*.s functions that are pure C in src/ today, ZERO have code after a single `jr $ra`; of 160 still-asm functions, exactly 2 do — func_8004A4E0 (69) and _patch_gte (9, a PsyQ hand-written library routine). GCC 2.7.2 emits the epilogue once, after the last insn of the body; case blocks that loop back via `j` sit before it.
- [s5] Re-verified the audit's ABI rows directly in asm/funcs/func_8004A4E0.s: $s4 has NO read in this body other than its own `addi $s4,$s4,0xB0` (0x8004A67C); it is loaded from 0x1F80001C($s1) at 0x8004A530 and after each `jal func_8004A940` (0x8004A688/6E4/724/760) and overwritten with a table base (D_80095328 / D_80095508 / D_80095488) immediately before each `jal func_8004A940` — pure callee input. $s5 is written once (`addu $s5,$zero,$zero` 0x8004A524) and read 5x as `3 - $s5` (0x8004A618/648/698/6F4/734) — C would fold it to `li 3`. Also: a bare `nop` at 0x8004A718 between `addiu $s4` and `jal` with no load/branch hazard to cover (the D_80095508 / D_80095488 arms have none) — not an assembler-inserted hazard nop; consistent with hand-edited source. Every `jal` / `j` delay slot in the case bodies is an unfilled `nop`.

- [s5] fact: canonical func_8004A4E0 = ASM-WHOLE via inline_asm_canonical.txt:199; queue.json carries the item as verdict C / origin regression with reopen_reason = owner 2026-10-08 re-audit of whole-body canonical entries (hand-written via scan_hand_coded STRONG + Judge, or compiled C).

- [s5] fact: scan_hand_coded.py --single func_8004A4E0 = tier LOW, 0/8, S1..S8 all unset (s5/scan_hand_coded.txt).

- [s5] fact: 0 of 1225 pure-C functions place code after a single jr $ra; func_8004A4E0 places 69 insns (all five jump-table case bodies) there; the only other asm function doing so is _patch_gte (s5/midret.txt).

- [s5] fact: $s4 has no local read besides addi $s4,$s4,0xB0 (0x8004A67C); it is set to D_80095328/D_80095508/D_80095488 immediately before each jal func_8004A940 and reloaded from 0x1C($s1) after — callee-input only.

- [s5] fact: $s5 is written once (0x8004A524, =0) and read 5x as 3-$s5 (0x8004A618/648/698/6F4/734); a compiled body would fold to li 3.

- [s5] fact: sandbox --disable all this session = 163, no_c_body true; fake_ablate finds no FAKE constructs in candidate.c.

## s6 (solver)
- inverse_compose classify refuses an INCLUDE_ASM target by design (s6/classify.txt); no solver residual exists without a C body.
- Dump-level confirmation of the epilogue-placement signal: our cc1 at project flags puts the single epilogue last, after all switch case blocks, in two switch-in-loop sketches (tmp/grind/func_8004A4E0/s6/sketch.s:153, :251); mechanism final.c:1066 FUNCTION_EPILOGUE is text-emitted after the insn stream (config/mips/mips.c:5073). This supplies the predicate citation the s5 frontier asked for.

- [s6] inverse_compose classify refuses INCLUDE_ASM targets by design; no solver axis exists for this function without a C body

- [s6] cc1 dump-level: epilogue always last after all switch case blocks (tmp/grind/func_8004A4E0/s6/sketch.s); predicate final.c:1066

- [s6] sandbox --disable all this session: score 163, no_c_body true

## s7 (forensics)
- [s7] fact: sandbox --disable all = 163, no_c_body true (tmp/grind/func_8004A4E0/s7/sandbox_diff.txt); canonical gate = ASM-WHOLE, whole-function grant in inline_asm_canonical.txt (s7/canonical.txt); fake_ablate: no FAKE constructs in candidate.c (s7/fake_ablate.txt). dump.ps1 cannot produce RTL dumps (no C body: "3AB48.rtl: No such file", s7/dump_ps1.txt).
- [s7] fact: CALLEE-SAVED USE CENSUS (tmp/grind/func_8004A4E0/s7/savescan4.py -> savescan4.txt; save/restore identified by pairing sw/lw R,off($sp), robust to scheduled prologues). Compiled-C corpus = 506 functions with a C definition in src/, not INCLUDE_ASM, not in inline_asm_canonical.txt.
  * Signal B "callee-saved reg WRITTEN in the body but NEVER READ in the body": compiled-C 0/506; asm side 1/91 = func_8004A4E0 $s6 (`addu $s6,$fp,$zero` at 0x8004A538 each outer iteration, no local read; consumed only by callees func_8004A76C/808/938/940 -> the audit's custom-ABI handoff). Predicate: flow.c:1719 insn_dead_p deletes a set of a dead register; the only hard-reg exemption is flow.c:1758 `global_regs[regno]` (file-scope `register T x asm("$22")` — the register-asm pin family, on the wall) or the frame pointer. So no admissible C input produces this insn.
  * Signal A "saved+restored but never otherwise referenced" (func_8004A4E0 $s3): NOT discriminating — compiled-C 5/506 do it (func_8002E6B0 $s4, func_8005763C $s0, math_RotMatrixXYZ $s1, math_RotMatrixYXZ $s2, math_RotMatrixZXY $s1; all inline-asm-free C bodies). Do not cite $s3 as hand-written evidence.
  * Earlier scanner versions (s7/savescan.py, savescan3a/3b) over/under-counted (outgoing-arg `sw $sN,0x10($sp)` mistaken for a save; scheduled prologues missed); savescan4 is the definitive count.

- [s7] sandbox --disable all = 163, no_c_body true (tmp/grind/func_8004A4E0/s7/sandbox_diff.txt)

- [s7] canonical gate ASM-WHOLE via whole-function grant (tmp/grind/func_8004A4E0/s7/canonical.txt)

- [s7] callee-saved written-never-read: compiled C 0/506, only func_8004A4E0 $s6 among 91 asm functions (s7/savescan4.txt); predicate flow.c:1758

- [s7] callee-saved saved-never-referenced: compiled C 5/506, so not a hand-written signal (s7/savescan4.txt)

- [s7] earlier scanner versions savescan.py / savescan3a / savescan3b miscounted (outgoing-arg sw mistaken for a save; scheduled prologues missed); savescan4 is the definitive count

## s8 (rederive)
- m2c refuses func_8004A4E0's asm outright: "Found jr instruction ... jump table is not provided" — jtbl_8001541C/jtbl_8001545C are C arrays in src/main/d_5BF0.c, not .s data m2c can consume inline (tmp/grind/func_8004A4E0/s8/m2c_out.c, m2c_err.txt).
- NEW decisive fact: 8 of jtbl_8001541C's 16 entries (0x8004A990, 0x8004ABD8, 0x8004AE0C, 0x8004AF84, 0x8004B0C8, 0x8004B0E4, 0x8004B27C, 0x8004B4D8) are jlabels that exist only inside func_8004A940's own body (asm/funcs/func_8004A940.s), a different function entirely with its own glabel/endlabel and its own prologue. func_8004A4E0's own switch dispatch jumps directly into the middle of func_8004A940, skipping its prologue, carrying over func_8004A4E0's live register state. See tmp/grind/func_8004A4E0/s8/jtbl_cross_function_evidence.txt.
- GCC 2.7.2 compiles one FUNCTION_DECL to completion before the next (toplev.c:2647 rest_of_compilation) and switch jump-table labels are generated/scoped per-function (stmt.c:4704, case_stack). No C source can make the compiler emit a jump-table entry resolving into a different function's compiled body. This is an architectural (not merely statistical/corpus) class kill of the pure-C hypothesis for this control-flow shape — the strongest single fact this ledger has produced across 8 sessions.

- [s8] canonical() verdict remains ASM-WHOLE citing the existing inline_asm_canonical.txt grant (unchanged this session, re-confirmed).

- [s8] m2c (tools/m2c/m2c.py -t mips-gcc) cannot even attempt a decompile of func_8004A4E0's asm without its jump tables supplied as raw .s data; they exist only as C arrays (src/main/d_5BF0.c), confirming this body was never treated as switch-lowered output by any decompiler tooling either.

- [s8] jtbl_8001541C (func_8004A4E0's own outer-dispatch jump table, 16 entries, src/main/d_5BF0.c:21-25): 8 of its 16 entries are jlabels that exist exclusively inside func_8004A940's own compiled body (asm/funcs/func_8004A940.s), not inside func_8004A4E0 and not exported/shared anywhere.

- [s8] GCC 2.7.2 compiles one FUNCTION_DECL to completion (RTL gen through final assembly) before starting the next (toplev.c:2647 rest_of_compilation); a switch statement's jump-table labels are generated and scoped entirely within that single function's compilation pass (stmt.c:4704, case_stack/struct nesting). No C construct lowered by this compiler can therefore produce a jump-table entry resolving into a different function's own body — this is a compiler-architecture impossibility, not a statistical corpus absence.

- [s8] This closes the loop on the 2026-09-29 custom-ABI audit's observation of register handoffs across jal boundaries ($s4/$s5/$s6/$fp/$s0): the handoff isn't merely 'unusual ABI', it's the SAME mechanism — func_8004A4E0 literally jumps into func_8004A940's interior carrying live GPRs, which is exactly what a hand-written dispatcher can do and a compiled switch+call sequence cannot.

- [s8] candidate.c and hypotheses.md/evidence.md updated with full citations; no C body exists to grind (src/main/3AB48.c body unchanged: INCLUDE_ASM("asm/funcs", func_8004A4E0);).

## s9 (forensics) — 2026-10-15
- Kill re-audit: canonical still ASM-WHOLE, sandbox scores the INCLUDE_ASM stub with no diff, zero FAKE constructs (tmp/grind/func_8004A4E0/s9/canonical.txt, sandbox_diff.txt). All prior instance kills stand.
- New census (tmp/grind/func_8004A4E0/s9/rascan.py → rascan.txt): `sw $ra, off(base≠$sp)` is 0/1164 in compiled C, 12/273 in asm, the asm hits being Sony/SN hand-asm (setjmp, _patch_gte, __SN_ENTRY_POINT, pad/card stubs) plus func_8004A940 (func_8004A4E0's jtbl/jal target, saves/restores $ra at 4($s1) = scratchpad 0x1F800004) and func_8004BB68/BCC0/C1F4. Predicate: mips.c:4632 save_restore_insns bases only off $sp; no RETURN_ADDR_RTX on MIPS 2.7.2 (expr.c:7196).
- Callee-saved live-in before first branch: 0/1164 compiled C; asm hits include all three func_8004A4E0 helpers (func_8004A76C $s0, func_8004A808 $s0, func_8004A940 $s0/$s1) → mechanical confirmation of the custom register ABI.
- Interpretation: func_8004A4E0 + func_8004A76C/A808/A940 is a hand-written u16-opcode bytecode interpreter ($s0 IP, $s4 handler table from 0x1F80001C, $s1 scratchpad context). This, with the s8 cross-function jtbl finding, is the strongest evidence set for the existing ASM-WHOLE grant; next session (recon/escalation) should fold s8+s9 into docs/audits/func_8004A4E0-canonical-2026-09-29.md.

- [s9] func_8004A4E0 + func_8004A76C/A808/A940 form a hand-written u16-opcode bytecode interpreter: $s0 = instruction pointer, $s4 = handler table loaded from scratchpad 0x1F80001C, $s1 = scratchpad context base, jalr dispatch at func_8004A940.s:11-12

- [s9] $ra saved off-$sp: compiled C 0/1164, asm 12/273 (tmp/grind/func_8004A4E0/s9/rascan.txt)

- [s9] callee-saved live-in at entry: compiled C 0/1164, asm 6/273 including all three func_8004A4E0 helpers

- [s9] canonical gate still ASM-WHOLE; sandbox no diff on INCLUDE_ASM stub (s9 re-audit)

## s10 (forensics) — 2026-10-15
- Kill re-audit: canonical still ASM-WHOLE (whole-function grant, inline_asm_canonical.txt:199); sandbox --disable all --diff scores the INCLUDE_ASM stub, no diff, zero FAKE constructs (tmp/grind/func_8004A4E0/s10/canonical.txt, sandbox_diff.txt). fake_ablate n/a (candidate.c has no C body). Prior instance kills stand.
- Scanner gap measured (frontier item from s9): `scan_hand_coded.py --single func_8004A4E0` rates the function itself tier=LOW, 0/8 signals (tmp/grind/func_8004A4E0/s10/scan_func_8004A4E0.txt). None of the ledger's decisive signals (s5 code after the single jr $ra, s7 $s6 written-never-read, s8 cross-function jtbl, s9 $ra-off-$sp in the helper, the 2 trapping addi / 4 ctc2 annotated "handwritten instruction" in the .s) is a scanner signal. Its STRONG evidence is only transitive: helpers func_8004A940 (S3+S4+S7: $s0..$s7 unsaved), func_8004A76C (S7 $s0), func_8004A808 (S3+S4+S7 $s0) are STRONG (scan_func_8004A940.txt / _8004A76C / _8004A808). func_8004A4E0's grant therefore rests on the owner-ruled Q40 audit + this ledger, not on its own scanner tier. tools/ is outside the grinder surface, so adding an S9 "jtbl target outside referencing function" signal is an operator follow-up.
- New corpus census (tmp/grind/func_8004A4E0/s10/jtblscan.py -> jtblscan.txt): for every (referencing function, jtbl_ symbol) pair, table words read from the oracle EXE (bounded by declared C array size, else next jtbl symbol / first non-.text word), each target mapped to its containing function by asm/funcs glabel ranges. Compiled C: 58 pairs, 0 with any target outside the referencing function. Asm: 8 pairs, exactly 1 crosses — func_8004A4E0/jtbl_8001541C, 8/16 targets inside func_8004A940. This is the statistical complement of the s8 class kill (toplev.c:2647 / stmt.c:4704): the signal is absent from every compiled-C switch in the corpus and unique to this function. Side note: words after jtbl_8001545C[5] in the EXE are function-entry addresses (func_8004A940+0x11A4, func_8004BCC0, func_8004C994, ...) — consistent with the $s4/0x1F80001C handler table of the bytecode interpreter.
- cc1 dump tools (dump.ps1, loop_movables, nrefs_census, label_census) have no input: there is no C body to compile, only the INCLUDE_ASM stub.

- [s10] canonical gate still ASM-WHOLE; sandbox no diff on the INCLUDE_ASM stub (s10 re-audit)

- [s10] scan_hand_coded --single func_8004A4E0: tier LOW 0/8; helpers func_8004A940/A76C/A808 STRONG via S7

- [s10] jtbl targets outside the referencing function: compiled C 0/58 pairs, asm 1/8 (only func_8004A4E0/jtbl_8001541C)

- [s10] cc1 dump tools (dump.ps1, loop_movables, nrefs_census, label_census) have no input: no C body exists, only the INCLUDE_ASM stub

- [s11] func_8004A4E0 remains ASM-WHOLE (whole-function canonical-asm grant, inline_asm_canonical.txt:199); sandbox --disable all scores the INCLUDE_ASM stub at 163 insns, no_c_body=true, build_insns=0 (tmp/grind/func_8004A4E0/s11/sandbox.txt)

- [s11] No sibling ledger in this session's auto-attached list shares a code block (only global-data references) with func_8004A4E0's switch/jtbl chassis

- [s11] tools/decomp_me_scrape.py requires curl_cffi, which is not installed in .venv and has no locally cached corpus; infeasible this session without a package install (tmp/grind/func_8004A4E0/s11/decompme_probe.txt)

- [s11] The s8 class kill (jtbl_8001541C dispatches 8/16 entries into the MIDDLE of func_8004A940's compiled body, sharing live register state across a function boundary -- unreachable from any C switch/goto/fnptr lowering under GCC 2.7.2's per-function RTL/assembly model, toplev.c:2647 + stmt.c:4704) generalizes to rule out decomp.me's corpus as a rederive source even if it were reachable, since every decomp.me scratch is itself a single-function compiled-C target under the same model

## s12 (rederive) — decomp.me corpus search, now unblocked
- [s12] `pip install curl_cffi` succeeded in `.venv` (previously missing, per s11). `python3
  tools/decomp_me_scrape.py download --compiler gcc2.7.2-psx --limit 500` fetched 316 scratches
  to `tmp/decomp_me_corpus/`. `search --asm-file asm/funcs/func_8004A4E0.s` returned best
  similarity 0.057 (unrelated func_80044240), no transplant-viable candidate anywhere in the
  corpus. Saved: `tmp/grind/func_8004A4E0/s12/decomp_me_search.txt`.
- [s12] Re-derived the jtbl_8001541C cross-function-target finding independently (grep on
  `src/main/d_5BF0.c`): entries `0x8004A990, 0x8004ABD8, 0x8004A5D0, 0x8004AE0C, 0x8004AF84,
  0x8004B0C8, 0x8004B0E4, 0x8004B4D8, 0x8004B27C` are all outside func_8004A4E0's own span
  (`0x8004A4E0`-`0x8004A60C`); matches candidate.c's s8 note verbatim. `m2c.py` still refuses
  outright on the missing-jump-table error (identical to s8).
- [s12] `canonical func_8004A4E0` re-confirmed ASM-WHOLE this session (same reason string,
  asm_insns=7, total=163).

- [s12] pip install curl_cffi succeeded in .venv, resolving s11's tooling-dependency gap.

- [s12] tools/decomp_me_scrape.py download --compiler gcc2.7.2-psx --limit 500 fetched 316 scratches to tmp/decomp_me_corpus/.

- [s12] tools/decomp_me_scrape.py search --asm-file asm/funcs/func_8004A4E0.s returned best similarity 0.057 against an unrelated function; no candidate near transplant-viable range.

- [s12] Independently re-derived jtbl_8001541C's cross-function-target list from src/main/d_5BF0.c (0x8004A990, 0x8004ABD8, 0x8004A5D0, 0x8004AE0C, 0x8004AF84, 0x8004B0C8, 0x8004B0E4, 0x8004B4D8, 0x8004B27C), all outside func_8004A4E0's own span (0x8004A4E0-0x8004A60C) -- matches s8's banked finding exactly.

- [s12] m2c.py still refuses the body outright on the missing-jump-table error, identical to s8.

- [s12] canonical func_8004A4E0 re-confirmed ASM-WHOLE this session (same reason string, asm_insns=7, total=163); sandbox --disable all re-confirmed {"score": 163, "no_c_body": true, "build_insns": 0}.

- [s13] sandbox func_8004A4E0 --disable all --diff this session: {"score": 163, "no_c_body": true, "build_insns": 0}, identical to every prior session; diff prints "no diff: function is still whole-body INCLUDE_ASM".

- [s13] fake_ablate.py on memory/grind/func_8004A4E0/candidate.c: "no FAKE-annotated constructs found ... nothing to ablate" — the mandated re-audit has nothing to overturn.

- [s13] Mandated modality this session is structural, not escalation — per the standing auto-ruling the function is NOT disposed of here even though the frontier (s8-s12) already independently converged on an ASM-WHOLE class kill across 4+ prior modalities; this session adds the structural-axis-specific confirmation that structural levers have no applicable surface absent a C body.

- [s14] sandbox func_8004A4E0 --disable all --diff this session: {"score": 163, "no_c_body": true, "build_insns": 0}, identical to s1-s13; diff output: "no diff: function is still whole-body INCLUDE_ASM — there is no C to compare".

- [s14] canonical func_8004A4E0 re-confirmed ASM-WHOLE (verdict unchanged, asm_insns=7, total=163, reason "whole-function canonical-asm grant in inline_asm_canonical.txt"); state.json session_count was 13 entering this session, now 14.

- [s14] Mandated modality is structural (not escalation) for the 2nd time (s2, s14); per the standing auto-ruling (A) the driver's exhaustion call governs, not the agent's — this session re-measures rather than disposes. No new structural surface appeared: the chassis is still the unmodified INCLUDE_ASM stub (no_c_body:true), so block-local var splits / declaration order / type narrowing / statement re-association have literally nothing to act on, matching s2 and s13's identical finding across 2 intervening sessions (s12 rederive, no structural work in between).

- [s14] sandbox func_8004A4E0 --disable all --diff this session: {"score": 163, "no_c_body": true, "build_insns": 0}, identical to s1-s13; diff output: no diff, function is still whole-body INCLUDE_ASM.

- [s14] canonical func_8004A4E0 re-confirmed ASM-WHOLE this session (verdict unchanged, asm_insns=7, total=163).

- [s14] state.json shows 13 prior sessions across 6 distinct modalities (recon, structural x2 now, permuter, enumerate, synthesis, solver, forensics x3, rederive x2) all converging on the same s8 cross-function-jtbl-dispatch + custom callee-saved ABI class kill; no new rule scoping change found that would re-open any prior ban (judge-sole-gate is the only rule this ledger cites, and it does not bear on this function's grant).

- [s15] Fresh sandbox --disable all --diff this session (HEAD b9601d742) reproduces s2/s13/s14's exact score fields: {"score":163,"no_c_body":true,"build_insns":0,"cheat_asm_stripped":71}.

- [s15] Fresh canonical this session reproduces the same ASM-WHOLE verdict and reason string, citing inline_asm_canonical.txt's whole-function canonical-asm grant.

- [s15] 7 of the ladder's modalities (recon/structural/permuter/enumerate/synthesis/solver/forensics/rederive — now including this enumerate pass a second time) all independently land on the same class kill: no C body exists to apply the modality's technique to.

- [s15] The underlying class kill (func_8004A4E0's own jump table dispatches into the middle of func_8004A940's body, bypassing its prologue; no switch/goto/fnptr lowering under GCC 2.7.2's per-function compilation model can do this) is unchanged since s8 and re-cited at docs/audits/func_8004A4E0-canonical-2026-09-29.md:31.

- [s16] sandbox func_8004A4E0 --disable all --diff at HEAD e0b805366 returns {"score":163,"no_c_body":true,"build_insns":0,"cheat_asm_stripped":71} and prints 'no diff: function is still whole-body INCLUDE_ASM — there is no C to compare', identical to every prior session's fields since s2.

- [s16] src/main/3AB48.c is unchanged: INCLUDE_ASM("asm/funcs", func_8004A4E0); — no C statements exist anywhere in the body, so the enumerate modality's precondition (an existing candidate region to mark ENUM-BEGIN/END and feed to tools/spelling_enum.py) cannot be met.

- [s16] This is the third time the enumerate modality has been dispatched for this function (s6-ish earlier, s15, now s16) and all three measurements agree character-for-character with the s2 canonical/sandbox baseline.

- [s16] The underlying ASM-WHOLE canonical grant (docs/audits/func_8004A4E0-canonical-2026-09-29.md, inline_asm_canonical.txt:199, owner ruling Q40) remains the standing disposition; 6+ distinct modalities (recon, forensics x2, rederive x2, structural, enumerate x2 now x3) have all independently re-confirmed it with zero FAKE constructs and zero src edits.

- [s17] canonical func_8004A4E0 at HEAD 0dbd72780 returns verdict ASM-WHOLE citing inline_asm_canonical.txt whole-function grant, unchanged since s1.

- [s17] sandbox --disable all at HEAD 0dbd72780 returns {"score":163,"no_c_body":true,"build_insns":0}, identical to every prior session's measurement.

- [s17] src/main/3AB48.c remains a single-line INCLUDE_ASM("asm/funcs", func_8004A4E0); stub; memory/grind/func_8004A4E0/candidate.c is comments-only.

- [s17] No sibling ledger has a transplantable surface (confirmed again s11, re-verified unchanged this session — all 7 siblings closed/COMPLETED, 0 further sessions, data-only overlap).

- [s17] docs/grind/decisions.md has no entry naming func_8004A4E0 — not yet driver-declared exhausted; this session's mandated modality was enumerate, not escalation, so no owner-gated filing was made.
