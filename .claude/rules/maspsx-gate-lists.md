---
name: maspsx-gate-lists
paths: ["maspsx_label_nop_funcs.txt", "expand_lb_funcs.txt", "expand_dest_funcs.txt", "multu_funcs.txt", "multu_pad_funcs.txt", "engine/cheats.py"]
description: "Adjudication (2026-07-13 audit) of the five per-function maspsx gate lists: label-nop / expand-lb / expand-dest are FIDELITY shims (no C spelling exists — probe-proven), multu / multu-pad are CHEAT-PATHWAY (a pure-C spelling exists; current entries vestigial/dormant). Engine + integrity checker + commit guard now track them; growth requires the [infra-rule] tag."
metadata:
  type: rule
---

# The five per-function maspsx gate lists — adjudicated 2026-07-13

The 2026-07-13 sanctioned-mechanism audit found that the per-function maspsx
gate lists were the ONE mechanism outside the cheat taxonomy: part of the
canonical build (`engine/buildconfig.py`), never sandbox-stripped, invisible
to `queue done` / `check_completion_integrity.py` / every commit hook — while
6 COMPLETED-C functions depended on entries. This rule records the evidence-
based adjudication and the enforcement that now exists.

## The adjudication

| List | Class | Evidence |
|---|---|---|
| `maspsx_label_nop_funcs.txt` | **fidelity** (deferred-global) | ASPSX emitted the load-delay nop across merge labels; upstream maspsx `is_label()` matches `$L` but our cc1 fork emits `.L`, so the nop is silently dropped ([[maspsx-label-nop-gate]], [[maspsx-is-label-dot-prefix]]). No C spelling can emit an assembler hazard nop. Per-function scoping exists ONLY to avoid shifting the indices under index-anchored regfix rules — see "endgame" below. |
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
- `tools/hooks/no_new_regfix_guard.py` — net additions to a gate list are
  blocked unless the commit body carries the matching tag
  (`[infra-rule: maspsx-label-nop]`, `[infra-rule: expand-lb]`,
  `[infra-rule: expand-dest]`) + a one-line justification citing the
  target-site evidence. Additions to `multu_funcs.txt` /
  `multu_pad_funcs.txt` are blocked UNCONDITIONALLY — fix the C.

## Current gate-dependent completions (all legitimate under this adjudication)

label-nop (fidelity): spu_DmaTransfer, cdrom_DmaToRam, gnd_get_fog,
AllocRobRmd, func_80088740 — COMPLETED-C stands.
expand-lb (fidelity): func_8003047C — COMPLETED-C stands.
expand-dest: func_8007CE0C — in queue (its other debt); the gate entry is
fidelity and may remain when it completes.
multu (vestigial): func_8007F87C, func_8007FA1C — canonical-asm; entries dead.

## Endgame / deferred work

1. **Globalize the label-nop fix** (align `is_label()` with `.L`) once
   index-anchored regfix rules no longer pin maspsx output indices — i.e. at
   or near regfix-zero. Before globalizing, survey ungated `.L`-label hazard
   sites: any site where the target LACKS the nop would falsify uniform-ASPSX
   behavior and demands investigation first. Until then the per-function gate
   is the established mechanism ([[no-compiler-divergence]] §2).
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
- [[lost-codegen-insert-cheat]] — the regfix sibling of multu_pad-style
  nop injection
