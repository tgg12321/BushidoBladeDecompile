# marionation_Exec — HANDOFF (session-9, 2026-07-06)

Narrative handoff for whoever resumes this function. The compact current-state is
`notes.md` (≤120-line cap); this file is the fuller story. History also lives in git
(`git log --oneline -- memory/wip/marionation_Exec/`).

## TL;DR — where we stand

- **Best committed form: `tmp/vDT30.c` = `progress/vDT30-8of8-saved-fixed-masked4.c`.**
  8/8 callee-saved registers correct + both check `andi ,0xff` masks + the saved-stage,
  all in **honest pure C**, **masked 4** (register-normalized distance; 178 vs 179 insns).
- This BEATS the project's committed "solution" (which needs **42 regfix rules + a
  register pin**) and every one of the 10+ prior sessions. The function is ~99% matched.
- **2 residuals block the byte-exact match: the do_timeout pair-swap and region-3.**
- `src/system.c` is UNTOUCHED (oracle green). vDT30 is a candidate you apply to test; it is
  NOT in the build. To reproduce: splice vDT30's marionation body into src and `sandbox`.

## The function

`marionation_Exec(s32 a0, u8 *a1)` in `src/system.c` — the "Marionation" engine main loop:
a VSync-timeout wait, a `debug_printf` on timeout (do_timeout), a status-polling inner loop,
two 8-byte copy blocks gated by flag checks (check / check2), and an outer goto-loop. It was
the single most-cheated function in the codebase. The hard parts, now SOLVED in pure C:
the full 8-register rotation, both redundant check `andi`s, and the saved-stage.

## What each residual is (vDT30, masked 4)

Run `wsl bash tmp/mar.sh tmp/adiff.py tmp/vDT30.c` to see them live.

1. **do_timeout pair-swap @56/57.** Target: `addu v0,v0,s5` (arg5 base) THEN `sll a0` (t0
   shift); ours has them swapped. ROOT: GCC evaluates call args right-to-left, so arg5 (5th
   arg) computes first → lower LUID → wins the scheduler tie. My printf STAGING computes t0
   first (needed to place the D_800A11D5 load late AND keep idx[0]→a0). Computing arg5 first
   (vDT25) fixes the order but swaps the do_timeout temp regs (idx[0]↔arg5 in v1/a0), because
   named staging vars are long-lived (→v1) whereas the target's natural temps are short-lived
   (→a0). 3-way tension {pair-swap, temps→a0, D_800A11D5-late}; vDT30 gets 2 of 3.

2. **region-3 dbr steal @149** (178 vs tgt 179). Target: `beqz a2; nop; sb zero,-1(s3); move
   a1,s4`; ours' delayed-branch pass steals `move a1,s4` (dst2=a1) into the beqz delay slot.
   ROOT (confirmed by reading gcc-2.7.2 `reorg.c mostly_true_jump`): the check2-beqz is a
   loop-continue branch that must be predicted TAKEN so reorg fills from the (unfillable)
   target thread → nop. Prediction-taken requires a `NOTE_INSN_LOOP_BEG`/`LOOP_VTOP` at the
   branch target — i.e. a REAL loop. `rare_destination` can't win it because BOTH the
   continue path and the copy+return fall-through hit a conditional branch first (rare diff
   0 → the EQ-default predicts not-taken → steal). The goto-loop simply cannot emit the note.

(The saved-stage, formerly residual #2, was FIXED this session — see below.)

## Session-9 results (all committed on branch work/marion)

1. **saved-stage FIXED → masked 6→4.** Replace `saved=*D_800A147C_2; saved&=3;` with
   `{ s32 _b; _b = *D_800A147C_2; saved = _b & 3; }` — a widening temp that de-ties the byte
   load from saved's callee-saved reg (byte takes its def-side v0 reuse instead of coalescing
   into s1). Clean C, no coercion. Now matches tgt `lbu v0,0(v0); andi s1,v0`.

2. **Disproved the andi/loop tension (BREAKTHROUGH).** I had earlier "proven" region-3 needs
   a real loop but a real loop breaks the check andi's. WRONG — the andi's only broke because
   `new_var` was set INSIDE the loop. **Hoist `new_var=0xFF; new_var3=0xFF;` before the
   `while(1)`** → update_equiv_regs folds them → andi's SURVIVE under a real loop. So the
   goto-loop is not mandatory; region-3 IS fixable in principle.

3. **Clean 8/8 real-loop form (vDT48, masked 18)** = `progress/vDT48-realloop-clean-8of8-
   masked18.c`. Recipe: real `while(1)` + new_var hoisted + saved widening-temp + **neg1**
   (`s32 neg1; neg1=-1;` after the poll; `while(i!=neg1)` keeps -1 caller-saved `li a3,-1`
   like tgt) + **elem_ptr** (`s32 *a2p=&D_800A11DC[D_800A11D5]; *a2p` — kills the D_800A11DC
   LICM hoist) + **tbl-def double-nest** (`do{do{tbl_125c=D_800A125C;}while(0)}while(0)`
   weights tbl→s5 WITHOUT the do_timeout scramble the staging-wrap caused) + drop do_timeout
   wrapper + clears un-nested. Region-3 IS fixed here; do_timeout is clean; 8 main vars in
   s0-s7. But it's masked 18 > vDT30's 4 because of the conservation barrier ↓.

## THE WALL — the conservation barrier (proven fundamental)

In a REAL loop, GCC's LICM promotes **exactly ONE** loop-invariant (new_var / new_var3 /
the D_800A11DC base) into a **9th callee-saved register (s8)** → stack frame +8 → cascades
through every sp-relative offset (~6 diffs). I verified you can move WHICH one takes s8
(elem_ptr moves it off the address onto new_var3; nv3-before-check2 moves it onto new_var)
but you can NEVER eliminate it — it is conserved: 9 things want callee-saved, GCC gives one
an s-reg. The goto-loop (no LICM) folds ALL of them via update_equiv_regs → 8 regs (this is
why vDT30 wins). **The target ALSO fits 8 in a real loop** — GCC there promotes none — so a
form exists, but it hinges on a global-alloc / update_equiv_regs priority tipping point I
traced through the compiler but could not flip from C.

Net: the region-3 fix a real loop buys (~2) is out-weighed by the conservation cost (~6) +
the tbl-def preamble perturbation (~3). **So vDT30 (goto-loop, masked 4) stays the best.**

## GCC-internals map (for a compiler-savvy resume)

- `tools/gcc-2.7.2/reorg.c:1370` `mostly_true_jump` + `:1321` `rare_destination` — region-3.
- `tools/gcc-2.7.2/loop.c:1529` `move_movables` — the hoist test is
  `threshold*savings*lifetime >= insn_count` (line 1631). D_800A11DC movable has savings=1,
  lifetime=2, insn_count=118 → hoisted. The single-use-address substitution (lines 735-768)
  that WOULD prevent it fails: a full `movsi(symbol_ref)` can't be substituted into an
  indexed `mem`. elem_ptr sidesteps this.
- `tools/gcc-2.7.2/local-alloc.c:947` `update_equiv_regs` — REG_EQUIV fold (line 1031) +
  live-length ×2 (1064). Folds new_var/new_var3 in the goto-loop; promotes one in the real
  loop. The asymmetry (one folds, one gets s8) is the conservation tipping point.
- Instrumented cc1 at `tmp/gccdbg/cc1` (verified byte-identical to canonical). Loop dump:
  `cc1 ... -dL` → `tmp/rtl/sc.i.loop`. Sched dump: `BB2_SCHED_DEBUG=1 ... ` (see
  `tmp/sched_dump.py`).

## Tooling (in the worktree, tmp/; regen from notes.md if lost)

- `tmp/regmatch.py <c>` — reg→value map + N/8 rotation match. `tmp/adiff.py <c>` — LCS diff
  (branch-addr normalized). Invoke via `wsl bash tmp/mar.sh tmp/<tool>.py tmp/<cand>.c`.
- `tmp/system.c.orig` — the honest baseline (candidate body); all splice tools use it.
- Candidate ladder in `progress/`: vDT30 (best, masked 4), vDT45/vDT48 (real-loop, 8/8).
- m2c at `tools/m2c` (`tmp/run_m2c.sh`). Permuter workspace `tmp/perm_mar/` (see below).

## How to resume — the two honest paths

The remaining gap is NOT more hand-grinding — I took the pure-C structural search to the
bottom of the relevant compiler passes. The realistic options:

1. **Bank vDT30 (masked 4) as the floor.** It's 99% matched, far past the cheated committed
   form, fully documented here. If the project standard allows a documented near-match as a
   parked state, this is it. (Note: NOT byte-exact, so `queue done` will refuse it; it can't
   be COMPLETED-C until masked 0.)
2. **A properly-configured decomp-permuter / decomp.me campaign** on the vDT48 base — its
   remaining diffs (pair-swap register-class, the conservation coin-flip) are exactly what
   these tools brute-force, and they don't need to "understand" the tipping point. My hand
   setup (`tmp/perm_mar/`, clean settings.toml with cheat-prone passes disabled, base.c with
   asm stripped so pycparser parses) hit a decomp-permuter import quirk — the parse step
   re-expands `include_asm.h`'s `.set noreorder`. It needs the real `import.py` flow, not a
   hand-patched base.c. This is the one untried lever with real upside.

## Open question for the owner (unresolved)

Is a documented masked-4 near-match an acceptable disposition for this function, or do we
push for the byte-exact match (permuter path)? Given `no-park-permanently`, `queue done`
requires masked 0, so masked 4 can only be a WIP/parked state, not COMPLETED. The owner
needs to decide: keep investing (permuter), or accept vDT30 as the documented floor.
