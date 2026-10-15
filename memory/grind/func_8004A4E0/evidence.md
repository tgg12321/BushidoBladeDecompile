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
