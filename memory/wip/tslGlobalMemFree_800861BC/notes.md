# tslGlobalMemFree_800861BC — WIP (current state 2026-08-04, round 2)

`src/main.c:1066`. Leaf, no calls. Clears a 16-word bitmask array, writes s16
globals derived from `D_8010280A` / `D_801027F7` / `D_801027FC`, picks one of
two adjacent u16 fields on `D_80102808 & 1`, ORs flag bits into `D_800F65E0`.

30 regfix rules; 0 asmfix; no prologue_config / frame_fix / maspsx-gate entries.

## Where it stands

| | score | frame | insns |
|---|---|---|---|
| committed HEAD | 37 | 0 | 127 |
| **`candidate.c`** | **34** | 0 | **130** |
| target | 0 | **8** | **132** |

The candidate's instruction stream **aligns 1:1 with target from the `andi`
onward**, including target's `blez`. The whole remaining structural gap is the
two `addiu $sp` adjusts of the 8-byte phantom slot.

## Three findings folded into the candidate (37 → 34)

1. **`pc2` names the `pc - 2` pointer.** Target precomputes it outside the loop
   (`addiu t1,v1,-2`); our source wrote `*((s16 *)((u8 *)pc - 2))` inline and
   GCC folded the `-2` into the load displacement. +1 insn, 37 → 36. Must be
   set immediately after `pc` (setting it later is inert).
2. **The tail reads the GLOBAL, not `*pc`.** Target rematerialises
   `&D_8010280C` late (`lui a1 / addiu a1`) rather than keeping the pointer
   live across the function. Writing `D_8010280C` instead of `*pc` in the last
   two stores recovers both instructions. +2 insns, → 35.
3. **The if/else condition is RELATIONAL.** Target branches `blez v0` on the
   `andi v0,v1,1` result; `if (D_80102808 & 1)` emits `beqz`. Spelling it
   `if ((s16)(D_80102808 & 1) > 0)` emits target's `blez` exactly. → 34.
   (`> 0` and `>= 1` without the `(s16)` cast are inert at 36/35 — the cast is
   load-bearing.)

All three are ordinary C: naming a pointer the code already forms, reading a
global directly, and testing a flag with a relational operator.

## The phantom slot — still open, and both producers found are ILLEGAL

Target reserves 8 bytes with **zero `($sp)` references and zero callee-saves**
(pure phantom, same class as `func_8003D9A0` / `func_8003DBE4`). Our cc1 emits
frame 0. Every producer found so far is disqualified:

- **Hoisting the duplicated `v`/`ofs` out of the if/else arms** → frame 8, but
  drops to 123 insns (9 short) and score 43. The arm duplication is
  load-bearing; collapsing it is what frees the pseudo. Rejected on cost.
- **`if (idx2 > idx) { idx = idx2; }` in the tail** → frame 8, 133 insns,
  score 34. **REJECTED AS A CHEAT:** `idx` is dead after that point, so this is
  a dead conditional store — the exact forbidden family in
  [[no-new-park-categories]] (the `func_8007B844` confirmed example).
  Recorded so nobody re-derives it.
- **`(D_80102808 & ~idx) & 1`** → frame 8, 133 insns, score 41. **REJECTED:**
  changes the condition's semantics. Probe only.

## Measured negative / inert (do not re-run)

- `if (i < 16)` guard around the `do`/`while` — inert; `i` is a constant 0 so
  the compare folds at tree level and no pseudo survives. (The `func_8003DBE4`
  folded-guard lever needs a genuine variable comparison.)
- `s16` local for the loop's shift amount (37); guard + shift local (37);
  reusing `idx2` as the shift carrier (34, inert on the base).
- No-op `if (idx2 >= idx) { }` relational probe (37).
- `s16 ofs` instead of `s32` (38, +2 insns); `u16 v` (34, inert);
  splitting `v = v - 1` into its own statement (51, severe).
- `*(((u8 *)&D_800F65E0) + idx) |= 0x30` instead of `D_8010280A` in the last
  store (46).
- `pc2 = pc - 1` spelling (inert vs the byte-offset form); `pc2` set at its
  original position rather than right after `pc` (inert).

## Resume here

The only open question is a phantom producer that neither collapses the arm
duplication nor inserts dead code. Untried:

1. **The combine orphan-USE route** (`combine.c:10836-10841`) — a HImode
   sign-extend whose intermediate strands at a `CODE_LABEL`. This body has s16
   locals (`v`, `idx`, `idx2`) and a label-rich if/else, so the
   `tslLineG5Init` shape may be reachable. Confirm with `tmp/usehunt.py` on a
   `-da` dump of main.c before hand-sweeping: check whether any near-miss form
   already produces a bare `(use (reg N))` in the combine dump.
2. Whether the two `sp` adjusts could come from something other than a phantom
   pseudo — e.g. a genuine temp GCC allocates and then register-allocates away.

Start from `candidate.c` (34), not HEAD. Do not re-run the probes listed above
and do not resurrect either rejected producer.

## Instruments

`tmp/tgm.py` … `tmp/tgm4.py` (rounds 1-4 sweeps), `tmp/tgm_bank.py`,
`tmp/frame_probe.sh`, `tmp/fdiff.sh`, `tmp/sbs.sh`, `tmp/usehunt.py`.
