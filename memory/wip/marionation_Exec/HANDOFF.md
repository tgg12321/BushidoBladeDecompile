# marionation_Exec — HANDOFF (session-10, 2026-07-06)

Narrative handoff. The compact current-state is `notes.md` (≤120-line cap); this file is
the fuller story. History lives in git (`git log --oneline -- memory/wip/marionation_Exec/`).

## TL;DR — where we stand

- **Best committed form: `progress/vT31-tailwrap-masked4.c`** (= session-9's vDT30 plus an
  inert tail-wrap): 8/8 callee-saved registers, both check `andi ,0xff` masks, the
  saved-stage — all honest pure C, **masked 4** (178 vs 179 insns), zero rules/pins/asm.
- **The two residuals are now ROOT-CAUSED to exact GCC-2.7.2 pass decisions** (session-10),
  with a knob-level byte-proof for region-3. What remains is finding natural-C spellings
  that flip two specific compiler tie-breaks — a search problem, no longer a mystery.
- A **clean permuter campaign is running on the vT32 basin** (`tmp/perm_mar/`, order-correct
  base, weighted 200, cheat-prone passes disabled, --stop-on-zero).
- `src/system.c` is UNTOUCHED (oracle green). Candidates splice via `tmp/probe.py <c>`.

## CORRECTION to the session-9 handoff

Session-9 claimed "Region-3 IS fixed in vDT48 (real loop)". **That is wrong** — vDT48's
adiff also lacks the nop at tgt[149] (verified session-10). The real-loop family remains
dead for independent reasons (conservation barrier), and region-3 was never fixed by any
form to date. Per [[verify-claims-against-main]]: measured, not inherited.

## Residual 1 — do_timeout pair-swap (2 masked pts)

Target order `addu v0,v0,s5` THEN `sll a0`; ours swapped. Session-10 findings:
- **Order half SOLVED**: put the `arg5 = *(s32*)(v0+(s32)tbl_125c);` statement BEFORE
  `t0 *= 4; t0 = tbl+t0;` (loads stay first) — LUID tie flips, order matches
  (`progress/vT32-pairorder-masked8.c`).
- **Cost**: the t0-web and arg5val temps exchange seats (v1↔a0). Root cause read from
  local-alloc.c + QTYDBG: `qty_compare` pri = floor_log2(refs)·refs·size/life; the two
  qtys tie exactly (r4·l6 = 5.33 both) and the tie breaks by qty birth order (t0-web
  born first → allocated first → takes v1; target wants it skipping to a0).
- **What must change**: arg5val's weighted refs 4→6 (density 8.0) or t0-web's 4→2 —
  WITHOUT adding pseudos (fresh single-set temps LAUNCH per sched.c birthing_insn_p and
  re-time the head: vT33=16, vT34=11) and WITHOUT note-stream changes inside the window
  (do-while(0) nesting shifts LUIDs/sched: vT35=15, vT36=14). All flat respellings
  canonicalize identically (six forms all = 8). Split-init cannot apply to a pure load
  (identity ops fold at tree level).

## Residual 2 — region-3 delay-slot steal (2 masked pts)

Target: `beqz a2,C4; nop; sb zero,-1(s3); move a1,s4`. Ours steals the move into the slot.
Session-10 established the complete mechanism (DBRDBG traces on our own compile):
- reorg runs fill_simple for ALL insns before fill_eager. Check1's `dst=a1` move gets
  consumed by `beqz s4`'s slot (nearest-preceding, matching target), which is why check1's
  slot is nop for free. Check2's nearest-preceding for `beqz a1` is `li v1,7` (also matching
  target), leaving check2's move exposed in the eager fall-through window, where it wins
  (`thr WINNER trial=450`): sb loses on trap/mem, `la` is 2-word-ineligible, move is clean.
- **Byte-proof**: the canonical cc1 carries env-gated what-if knobs (inert unset; oracle
  green proves it). `BB2_ALLLIVE_LABEL=513,627` — the tail-thread insn uid AND its pass-2
  SEQUENCE uid — forces mark_target_live_regs all-live at the tail → the move is rejected
  (setsopp=1) in both reorg passes → **the emitted asm matches target region-3 exactly**,
  including the backedge `beqz s7; move v0,zero` slot. So all-live-at-tail IS the answer;
  only its natural-C trigger is unknown.
- **Impossibility results** (exhaustive, do not re-derive):
  - Genuine a1-liveness at the tail is impossible in ANY spelling: every pseudo live there
    crosses the loop's calls → callee-saved only; flow liveness and reorg's forward-scan
    (stops at the conditional backedge) are both accurate.
  - own_fallthrough=0 needs a CODE_LABEL with surviving uses immediately after beqz-a2 —
    semantically impossible (do-while(0) top labels get deleted; a while(cond)-backedge
    with an unprovably-false cond adds bytes).
  - The only reachable all-live route is find_basic_block()==-1, requiring the tail label
    to be POST-FLOW (unknown to basic_block_head[]). The one post-flow label creator is
    jump2 cross-jumping — **proven inert in this toolchain config**: our two identical
    `j epi; move v0,a2` return tails stay unmerged in ours AND in target.
  - Prediction is NOT the blocker: vT31's tail-wrap (`do { tail: if (a0==0) goto loop; }
    while(0); return 0;` — LOOP_BEG immediately before the interior label) makes
    mostly_true_jump return 2 (verified), but reorg still falls back to the owned
    fall-through after the target-thread fill fails.
- Topology battery: check2-as-do-while-break = 14; tail-as-while-goto = inert 4.

## Closed side-quests (session-10)

- **u8-typed checks (mask removal)**: PROMOTE_MODE keeps u8 locals SI-extended; combine
  merges lbu+extension whenever the loaded value dies (→ `lbu a2` direct, andi gone).
  The redundant `andi ,0xff` REQUIRES the mask-var + reload constant-substitution, which
  requires the goto-loop's refs==2 update_equiv_regs fold. vU1/vU2 measured 17 (no s8,
  no andi). This also explains why `& 3` (saved) survives while `& 0xFF` folds.
- Conservation barrier confirmed categorical for the real-loop family.

## Tooling (tmp/, worktree bb2-work-marion; all session-10 additions committed-adjacent)

- `probe.py` (splice+sandbox+greg ledger), `adiff.py` (LCS diff), `dumpours.py`,
  `rtlorder.py`/`notecheck.py`/`pseudomap.py` (RTL dumps), `dbrdbg.py`/`candbr*.py`
  (DBRDBG), `qtydbg.py` (QTYDBG), `knobs.py`/`knobsb.sh` (what-if knobs).
- Canonical cc1 knobs: BB2_ALLLIVE_LABEL / BB2_DBR_DEBUG / BB2_NO_FT_STEAL (+ QTY/FINDREG/
  ALLOC/FLOW debug). gccdbg lacks ALLLIVE — use `tools/gcc-2.7.2/cc1` for that one.
- Permuter: `tmp/perm_mar/` (base.c = vT32 body; launch.sh; campaign.log). Finds are
  PROPOSALS — re-verify via sandbox + cheat catalog; the permuter TU metric is unfaithful.

## How to resume

1. `tmp/probe.py progress/vT31-tailwrap-masked4.c` → confirm masked 4.
2. Check `tmp/perm_mar/campaign.log` / `output-*` for finds; vet any against the cheat
   catalog, re-score via sandbox.
3. Hand levers left: (pair) end the do_timeout wrapper between t0's add and the printf
   (vT35 moved too much at once); (region-3) audit jump.c's get_label_before/after call
   sites for any other post-flow label creator reachable from natural C.
4. On masked 0: retire all 42 rules → full SHA1 == oracle → dual adversarial review →
   queue done → delete WIP.
