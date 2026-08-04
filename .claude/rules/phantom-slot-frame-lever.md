---
name: phantom-slot-frame-lever
paths: [".claude/rules/phantom-slot-frame-lever.md"]
# on-demand only: surfaced via codegen-technique-index (auto-loads on src/*.c)
description: "DIAGNOSIS RECIPE (2026-08-04): when the target frame reserves 8/16/24 bytes no instruction touches, the slots are unallocated pseudos reload's alter_reg pays off — reproducible from ordinary live C. Three measured producers + the cheap gradient instrument. NOT a sanction; every spelling must independently pass the ordinary cheat tests."
metadata:
  type: reference
---

# Phantom frame slots — the honest lever family

**Provenance:** derived and byte-verified on func_8003D9A0 (COMPLETED-C,
2026-08-04, layer-2 PASS) and func_8003DBE4 (COMPLETED-C same day — its
forbidden `s32 buf[2]` dead-array cheat was DELETED and replaced by this
lever). Partial applications: saTan4FireDisp (2 slots, natural named locals),
func_8007CE0C (2 slots via symmetric clamp-accumulator width). Extends
[[phantom-frame-slots-gcc272]] (the memory that first proved the frame can
reserve untouched bytes without dead declarations) with the CAUSAL mechanism
and reproducible producers.

## Symptom

Target prologue reserves N more frame bytes than your honest build
(`addiu sp,sp,-X` deltas of 8/16/24/32), zero `($sp)` references touch the
extra bytes, and the whole score gap is sp-offset cascade through the
save/restore block. The 2026-08-04 frame census (`tmp/frame_census.py`
pattern) locates every such function mechanically.

## Mechanism (measured, GCC 2.7.2)

An UNALLOCATED pseudo — one with refs but no hard register — reaches reload,
and `alter_reg` assigns it a stack slot that costs zero instructions.
`get_frame_size()` counts it; nothing ever loads or stores it. Producers
measured in this tree (57 instances across 31 TUs — census instrument
`tmp/orphan_census.py` pattern):

1. **Folded loop-guard compare** — a guard comparison pseudo whose compare
   jump/combine fold into a bare branch, leaving the pseudo ref'd but dead.
   Spellings that produce it are ordinary C: `s2 = a1 - 1; if (s2 != -1)`
   (func_8003D9A0), the rotated-while guard `if (i < limit)` re-using the
   loop's own exit comparison (func_8003DBE4). A CONSTANT-folding guard
   (`if (0 < 16)`) produces nothing — the comparison must involve a real
   variable.
2. **combine orphan-USE** (`combine.c:10836-10841`, `distribute_notes`
   REG_DEAD case) — when a death note finds no home and the backward scan
   from i3 hits a CODE_LABEL/JUMP_INSN, combine emits a bare `(use (reg))`.
   Classic source shape: an HImode sign-extend intermediate stranding at a
   label (tslLineG5Init pseudo 92). For a site to orphan, the `reg:HI` being
   widened needs a second use AS AN HIMODE VALUE (func_8007CE0C r8: a
   16-bit accumulator consuming the narrow copy) — an s32 consumer takes
   the widened value and the site folds clean instead. The second HImode
   use must be functionally required by the algorithm (a value the function
   genuinely consumes narrow) — introducing a use merely to trigger
   orphaning is the forbidden no-semantic-purpose class.
3. **Live named locals on multi-read fields** — naming a thrice-read field
   in an s16 local moves vars in +8 steps at zero instruction cost when the
   local is genuinely live (saTan4FireDisp `sid`).

## Instruments

- `.frame` gradient: compile the TU with the project cc1 and read
  `# vars= N` for the function (`tmp/frame_probe.sh` pattern —
  cpp | cc1 -mel flags | awk on `.ent <func>` → `.frame`). Strictly better
  than the sandbox score for frame work: it separates "wrong frame" from
  "wrong codegen".
- Orphan detector: grep the `-da` greg dump's unallocated set, or census
  bare `(use (reg N))` in the combine dump.

## Boundaries (this is a recipe, NOT a sanction)

- Every spelling must independently pass the 6-test checklist: the locals
  must be REAL and LIVE, the guard comparison must be the function's own
  logic. The dead-conditional-store form the mechanism can also produce
  (`if (idx2 > idx) { idx = idx2; }` with idx dead after —
  tslGlobalMemFree round 3) is the FORBIDDEN family and was rejected on
  sight; finding a producer does not legitimize a construct.
- The INVERSE problem (our frame one slot too big) is the same mechanism
  mirrored: find which pseudo orphans in OUR build and un-strand it
  (give it a home or fold its compare) — display-twins candidate work.
- Both metrics matter: a spelling can fix `vars` and still regress the
  score by materializing real instructions (saTan4FireDisp `lim_top`,
  "real shifts, score 55"). Screen with the frame gradient AND the
  orphan detector before scoring.
- Functions where every reachable producer costs instructions or requires
  dead code exist (tslGlobalMemFree, banked): the recipe bounds the search,
  it does not guarantee a hit.
