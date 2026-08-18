# ASM-SUSPECT triage — 2026-08-18

**Outcome: ZERO hand-coded-asm candidates among the 47 `ASM-SUSPECT` queue items.**
A four-agent triage read every one against `tools/scan_hand_coded.py` and the target
disassembly. Every function scored tier **LOW** (0-2 of 8), and **none** of the
decisive signals (S1 uniform multu pacing, S2 empty-body branch, S6 BIOS jumptable
with delay-slot register setup) fired anywhere in the population. 43 are ordinary
GCC 2.7.2 output; 4 are compiled Sony PsyQ library C (plus `sprintf`, also library C).

None of them is canonical-asm work. All 47 are pure-C targets — the label was
never evidence about the original source.

## The two systemic findings

### 1. The `> 500` ASM-SUSPECT band is a "never attempted" artifact

26 of the 47 sit above `NEAR_CERTAIN_DISTANCE` (500). Every one of those has
**zero rules and no C body in `src/`** — the function is still asm-supplied, so the
honest pure-C distance that `engine/queue.py` records is the function's *entire*
instruction count (`generate()` sets `dist = len(score.normalized_insns(...))` for
the no-C-body case, precisely so easiest-first ordering stays meaningful). A
distance of 2991 for `func_80058580` therefore says "2991 instructions of C have
not been written yet", not "this looks like hand assembly".

`.claude/rules/canonical-gate-distance-not-evidence.md` already made this call for
the 2026-06-09 population; this triage confirms it holds for the whole band, with
the scanner run per function as the independent check.

### 2. The `hand_coded_tier` guardrail never populated (root cause)

Every ASM-SUSPECT entry in `engine/queue.json` carried **no `hand_coded_tier` field
at all** (all 264 items), even though the gate's `> 500` branch is supposed to
consult `scan_hand_coded` for corroboration. Three independent causes:

1. **`engine/queue.py:_route()` threw the tier away.** It re-implemented
   `canonical._verdict`'s structural tier instead of calling it (the docstring
   claimed "mirrors canonical._verdict EXACTLY" while keeping a second copy of the
   rules). It called `canonical._hand_coded_tier(func)` for the `> 500` decision
   and returned only a verdict string, so `generate()` had nothing to persist.
   `canonical._verdict` *did* set `hand_coded_tier`, but that dict is the
   `engine canonical <func>` CLI path — the queue never touches it.
2. **The `50 < distance <= 500` band never ran the scanner at all.** Both
   `_route` and `_verdict` reached `ASM-SUSPECT` on distance alone there, so 21 of
   the 47 items had no tier because none was ever computed.
3. **Sticky items bypass routing entirely.** The 26 owner_override items (the
   2026-06-09 audit population the owner returned to active on 2026-08-01) are
   carried verbatim from the previous queue by `generate()`'s `if func in prev`
   branch, so even a fixed `_route` would not have annotated them.

## The fix (2026-08-18, engine)

- `engine/canonical.py` — `_verdict` consults `scan_hand_coded` for **any**
  distance above `SUSPECT_DISTANCE` (not just above 500) and **records the tier on
  the verdict** in both bands. A scanner-confirmed **`LOW`** tier is treated as
  affirmative counter-evidence and the item is verdicted an ordinary **`C`**
  target. `TIGHT_C` / `UNAVAILABLE` are *not* counter-evidence (unknown is not a
  finding) and keep the conservative `ASM-SUSPECT` label; `STRONG` / `POSSIBLE`
  above 500 still route `ASM-STRUCTURAL` exactly as before.
- `engine/queue.py` — `_route_with_evidence()` **delegates** to
  `canonical._verdict` instead of duplicating the tier rules (so the two can no
  longer drift) and returns `(verdict, tier)`; `generate()` persists
  `hand_coded_tier` on every scanned item. Sticky items get the tier recorded too;
  an *active* sticky `ASM-SUSPECT` with tier `LOW` also sheds the label, with the
  reason appended to its `regate` note. **Status is never moved by the gate** — a
  park or an owner ruling stays as it is.
- `engine/canonical.py` — `_hand_coded_bulk_tiers()` prefills the tier cache from
  ONE `scan_hand_coded.py --all --json` pass. `--single` re-runs the whole-tree
  scan internally (~5s per call), and the gate now consults the tier for ~160
  queue items, so the naive path would have added ~13 minutes to every `queue
  regen`; the bulk dump carries every non-LOW function, so "has an asm file, not
  in the dump" is LOW by construction. Measured: the same 47 lookups went from
  >120s to 8.8s, with identical results. A missing asm file still falls through to
  the per-function path and reports `UNAVAILABLE`.
- `engine/test_engine.py` — coverage for tier persistence in both bands, the
  `LOW` re-verdict (fresh and sticky), the untouched `POSSIBLE`/`STRONG` routing at
  `> 500`, `UNAVAILABLE` staying SUSPECT, and parked/authorize stickiness.
  Suite: **332 passed, 0 failed** (was 304 before this change).

Verified out-of-band (`tools/scan_hand_coded.py` run over the live queue): all 47
current ASM-SUSPECT items return tier `LOW`, so the next `queue regen` re-verdicts
all 47 to `C`. All of them are already in the `active` lane, so no item changes
status — only the label and the newly-recorded evidence change.

## The population

### Compiled Sony PsyQ library C (4 functions + `sprintf`)

| function | file | distance | rules | upstream |
|---|---|---|---|---|
| `CD_ready` | `system` | 56 | 42 | libcd `bios.c` (the CVS-tagged v1.86 1997/03/28 build linked into BB2) |
| `_spu_pitch2note` | `main` | 64 | 0 | libspu |
| `_spu_gcSPU` | `main` | 121 | 104 | libspu SPU-malloc garbage collection |
| `_comb_control` | `main` | 763 | 0 | LIBCOMB link-cable driver (`docs/naming/function-names.csv:1452`, libscan-verbatim VERIFIED); no reference C located |
| `sprintf` | `text1b_b` | 535 | 0 | libc (stdarg homing prologue + jump table) |

### Ordinary GCC 2.7.2 output, never attempted (no C body; distance == full instruction count)

| function | file | distance | rules | note |
|---|---|---|---|---|
| `func_8003993C` | `code6cac_c_mid` | 526 | 0 | no C body in `src/` — distance measures the whole function |
| `func_80034708` | `code6cac_b` | 544 | 0 | no C body in `src/` — distance measures the whole function |
| `func_8005D814` | `text1b` | 545 | 0 | no C body in `src/` — distance measures the whole function |
| `func_8006A880` | `text1b` | 552 | 0 | no C body in `src/` — distance measures the whole function |
| `func_8005F1C8` | `text1b` | 564 | 0 | no C body in `src/` — distance measures the whole function |
| `func_80027AD8` | `code6cac_b` | 574 | 0 | no C body in `src/` — distance measures the whole function |
| `func_8001CE60` | `code6cac` | 588 | 0 | no C body in `src/` — distance measures the whole function |
| `func_80022580` | `code6cac` | 621 | 0 | no C body in `src/` — distance measures the whole function |
| `func_8006C21C` | `text1b` | 622 | 0 | no C body in `src/` — distance measures the whole function |
| `func_800720FC` | `text1b` | 690 | 0 | no C body in `src/` — distance measures the whole function |
| `func_80070188` | `text1b` | 698 | 0 | no C body in `src/` — distance measures the whole function |
| `func_80030D7C` | `code6cac_b` | 709 | 0 | no C body in `src/` — distance measures the whole function |
| `func_800198D0` | `code6cac` | 749 | 0 | no C body in `src/` — distance measures the whole function |
| `func_8005C8A8` | `text1b` | 753 | 0 | no C body in `src/` — distance measures the whole function |
| `func_8005E54C` | `text1b` | 799 | 0 | no C body in `src/` — distance measures the whole function |
| `func_80070F78` | `text1b` | 810 | 0 | no C body in `src/` — distance measures the whole function |
| `func_80029454` | `code6cac_b` | 1025 | 0 | no C body in `src/` — distance measures the whole function |
| `func_80055B60` | `text1b` | 1110 | 0 | no C body in `src/` — distance measures the whole function |
| `func_8002AB08` | `code6cac_b` | 1112 | 0 | no C body in `src/` — distance measures the whole function |
| `func_80023F08` | `code6cac` | 2983 | 0 | no C body in `src/` — distance measures the whole function |
| `func_80058580` | `text1b` | 2991 | 0 | no C body in `src/` — distance measures the whole function |
| `func_80036140` | `code6cac_b2_post` | 512 | 0 | no C body in `src/` — distance measures the whole function |
| `func_8006F97C` | `text1b` | 515 | 0 | no C body in `src/` — distance measures the whole function |
| `func_80055138` | `text1b` | 516 | 0 | no C body in `src/` — distance measures the whole function |

### Ordinary GCC 2.7.2 output, with an existing body and/or rule debt

| function | file | distance | rules | debt |
|---|---|---|---|---|
| `func_800430E4` | `text1a_c` | 52 | 25 | regfix rule debt (count in the rules column) |
| `func_8007352C` | `text1b` | 54 | 11 | regfix rule debt (count in the rules column) |
| `func_8003F6D8` | `config` | 55 | 0 | no rules; C body present, honest distance only |
| `func_8002E6B0` | `code6cac_b` | 57 | 2 | whole-body asmfix (body still asm-supplied) |
| `func_800393C8` | `code6cac_c_mid` | 57 | 2 | whole-body asmfix (body still asm-supplied) |
| `func_80042C80` | `text1a_c` | 58 | 63 | regfix rule debt (count in the rules column) |
| `func_8003F824` | `config` | 61 | 26 | regfix rule debt (count in the rules column) |
| `func_80047BE0` | `sound` | 63 | 0 | no rules; C body present, honest distance only |
| `func_800335D8` | `code6cac_b` | 65 | 43 | regfix rule debt (count in the rules column) |
| `func_8005D554` | `text1b` | 65 | 88 | regfix rule debt (count in the rules column) |
| `func_80074B18` | `text1b` | 79 | 29 | regfix rule debt (count in the rules column) |
| `func_80017848` | `ings` | 84 | 2 | whole-body asmfix (body still asm-supplied) |
| `func_80048864` | `text1b` | 87 | 36 | regfix rule debt (count in the rules column) |
| `func_8003EB84` | `code6cac_c2` | 98 | 2 | whole-body asmfix (body still asm-supplied) |
| `func_80070C70` | `text1b` | 118 | 32 | regfix rule debt + an asmfix splice |
| `func_8003DE14` | `code6cac_c2` | 119 | 36 | regfix rule debt (count in the rules column) |
| `func_8002CA8C` | `code6cac_b` | 177 | 2 | whole-body asmfix (body still asm-supplied) |
| `func_80056CB8` | `text1b` | 188 | 15 | regfix rule debt + an asmfix splice |

## Reconciliation item for the owner

`sprintf` (`text1b_b`) is recorded in `memory/closer/psyq-library-census.md:228` as
`func_80079A30 | text1b_b | parked | 533`, while `engine/queue.json` carries it as
`sprintf | text1b_b | active | 535`. The census row predates the
`func_80079A30` -> `sprintf` rename and its status/distance snapshot is stale in
both fields. Worth reconciling when the census is next regenerated; nothing in the
gate depends on it.

## Related

- `.claude/rules/canonical-gate-distance-not-evidence.md` — the 2026-06-09 audit
  this triage extends from 26 items to the whole band
- `.claude/rules/hand-coded-asm-recognition.md` — the S1..S8 evidence framework
- `.claude/rules/no-new-park-categories.md` — why "it's a big function" is not a
  disposition
