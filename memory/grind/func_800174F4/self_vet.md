# SELF-VET — func_800174F4 (grind session 6, forensics)

Diff under vet: `src/ings.c`, the body of `func_800174F4` only (the pre-session
body is the session-0 cheat scaffold; it is REPLACED, not extended). Honest
measurement with the edits in place:
`sandbox func_800174F4 --disable all` -> score 0, build_insns 136 ==
target_insns 136, rules_dropped 1 (the sole regfix rule is disabled in that
measurement and the bytes still match).

CONSTRUCTS: (1) `do { i = 0; } while (0);` wrap on the counter's pre-call
initialisation; (2) `do { ... } while (0);` wrap spanning the whole case-1/2
loop body (`prim = func_8005D554(...); i++;`); (3) `mode` — one local holding
the switch selector and, in case 20, the `D_800A37A0` limit (two live ranges);
(4) goto-form loop (`inner_loop:` + `goto inner_loop;`) with an explicit entry
guard and a `break` exit; (5) `h` — one local reused for 0xF0 / the loop limit /
the `D_800A37A8[]` table value. Everything else in the body is ordinary program
logic: no pins, no `__asm__`, no volatile, no aliases, no casts beyond the
project's existing `(u8 *)`/`(s32)` pointer conventions, no dead locals, no dead
stores, no unused variables, no `if (1)`, no empty bodies.

REMOVED by this diff (net honesty gain): the inherited `register s32 s0_var
asm("s0")` / `register s32 s2_var asm("s2")` / `register s32 a1_val asm("a1")`
pins, the empty-body dead-read `if ((a1_val && a1_val) && a1_val) { }`, and the
`new_var`/`new_var2` constant-holder locals (all measured worth ZERO distance in
session 1).

## T1 semantic purpose
- (1) and (2), the two wraps: NO observable effect on the function's output.
  `do { X } while (0);` executes X exactly once, which is what the unwrapped
  statement does. They are match devices and are declared as such with inline
  `/* FAKE: ... */` annotations at both sites. This is the one construct class in
  the diff that fails T1 on its own terms, which is precisely why it is only
  admissible under the do-while(0) family below.
- (3) `mode`: has semantic purpose — it holds the value being dispatched on and
  is then re-used to hold case 20's limit. It carries a live value at every
  read; deleting either def changes behaviour. It also DELETES a local
  (`a1_val`) relative to the previous form.
- (4) the goto loop: has semantic purpose — it is the loop.
- (5) `h`: has semantic purpose — every def/use carries a live value.

## T2 human-programmer
- (3), (4), (5): yes. Reusing a scratch local, writing a guarded bottom-tested
  loop, and reusing a `short` for a size/limit/table value are all ordinary
  1990s PS1 C. A reader asks nothing about them.
- (1), (2): a reader WOULD ask "why is this here?" about a bare
  `do { ... } while (0);` in-line — the annotations answer it, and the
  construct is indistinguishable from an ordinary macro-body expansion, which
  is why the project sanctions it. Not claimed as natural geometry.

## T3 GCC-internals justification
Honest disclosure: YES, both wraps are justified by a named GCC internal —
`flow.c:2081/2329/2515/2725` (`reg_n_refs[regno] += loop_depth`, with
`loop_depth` set from `NOTE_INSN_LOOP_BEG` at `flow.c:1385/1401/1447`) feeding
`global.c:allocno_compare`'s priority
`floor_log2(n_refs)*n_refs/live_length`. Measured this session: the counter
allocno goes n_refs 4 -> 7 at live_length 13, priority 6153 -> 10769, passing
`h`'s 9333, so the counter is allocated first and takes $s0. Under the general
policy a GCC-internals mechanism is the cheat signal; it is disclosed here
because the do-while(0) family is sanctioned SPECIFICALLY for codegen effect and
REQUIRES the mechanism to be named in the FAKE annotation. Constructs (3)/(4)/(5)
need no internals justification — they are program logic.

## T4 permuter/search provenance
- (1)/(2) were NOT found by a search tool. They were derived from the ledger's
  F7 frontier statement (session 5's reading of flow.c) and confirmed by
  reading the allocno table out of an instrumented cc1 BEFORE scoring.
- (3) `mode` originated as a session-4 permuter PROPOSAL, was measured honestly,
  hand-reduced (the permuter's `a1_val` intermediate deleted), and is a
  frozen-list family — not a construct that survives only because detectors miss
  its spelling.
- Nothing in the diff passes detectors by spelling. Both wraps are grep-visible
  and self-declared.

## T5 family check
- (1)/(2) -> `do { ... } while (0);` wrap. Sanctioned family, frozen list.
  NOT nested (two sibling single-level wraps in the same block; prerequisite 3
  of the rule does not apply).
- (3) -> "variable reuse for codegen control". Sanctioned family, frozen list.
- (4) -> "mixed exit forms" / ordinary loop geometry; the goto form is also
  forced by measurement (the natural `for`/`while` forms score 39 at 135 insns).
- (5) -> "variable reuse"; also forced by measurement (splitting any role costs
  3 instructions, s1 K2/K3).
- No construct in the diff matches ANY forbidden family: no register-asm pin, no
  hardcoded-`$N` asm, no regfix/asmfix, no scheduling barrier, no volatile or
  alias coercion, no unused-local-array frame coercion, no dead param assign, no
  dead conditional store, no empty-body dead read, no `if (1)` wrapper, no
  dead-goto label pad, no DImode chain, no opaque `s32 one = 1;`, no lowercase
  `asm(...)`, no alias rename, no redundant width cast, no `bb2.ld` reorder.

## T6 naming-announces-intent
No `pad`, `_pad`, `dummy`, `unused`, `spill`, `sp_*`, `_buf`, `tail`, `slack`,
`_frame_pad`. The locals are `env`, `h`, `prim`, `mask`, `mode`, `v0`, `i`,
`a2_val`, `a0_val`, `div_result`, `counter`, `new_val` — every one of them is
read for its value. `sp18`/`sp20` are the stack buffers actually passed to
`ClearOTagR`/`SetDefDrawEnv`. There are no address-of-only, discard-only or
declaration-only locals.

SANCTIONED-FAMILY-CLAIMS:
  FAMILY: do-while(0) wrap (any body, any codegen effect)
  SCOPE: "**`do { <any body> } while (0);` — including empty bodies — is a sanctioned pure-C match device for ANY codegen effect, including register allocation.**"
  PRECEDENT: .claude/rules/do-while-zero-exception.md:23

  FAMILY: do-while(0) wrap (frozen SOTN-accepted list entry)
  SCOPE: "- **`do { ... } while (0);` wrap** (empty or non-empty body)"
  PRECEDENT: .claude/rules/no-new-park-categories.md:193

  FAMILY: Variable reuse for codegen control
  SCOPE: "- **Variable reuse for codegen control** ([[defeat-licm-hoist-var-reuse]]):"
  PRECEDENT: .claude/rules/no-new-park-categories.md:170

Prerequisite conformance for the do-while(0) family (rule §"Prerequisites"):
  1. inline FAKE annotations present at BOTH construct sites (not file-header
     prose), naming the effect and the mechanism — see below.
  2. "Prefer natural geometry first": five prior sessions of natural geometry
     are recorded in memory/grind/func_800174F4/hypotheses.md — 33 banked
     rejected forms, every sanctioned axis on both competing allocnos measured
     dead (counter n_refs K10, counter live_length pinned by the delay-slot
     requirement, `h` live_length K5/K11, `h` n_refs H-S5-1, conflict-level
     route ruled out by the s5 `-dg` reading, and two permuter campaigns from
     the ALIGNED basin returning zero finds, H-S5-3).
  3. Nested wraps: N/A — the two wraps are siblings, single level each.

ANNOTATION-CONFORMANCE:
  /* FAKE: do-while(0) wrap, mechanism: flow.c loop-note reference
   * weighting (reg_n_refs += loop_depth) feeding
   * global.c:allocno_compare, lever-exhaustion: memory/grind/
   * func_800174F4/hypotheses.md K5/K10/K11/K12/H-S4-2/H-S5-1/H-S5-2.
   * Effect: seats the loop counter in $s0 and h in $s1 while the
   * counter is initialised before rand(). */
  /* FAKE: do-while(0) wrap, mechanism: flow.c loop-note reference
   * weighting (reg_n_refs += loop_depth) feeding
   * global.c:allocno_compare, lever-exhaustion: as above.
   * Effect: the companion wrap for the counter's in-loop refs; it
   * must span the whole body so no code label lands between the
   * call and `i++` (that placement costs reorg.c the jal delay
   * slot). */
Both carry what + named GCC-pass mechanism + lever-exhaustion pointer.
