# Evidence bank — func_8006E10C

- WIP rejected_form: {'form': 'base computed at top of function (base_top)', 'score': 21, 'reason': 'moves the lui s0 base materialization too early; worse'}

- WIP rejected_form: {'form': 'base_first / ff0_then_base / base_before_ff0_after orderings', 'score': 13, 'reason': 'do not flip the s0/s1 tie or the jal-delay-slot fill'}

- WIP rejected_form: {'form': 'ff0 = 0xF0 ASSIGNED INSIDE BOTH if-else branches (v2 et al)', 'score': 10, 'reason': "CHEAT (cheat-reviewer FAIL 2026-06-22): dead-conditional-store variant - both branches write identical constant; only purpose is to coerce GCC's scheduler to fill func_80036EA8's jal delay slot with `li $reg,240`. New spelling of volatile_cheats family."}

- WIP rejected_form: {'form': 'LITERAL 0xF0 in y-coord arg + variable ff0 in 5th arg of the SAME call pattern (v9-v14/v19-v21/v24/v25/v27)', 'score': 8, 'reason': "CHEAT (cheat-reviewer FAIL 2026-06-22): CSE-defeat-via-rewrite - mixing literal/variable spellings of the same value at different arg slots specifically to defeat GCC's CSE so it emits li a2,240 instead of move a2,ff0_reg. No human programmer would write this."}

- WIP rejected_form: {'form': 'base2 first via (s32)&D_800F7438 + 0x40EC (v22)', 'score': 14, 'reason': "computing base2 before base via independent expression worsens base's later materialization"}

- WIP rejected_form: {'form': 'drop temp_s3 variable, inline D_800A3500 twice (v26)', 'score': 17, 'reason': 'removes the eager lw $s3 load but introduces 17 other diffs; bi=102 vs target=103'}

- == imported from memory/wip notes.md ==
# func_8006E10C (text1b.c) — WIP / BLOCKED (register-alloc + scheduling wall, cheat-reviewer-confirmed)

## TL;DR
HEAD matches ONLY via 5 register-asm pins (`ff0 asm("$17")`, `temp_s3 asm("$19")`,
`a0v asm("$4")`, `a1v asm("$5")`) — cheat-asm. Clean pin-free body (candidate.c)
scores **13** on the honest distance: the residual is a coupled **s0<->s1 register
swap + jal-delay-slot fill** that GCC 2.7.2's global allocator picks deterministically.
The 2026-06-22 sweep found a 13->8 lever pair, but cheat-reviewer FAILED both
sub-levers as cheats-by-any-spelling — they are recorded in `rejected/` and MUST NOT
be re-derived. Endpoint: instrumented cc1 dumps (per register-alloc-pure-c §Order-of-attack)
or directed permuter from the clean base, or canonical-asm review.

## The gap (103-insn display-init function)
- Target: `base` (=&D_800F7438) in **$s0**, `ff0` (=0xF0/240) in **$s1**, `base2`(=base+0x40EC) in **$s2**, `temp_s3`(=D_800A3500) in **$s3**.
- Target's `li s1,240` is in the **func_80036EA8 jal delay slot**.
- GCC clean: `base` in $s0, `ff0` in **$s2**, `base2` in **$s1**, `temp_s3` in $s3.
- GCC clean: `nop` in the jal delay slot, `ff0` materialized later (deferred near first use).
- Per register-alloc-pure-c §Step-0: target's $s1 for ff0 is the PREFERRED (lower-numbered) reg —
  this build is the anomaly; something blocks $s1 for ff0 in the global allocno priority.

## Levers measured 2026-06-22 (28 variants via tools/sweep_variants.py)
| form | score | classification |
|---|---|---|
| clean (pin-free, no rewrites) | 13 | clean baseline |
| 4 decl-order swaps + ptr-form + base2-split | 13 | clean, no lever |
| short ff0 | 13 | clean, no lever |
| ff0 hoisted above if-else | 12 | clean, +regresses tail |
| base_top | 21 | clean, regresses prologue |
| base2 first via independent expression | 14 | clean, regresses base |
| drop temp_s3 var (inline D_800A3500 x2) | 17 (bi=102) | clean, regresses elsewhere |
| ff0 = 0xF0 INSIDE BOTH branches (v2 et al) | **10** | **CHEAT** (dead-conditional-store) |
| + LITERAL 0xF0 in 3 y-coord arg slots (v9 et al) | **8** | **CHEAT** (CSE-defeat-via-rewrite) |

## Why the 13->8 levers are cheats (cheat-reviewer 2026-06-22, FAIL)
1. `ff0 = 0xF0` written identically in BOTH if-else branches: a dead-conditional-store
   variant whose only purpose is scheduling coercion (puts `li $reg,240` adjacent to
   the func_80036EA8 jal so the assembler fills the delay slot). Mechanical detector
   `find_dead_conditional_stores` does not flag it (omits the canonical outer store),
   but it's the same family per `no-new-park-categories §cheats-by-any-spelling`.
2. Mixing LITERAL `0xF0` (y-coord arg) with VARIABLE `ff0` (5th arg, same value) at
   the same call pattern: CSE-defeat-via-rewrite. Forces GCC to emit `li a2,240`
   instead of `move a2, ff0_reg`. No human programmer would mix the two spellings.

Both sub-levers were necessary for the 13->8 drop. Saved to
`rejected/ff0_split_branches_and_literal_mix.c` (named for both violated rules).

## Clean levers that DID NOT help (do not re-derive)
- base_pointer ptr form for base
- block-local split for base2's last use (Lever A)
- base2 precomputed alongside base (multiple positions)
- base2 omitted entirely as a source variable (GCC's CSE recreates the same pseudo)
- ff0 = 0xF0 hoisted above the if-else (scored 12 — worse)
- short / u8 / different type for ff0

## Endpoint
Per register-alloc-pure-c §Order-of-attack:
1. Instrumented cc1 dumps (`BB2_ALLOC_DEBUG` / `BB2_PRIO_DEBUG`, separate `tmp/gccdbg/cc1` build)
   to identify which pseudo wins $s1 across base2's live range vs ff0's. The allocno
   priority tiebreak that picks base2 over ff0 for $s1 is the question.
2. Directed permuter from the clean pin-free base — PERM_VAR over (ff0/base2) reg-asm
   hints as DIAGNOSTIC ONLY per register-asm-pins.md; the closing form MUST drop the
   pin and rely on C structure. NOTE: 103 insns + many jal relocations -> isolated
   permuter scores unreliable; any find MUST be SHA1-verified in the full build.
3. If both wall: sibling-class to cpu_side_move_dir_4 / marionation_Exec (the GCC 2.7.2
   global.c:624 allocno-priority tiebreaker wall — see register-alloc-deep-dive).
   Park with instrumented evidence.


- == s1 (2026-07-27, recon) ==
- CORRECTION to WIP map: current clean build allocates base->$s1, ff0->$s0 (reused after v0 dies), base2->$s2 — NOT "ff0 in $s2, base2 in $s1" as the WIP notes claimed. Target: v0->$s0 then base->$s0 (s0 REUSED across two values), ff0->$s1, base2->$s2.
- Both target AND clean build emit literal `addiu $a2,0xF0` for the y-coord args of gpu calls 2/3 — cse constant-propagates into arg regs naturally; NO literal/variable mixing needed in source (the killed v9-form cheat was never necessary for that part).
- MECHANISM (dump-proven, tmp/grind/func_8006E10C/s1/text1b.i.*): insn 48 `(set (reg 72) (const_int 240))` sits before call 55 (func_80036EA8) through rtl/jump/cse/loop/cse2/flow/combine; the SCHED (sched1) dump shows it moved to just before call 95 (first gpu_InitDrawEnv), LOG_LINKS (nil) = zero dependencies. sched1's constant-sink-to-first-use is the single root cause: shrunk live range -> no conflict with v0's pseudo -> greg gives reg72 $s0 -> base pushed to $s1 -> all 13 diffs cascade.
- The whole post-if-else region is ONE basic block (no branches after label .L8006E178), so NO source statement ordering can anchor the li — explains why all 5+ 2026-06-22 ordering variants flatlined at 13.
- LEVER FOUND (sanctioned): single-level `do { ff0 = 0xF0; } while (0); /* FAKE */` before the call (do-while-zero-exception, owner ruling 2026-07-06 sanctions ANY codegen effect). Loop notes fence the sched1 sink -> li stays at the jal -> ff0 conflicts with v0 -> ff0=$s1, reorg fills the jal delay slot with `li s1,240`, base shares $s0 with v0. SANDBOX --disable all = 0 (103/103 insns), verified li in delay slot.
- Label-drift check: only text1b function AFTER func_8006E10C with label-referencing rules is func_80070C70, whose rules use drift-proof `rename "{lbl#N}"` capture; all literal-.L rules (tslPrintScreen .L26, SetPacketData .L761, saTan4FireDisp .L272, AllocRobRmd .L61, mk_leaf_newpos .L631) are in other files or earlier in text1b. Wrap's extra label numbers are low-risk for the full build.
