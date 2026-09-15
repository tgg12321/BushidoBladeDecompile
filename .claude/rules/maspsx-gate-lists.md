---
name: maspsx-gate-lists
paths: ["maspsx_prefill_label_funcs.txt", "expand_lb_funcs.txt", "expand_dest_funcs.txt", "multu_funcs.txt", "multu_pad_funcs.txt", "engine/cheats.py"]
description: "Adjudication (2026-07-13 audit) of the per-function maspsx gate lists (FIVE at the time; label-nop RETIRED 2026-09-14, four remain): label-nop / expand-lb / expand-dest are FIDELITY shims (no C spelling exists — probe-proven), multu / multu-pad are CHEAT-PATHWAY (a pure-C spelling exists; current entries vestigial/dormant). Engine + integrity checker + commit guard now track them; growth requires the [infra-rule] tag."
metadata:
  type: rule
---

# The per-function maspsx gate lists — adjudicated 2026-07-13

> **Update 2026-09-14 — `maspsx_label_nop_funcs.txt` is RETIRED and DELETED.**
> Its `.L`-label arm is now global. The per-function scoping was never
> protecting ASPSX fidelity: it was containing a missing-guard bug (the arm
> skipped the `$at`/`$gp` expansion checks the non-label path always applied).
> Fixing the guard made the whole list redundant — oracle green with zero
> entries. Full record: [[maspsx-label-nop-gate]]. **Five lists became four.**

> **Historical framing note (2026-08-30):** this rule predates the removal of the
> regfix/asmfix rule system (retired at zero rules; machinery deleted). Where the
> symptom text says a function "carries a rule", read it as "the honest build shows
> this diff shape vs target". The technique itself is unchanged.

The 2026-07-13 sanctioned-mechanism audit found that the per-function maspsx
gate lists were the ONE mechanism outside the cheat taxonomy: part of the
canonical build (`engine/buildconfig.py`), never sandbox-stripped, invisible
to `queue done` / `check_completion_integrity.py` / every commit hook — while
6 COMPLETED-C functions depended on entries. This rule records the evidence-
based adjudication and the enforcement that now exists.

## The adjudication

| List | Class | Evidence |
|---|---|---|
| `maspsx_prefill_label_funcs.txt` | **fidelity** (assembler label placement; owner ruling 2026-09-04) | ASPSX "retarget iff filled": it filled a branch's delay slot with the instruction at the target label and pointed THAT branch one word past it; an unfilled branch kept pointing at the label. Our cc1's reorg does the filling itself and, having proven the instruction redundant on the unfilled paths, deletes the pre-instruction label and retargets the unfilled branches too — two branch words differ, no C spelling can move an assembler label (main: 7 escape hatches probed, permuter blind by design; cc1psx on the identical `ings.i` keeps the single label). The gate re-emits the label before P for opted-in functions only: an unfilled reorder-mode branch to L whose preceding instruction P is verbatim a filled branch's delay slot is retargeted to a fresh `L_pf` label before P. No instruction added/removed/reordered; the label emits no bytes. Per-function because a read-only census found 36 target sites in 33 matched functions that legitimately sit on the post-P label — never globalize. Growth tag: `[infra-rule: maspsx-prefill-label]` + target-site evidence. Record: decisions.md 2026-09-04 OWNER RULING (main). |
| `expand_lb_funcs.txt` | **fidelity** (unmodeled context) | The target's adjacent `lbu; sll 24; sra 24` (e.g. func_8003047C @ 0x800304AC) is UNREACHABLE from any C in this fork: a 4-spelling probe (2026-07-13, tmp/lb_probe.c — explicit shifts, (s8) cast, plain s8 load, named-temp shifts) all fold to `lb` in combine. The Makefile documents the expansion as ASPSX behavior "in certain contexts" (Makefile:106-108); the per-site list encodes which sites the original expanded. |
| `expand_dest_funcs.txt` | **fidelity** (assembler-internal) | Which scratch register ($at vs $rdest) the assembler uses to expand a macro load is not controllable from C at all. The list models an ASPSX-internal choice our maspsx doesn't fully capture. |
| `multu_funcs.txt` | **cheat-pathway; current entries VESTIGIAL** | `mult`→`multu` IS reachable from C (unsigned operand types emit `multu` naturally) — gating a C function through this list instead of fixing the types is a cheat by config. The 2 current entries (func_8007F87C, func_8007FA1C) are DEAD: both are now whole-body canonical asm writing `multu` literally, and glabel bodies never emit `.ent`, so maspsx `current_func` never matches them. |
| `multu_pad_funcs.txt` | **cheat-pathway; DORMANT** | Injects literal nops between mult/mflo from config — bytes not from compilation. 0 active entries. |

Why fidelity gates are NOT sandbox-stripped: stripping them would score the
function against a WRONG toolchain model (the honest distance would include a
gap no C can close). They are toolchain config, like `fix_lwl` — but unlike
`fix_lwl` they are per-function, so they carry a transparency + growth duty.

Key honesty property shared by all five: an entry only "works" if the target
bytes have the shape (the oracle enforces it), and each gate can apply only
its one narrow, semantically-neutral transform at pattern-matched sites — no
gate can inject arbitrary bytes.

## Enforcement (wired 2026-07-13)

- `engine/cheats.py: MASPSX_GATE_LISTS` + `maspsx_gate_entries(func)` — the
  single source of truth for the classification.
- `engine/queue.py: mark_done()` — REFUSES completion for a non-canonical
  function named in a cheat-pathway list; reports fidelity-gate dependencies
  in its success payload (`maspsx_gates`).
- `tools/check_completion_integrity.py` — violation for any COMPLETED-C
  function in a cheat-pathway list; informational listing of every
  fidelity-gate-dependent completion; vestigial note for canonical bodies.
- ~~`tools/hooks/no_new_regfix_guard.py`~~ — **this hook no longer exists.**
  It was removed on 2026-08-30 with the regfix machinery, which silently left
  gate-list growth UNENFORCED; the stale citation survived here until
  2026-09-14. The live commit-msg chain is `park_src_guard`,
  `wip_compaction_guard` and `detector_config_guard`, and the last watches
  only `volatile_extern_allowlist.txt` + `inline_asm_canonical.txt`.
  **The growth duty below is therefore convention, not enforcement** for the
  four remaining lists. A commit adding to one still carries the matching tag
  (`[infra-rule: expand-lb]`, `[infra-rule: expand-dest]`,
  `[infra-rule: maspsx-prefill-label]`) + a one-line justification citing the
  target-site evidence; additions to `multu_funcs.txt` / `multu_pad_funcs.txt`
  remain forbidden outright — fix the C. Re-arming this mechanically
  (extending `detector_config_guard.py` to the gate lists) is OPEN WORK.

## Current gate-dependent completions (all legitimate under this adjudication)

label-nop: RETIRED 2026-09-14 — the list is deleted and no completion depends
on it. The 9 functions that were listed still byte-match with the arm global.
prefill-label (fidelity): main — first and only entry (owner ruling 2026-09-04).
expand-lb (fidelity): func_8003047C — COMPLETED-C stands.
expand-dest: func_8007CE0C — in queue (its other debt); the gate entry is
fidelity and may remain when it completes.
multu (vestigial): func_8007F87C, func_8007FA1C — canonical-asm; entries dead.

## Endgame / deferred work

1. ~~**Globalize the label-nop fix**~~ — **DONE 2026-09-14**, though not the
   way this item predicted. The blocker was never `is_label()` (the fork
   handled `.L` in its own dedicated block all along) and never the regfix
   indices: it was a missing `$at`/`$gp` expansion guard in the `.L` arm.
   Guard added, arm globalized, list deleted. Note for anyone planning a
   similar change: the target-side survey this item called for WAS run (33
   sites, 0 falsifiers) and was **not sufficient** — it is blind to sites that
   exist in our output but have no counterpart label in the target, which is
   precisely where the one regression lived. Only a full build against the
   oracle settles a gate change. See [[maspsx-label-nop-gate]].
2. **Delete the two vestigial `multu_funcs.txt` entries** at the next idle
   rebuild window (byte-neutral — verify with `verify-oracle --rebuild`; not
   done mid-grind because gate files are oracle-staleness-watched).
3. `EXPAND_LB_FILES := code6cac_b` (Makefile:109) is a redundant duplicate:
   the base MASPSX_FLAGS already carry `--expand-lb --expand-lb-funcs=...`,
   and the funcs list restricts expansion globally. Fold it away in the same
   idle window.

## Related

- [[maspsx-label-nop-gate]] — the label-nop mechanism + why per-function
- [[no-compiler-divergence]] — names the gates "the established mechanism";
  new GLOBAL maspsx behavior changes still need user sign-off
- [[no-new-park-categories]] — cheats-by-any-spelling; the multu/multu-pad
  classification is that policy applied to build config
- `lost-codegen-insert-cheat` (retired rule, deleted 2026-08-30) — the regfix sibling of multu_pad-style
  nop injection
