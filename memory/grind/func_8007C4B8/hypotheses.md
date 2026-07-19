# Hypothesis ledger — func_8007C4B8

## s1 (recon, 2026-07-19) — frontier established

Baseline confirmed: sandbox --disable all = score 4, 164/164 insns, 1 rule dropped (regfix.txt:3619 `reorder 3,4,1,2 @ 1-4`), 150 cheat-asm insns stripped elsewhere. The 4-insn gap is EXCLUSIVELY the prologue save+def pair-flip (WIP notes confirmed against fresh objdump/asm read — RA is identical, only emit order of the two `sw sN, KK(sp); move sN, aN` pairs differs).

**Key policy shift discovered in the current session (not captured in state.json judge_constraints yet):** the 2026-06-15 disposition ("keep parked, no sanctioned lever") is SUPERSEDED by the OWNER RULING 2026-07-17 10:35 (docs/grind/decisions.md:646). The `param-local-alias-prologue-pair-flip` tombstone was NARROWED: the reversed-pair form stays forbidden, but a SINGLE forward-order FAKE-annotated param alias whose sole mechanism is combine's single-use entry-copy merge is sanctioned last-resort, with a per-use dossier. Scope explicitly includes "the twins func_8007C2A0 / func_8007C4B8 by the same per-function dossier standard when they reach the queue top."

The sanctioned form has been already used successfully on hirahira_w_frie (src/text1a_c.c:955, `s32 *b = base; /* FAKE ... */`, sandbox 0). That form is the DIRECT template for the primary frontier hypothesis below.

## Frontier

### H1 (primary) — single forward-order FAKE alias on `out` (arg0)

**Statement:** Adding `s32 *o = out; /* FAKE ... */` at the top of the function body, then rewriting every subsequent `out[N] = ...` / `((s8 *)out)+3` reference to use `o` instead, will land the target prologue pair order (s0-pair-first) via combine's single-use entry-copy merge — same mechanism as hirahira_w_frie 2026-07-17.

**Mechanism:** `out` is the FIRST param (a0). Currently `out` has 8+ direct uses in the body, so cc1's expand_function_start emits its entry copy `move s1, a0` early (LUID L, before r's `move s0, a1` at LUID L+1). Because save_restore_insns pairs each `sw sN` with its first def in LUID order, s1's pair emits before s0's. Aliasing `out` into a body-local `o` reduces `out` itself to ONE remaining use (the alias init). Combine's single-use merge (comb.c: substitute + dead-store elim) then relocates the arg0 entry copy INTO the alias init, which sits AFTER r's entry copy in linearized order — flipping which pair emits first. Twin-verified: hirahira_w_frie's dossier (memory/grind/hirahira_w_frie/, decisions.md 10:35 and 11:25) documents the same mechanism landing sandbox 0.

**Next probe (for a synthesis session, not this recon turn):** Apply the alias to src/display.c func_8007C4B8, run sandbox --disable all. Expected outcome: score 0. If not 0, capture the disassembly diff and reduce.

**Prerequisites the dossier must document before commit:** measured mechanism-level exhaustion of remaining sanctioned axes (K&R decl-block reversal, do-while(0) wrap of entry, initialized-vs-uninitialized decl permutation) — same axes hirahira's s2b measured; every one landed on floor 4 there. GCC pass named (cc1 combine, single-use entry-copy merge). Mandatory `/* FAKE: ... */` annotation naming pass + exhaustion pointer. Layer-1 + layer-2 reviewer PASS on the final form (default-FAIL).

### H2 (secondary, ranked lower) — single forward-order FAKE alias on `r` (arg1)

**Statement:** Aliasing arg1 (`Rect *_r = r;`) instead of arg0 might also flip the prologue pair, but the mechanism differs — instead of relocating arg0's copy LATER, it would relocate arg1's copy LATER (worse direction for us — target wants arg1's pair FIRST, not later).

**Mechanism:** Same combine merge but on the WRONG param. Expected to preserve the current pair order or worsen it. Included as a KILL-candidate — one measurement suffices to disprove.

**Next probe:** After H1 lands or fails, measure `Rect *_r = r;` alone for completeness. Expected: score >= 4 (unchanged or worse).

### H3 (structural fallback if H1 fails) — non-alias body restructures the WIP already killed

**Statement:** The three rejected forms in memory/grind/func_8007C4B8/rejected/ (v1_buf_reorder, v1_flag_cache, v2_flag_else_short) plus fleet-bw1's sibling cross-check already prove body-level restructures cannot shift the LUID of the move-sN-aN insns (they are set during expand_function_start BEFORE body RTL). This hypothesis space is DEAD — no further body variants to try.

**Verdict (already):** KILLED. Documented here so future sessions do not re-derive.

## Judge-constraint note

State.json currently carries no judge_constraints for this function. If H1 fails synthesis, this ledger should surface an owner-gated escalation citing decisions.md 10:35+11:25 and asking whether the twin sanction applies to a function whose alias would need to cover 8+ uses (vs hirahira's forward-order alias covering ~7 uses of `base` — similar order of magnitude, so no fresh policy question expected).

## [s1] Baseline honest floor for func_8007C4B8 is distance 4 with 1 rule dropped (regfix.txt:3619 reorder 3,4,1,2 @ 1-4), 164/164 build vs target insns — the 4-insn gap is entirely the prologue save+def pair-flip.
- mechanism: sandbox --disable all strips regfix rules + cheat-asm; the residual diff is only the two prologue pairs (sw s0/move s0,a1 and sw s1/move s1,a0) swapped in emit order. RA identical on both sides.
- probe: & tools/wteng.ps1 main sandbox func_8007C4B8 --disable all
- result: score=4, target_insns=164, build_insns=164, rules_dropped=1, cheat_asm_stripped=150
- verdict: CONFIRMED

## [s1] Owner ruling 2026-07-17 10:35 (decisions.md:646) NARROWED the param-local-alias-prologue-pair-flip tombstone — a single forward-order FAKE-annotated param alias is sanctioned last-resort for the twins func_8007C2A0 / func_8007C4B8, subject to a per-use dossier.
- mechanism: hirahira_w_frie sanction extended by explicit scope clause: 'the twins func_8007C2A0 / func_8007C4B8 by the same per-function dossier standard when they reach the queue top.' The sanction extends pointer-alias-fake-exception (SOTN `fakeEntity = self; // !FAKE`) from globals to parameters.
- probe: Grep decisions.md for func_8007C4B8; read lines 623-655.
- result: Ruling confirmed in tree; hirahira_w_frie applied same pattern successfully (final PASS 2026-07-17 11:25, src/text1a_c.c:955-1007, `s32 *b = base; /* FAKE ... */`).
- verdict: CONFIRMED

## [s1] The body-restructure hypothesis space is dead — the three rejected forms plus fleet-bw1's sibling cross-check prove move-sN-aN LUIDs are set in expand_function_start BEFORE body RTL, so no body-level variant can shift the prologue pair order.
- mechanism: cc1's expand_function_start walks param decls left-to-right assigning LUIDs; save_restore_insns pairs sw sN with first def in LUID order. Body RTL is generated later so cannot influence prologue LUIDs. Confirmed by v1_flag_cache measurement (score 52, +1 lbu added target lacks).
- probe: Reviewed memory/grind/func_8007C4B8/rejected/ and WIP meta.json sessions log.
- result: 3 disproven body variants + sibling cross-check confirm no body-level lever exists in this allocation class.
- verdict: KILLED

## [s2] K&R decl-block reversal (locals declared s32/s16/s16/s16/u16 vs baseline u16/s16/s16/s16/s32) shifts the prologue save+def pair emit order
- mechanism: cc1's expand_function_start walks PARAM decls (not local decls) to assign LUIDs to `move sN, aN`; save_restore_insns pairs `sw sN` with first def in LUID order. Local decl block is processed AFTER expand_function_start, so cannot shift param LUIDs.
- probe: Reverse local decl block, sandbox --disable all
- result: score=4 (unchanged), 164/164 insns, identical build.o — floor unchanged
- verdict: KILLED

## [s2] do-while(0) wrap of the entire function body shifts codegen enough to flip the prologue pair order
- mechanism: do-while(0) emits NOTE_INSN_LOOP_BEG at entry; sanctioned lever for reorg.c invert-jump peephole suppression (LABEL_OUTSIDE_LOOP_P). Could plausibly reshuffle prologue via loop-note-aware sched1.
- probe: Wrap full body in `do { ... } while (0);` sandbox --disable all
- result: score=27 (WORSENED from 4), same 164/164 insns — loop note reshuffles body pseudo allocation to different callee-saves without touching prologue pair order (LUIDs still set in expand_function_start before loop-note insertion)
- verdict: KILLED

## [s2] Initializing var_a3 at its declaration (`s32 var_a3 = 7;`) instead of the body-level assignment shifts prologue pair order via changed LUID for var_a3's first def
- mechanism: Move the `var_a3 = 7;` assignment from body to decl-time initializer; if GCC emits the `li $tN, 7` in a different position it might reorder save_restore_insns pairing decisions on adjacent pseudos.
- probe: Move `var_a3 = 7` to `s32 var_a3 = 7;`, remove body assignment, sandbox --disable all
- result: score=35 (WORSENED), build_insns=166 (+2 vs target 164) — init hoists `li` to prologue where target does it lazily, adding 2 spurious insns; prologue pair order unchanged (still wrong)
- verdict: KILLED
