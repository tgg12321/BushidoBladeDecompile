---
name: maspsx-gate-lists
paths: ["maspsx_prefill_label_funcs.txt", "maspsx_comm_syms.txt", "expand_lb_funcs.txt", "expand_dest_funcs.txt", "multu_funcs.txt", "multu_pad_funcs.txt", "engine/cheats.py"]
description: "Adjudication (2026-07-13 audit) of the per-function maspsx gate lists (FIVE at the time; label-nop RETIRED 2026-09-14, four remain; a fifth, maspsx_comm_syms.txt, authorized 2026-09-26 and not yet built): label-nop / expand-lb / expand-dest are FIDELITY shims (no C spelling exists — probe-proven), multu / multu-pad are CHEAT-PATHWAY (a pure-C spelling exists; current entries vestigial/dormant). Engine + integrity checker + commit guard now track them; growth requires the [infra-rule] tag."
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
| `maspsx_comm_syms.txt` | **fidelity** (assembler storage-class knowledge; owner ruling 2026-09-26, fourth batch) | Sony ASPSX 2.34 gives a tentative (`.comm`) definition gp for its base byte only, never for `sym+N`; our C declares every such variable `extern`, so maspsx cannot know the storage class. Per-function rows, each proven by ASPSX reproducing the shipped bytes; admission prongs (a)-(d) in § "`maspsx_comm_syms.txt`" below. Authorized; not yet built at the time of the ruling. Growth tag: `[infra-rule: maspsx-comm]`. Record: decisions.md 2026-09-26 OWNER RULING — maspsx COMMON gate. |
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

## `maspsx_comm_syms.txt` — the COMMON (tentative-definition) gate (owner ruling 2026-09-26, fourth batch)

**Question and answer.** Filed question: docs/grind/borderline.md 2026-09-26
func_80036140 (item (a), "tell maspsx that g_cd_atv / D_800A36B8 were COMMON
in the original TU"). Evidence: memory/grind/func_80036140/research-common-gp.md
(commit 2974e2e6b). The context given to the owner (record:
docs/grind/owner-rulings-2026-09-26.md, batch 4): Sony ASPSX 2.34, run under
dosemu2, shows that tentative (`.comm`) definitions get gp only for the first
byte, never for `sym+k`; a census of 2,271 gp accesses in the shipped binary
found zero exceptions; all 34 src objects build byte-identical with the
proposed list. The question put to the owner, verbatim: "Add a small
per-function list telling our assembler shim which variables were plain
'declared, no initial value' in the original source (so it stops using the gp
register for their byte offsets) — proven by Sony's own ASPSX assembler
reproducing the shipped bytes, zero exceptions across the binary, and all
current files byte-identical with it?" Owner (Trenton) chose, verbatim:
**"Allow, gated list (Recommended)"**, whose text is: "maspsx_comm_syms.txt
naming only functions whose listed variables are proven by Sony's assembler;
full oracle + layer-2 before use."

**Class: fidelity** (an assembler-internal choice no C spelling reaches while
our C declares every such variable `extern`; research-common-gp.md §0-§1).
Like the other fidelity gates it is part of the canonical build and is never
sandbox-stripped. It is per-function and must never be globalized: the same
census shows `sym+k` gp accesses that are correct for initialized and
`static` objects (g_anim_select, D_800A34F0, D_800A3588/358C), which a global
rule would break.

**Rule text** (the author's narrowing, not the owner's words). The file
format is `func: sym, sym` (the `sdata_exclude.txt` format). A row is admitted,
and the gate may be used, ONLY when all of (a)-(d) hold:

- **(a) Each listed variable is proven by Sony's assembler, per function.**
  For every row, the function's ledger (`memory/grind/<func>/`, or for a
  function already COMPLETED-C the ledger of the function whose landing adds
  the row) banks all of:
  1. **The shipped signature.** In that function's own original bytes, the
     listed symbol's base access is gp-relative and at least one `sym+N`
     access (N != 0) is a direct `lui`/`%lo` access, each listed by address
     and instruction.
  2. **Sony's assembler reproduces it.** The original PsyQ ASPSX 2.34 (the
     psyq3.5 archive upstream maspsx uses, run under dosemu2) assembles the
     original PsyQ cc1psx's `-G8` output for the function (or its TU) with
     the symbol as a tentative definition (`T sym;`), and the result matches
     the shipped words of that function, relocation immediates masked.
  3. **The alternatives do not.** The same run with the symbol declared
     `extern`, `static` and initialized, and with the tentative definition
     compiled at `-G0`, each differs from the shipped words.
  4. The outputs, the command lines and the per-word comparison are banked.
  This is a calibration use of cc1psx and ASPSX under
  [[cc1psx-calibration-only]] and [[no-compiler-divergence]]: neither is ever
  a build path. A row whose function or symbol lacks any of 1-4 is not
  admitted, even when its bytes would match.
- **(b) The gate does one thing.** In a listed function (matched on `.ent`,
  so `INCLUDE_ASM` bodies are never touched), an operand `sym+N` with N != 0,
  where `sym` is listed for that function, is assembled in maspsx's ordinary
  non-gp form instead of gp-relative. Nothing else changes: not the base
  access `sym`, not any unlisted symbol or unlisted function, no storage is
  emitted, and no instruction is added, removed or reordered beyond maspsx's
  existing non-gp expansion of that operand. The gate can only remove gp,
  never add it.
- **(c) Bytes.** The maspsx change and its first rows land in one change with
  `verify-oracle --rebuild` (full-build SHA1 == oracle) and `engine test`
  green. That change also proves, by building every `src/*.c` with the
  Makefile's exact per-file recipe once with the stock maspsx and once with
  the gated maspsx and its rows, that every object is byte-identical. A later
  row addition proves the same for every object except the TU of the function
  it lands, which the oracle SHA1 then decides. `--comm-syms` goes on both
  `MASPSX_FLAGS` and `MASPSX_FLAGS_GP` in the Makefile, mirrored verbatim in
  `engine/buildconfig.py`. The list is registered everywhere a gate list must
  be, in the same change: `engine/cheats.py` `MASPSX_GATE_LISTS` as
  "fidelity", `PIPELINE_DEPS`, the oracle watch, the dossier, the root
  allowlist, the row in the adjudication table above, and BOTH halves of the
  add-scope-allow denylist pair (integration-handoff-self-serve.md and
  `_SCOPE_GRANT_DENY` in `tools/grinder/grindlib.py`), kept in sync.
- **(d) Review.** A fresh layer-2 `cheat-reviewer` PASSes the maspsx change,
  the list and the registration before the gate is used, and every later row
  addition is covered by the landing's own fresh layer-2. Growth carries the
  tag `[infra-rule: maspsx-comm]` with the (a) evidence cited, as for the
  other fidelity lists. The Grinder cannot add rows: `tools/`, the Makefile
  and the list are outside session scope.

**What this does not decide.** It admits no C construct. The CdlATV record
merge and any record extension that func_80036140 needs are judged under the
aggregate-merge entry of [[no-new-park-categories]] on their own evidence, and
its `-G8` file under compiler-flags-canonical.md § "Per-file -G8 by proof". It
pre-decides no landing. Record: docs/grind/decisions.md 2026-09-26 OWNER
RULING — maspsx COMMON gate (`maspsx_comm_syms.txt`).

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
