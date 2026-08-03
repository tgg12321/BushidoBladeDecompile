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

- [s5] Session start: src/text1b.c was AGAIN at the floor-18 session-start form (fourth consecutive session); `python3 tmp/grind/func_80060544/s3/apply_candidate.py` restored the candidate and `sandbox func_80060544 --disable all` re-measured score 2 / build_insns 133 / target_insns 133. The candidate form is in place in src/ at session end, unchanged from s2.

- [s5] THE DECIDING PASS IS NAMED: sched1. Running the instrumented cc1 with `-da` on both reduced-TU variants and reading the linear order of the `D_8009B7D0` set and the `a1` set in EVERY dump (tmp/grind/func_80060544/s5/passorder.py) shows both variants emit la-then-a1 through .rtl/.jump/.cse/.loop/.cse2/.flow/.combine, and only the score-0 variant flips to a1-then-la in **.sched**. Every later dump carries that order forward unchanged. So the s2 frontier's alternative "(c) it is an EMISSION-order difference from expand_call" is dead: emission order is identical in the winning and losing builds.

- [s5] THE DECIDING QUANTITY IS NOT INSN_PRIORITY. With `BB2_PRIO_DEBUG=1` on both variants, every insn in the Case3 block reports `final_pri=1` — in the winning variant too. With `BB2_RANK_DEBUG=1`, every `rank_for_schedule` comparison inside that block reports `cls=3 ... cls2=3 val=0`, i.e. a total tie on both the priority test and the class test, in BOTH variants; mapping the UID sets onto each other (base 136/139/142/145/147/149 <-> reuse 136/142/145/148/150/152) shows the two builds perform the SAME comparisons with the SAME results. GCC 2.7.2's own comment at tools/gcc-2.7.2/sched.c:1472 explains the universal priority 1: on a target where every insn has latency 1, `priority() - 1` collapses to 1 for the whole block. The s3 frontier ("target's build won on INSN_PRIORITY outright") is therefore KILLED at the measurement level, and no future session should probe insn priority or dependence height for this block again.

- [s5] The winning and losing RTL are IDENTICAL at .flow except for ONE `(note NOTE_INSN_DELETED)` between the la and the `sw p_static` — the corpse of the staging copy after it is coalesced. That corpse is NOT the cause: `note_sweep.py` measured nine Case3-arm variants and four of them (`end_off` staged one statement LATER, staging through `geom`, a duplicated `s.p_static` store, and `end_off`->`new_var6` two-hop staging) produce exactly the same single corpse note and do NOT flip the schedule. Only `end_off = stat;` immediately before the store, with exactly one hop, flips it. Full table in memory/grind/func_80060544/hypotheses.md s5 H-F3.

- [s5] CARRIER-IDENTITY SPECIFICITY (mechanism for s4's H9): the flip is keyed to which particular pseudo carries the value AND to the hop position, not to any structural property of the block. `geom` (also multiply-assigned and dead at that point) is inert; a fresh local is inert; two hops through `end_off` is inert; `end_off` one statement later is inert. This is a sched1 bookkeeping difference tied to one pseudo's identity — which is why every C-level structural axis measured flat across s2/s3, and why the only known route to 0 remains the judge-FAILed dead store.

- [s5] Reusable diagnostic harness (fast — a reduced-TU cc1 compile is under a second, versus minutes for a sandbox run): `tmp/grind/func_80060544/s5/rank_probe.sh` (instrumented-cc1 PRIO/RANK dumps for two variants), `passorder.py` (which pass flips the block order), `blockdump.py` (compact per-insn RTL listing of the Case3 block from any `-da` dump), `note_sweep.py` (N Case3-arm variants -> emitted order + corpse-note count + asm length). Any future session can add a variant to `note_sweep.py`'s table and get the answer in one command. NOTE: these must run under WSL — `tools/gcc-2.7.2/cc1` is a Linux ELF and Git Bash reports `Exec format error`; and the instrumented binary is `tools/gcc-2.7.2/cc1`, NOT `build/cc1` (per [[instrumented-cc1-location]]).

- [s5] PERMUTER CAMPAIGN C (mandated modality), ALT CHASSIS: built tmp/perm_60544_alt via tmp/grind/func_80060544/s5/mkalt.sh — pointer-typed `stat` carrier, `while (i < 4)` loop head, `stat` declared last; three changes s3 measured individually inert, so the chassis has the same floor but different pseudo numbering, types and loop head. Validation gate passed (133 insns, single `move a1,zero` displacement, permuter base score 60). Launched `-j 6` with the whole staging/extra-assignment family zeroed (perm_temp_for_expr, perm_duplicate_assignment, perm_chain_assignment, perm_long_chain_assignment, perm_add_self_assignment) plus perm_inline=0. Result: 32,859 iterations / 1,293 s, ONE find, at score 60 (== base), at t+104 s. Harvested with --stop inside the session; `ps aux | grep permuter` confirms zero processes remain.

- [s5] Campaign C's single sideways find is the SAME class of noise as s4's: a `do { ... } while (0)` wrapper around the goto ladder plus `(&s)->p_static` instead of `s.p_static`. Banked at tmp/grind/func_80060544/s5/C_output-60-1/. Across s4 and s5 the permuter has now sampled 96,679 forms of this function from THREE chassis (floor-2 candidate with staging disabled, floor-2 candidate full-space, alt chassis with staging disabled) and produced exactly one score-0 point in total — the judge-FAILed dead store. The chassis-change hypothesis (s4 frontier item 3) is therefore measured DEAD for the one alternative chassis that could be built out of known-inert respellings, and the mechanism finding above explains why: the flip depends on a specific pseudo's identity, which is not something the permuter's mutation space explores except by manufacturing staging assignments — the family that has to stay disabled.

- [s5] src/text1b.c was again at the floor-18 session-start form (fourth consecutive session); tmp/grind/func_80060544/s3/apply_candidate.py restored the candidate and `sandbox func_80060544 --disable all` re-measured score 2 / build_insns 133 / target_insns 133. The candidate form is in place in src/ at session end, unchanged from s2.

- [s5] The deciding pass is sched1 and nothing else — proven by dumping the block order in every -da dump of both the winning and losing variant.

- [s5] INSN_PRIORITY is a universal 1 inside the Case3 block in BOTH variants, and every rank_for_schedule comparison there is a total tie (cls=3 cls2=3 val=0) in BOTH. No future session should probe insn priority, dependence height, or ready-list class for this block.

- [s5] The NOTE_INSN_DELETED corpse left by the coalesced staging copy is a side effect, not the cause: four variants reproduce the corpse without reproducing the flip.

- [s5] The flip depends on carrier-pseudo identity and hop position: only `end_off = stat;` one hop immediately before the p_static store flips it; geom, a fresh local, two hops, and the same statement one line later are inert.

- [s5] Reusable fast diagnostic harness now exists (a reduced-TU cc1 compile is sub-second vs minutes for a sandbox run): s5/rank_probe.sh (instrumented PRIO/RANK dumps), s5/passorder.py (which pass flips the order), s5/blockdump.py (compact per-insn RTL of the Case3 block), s5/note_sweep.py (variant table -> emitted order + corpse count + asm length). Add a row to note_sweep.py and get an answer in one command.

- [s5] Harness gotchas confirmed: tools/gcc-2.7.2/cc1 is a Linux ELF, so these scripts must run under WSL (Git Bash gives 'Exec format error'); and the INSTRUMENTED binary is tools/gcc-2.7.2/cc1, not tools/gcc-2.7.2/build/cc1.

- [s5] Permuter campaign C: alt chassis, staging family zeroed, 32,859 iterations / 1,293 s / -j 6, base 60, best find 60, harvested with --stop, zero surviving processes. Artifacts copied into tmp/grind/func_80060544/s5/.

- [s5] The alt chassis itself is banked at memory/grind/func_80060544/rejected/alt-chassis-inert-respelling.c — equal to the candidate (2/133) and strictly more verbose, so it is not a candidate; it is recorded so no session rebuilds it or re-runs a campaign on it.

## Measured facts (session s6, 2026-08-03, modality: forensics)

20. **PROCESS (fifth consecutive occurrence).** `src/text1b.c` was again at the
    floor-18 session-start form. `python3 tmp/grind/func_80060544/s3/apply_candidate.py`
    restored it; `sandbox func_80060544 --disable all` re-measured **score 2 /
    build_insns 133 / target_insns 133** both before and after probing, with the
    candidate form in place in `src/` at session end. Run apply_candidate.py FIRST,
    every session.

21. **THE THIRD INSTRUMENT: `BB2_SCHED_DEBUG`.** `tools/gcc-2.7.2/sched.c` carries
    a third env-gated hook that s3/s5 did not know about (they only used
    `BB2_PRIO_DEBUG` and `BB2_RANK_DEBUG`): at sched.c:3691 it prints
    `SCHEDDBG block=<b> n_insns=<n> n_ready=<n>` plus every insn's priority and
    ref count, and at sched.c:3950 it prints, for EVERY scheduling step,
    `SCHEDDBG PICK clock=<n> picked=<uid> (pri=<n> luid=<n>)` followed by the FULL
    ready-list array with each entry's priority and LUID and the current
    `last_scheduled_insn`. That is the ready-list instrument the s5 frontier asked
    for, and it required no change to `tools/`. Driver:
    `tmp/grind/func_80060544/s6/sched_probe.sh` (both variants, `-da`, one command);
    reader: `tmp/grind/func_80060544/s6/blk.py <variant> <uid>...` prints the whole
    SCHEDDBG section for whichever block contains those UIDs. NB the file contains
    the block TWICE — the first occurrence is sched1, the second is sched2 (whose
    LUIDs are renumbered from the post-sched1 order, which is itself a free check on
    what sched1 produced).

22. **THE DECIDING QUANTITY IS NAMED: a LAUNCH_PRIORITY promotion of the Case3
    `la`, via `adjust_priority()` -> `birthing_insn_p()`.** In the losing build the
    clock-5 ready list is `[147(p=1,l=4) 145(p=1,l=3) 136(p=1,l=0)]` and sched1
    picks 147 (the `a1 = 0` set-up); in the winning build it is
    `[136(p=2130706433,l=0) 150(p=1,l=5) 148(p=1,l=4)]` and it picks 136 (the `la`).
    `2130706433` == `0x7f000001` == `LAUNCH_PRIORITY` (sched.c:187). Clocks 1-4 are
    identical in both builds, so the dependence graph, the ready-set membership and
    the arrival order are all identical — exactly ONE insn's priority field differs.
    Chain of code: schedule_block sets the just-scheduled insn to LAUNCH_PRIORITY
    (sched.c:3985); schedule_insn computes
    `max_priority = MAX (INSN_PRIORITY (ready[0]), INSN_PRIORITY (insn))`
    (sched.c:2601) and calls `adjust_priority` on each newly-ready insn;
    adjust_priority (sched.c:2531) always takes its `n_deaths == 0` branch (GCC's own
    comment at sched.c:2544 says REG_DEAD notes are already gone) and does
    `if (birthing_insn_p (PATTERN (prev))) INSN_PRIORITY (prev) = max_priority;`;
    `birthing_insn_p` (sched.c:2496) is `bb_live_regs[dest] && reg_n_sets[dest] == 1`.

23. **CORRECTION to the s5 reading of `rank_for_schedule`.** `RANKDBG` prints
    BEFORE the third tiebreak, so s5's "every comparison reports val=0" does NOT
    mean the comparator returned 0. `rank_for_schedule` (sched.c:2399) falls through
    to `return INSN_LUID (y) - INSN_LUID (x)`, i.e. the HIGHER LUID sorts FIRST, and
    the a1 set-up — emitted by expand_call after all the stores — always holds the
    highest LUID of the three insns left at clock 5. The tie is therefore always
    resolved in favour of `a1`, deterministically, and the birthing promotion is the
    only mechanism that can override it. (This does not overturn any s5 verdict; it
    sharpens why they were all dead.)

24. **THE PREDICATE, MEASURED — the flip needs a carrier that is (1) DEAD after
    the p_static store AND (2) has EXACTLY ONE other assignment.**
    `carrier_setcount.py` staged the Case3 address through nine existing locals:
    only `end_off` flips at the correct size (117 asm lines), `mid_off` flips only
    by destroying the function (113 lines, its own later use clobbered), and
    `fresh` (0 other sets), `last` / `new_var3` / `new_var6` (1 other set but
    genuinely read later, so the copy survives) and `geom` / `idx` / `prev` (2 other
    sets) are all inert. Condition (1) is what lets flow/combine delete the copy and
    RETARGET the `la`'s destination onto the carrier's pseudo (`.flow`: base's `la`
    sets `reg 75` = `stat`; the `end_off` variant's sets `reg 77` = `end_off`);
    condition (2) is what makes combine's `reg_n_sets` decrement
    (combine.c:2309/2332) land on the 1 that birthing_insn_p tests. A naive
    "reg_n_sets == 1" model is FALSIFIED by the `last`/`new_var3`/`new_var6` rows.

25. **THE MODEL CONFIRMED IN BOTH DIRECTIONS** (`predicate_test.py`, diagnostics
    only, never candidates): a manufactured brand-new local with exactly one
    prologue assignment and no later read FLIPS the schedule at the same 117-line
    size (a second, independent route to target's block order — and still a dead
    store); and `end_off` given a genuine later read (returning through it instead
    of `new_var6`) STOPS flipping. Both predictions were stated before the runs.

26. **CONSEQUENCE — the case analysis for this basic block is now complete.**
    Priority is a universal tie (s5), the ready-list class is a universal tie
    because every insn has latency 1 (s5), the LUID tiebreak always favours the a1
    set-up and emission order is identical in the winning and losing builds (s5
    H-F2, plus s2's six statement orderings and five call-argument respellings), so
    the birthing promotion is the ONLY lever — and it requires a carrier that is
    dead after the store, i.e. a dead store. On the measured evidence there is no
    live-data pure-C spelling of this block that reproduces target's order; every
    known route passes through the construct family the Judge FAILed. That is the
    analysis a future escalation would cite; exhaustion remains the driver's call.

27. **A CLEAN per-arm-local route to `reg_n_sets == 1` is KILLED.** `arm_sweep.py`:
    Case3-only local 119 asm lines, all-three-arms-own-locals 122, other-two-arms-
    own-locals 122, Case3-only pointer-typed local 119 — versus the base's 117 — and
    all four have `launch=0`, so they do not even fire the promotion they were built
    to fire. The s1 H2 shared `stat` carrier is load-bearing and cannot be split.
    Banked in `rejected/case3-single-set-carrier-locals.c`.

- [s6] NEW FAST GATE for any future candidate form of this function: compile the reduced TU with `BB2_SCHED_DEBUG=1` and grep the ready lists for the Case3 `la`'s UID with `p=2130706433`. If the promotion does not fire, the form CANNOT reach distance 0, and no sandbox run (minutes) is needed — the cc1 compile is sub-second. `arm_sweep.py` / `carrier_setcount.py` / `predicate_test.py` each implement this end-to-end; add a variant function and re-run.

- [s6] GENERALISABLE (worth trying on any queue item whose residual is a single intra-block instruction placement with a matching instruction count): GCC 2.7.2's sched1 resolves an all-tied ready list by LUID, so emission order normally wins; the one thing that overrides it is `adjust_priority`'s `birthing_insn_p` promotion to LAUNCH_PRIORITY, which fires for an insn setting a LIVE pseudo whose `reg_n_sets` is 1. `BB2_SCHED_DEBUG=1` makes this directly visible. Any "one instruction is scheduled one slot late/early inside an otherwise byte-identical block" residual should be checked against this mechanism before any structural C search.

- [s6] The `tools/gcc-2.7.2/cc1` instrumented binary must be run under WSL (Linux ELF; Git Bash reports `Exec format error`) and is NOT `tools/gcc-2.7.2/build/cc1` (which has none of the BB2 hooks — verified this session: 0 occurrences of RANKDBG/SCHEDDBG in build/cc1, 1 and 8 respectively in tools/gcc-2.7.2/cc1).

- [s6] src/text1b.c was AGAIN found at the floor-18 session-start form (fifth consecutive session); tmp/grind/func_80060544/s3/apply_candidate.py restored it and `sandbox func_80060544 --disable all` re-measured score 2 / build_insns 133 / target_insns 133, both before and after probing. The candidate form is in place in src/ at session end, unchanged from s2.

- [s6] THE THIRD INSTRUMENT: tools/gcc-2.7.2/sched.c carries an env-gated BB2_SCHED_DEBUG hook (sched.c:3691 and :3950) that prints, for every scheduling step of every block, `SCHEDDBG PICK clock=<n> picked=<uid> (pri=<n> luid=<n>)` plus the FULL ready-list array with per-entry priority and LUID and the current last_scheduled_insn. s3 and s5 only used BB2_PRIO_DEBUG and BB2_RANK_DEBUG. The dumps contain each block TWICE -- the first occurrence is sched1, the second sched2, whose LUIDs are renumbered from the post-sched1 order and therefore double as a free readout of what sched1 produced.

- [s6] THE DECIDING QUANTITY IS NAMED. Losing build, clock 5: ready = [147(p=1,l=4) 145(p=1,l=3) 136(p=1,l=0)] -> picks 147 (`a1 = 0`). Winning build, clock 5: ready = [136(p=2130706433,l=0) 150(p=1,l=5) 148(p=1,l=4)] -> picks 136 (the `la`). 2130706433 == 0x7f000001 == LAUNCH_PRIORITY (sched.c:187). Clocks 1-4 are identical in both builds, so the dependence graph and ready-set arrival order are identical and exactly one insn's priority field differs.

- [s6] THE CODE PATH: schedule_block sets the just-scheduled insn to LAUNCH_PRIORITY (sched.c:3985); schedule_insn computes max_priority = MAX(INSN_PRIORITY(ready[0]), INSN_PRIORITY(insn)) (sched.c:2601) and calls adjust_priority on each newly-ready insn; adjust_priority (sched.c:2531) always takes its n_deaths==0 branch -- GCC's own comment at sched.c:2544 notes REG_DEAD notes are already gone -- and does `if (birthing_insn_p (PATTERN (prev))) INSN_PRIORITY (prev) = max_priority;`; birthing_insn_p (sched.c:2496) is `bb_live_regs[dest] && reg_n_sets[dest] == 1`.

- [s6] CORRECTION TO s5: RANKDBG prints between the class test and the LUID test, so `val=0` does NOT mean the comparator returned 0. rank_for_schedule (sched.c:2399) falls through to `return INSN_LUID (y) - INSN_LUID (x)` -- HIGHER LUID FIRST -- and the a1 set-up, emitted by expand_call after the stores, always holds the highest LUID among the three insns left at clock 5. The tie is therefore always resolved in a1's favour, deterministically.

- [s6] THE PREDICATE (carrier_setcount.py, nine existing locals staged as the Case3 carrier; `others` = pre-existing assignments): fresh/0 inert; end_off/1 FLIPS (launch=1, 117 asm lines); last/1, new_var3/1, new_var6/1 inert; geom/2, idx/2, prev/2 inert; mid_off/2 flips only by destroying the function (113 lines). The naive `reg_n_sets == 1` model is FALSIFIED. The conjunction that fits every row: the carrier must be (1) DEAD after the s.p_static store, so flow/combine deletes the copy and RETARGETS the la's destination onto the carrier's pseudo (.flow: base's la sets reg 75 = `stat`; the end_off variant's sets reg 77 = `end_off`), and (2) carry EXACTLY ONE other assignment, so combine's reg_n_sets decrement on deleting the copy (combine.c:2309/2332) lands on 1.

- [s6] MODEL CONFIRMED IN BOTH DIRECTIONS (predicate_test.py, diagnostics only, predictions stated before the runs): a manufactured brand-new local with exactly one prologue assignment and no later read FLIPS the schedule at the same 117-line size -- a second, independent route to target's block order, and still a dead store; and `end_off` given a genuine later read (returning through it instead of new_var6) STOPS flipping.

- [s6] CONSEQUENCE -- the case analysis for this basic block is complete: priority is a universal tie (s5), the ready-list class is a universal tie because every insn on this target has latency 1 (s5), the LUID tiebreak always favours the a1 set-up and emission order is identical in the winning and losing builds (s5 H-F2, plus s2's six statement orderings and five call-argument respellings), so the birthing promotion is the ONLY lever -- and it requires a carrier that is dead after the store, i.e. a dead store. On the measured evidence there is no live-data pure-C spelling of this block that reproduces target's instruction order; every known route passes through the construct family the Judge FAILed (rejected/judge-fail-0803-1310.c).

- [s6] THE CLEAN ROUTE TO THE PREDICATE IS KILLED (arm_sweep.py): Case3-only local 119 asm lines, all-three-arms-own-locals 122, other-two-arms-own-locals 122, Case3-only pointer-typed local 119, versus the base's 117 -- and all four have launch=0, so they do not even fire the promotion they were built to fire. The s1 H2 shared `stat` carrier is load-bearing and cannot be split per arm.

- [s6] NEW FAST GATE for any future candidate form of this function: compile the reduced TU with BB2_SCHED_DEBUG=1 and grep the ready lists for the Case3 la's UID with p=2130706433. If the promotion does not fire, the form CANNOT reach distance 0, and no sandbox run (minutes) is needed -- the cc1 compile is sub-second. arm_sweep.py / carrier_setcount.py / predicate_test.py each implement this end to end; add a variant function and re-run.

- [s6] GENERALISABLE to any queue item whose residual is a single intra-block instruction placement with a matching instruction count: GCC 2.7.2's sched1 resolves an all-tied ready list by LUID, so emission order normally wins; the one thing that overrides it is adjust_priority's birthing_insn_p promotion to LAUNCH_PRIORITY, which fires for an insn setting a LIVE pseudo whose reg_n_sets is 1. BB2_SCHED_DEBUG=1 makes this directly visible; check it before any structural C search.

- [s6] HARNESS GOTCHAS re-confirmed: tools/gcc-2.7.2/cc1 is a Linux ELF and must be run under WSL (Git Bash gives `Exec format error`), and it is the instrumented binary -- tools/gcc-2.7.2/build/cc1 contains ZERO occurrences of RANKDBG/SCHEDDBG while tools/gcc-2.7.2/cc1 contains 1 and 8 respectively.

## Measured facts (session s7, 2026-08-03, modality: forensics)

28. **PROCESS (sixth consecutive occurrence).** `src/text1b.c` was again at the
    floor-18 session-start form. `python3 tmp/grind/func_80060544/s3/apply_candidate.py`
    restored it and `sandbox func_80060544 --disable all` re-measured
    **score 2 / build_insns 133 / target_insns 133** with the candidate form in
    place in `src/` at session end. Run apply_candidate.py FIRST, every session.

29. **THE s6 ANOMALY IS RESOLVED — it is loop.c invariant hoisting, not a
    failure of birthing_insn_p.** s6 measured four forms whose Case3 `la` has a
    single-set destination pseudo (the exact thing birthing_insn_p wants) and
    recorded that none of them fired the promotion, concluding "single-set-ness
    is neither necessary nor sufficient". The real reason is that a dedicated
    address local for the Case3 arm is assigned exactly once with a
    LOOP-INVARIANT value, so loop.c hoists the `la` into loop 1's preheader:
    `aw_W0_base` emits `la $2,D_8009B7D0` at `out.s:89` inside the Case3 block,
    while `aw_W1_case3_own` and `aw_W4_case3_own_ptr` emit
    `la $22,D_8009B7D0` at `out.s:42`, BEFORE the loop head `.L2`, holding the
    address in a callee-save across the whole loop (which is where their +2
    instructions come from). There is no `la` left in the block for
    adjust_priority to promote. The shared `stat` carrier is immune to the hoist
    only because it is set in all three arms. Probe:
    `tmp/grind/func_80060544/s7/anom.py`.

30. **THE MECHANISM IS A TWO-PASS CONJUNCTION (loop.c AND combine), and the two
    halves pull against each other.** For the promotion to fire, the Case3 `la`
    must (R1) still be in the block at sched1 — which, with the arm inside
    loop 1, forces a destination pseudo that is set more than once INSIDE the
    loop, i.e. the shared `stat`, since any single-set address local is
    loop-invariant and gets hoisted (fact 29) — and (R2) have
    `reg_n_sets[dest] == 1` at sched1, which with a multiply-set `stat` can only
    arise from combine RETARGETING the `la` onto another pseudo, which requires
    deleting a copy, which requires the copied value never to be read.
    **A carrier whose copied value is never read is a dead store by definition,
    not by policy.** This upgrades s6's inductive "every measured route is a dead
    store" to a closed case analysis over the two passes that decide the outcome.

31. **THE FIRST CLEAN ROUTE EVER MEASURED — and it is five instructions too
    short.** The one shape that dissolves the R1/R2 conflict is to PEEL the
    `i == 3` iteration out of loop 1 (loop runs `i = 0..2`; the Case3 body
    follows the loop with its own address local): there is then no loop to hoist
    out of, the local stays single-set, and the promotion fires with no staging
    copy anywhere. `tmp/grind/func_80060544/s7/peel_sweep.py`:
    `Y1_peel_own_local` = A1-FIRST / la dest single-set / launch=1 /
    **112 asm lines**; `Y2_peel_shared_stat` (same peel, shared carrier) =
    a1-after / 3 sets / launch=0 / 112. The base is 117 asm lines == target's 133
    instructions, so the peel is five instructions off and cannot match; Y2
    isolates the -5 as the peel's own cost, not the local's. This independently
    re-confirms s3's finding (from a completely different direction) that the
    four-way in-loop ladder is the original's block structure.

32. **THE s6 FRONTIER IS KILLED: relocating the carrier's other definition does
    not produce a live-data route.** `tmp/grind/func_80060544/s7/reloc_sweep.py`
    (sub-second instrumented-cc1 gate; columns = emitted order, la destination
    and its .combine set count, LAUNCH_PRIORITY seen, `la` still inside the loop,
    asm lines):
    ```
    X0_base (control)          a1-after  r75/3sets  launch=0  in-loop  117
    X2_endoff_after_loops      a1-after  r77/2sets  launch=0  in-loop  117
    X3_endoff_between_loops    A1-FIRST  r77/1sets  launch=1  in-loop  115
    X4_endoff_after_nostage    a1-after  r75/3sets  launch=0  in-loop  115
    X5_newvar3_late            a1-after  r87/2sets  launch=0  in-loop  117
    X9_stage_ctl (judge-FAIL)  A1-FIRST  r77/1sets  launch=1  in-loop  117
    ```
    Moving `end_off`'s definition DOWNSTREAM of the store (to just before the
    initTexPage tail) leaves the staging copy undeleted and kills the promotion;
    the same for `new_var3`. Moving it to between the two loops does fire, but
    the relocation itself costs two instructions (X4 is the cost control), so it
    can never reach target's 133 — and its Case3 statement is still a dead store.

- [s7] Session start: src/text1b.c was AGAIN at the floor-18 session-start form (sixth consecutive session); apply_candidate.py restored it and the sandbox re-measured score 2 / build_insns 133 / target_insns 133. Unchanged at session end — s7 proposed no new candidate form (the floor-2 form from s2 remains the best known).

- [s7] THE s6 ANOMALY IS RESOLVED: the four single-set-destination forms in s6's arm_sweep do not fail birthing_insn_p — their `la` is HOISTED OUT OF THE LOOP by loop.c (a dedicated Case3 address local is assigned once with a loop-invariant value), so there is no `la` in the Case3 block for adjust_priority to promote. Measured directly in the emitted asm: base has `la $2,D_8009B7D0` at out.s:89 inside block .L12; the single-set forms have `la $22,D_8009B7D0` at out.s:42, before the loop head .L2, living in a callee-save across the loop (their +2 instructions). Probe: tmp/grind/func_80060544/s7/anom.py.

- [s7] THE MECHANISM IS TWO PASSES, NOT ONE, and they conflict: (R1) keeping the `la` in the Case3 block requires a destination pseudo set MORE THAN ONCE inside loop 1 (the shared `stat`), because a single-set address local is loop-invariant and loop.c hoists it; (R2) `reg_n_sets[dest] == 1` at sched1 then requires combine to RETARGET the la onto another pseudo, which requires deleting a copy, which requires the copied value never to be read. A copied value that is never read IS a dead store — so the dead-store requirement is a consequence of the pass structure, not an artifact of the forms tried so far.

- [s7] FIRST CLEAN (dead-store-free) ROUTE EVER MEASURED, and it is dead on size: peeling the `i == 3` iteration out of loop 1 removes the loop the hoist happens out of, so a dedicated single-set address local for the peeled arm keeps the la in place and the promotion fires. Y1_peel_own_local = A1-FIRST, la dest single-set, launch=1, 112 asm lines vs the base's 117 (== target's 133 insns); Y2_peel_shared_stat (same peel, shared carrier) = launch=0, also 112, isolating the -5 as the peel's own cost. Independently re-confirms s3's instruction-count finding that the four-way in-loop ladder is the original's block structure. Probe: tmp/grind/func_80060544/s7/peel_sweep.py; banked in rejected/case3-carrier-relocation-and-peel.c.

- [s7] THE s6 FRONTIER IS KILLED (reloc_sweep.py): relocating the staging carrier's OTHER definition downstream of the Case3 store leaves the copy undeleted and kills the promotion (end_off moved to just before the initTexPage tail: 2-set destination, launch=0; new_var3 moved to just before its own uses: 2-set destination, launch=0). Relocating end_off's definition to between the two loops DOES fire (1 set, LAUNCH_PRIORITY) but the relocation itself costs two instructions (115 asm lines; the no-staging control is also 115), so it cannot reach target's 133 — and it is still a dead store. There is no relocation that makes a genuinely-read variable dead at exactly the p_static store while leaving the function on target's instruction count.

- [s7] GENERALISABLE (beyond this function): when a residual is one instruction's placement inside an otherwise byte-identical block, check BOTH passes — loop.c invariant hoisting decides whether the insn is even in the block, and combine's retarget/`reg_n_sets` bookkeeping decides whether sched1's adjust_priority can promote it. A single-set destination pseudo is not automatically a win: if the value is loop-invariant, single-set-ness is exactly what makes loop.c take the insn out of the block. `BB2_SCHED_DEBUG=1` plus a check of the emitted `la`'s position relative to the loop head measures both in one sub-second compile.

- [s7] HARNESS: tmp/grind/func_80060544/s7/reloc_sweep.py and s7/peel_sweep.py extend the s6 gate with two new columns — `la_in_loop` (was the address hoisted out of the loop?) and the la's set count read from `.combine` rather than `.flow` (combine is the last dump before sched1, so its RTL is what sched1 actually sees; the two differ exactly when the retarget fires). Both must be run under WSL (`bash tools/wsl.sh 'python3 …'`) because tools/gcc-2.7.2/cc1 is a Linux ELF; the Windows-side python3 fails with WinError 193.

- [s7] Floor re-measured at session start and end with the s2 candidate form in place in src/text1b.c: sandbox func_80060544 --disable all = score 2, build_insns 133, target_insns 133. s7 proposed no new candidate form; the s2 form remains the best known. (src/text1b.c was AGAIN found at the floor-18 session-start form — sixth consecutive session — and restored with tmp/grind/func_80060544/s3/apply_candidate.py.)

- [s7] THE s6 ANOMALY IS RESOLVED. s6 measured four forms whose Case3 la has a single-set destination pseudo and recorded that none fired the promotion, concluding 'single-set-ness is neither necessary nor sufficient'. The real cause is loop.c invariant hoisting: base emits `la $2,D_8009B7D0` at out.s:89 inside the Case3 block, while aw_W1_case3_own and aw_W4_case3_own_ptr emit `la $22,D_8009B7D0` at out.s:42, before the loop head .L2, holding the address in a callee-save across the whole loop (their +2 instructions). No la in the block means nothing for adjust_priority to promote.

- [s7] THE MECHANISM IS A TWO-PASS CONJUNCTION THAT CONFLICTS WITH ITSELF: (R1) keeping the la in the Case3 block at sched1 requires a destination pseudo set MORE THAN ONCE inside loop 1 — i.e. the shared `stat` carrier — because any single-set address local is loop-invariant and gets hoisted; (R2) reg_n_sets[dest] == 1 at sched1 then requires combine to RETARGET the la onto another pseudo, which requires deleting a copy, which requires the copied value never to be read. A never-read copy is a dead store by definition. The dead-store requirement is therefore a consequence of the pass structure, not an artifact of the forms tried so far — this upgrades s6's inductive statement to a closed case analysis.

- [s7] THE FIRST DEAD-STORE-FREE ROUTE EVER MEASURED, and it is five instructions off: peeling the i == 3 iteration out of loop 1 (Y1_peel_own_local) gives A1-FIRST / single-set la destination / launch=1 at 112 asm lines against the base's 117 (== target's 133 insns). The control Y2 (same peel, shared carrier) is also 112 with launch=0, isolating the -5 as the peel's own cost.

- [s7] RELOCATION TABLE (reloc_sweep.py; columns = order, la destination/set count in .combine, LAUNCH_PRIORITY, la still in loop, asm lines): X0_base a1-after r75/3sets launch=0 in-loop 117; X2_endoff_after_loops a1-after r77/2sets launch=0 in-loop 117; X3_endoff_between_loops A1-FIRST r77/1sets launch=1 in-loop 115; X4_endoff_after_nostage a1-after r75/3sets launch=0 in-loop 115; X5_newvar3_late a1-after r87/2sets launch=0 in-loop 117; X9_stage_ctl (the judge-FAILed form) A1-FIRST r77/1sets launch=1 in-loop 117.

- [s7] THE COMPLETED CASE ANALYSIS for the Case3 block, with each rung's mechanism named: INSN_PRIORITY is a universal 1 in the winning build too and is immovable by dependence height (s5/s3); the ready-list class is a universal 3 because every insn on this target has latency 1 (s5); rank_for_schedule falls through to INSN_LUID(y) - INSN_LUID(x), higher LUID first, and the a1 set-up emitted by expand_call after the stores always holds the highest LUID at clock 5 (s6), with emission order identical in the winning and losing builds (s5) and inert to all six intra-arm statement orderings and all five call-argument respellings (s2); therefore the birthing promotion is the ONLY override, and s7 shows it requires either a dead store (in-loop) or a five-instruction loop peel.

- [s7] GENERALISABLE beyond this function: when a residual is one instruction's placement inside an otherwise byte-identical block, check BOTH passes — loop.c invariant hoisting decides whether the insn is even in the block, and combine's retarget/reg_n_sets bookkeeping decides whether sched1's adjust_priority can promote it. A single-set destination pseudo is not automatically a win: if the value is loop-invariant, single-set-ness is exactly what makes loop.c take the insn OUT of the block. One sub-second instrumented-cc1 compile measures both (BB2_SCHED_DEBUG=1 plus the emitted la's position relative to the loop head).

- [s7] HARNESS: s7/reloc_sweep.py and s7/peel_sweep.py extend the s6 gate with two new columns — la_in_loop (was the address hoisted?) and the la's set count read from .combine rather than .flow (combine is the last dump before sched1, so its RTL is what sched1 sees; the two differ exactly when the retarget fires). Both must run under WSL (`bash tools/wsl.sh 'python3 …'`): tools/gcc-2.7.2/cc1 is a Linux ELF and the Windows python3 fails with WinError 193.

- [s7] No build-pipeline file was touched: no edits to regfix.txt / asmfix.txt / .claude/rules / engine / tools / Makefile / *.ld, no queue done, no retire, no commit. The only tracked-file change is src/text1b.c carrying the unchanged s2 candidate form, plus the memory/grind ledger updates.
