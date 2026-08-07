# marionation_Exec — HANDOFF (end of session-10, 2026-07-07)

Narrative handoff. The compact current-state is `notes.md` (≤120-line cap); this file is
the fuller story. History lives in git (`git log --oneline -- memory/wip/marionation_Exec/`).

## TL;DR — where we stand

- **candidate.c = vT40, masked 4 (verified), policy-clean, layer-1 reviewed.** 8/8
  callee-saved registers, both `andi` masks, all shapes; zero regfix rules, zero pins,
  zero asm; every match device FAKE-annotated per site with measured justifications.
- **Two 2-insn residuals remain** (do_timeout pair order; region-3 delay-slot nop). Both
  are root-caused to exact GCC-2.7.2 pass decisions, and session-10 CLOSED every
  analytically-derivable lever (list below). The open path is sampling the
  unknown-original-source-shape space (rich-pass permuter — stopped 2026-07-07 at the
  owner's request to free the machine for pipeline work; ~107k iterations done).
- **Policy**: the construct-honesty line is owner-ruled and recorded on MAIN
  (`5c3a192f` + provenance addendum `05a757e4`): do-while(0) sanctioned for ANY codegen
  effect incl. register allocation, FAKE-annotated; nested wraps need measured
  single-level-insufficient justification. Forbidden (unchanged): regfix, pins, __asm__
  beyond canonical, toolchain edits, semantic-lie C (cross-symbol derivation, volatile).
- `src/system.c` is PRISTINE (oracle green). All work lives in candidates under
  memory/wip/marionation_Exec/{candidate.c, progress/, rejected/} and tmp/ tooling.

## The review saga (governance record — do not re-litigate)

Three adversarial layer-1 rounds on the wrap lineage:
1. **rev-vt31 FAIL** under the old mechanism-scoped do-while-zero rule (correct
   application; rule since superseded by the owner's construct-honesty ruling).
2. **rev-vt40 FAIL** on (a) provenance — refused to credit a worktree-local rule file
   claiming an owner ruling (correct per no-self-sanctioning; two prior real violations
   exist) — and (b) an INFERRED figure in the nested-wrap justification. Fixed: the
   ruling + owner's verbatim engagement now live on MAIN (5c3a192f, 05a757e4); the
   nested justification was re-measured for real (single-level: i1496 pri 933 < arg1
   952 → mis-seats, masked 4→14).
3. **rev-vt40-r3 final: PASS-except-one.** Independently passes every construct on
   pre-dated sanctions; holds ONLY the nested wrap (candidate.c line ~111) at
   NEEDS_USER because it is the sole construct with no pre-existing community citation.
   **THE LAYER-2 COMPLETION REVIEWER MUST MAKE AN INDEPENDENT CALL ON THE NESTED WRAP**
   (recorded in meta.json; do not present it as resolved).

## Residual 1 — do_timeout pair (tgt: `addu v0,v0,s5` THEN `sll a0`; ours swapped)

Root cause fully quantified (QTYDBG + sched1 dumps): **a coupled fixed point.**
- vT40 (t0-statements first): seats all correct (arg5-addr temp tight → v0; density 32)
  but t0-sll's lower LUID wins the sched2 tie → order swapped.
- vT32 (arg5 first): order correct, but sched1 stretches the arg5-addr temp and the
  t0-web/arg5val qtys tie exactly (5.33 v 5.33, qty_compare = floor_log2(refs)·refs·
  size/life) → seats trade (masked 8).
- **CLOSED levers (all measured)**: EXHAUSTIVE 140-ordering sweep (every dependency-
  valid do_timeout interleaving; floor = 4; tmp/ordersweep.log); fresh single-set temps
  (launch → head re-times: vT33=16, vT34=11); ANY loop-note insertion inside the window
  (arg5-stmt wraps single/double vT42/vT35/vT36 = 14/15/14; call wrap vT43=12); flat
  respellings (6 forms canonicalize = 8); identity-op second refs (all tree-fold);
  s64/size games (byte-visible); value-staging via cnt/i/status (their other live
  ranges pin wrong hard regs).

## Residual 2 — region-3 (tgt: `beqz a2; nop; sb; move a1,s4`; ours steals the move)

Mechanism complete (DBRDBG traces): fill_simple eats check1's move into beqz-s4's slot
(hence check1's nop is free); check2's move stays in the eager fall-through window and
wins. The knob byte-proof stands: BB2_ALLLIVE_LABEL=513,627 on the canonical cc1
reproduces target's region-3 EXACTLY (diagnostic only).
- **CLOSED levers (all proven/measured)**: genuine a1-liveness at the tail (impossible —
  every live-there pseudo crosses calls → callee-saved; flow + reorg forward-scan both
  accurate); own_fallthrough=0 via a label (needs a byte-visible user: the permuter's
  masked-3 `while(status)` find proved the mechanism works but pays an extra bnez →
  180 insns, unmatchable — AND reads status uninitialized → REJECTED, banked at
  rejected/permuter-while-status-uninit-masked3.c); manufactured jumpers (jump1 cleans
  adjacent-jump forms back to the attractor); young-label→find_basic_block(-1)→all-live
  (cross-jump proven inert in this config); 9-variant check/tail topology sweep (all
  identical 4/178); dst/dst2 merge on this chassis (masked 19); **cc1psx parity: PsyQ's
  own compiler emits the IDENTICAL steal for our source (tmp/psxregion3.py) — the
  compiler fork is NOT the variable; the ORIGINAL SOURCE was shaped differently in a
  way not yet guessed.**

## How to resume

1. `tmp/probe.py memory/wip/marionation_Exec/candidate.c` → confirm masked 4.
2. Relaunch the rich-pass permuter: `bash tmp/perm_mar/launch_detached.sh` (base.c =
   vT40 body; settings.toml = construct-honesty passes, only external/function type
   randomization off), then `nohup setsid bash tmp/perm_watch.sh &` for auto-triage
   (sub-200 finds get honest sandbox scores in tmp/perm_mar/triage.log). ~20 iters/sec.
   Finds are PROPOSALS: vet semantics (uninitialized reads!), bans (volatile/
   cross-symbol), and remember insns must be able to reach 179.
3. Fresh structural ideas belong in the unknown-source-shape space (everything
   enumerable was enumerated — see notes.md SESSION-10c CLOSURES before trying
   anything "new").
4. On masked 0: retire all 42 rules → full SHA1 == oracle → LAYER-2 fresh adversarial
   review (independent nested-wrap ruling required) → reintegrate to main under the
   reintegration lock → `queue done marionation_Exec` → delete this WIP dir.

## Tooling map (tmp/, this worktree)

probe.py (splice+sandbox+greg ledger) · adiff.py (LCS diff) · dumpours.py ·
ordersweep.py/tailsweep.py/pairsweep.py (batteries) · sched1dump.py / rtlorder.py /
notecheck.py / pseudomap.py (RTL dumps) · dbrdbg.py / candbr*.py (DBRDBG) · qtydbg.py
(QTYDBG) · knobs.py / knobsb.sh (canonical-cc1 what-if knobs: BB2_ALLLIVE_LABEL /
BB2_DBR_DEBUG / BB2_NO_FT_STEAL — env-gated, inert unset) · psxregion3.py (cc1psx
side-by-side) · killsearch.sh (stop the campaign cleanly) · perm_mar/ (permuter
workspace; archive_pre_rich/ holds the earlier campaigns' outputs).

## Session-10 ledger (one line each)

Ruling saga: interim not-sanctioned → owner-directed SOTN re-evaluation → FINAL
construct-honesty sanction on main. Reviews: FAIL → FAIL(provenance+figure, both
fixed) → PASS-except-nested-wrap. Measurements: exhaustive ordering floor 4; wrap
toolbox closed for the pair; cc1psx parity negative; label-cost theorem; merge axis
closed; ~107k rich-pass permuter iterations (best usable: none; best signpost:
masked-3 rejected). All committed on work/marion (through 1d703e2b + this handoff).
