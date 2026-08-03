# Evidence bank — func_80060544

Function: `func_80060544(s32 arg0, s32 arg1)` in `src/text1b.c` (currently at line 13114).
Target asm: `asm/funcs/func_80060544.s`, 133 instructions.
Canonical gate: **C** (`asm_insns 0`, "pure-C distance 18 <= 50 — pure-C target").
`diagnose` verdict at session start: LARGE (d29 / 27 differing insn) — that number is
computed against the *rules-enabled* reference and is NOT the honest floor; the honest
cheat-free floor is what `sandbox --disable all` prints.

## Floor history
| session | floor (`sandbox --disable all`) | build_insns | note |
|---|---|---|---|
| s1 start | **18** | 134 | 11 regfix rules dropped by the sandbox, 324 cheat-asm insns stripped file-wide |
| s1 end   | **4**  | 134 | three levers landed; edits in place in `src/text1b.c` |
| s2 start | **18** | 134 | **the s1 edits were NOT in the tree** — src/text1b.c was at the session-start form. Re-applied `candidate.c` first (back to 4), then probed. |
| s2 end   | **2**  | **133** | the paired m2c-carrier deletion (H4 closed); build_insns now EQUALS target_insns for the first time |

## What the function does (shape, from target asm + the C)
Two sequential loops that fill a 0x28-byte stack struct (`S544 s` at `sp+0x18`) and hand
`&s` to `func_8007352C` each iteration, chaining the returned value into the next
iteration's `s.arg1_field` (a display-primitive linker: each call returns the next
primitive pointer). Loop 1 runs `i = 0..3` over a 3-word-stride geometry table
(`D_8009B770`, stride 0xC) and selects a per-`i` static table
(`i==0 -> D_8009B7D8`, `i in {1,2} -> D_8009B800`, `i==3 -> D_8009B7D0`); the `i==3` arm
additionally calls `func_80073728(&s, 0)` and skips the `func_8007352C` call. A one-off
`func_8007352C` with `D_8009B7A0` / `D_8009B820` follows, then loop 2 runs `j = 0..1`
over walking pointers `D_8009B3B0` (stride 0xC) and `D_8009B840` (stride 8). The tail is
`initTexPage(arg0+0x5DC, 1, 0, saMotionSet(s.p_geom, 0), 0)` then
`ot_Link(D_800A374C + arg1*4, arg0+0x5DC)`, returning `(arg0+0x5F4) - arg0`.

## The 11 regfix rules the sandbox drops (what they were papering over)
- `reorder 17,18,15,16 @ 15-18` — prologue init-order: target emits `$s5 = 3` BETWEEN the
  `i` and `idx` zero-inits; our literal-3 build emitted it after both. **Closed by the
  `last = 3` lever (H3).**
- 6x `subst la $3 -> $2` / `sw $3 -> $2` on `D_8009B770` / `D_8009B7D8` / `D_8009B800` /
  `D_8009B7D0` plus `addu $2,$17,$3 -> $2` — the $v1-instead-of-$v0 address-pseudo
  cluster. **Closed by the geom split-init (H1) and the shared `stat` local (H2).**
- `delete @ 77` — the extra `move $s5,$s7`. **Still open (H4).**
- `subst sll $4,$21,2 -> sll $4,$23,2` — the $s5/$s7 consequence of the same extra copy.
  **Still open (H4).**

## Measured facts (session s1, 2026-08-03)
1. **The address-materialisation pseudo splits from its consumer when the whole address
   expression is written as ONE expression.** `s.p_geom = (s32 *)((s32)&D_8009B770 + idx);`
   emits `lui $v1 / addiu $v1 / addu $v0,$s1,$v1`; target emits `lui $v0 / addiu $v0 /
   addu $v0,$s1,$v0`. Rewriting as split-init accumulation
   (`geom = (s32)&D_8009B770; geom += idx; s.p_geom = (s32 *)geom;`) makes it one
   self-updating pseudo, which local_alloc puts in $v0. Floor 18 -> 15.
   Sanctioned family: [[split-init-accumulation-sanctioned]].
2. **The same mechanism governs the three per-arm static-table stores.** Three direct
   `s.p_static = &D_800...;` stores in three branch arms produce three separate pseudos,
   all allocated $v1. Routing all three through ONE shared named local
   (`stat = (s32)&D_800...; s.p_static = (s32 *)stat;`) collapses them to one pseudo that
   gets $v0, matching target in all three arms. Floor 15 -> 8. This is a *cheap, general*
   finding: it is lever (1) applied to a multi-arm select, and it is worth trying on any
   function whose regfix cluster is a pure `la $3 -> $2` rename family.
3. **Target's `3` was set in the C source, not hoisted by loop.c.** Target's prologue is
   `addu $s0,$zero,$zero` (i=0) / `addiu $s5,$zero,0x3` / `addu $s1,$zero,$zero` (idx=0).
   A CSE'd loop-invariant constant cannot land there: loop.c emits preheader movables
   immediately before the loop start, i.e. after BOTH inits — which is exactly what the
   literal-3 C produced. Writing `last = 3;` between `i = 0;` and `idx = 0;` and using it
   for the two equality tests (`i == last`, `i != last`; the `i < 3` / `i < 4` tests stay
   `slti` immediates and are unaffected) reproduces target's prologue exactly.
   Floor 8 -> 4. See candidate.c's header for the cheat-family note on `last`.
4. **`new_var` (the `arg1` carrier for the `ot_Link` index) is LOAD-BEARING.** Removing it
   regresses 18->48 from the old base and 4->10 from the new base; hoisting its init to
   the top regresses 8->14; additionally routing `s.arg2_field` through it also gives 14.
   The documented [[drop-param-alias-local]] recipe does NOT apply to this function.
   Details + exact edits in `rejected/`.

## Artifacts
- `tmp/grind/func_80060544/s1/pairs.py` — prints the instruction-level diff between the
  cheat-invisible sandbox object (`tmp/sandbox/func_80060544/text1b.o`) and the
  byte-matching reference (`build/src/text1b.o`) via `engine.diagnose.diff_pairs`.
  This is the gradient microscope for this function; re-run it after every edit.
- `tmp/grind/func_80060544/s1/pairs_floor4.txt` — the diff at the floor-4 state.
- `tmp/grind/func_80060544/s1/diff.sh` — an earlier objdump-vs-`.s` text differ. It is
  NOT useful (objdump mnemonics vs splat `.s` mnemonics never line up); use pairs.py.

## Reading the pairs.py output
Every `beqz/bnez/j/beq` hunk in the output is ADDRESS NOISE, not a real diff: the sandbox
object strips 324 cheat-asm instructions file-wide, so `func_80060544` sits at a different
offset than in the reference object and every branch/jump literal differs. The engine's
score masks these. Only non-branch hunks are real.

- [s1] Canonical gate: verdict C, asm_insns 0, 133 target instructions - a pure-C target, no canonical-asm question anywhere in this function.

- [s1] Honest cheat-free floor moved 18 -> 4 this session; build_insns 134 vs target 133 throughout (the single extra insn is the `move $s5,$s7` in H4).

- [s1] The 11 regfix rules the sandbox drops decompose exactly into the three mechanisms found: 1 prologue `reorder` (closed by H3), 7 `la`/`sw`/`addu` $v1->$v0 substs (closed by H1+H2), and `delete @ 77` + the `sll $4,$21,2 -> $4,$23,2` subst (the still-open H4 pair).

- [s1] `engine diagnose` reports LARGE / d29 / 27 differing insns for this function - that figure is measured against the rules-enabled reference and is NOT the honest floor. Do not use it as a gradient; `sandbox --disable all` is the gradient.

- [s1] The gradient microscope for this function is tmp/grind/func_80060544/s1/pairs.py (engine.diagnose.diff_pairs between tmp/sandbox/func_80060544/text1b.o and build/src/text1b.o). Every branch/jump hunk it prints is address noise - the sandbox strips 324 cheat-asm insns file-wide, so the function sits at a different offset in the two objects. Only non-branch hunks are real.

- [s1] A plain objdump-vs-splat-`.s` text diff (tmp/grind/func_80060544/s1/diff.sh) is useless here: objdump mnemonics (`move a1,zero`) never line up with the splat `.s` spelling (`addu $a1, $zero, $zero`). Do not rebuild that wheel.

- [s1] Generalisable finding: a regfix cluster consisting purely of `subst "la $3,SYM" "la $2,SYM"` renames is a signature for the split-address-pseudo problem, and both fixes (split-init accumulation, one shared named carrier across branch arms) are sanctioned pure-C forms. Worth sweeping the rule corpus for that shape.

## Measured facts (session s2, 2026-08-03, modality: structural)

5. **PROCESS — the s1 edits were not persisted.** `src/text1b.c` was found at the
   floor-18 session-start form. `memory/grind/func_80060544/candidate.c` is the only
   copy of the work; re-apply it and re-measure BEFORE probing. (Cost this session
   ~2 turns; see [[grinder-stale-digest-uncommitted-ledger]] for the sibling failure.)

6. **H4 IS CLOSED. The extra `move $s5,$s7` + `sll $a0,$s5,0x2` disappear when TWO
   redundant m2c carrier variables are deleted TOGETHER — and only together.**
   - (a) `new_var` (the `arg1` carrier read by the `ot_Link` index).
   - (b) `new_var4`, ONE HOP of the *end_off* carrier chain
     (`end_off -> new_var6 -> new_var4 -> a reassignment of new_var3 -> return`),
     together with the `new_var3 = new_var4;` reassignment, leaving
     `end_off -> new_var6` and `return new_var6 - arg0;`.
   Measured lattice from the floor-4 base (`sandbox --disable all`):
   | form | score | insns |
   |---|---|---|
   | baseline (s1 candidate) | 4 | 134 |
   | drop `new_var` only | 10 | 134 |
   | collapse end_off chain to ONE pseudo only | 13 | 134 |
   | keep one hop (`new_var6`) only | 11 | 134 |
   | `end_off` straight into `new_var3` only | 14 | 134 |
   | collapse-to-one + drop `new_var` | 6 | 133 |
   | `end_off`->`new_var3` + drop `new_var` | 12 | 133 |
   | **one hop (`new_var6`) + drop `new_var`** | **2** | **133** |
   Mechanism: the two pseudos are in a callee-save PRIORITY RACE. Target puts arg1
   in `$s7` and end_off in `$s8`. Dropping `new_var` alone flips them (arg1 -> `$s8`,
   end_off -> `$s7`) and grows a *different* spurious `move $s5,$s7` — which is
   exactly why s1 recorded H4a as KILLED. Shortening the end_off chain lowers that
   allocno's ref count so it loses the race, and arg1 keeps `$s7`. The chain has an
   OPTIMUM LENGTH of exactly two pseudos: both shorter (6) and longer (12) lose.
   **Generalisable:** an m2c carrier-variable chain is a TUNABLE, not noise — when a
   param-alias deletion regresses because of a callee-save flip, the fix is to
   re-length a *neighbouring* carrier chain, not to put the alias back. This is the
   correction to the [[drop-param-alias-local]] KILL recorded in s1: that recipe DOES
   apply here, it just needed the paired edit.

7. **The residual 2 is ONE scheduling placement, and the instruction count now
   matches.** `target_insns=133 built_insns=133`; the only non-branch hunk is a
   delete/insert pair for `move a1,zero` in the `Case3` arm.
   Target's Case3 block (`asm/funcs/func_80060544.s:60-66`):
   `addu a1,zero,zero / lui v0 / addiu v0 / sw v0,0x1C(sp) / jal / [sw s2,0x24(sp)]`,
   with `addiu a0,sp,0x18` (the struct-pointer argument) in the delay slot of the
   PREDECESSOR branch (line 47) — reorg pulls it out of the top of this block in
   BOTH builds. So target's pre-reorg block order was
   `[a0=&s, a1=0, la, sw p_static, sw pad0C, jal]` — both argument set-ups adjacent
   at the FRONT, ahead of the p_static store — while ours is
   `[a0=&s, la, sw p_static, a1=0, sw pad0C, jal]`, with the a1 set-up glued to the
   call.

8. **H5 is dead on every C-level structural axis.** Statement reordering inside the
   arm is strictly worse (pad0C-first 4, p_static-last 4, reversed equality test 3,
   materialising the la in the predecessor block 4, Case3 storing the table address
   directly 39/insns 136). Call-argument respelling is completely INERT — hoisting
   either or both arguments into named locals, routing the call result through an
   extra pseudo, routing the stored value through an extra pseudo: all measure
   exactly 2/133, unchanged. A function-wide named `S544 *` for `&s` is catastrophic
   (44 and 38, insns 137 — it forces a callee-save home for `&s` instead of the
   per-call `addiu $a0,$sp,0x18`). Banked in `rejected/case3-arm-statement-reorderings.c`
   and `rejected/case3-call-arg-respellings.c`.

## Tooling notes (s2)

- `tmp/grind/func_80060544/s2/sweep.py` is the session's workhorse: it splices a
  mutated copy of the function into `src/text1b.c`, runs the sandbox, records the
  score, and always restores the file. `--dump <variant>` additionally prints the
  real (non-branch-noise) diff hunks. Variants are (old, new) substring pairs and a
  stale `old` is a hard error, so a variant can never silently measure the base.
- **`engine.pipeline.sh` interpolates paths into an UNQUOTED shell string, and this
  repo's absolute path contains spaces.** Always pass repo-RELATIVE paths to
  `engine.score` / `engine.diagnose` helpers; an absolute path yields a bogus
  `KeyError: func not found in <path>`. Cost ~3 turns to diagnose.
- `build/src/text1b.o` is NOT a durable reference — a stale tree can leave it without
  the symbol. `tmp/grind/func_80060544/s2/mkref.sh` builds a trustworthy one: it
  swaps the pristine `git show HEAD:src/text1b.c` in, runs a full `make` (which also
  re-verifies the oracle: "OK: bb2 matches!"), copies `build/src/text1b.o` to
  `s2/ref.o`, and restores the working tree via an EXIT trap.

- [s1] Judge note on the surviving `last = 3;`: it is a LIVE, twice-read local (not a dead store), and reads naturally as `index of the last/special-cased element`, but it is a constant-holder and therefore sits in the [[named-local-fake-exception]] family. It is deliberately un-annotated pending a ruling; if the Judge wants the /* FAKE */ annotation plus documented lever exhaustion, that is a one-comment fix.

- [s2] PROCESS: src/text1b.c was found at the floor-18 SESSION-START form at the top of s2 — the s1 edits were not persisted to the tree. memory/grind/func_80060544/candidate.c was the only copy. Re-apply candidate.c and re-measure BEFORE probing; it cost ~2 turns to notice. Sibling failure mode: [[grinder-stale-digest-uncommitted-ledger]].

- [s2] The honest cheat-free floor moved 4 -> 2 this session, and build_insns moved 134 -> 133, which EQUALS target_insns (133). Every register, every store and every delay slot in the function now matches target; the entire residual is one instruction's PLACEMENT.

- [s2] The only non-branch diff hunk at floor 2 is a delete/insert pair for `move a1,zero` in the Case3 arm (tmp/grind/func_80060544/s2/pairs_floor2.txt).

- [s2] Target's Case3 block (asm/funcs/func_80060544.s:60-66) is `addu a1,zero,zero / lui v0 / addiu v0 / sw v0,0x1C(sp) / jal func_80073728 / [sw s2,0x24(sp)]`, and the `addiu a0,sp,0x18` that supplies the struct-pointer argument sits in the DELAY SLOT OF THE PREDECESSOR BRANCH (line 47) — reorg pulls it out of the top of this block in BOTH builds. So target's pre-reorg block order was [a0=&s, a1=0, la, sw p_static, sw pad0C, jal] — both argument set-ups adjacent at the FRONT, ahead of the p_static store — while ours is [a0=&s, la, sw p_static, a1=0, sw pad0C, jal], with the a1 set-up glued to the call.

- [s2] GENERALISABLE FINDING: an m2c carrier-variable chain is a TUNABLE, not noise. When deleting a param-alias local regresses the score because the callee-save assignment FLIPS between two long-lived pseudos, the fix is to re-length a NEIGHBOURING carrier chain (changing that allocno's ref count and hence its priority), not to put the alias back. This is the correction to the [[drop-param-alias-local]] KILL that s1 recorded: the recipe does apply to this function, it just needed the paired edit. Worth trying on any queue item where a clean alias deletion regresses with a register-role swap in the diff.

- [s2] TOOLING GOTCHA (cost ~3 turns): engine.pipeline.sh interpolates paths into an UNQUOTED shell string, and this repo's absolute path contains spaces. Always pass repo-RELATIVE paths to engine.score / engine.diagnose helpers — an absolute path produces a misleading `KeyError: <func> not found in <path>` even when objdump shows the symbol is present.

- [s2] TOOLING: build/src/text1b.o is NOT a durable diff reference (a stale tree can leave it without the symbol). tmp/grind/func_80060544/s2/mkref.sh builds a trustworthy one — it swaps the pristine `git show HEAD:src/text1b.c` in, runs a full `make` (which re-verified the oracle this session: `OK: bb2 matches!`), copies build/src/text1b.o to s2/ref.o, and restores the working tree via an EXIT trap.

- [s2] The `last = 3;` constant-holder local from s1 is unchanged and still un-annotated, pending the Judge's view; it is a live, twice-read local that reads as `index of the last/special-cased element`. If the Judge wants the /* FAKE */ annotation plus documented lever exhaustion, that is a one-comment fix.

## Measured facts (session s3, 2026-08-03, modality: structural)

9. **PROCESS (second occurrence).** `src/text1b.c` was AGAIN found at the
   floor-18 session-start form; the s2 edits were not persisted. Re-applying
   `candidate.c` restored score 2 / insns 133 immediately. This is now a
   reliable feature of the pipeline, not an accident:
   `tmp/grind/func_80060544/s3/apply_candidate.py` splices candidate.c's
   function body into `src/text1b.c` in one command — run it FIRST every
   session (see [[grinder-stale-digest-uncommitted-ledger]]).

10. **The m2c goto-ladder dispatch is not one spelling among many — it is the
    only dispatch shape that reproduces target's block structure.** Every
    natural-C reshape of the `i` dispatch changes the instruction COUNT (which
    had been sitting exactly on target's 133) and regresses hard:
    if/else-if chain 18 (insns 126), if/else with the `i == last` arm first 22
    (122), real `switch (i)` 36 (137), goto ladder with the `i == last` test
    hoisted to the front 14 (126). The four-way `i<3 / i>0 / i==0 / i==last`
    ladder with three labelled arms and a shared `Skip:` join is what the
    original compiled to.

11. **The loop shape is likewise settled.** `while (i < 4) { ... }` and
    swapping the two increments (`idx += 0xC;` before `i += 1;`) are exactly
    INERT (2 / 133). Moving the `idx` increment to the top of the `Skip` block
    gives 4 (insns 132) and a `for (i = 0; i < 4; i++)` header gives 6 — the
    latter also destroys the s1 `last = 3` prologue lever, which requires
    `i = 0;` and `idx = 0;` to be separate statements with `last = 3;` between
    them.

12. **Declaration order, carrier type and the call's cast are all INERT
    (2 / 133).** `s32 *stat` instead of `s32 stat` + casts; `stat` declared
    first; `stat` declared last; and `func_80073728((GameObj *)(&s), 0)` — the
    spelling every OTHER call site of this callee in text1b.c uses — all
    measure exactly the base. Together with s2's five inert call-argument
    respellings, the C-level surface around this basic block is now measured
    flat in every direction we can name.

13. **FRONTIER PROBE (b) IS KILLED: the `move a1,zero` placement is NOT a
    sched1 dependence-height tie.** Raising the height of the chain feeding
    `sw p_static` inside the arm by one insn (`stat += idx;`, 3 / 134) and by
    two (`stat += idx; stat += prev;`, 4 / 135) inserts the new insns BEFORE
    the a1 set-up and does not move it: at chain heights 1, 2 and 3 the a1
    set-up stays pinned to the slot immediately preceding the `sw` that
    precedes the `jal`. A priority tie would have flipped by height 2.
    (`stat += i - last;` and `stat += (i - last) * 2;` measure 2 / 133 because
    `i == last` is provable inside the arm and GCC folds them away entirely —
    they are not evidence either way.) Banked in
    `rejected/sched-height-probes-diagnostic.c`; these forms are diagnostics,
    never candidates.

14. **THE COUNTER-EXAMPLE — our own build DOES emit args-first for this
    callee.** `func_8005D46C` (src/text1b.c:12718) calls `func_80073728` twice
    with the same shape (a run of struct-field stores, then
    `ret = func_80073728((GameObj *)(&s), 0);`), and the cheat-free sandbox
    build of THAT function emits `addiu a0,sp,16 / move a1,zero` as the first
    two insns of the call's basic block, ahead of the whole store cluster —
    exactly what func_80060544's target does and our build does not. So this
    is not a toolchain limitation and not a property of the callee: it is a
    property of THIS basic block. The visible structural difference is size and
    chain depth: the sibling's block is ~20 insns with several independent
    chains of height >= 4 and nine field stores; the Case3 block is five insns
    with one chain of height 2. Full objdump excerpts in
    `tmp/grind/func_80060544/s3/results.txt`.

- [s3] The exact question for the next (forensics) session is now narrow: in the backward list-scheduling pass, ours picks the `a1` set-up over the address chain's insns at the step right after `sw p_static` is placed, while target's build picks the chain. rank_for_schedule (tools/gcc-2.7.2/sched.c:2399) compares INSN_PRIORITY first, then a 3-way class relative to last_scheduled_insn (data-dependent = 1 worst, anti/output = 2, independent-or-latency-1 = 3 best), then INSN_LUID (higher LUID preferred). The chain insns are DATA-dependent on the just-placed `sw` (class 1); the a1 set-up is independent (class 3) and, being emitted by expand_call after the stores, also has the HIGHER LUID. So both tiebreaks favour a1 — which means target's build must have won on INSN_PRIORITY outright, and s3 measured that raising the chain's height does NOT achieve that. Read the `.sched` dump and the INSN_PRIORITY values for that block; that is the one fact still missing.

- [s3] cc1 in this tree is already instrumented with env-gated scheduler diagnostics: `BB2_RANK_DEBUG` makes rank_for_schedule print `RANKDBG last=<uid> y=<uid> cls=<n> x=<uid> cls2=<n> val=<n>` for every comparison (tools/gcc-2.7.2/sched.c:2436-2446). That is the cheapest possible instrument for the remaining question and it needs no new tooling. NB [[instrumented-cc1-location]]: the instrumented binary is tools/gcc-2.7.2/cc1, not tools/gcc-2.7.2/build/cc1.

- [s3] src/text1b.c was AGAIN found at the floor-18 session-start form (second consecutive session); memory/grind/func_80060544/candidate.c remains the only durable copy. tmp/grind/func_80060544/s3/apply_candidate.py now re-applies it in one command - run it FIRST every session.

- [s3] Base re-established and re-verified at the end of the session: `sandbox func_80060544 --disable all` = score 2, build_insns 133, target_insns 133, with the candidate form in place in src/text1b.c.

- [s3] The single non-branch diff hunk is unchanged: a delete/insert pair for `move a1,zero`. Ours emits [lui, addiu, move a1,zero, sw v0,0x1C(sp), jal, delay sw s2]; target emits [addu a1,zero,zero, lui, addiu, sw, jal, delay sw]. In both builds reorg steals the `addiu a0,sp,0x18` from the top of the block into the predecessor branch's delay slot.

- [s3] C-level structural surface for this block is now measured flat in every named direction: intra-arm statement order (s2), call argument/result respelling (s2), dispatch shape (s3), loop shape (s3), declaration order (s3), carrier type (s3), call-site cast (s3), dependence height (s3).

- [s3] Scheduler mechanics for the remaining question, read from tools/gcc-2.7.2/sched.c:2399 rank_for_schedule: comparison order is (1) INSN_PRIORITY, (2) a three-way class relative to last_scheduled_insn - data-dependent = 1 (worst), anti/output = 2, independent-or-latency-1 = 3 (best), (3) INSN_LUID with the HIGHER LUID preferred. The address-chain insns are data-dependent on the just-placed `sw` (class 1) and the a1 set-up is independent (class 3); the a1 set-up, emitted by expand_call after the stores, also holds the higher LUID. Both tiebreaks favour a1, so target's build must have beaten it on INSN_PRIORITY outright - and s3 measured that raising the CHAIN's height does not achieve that, which points at a1's own priority (or its presence in the ready list) rather than the chain's.

- [s3] tools/gcc-2.7.2/sched.c carries env-gated instrumentation: BB2_RANK_DEBUG=1 makes rank_for_schedule print `RANKDBG last=<uid> y=<uid> cls=<n> x=<uid> cls2=<n> val=<n>` for every comparison (sched.c:2436-2446), and the string is present in the built tools/gcc-2.7.2/cc1 binary (per [[instrumented-cc1-location]], that is the instrumented one, NOT tools/gcc-2.7.2/build/cc1). No new tooling is needed for the next probe.

- [s3] Rejected forms banked this session: memory/grind/func_80060544/rejected/case3-dispatch-and-loop-reshapes.c (dispatch + loop + declaration/type/cast axes, with every measurement) and rejected/sched-height-probes-diagnostic.c (the dependence-height diagnostics, explicitly marked never-candidates).

## Measured facts (session s4b, 2026-08-03, modality: permuter)

15. **PROCESS (third consecutive occurrence).** `src/text1b.c` was again at the floor-18
    session-start form. `python3 tmp/grind/func_80060544/s3/apply_candidate.py` restored it
    and `sandbox func_80060544 --disable all` re-measured **score 2 / build_insns 133 /
    target_insns 133** before any probing. Run it FIRST, every session.

16. **THE PERMUTER WORKSPACE NOW EXISTS, IS VALIDATED, AND IS REUSABLE: `tmp/perm_60544`.**
    Built by `tmp/grind/func_80060544/s4/mkws.sh` from the reduced TU
    `tmp/grind/func_80060544/s4/base_src.c` (the candidate function plus the struct
    typedef and the `D_*` externs). Validation gate, re-run by mkws.sh on every build: the
    reduced-TU build of `func_80060544` is instruction-identical to the cheat-free sandbox
    build — 133 insns, and the ONLY diff against `target.o` is the single `move a1,zero`
    displacement. So **permuter base score 60 == sandbox floor 2**, and the permuter's
    gradient is the honest gradient. The compile.sh in the workspace deliberately OMITS the
    regfix / regfix_stage2 / asmfix stages, which is exactly what `--disable all` does.
    Two defects had to be fixed before the chassis was usable — both cost the FIRST s4
    campaign its life, and both are recorded here so no future session pays again:
    - (a) **The implicitly-declared callees need prototype-less `extern s32 f();`
      declarations** (`func_8007352C`, `func_80073728`, `initTexPage`, `saMotionSet`,
      `ot_Link`). They are codegen-INERT — an old-style `()` declaration is precisely what
      an implicit declaration produces, and the post-fix build re-passed the
      instruction-identical validation gate — but without them decomp-permuter's pycparser
      typemap dies with `KeyError: 'func_8007352C'` the instant a mutation has to type a
      call expression. That traceback is what killed the first s4 campaign after a handful
      of iterations (see the tail of the old `campaign.log`).
    - (b) **`mktemp /tmp/...` is FLAKY in this WSL instance** — it twice returned an empty
      string, or a path that had already vanished by the next command in the same script,
      producing the misleading `awk: fatal: cannot open file '/tmp/p60544XXXXXX.s'`.
      `compile.sh` now uses a deterministic workspace-local scratch path
      (`tmp/perm_60544/work/p$$.s`). Prefer that pattern in any new per-function chassis.

17. **CAMPAIGN A — directed "no-staging" chassis: DRY.** 31,745 iterations / 20.5 min at
    `-j 6`, launched via `tools/permuter_campaign.py` (label `no-staging-A`). The entire
    extra-assignment / staging mutation family was zeroed in `settings.toml`
    (`perm_temp_for_expr`, `perm_duplicate_assignment`, `perm_chain_assignment`,
    `perm_long_chain_assignment`, `perm_add_self_assignment`, `perm_inline` = 0.0), on the
    reasoning that the only known score-0 form is a staging carrier the Judge already
    FAILed, so removing that family forces the search into structurally different space.
    Result: **no find below base.** One sideways find at score 60 (== base) after 80 s;
    nothing else in 31.7k iterations. Log: `tmp/grind/func_80060544/s4/campaignA_no_staging.log`.

18. **CAMPAIGN B — full mutation space: ALSO DRY.** 32,075 iterations / 23 min at `-j 6`
    (label `full-space-B`), defaults everywhere except `perm_inline = 0.0`; i.e. the
    staging family fully re-enabled, the same space in which the first s4 campaign found
    its score-0. Result: **no find below base.** One sideways find at score 60 after 268 s
    — and its function body is textually IDENTICAL to campaign A's 60-find, so both
    chassis converged on the same equal-score alternative spelling of the same single
    misplacement. Log: `tmp/grind/func_80060544/s4/campaignB_full_space.log`.

19. **COMBINED RESULT — the permuter modality is MEASURED DRY for this function.**
    63,820 iterations from the floor-2 chassis produced no score-0 and nothing at all
    between 0 and 60 (the metric is quantised here: 60 == the one reordering, so any
    partial improvement was impossible by construction — either the `move a1,zero` lands
    right, or it does not). The ONLY route to 0 that anyone has ever found remains the
    first-s4 find `end_off = stat; s.p_static = (s32 *)end_off;` — a dead store into an
    already-consumed local — which the Judge FAILed and which is banked in
    `rejected/judge-fail-0803-1310.c`. Random search over the full GCC-oriented mutation
    pass set does NOT reach the same byte-match by any other spelling from this chassis.
    That is the session's headline measurement: it removes "just permute it" from the
    remaining option set and leaves the RTL/scheduler forensics probe (s3's frontier) as
    the live axis.

- [s4b] The permuter chassis for this function is built, validated and reusable at `tmp/perm_60544` (rebuild with `bash tmp/grind/func_80060544/s4/mkws.sh`). Its base score 60 corresponds exactly to sandbox floor 2, and the score metric is QUANTISED: the whole residual is one reordering, so the only scores reachable are 60 (unchanged) and 0 (matched). There is no gradient for the permuter to climb here, which is the structural reason a 64k-iteration search is dry — this is a needle-in-a-haystack search, not a hill climb.

- [s4b] decomp-permuter workspaces for BB2 functions MUST declare every implicitly-declared callee with a prototype-less `extern s32 f();`. It is codegen-inert but pycparser's typemap needs it; without it the campaign dies with `KeyError: '<callee>'` on the first mutation that types a call. This killed one whole campaign.

- [s4b] `mktemp /tmp/...` is unreliable in this WSL instance (empty return / vanished path, twice in one session). Use deterministic workspace-local scratch paths in permuter compile.sh chassis.

- [s4b] Both campaigns (staging-disabled and full-space) converged on the SAME single sideways form at score 60, banked at `tmp/grind/func_80060544/s4/A_output-60-1/` and `.../B_output-60-1_source.c`. It is an equal-score alternative spelling with the same misplaced `move a1,zero`; it is NOT a candidate and NOT progress.

- [s4] Session start: src/text1b.c was AGAIN (third consecutive session) at the floor-18 session-start form; tmp/grind/func_80060544/s3/apply_candidate.py restored the candidate and `sandbox func_80060544 --disable all` re-measured score 2 / build_insns 133 / target_insns 133. Re-verified at session end: still 2 / 133 / 133 with the candidate form in place in src/.

- [s4] A validated, reusable decomp-permuter chassis for this function now exists: tmp/perm_60544, rebuilt in one command by `bash tmp/grind/func_80060544/s4/mkws.sh`. It compiles the reduced TU tmp/grind/func_80060544/s4/base_src.c through the cheat-free pipeline (cc1 | prologue_fix | maspsx | fix_lwl | multu_pad, with regfix/regfix_stage2/asmfix deliberately OMITTED, mirroring `--disable all`), extracts the func_80060544 region and assembles it. mkws.sh re-runs its own validation gate on every build: 133 insns, and the only diff against target.o is the single `move a1,zero` displacement. Permuter base score 60 corresponds exactly to sandbox floor 2.

- [s4] CHASSIS FIX 1 (generalisable, cost the FIRST s4 campaign its life): a BB2 permuter workspace must give every implicitly-declared callee a prototype-less declaration (`extern s32 func_8007352C();`, likewise func_80073728, initTexPage, saMotionSet, ot_Link). These are codegen-INERT — an old-style `()` declaration is exactly what an implicit declaration produces, and the post-fix build re-passed the instruction-identical validation gate — but without them decomp-permuter's pycparser typemap raises `KeyError: 'func_8007352C'` on the first mutation that has to type a call expression, and the campaign dies after a handful of iterations.

- [s4] CHASSIS FIX 2 (generalisable): `mktemp /tmp/...` is FLAKY in this WSL instance — twice it returned an empty string or a path that had already vanished by the next command, surfacing as the misleading `awk: fatal: cannot open file '/tmp/p60544XXXXXX.s'`. Permuter compile.sh chassis should use a deterministic workspace-local scratch path (tmp/perm_60544/work/p$$.s) instead.

- [s4] Campaign A (label no-staging-A, staging mutation family zeroed): 31,745 iterations, 1,228 s wall, -j 6, base 60, best find 60. Campaign B (label full-space-B, defaults except perm_inline=0): 32,075 iterations, ~1,380 s wall, -j 6, base 60, best find 60. Both harvested with --stop inside the session; `Get-Process` confirms zero permuter processes remain. Telemetry recorded via tools/permuter_campaign.py; logs and campaign_meta.json copied into tmp/grind/func_80060544/s4/.

- [s4] The two campaigns' sideways finds are the SAME form: diffing the extracted function bodies of A_output-60-1/source.c and B_output-60-1_source.c yields no differences. It is an equal-score alternative spelling carrying the same misplaced `move a1,zero` — not a candidate, not progress, banked only as evidence of where random search lands.

- [s4] The only score-0 form ever produced for func_80060544 remains the first-s4 permuter find `end_off = stat; s.p_static = (s32 *)end_off;` (a dead store into an already-consumed local), which the Judge FAILed and which is banked at memory/grind/func_80060544/rejected/judge-fail-0803-1310.c. It was NOT re-proposed this session and must not be re-proposed.

- [s4] Practical consequence for the ladder: 'just run the permuter' is now a measured-dead option for this function AT THIS CHASSIS. Any future permuter work must change the starting form (a structurally different chassis), not the iteration count — repeating a uniform random sample over a quantised score is measured waste.
