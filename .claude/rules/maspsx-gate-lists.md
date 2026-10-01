---
name: maspsx-gate-lists
paths: ["maspsx_prefill_label_funcs.txt", "maspsx_comm_syms.txt", "expand_lb_funcs.txt", "expand_dest_funcs.txt", "multu_funcs.txt", "multu_pad_funcs.txt", "engine/cheats.py"]
description: "Per-function maspsx gate lists: prefill-label / expand-lb / expand-dest are FIDELITY shims (no C spelling exists); multu / multu-pad / comm-syms are CHEAT-PATHWAY. Growth needs the [infra-rule] tag + target-site evidence."
metadata:
  type: rule
---

# The per-function maspsx gate lists

maspsx behaviour changes are admissible only as GLOBAL models of documented ASPSX behaviour, verified against
Sony's tools and byte-neutral for every completed function (decisions.md 2026-09-30 "maspsx `.L`-label
mflo-hazard fix adopted"). The per-function gates below are the only exceptions, classified as follows.

| List | Class | Operative content |
|---|---|---|
| `maspsx_prefill_label_funcs.txt` | **fidelity** (owner ruling 2026-09-04) | Models ASPSX "retarget iff filled": an unfilled reorder-mode branch to L whose preceding instruction P is verbatim a filled branch's delay slot is retargeted to a fresh `L_pf` label before P. No instruction added/removed/reordered; the label emits no bytes. Per-function, never globalize (36 target sites in 33 matched functions legitimately sit on the post-P label). Tag `[infra-rule: maspsx-prefill-label]` + target-site evidence. |
| `expand_lb_funcs.txt` | **fidelity** | The target's adjacent `lbu; sll 24; sra 24` is unreachable from any C in this fork (all spellings fold to `lb` in combine); the list encodes which sites ASPSX expanded. Tag `[infra-rule: expand-lb]`. |
| `expand_dest_funcs.txt` | **fidelity** | Which scratch register ($at vs $rdest) the assembler uses to expand a macro load is not controllable from C. Tag `[infra-rule: expand-dest]`. |
| `multu_funcs.txt` | **cheat-pathway** (entries vestigial) | `multu` is reachable from C via unsigned operand types — gating a C function through this list is a cheat by config. The 2 entries (func_8007F87C, func_8007FA1C) are dead: whole-body canonical asm never emits `.ent`. |
| `multu_pad_funcs.txt` | **cheat-pathway** (dormant) | Injects literal nops from config. 0 entries. |
| `maspsx_comm_syms.txt` | **RETIRED — CHEAT** (owner ruling 2026-09-30) | A per-function assembler toggle is a lever outside the committed C. The list stays EMPTY; no row, no per-function or per-symbol toggle, no Q9/Q15/Q18 revival. Record: decisions.md 2026-09-30 OWNER RULING — the per-function maspsx COMMON gate is a cheat. |

Fidelity gates are NOT sandbox-stripped (stripping would score against a wrong toolchain model). Each gate
applies only its one narrow, semantically-neutral transform at pattern-matched sites, and only "works" if the
target bytes have that shape (the oracle enforces it). Additions to `multu_funcs.txt` / `multu_pad_funcs.txt`
are forbidden outright — fix the C. A commit adding to a fidelity list carries its tag + a one-line
justification citing the target-site evidence (convention: no hook enforces this today).

`sdata_syms.txt`, `sdata_funcs.txt` and `sdata_exclude.txt` are deleted: the per-file gp model
([[per-file-gp-model]], adopted 2026-10-01, b6c0c0d24..4fca9a4dd) decides gp per file from that file's own
definition of the symbol, per its rule table (gp only for a <= 8-byte definition in the file; `.comm` at the
base offset only; an indexed `S($reg)` operand never). No gp list rows exist to add.

## The global COMMON model — tentative definitions, every file (owner ruling 2026-09-30, Q62)

maspsx parses the three-field `.comm name,size,align` directive, and `--use-comm-section` is on for EVERY file
(Makefile, mirrored verbatim in `engine/buildconfig.py`). A tentative definition stays a COMMON symbol GNU ld
resolves to its symbol-file address; gp is used at the base only (measured ASPSX 2.34 behaviour). A file-scope
tentative definition (`CdlATV g_cd_atv;`) is chosen by evidence (the target's gp-at-base / lui-at-offset
shape), and only for an object with no starting value: the original EXE bytes over its whole extent are zero
(ledger cites disc offset + bytes), no TU gives it an initializer, and its address comes from a symbol-file
row. A tentative definition over non-zero original bytes is false C and refused. Type/layout stay under the
aggregate-merge entry of [[no-new-park-categories]]. Record: decisions.md 2026-09-30 OWNER RULING — the global
COMMON model.

## Enforcement

- `engine/cheats.py: MASPSX_GATE_LISTS` + `maspsx_gate_entries(func)` — single source of truth for the classes.
- `engine/queue.py: mark_done()` refuses completion for a non-canonical function in a cheat-pathway list and
  reports fidelity-gate dependencies (`maspsx_gates`).
- `tools/check_completion_integrity.py` flags any COMPLETED-C function in a cheat-pathway list.
- Gate lists are on the add-scope-allow denylist (`_SCOPE_GRANT_DENY` in `tools/grinder/grindlib.py`).

Current dependents: prefill-label — `main`; expand-lb — func_8003047C; expand-dest — func_8007CE0C.

## Related

[[per-file-gp-model]] · [[no-compiler-divergence]] · [[no-new-park-categories]]
