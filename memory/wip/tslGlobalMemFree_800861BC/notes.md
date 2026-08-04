# tslGlobalMemFree_800861BC — WIP (current state 2026-08-04, ra_solver round)

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

## Three findings folded into the candidate (37 → 34)

1. **`pc2` names the `pc - 2` pointer.** Target precomputes it outside the loop
   (`addiu t1,v1,-2`); our source wrote it inline and GCC folded the `-2` into
   the load displacement. +1 insn, 37 → 36. Must be set immediately after `pc`.
2. **The tail reads the GLOBAL, not `*pc`.** Target rematerialises
   `&D_8010280C` late rather than keeping the pointer live. Writing
   `D_8010280C` instead of `*pc` in the last two stores recovers both. → 35.
3. **The if/else condition is RELATIONAL.** Target branches `blez v0` on the
   `andi v0,v1,1` result; `if (D_80102808 & 1)` emits `beqz`. Spelling it
   `if ((s16)(D_80102808 & 1) > 0)` emits target's `blez`. → 34. (`> 0` and
   `>= 1` without the `(s16)` cast are inert — the cast is load-bearing.)

All three are ordinary C: naming a pointer the code already forms, reading a
global directly, and testing a flag with a relational operator.

## ra_solver round (2026-08-04)

Model extracted; simulate matches the dump **7/7** (free regression check).
Seven global allocnos: 78→`$v0` (pri 20000, pref `{v0}`), 113→`$v1`, 79→`$a1`,
74→`$a2`, 73→`$a3`, 108→`$t0`, 81→`$t1`.

**`use_only` is EMPTY** — mechanical confirmation, from the extractor's
independent lreg path, that this body contains **no phantom-slot pseudo** (the
NO_REGS/orphan class). That corroborates round 3's `tmp/orphan_probe.py` result
(`UNALLOCATED: none`) by a different code path, so frame-0 is double-sourced.

**NEW — the residual is not only the `sp` adjusts.** `tmp/ra/dropsp.py` removes
target's `addiu sp,sp,-8` (line 1) and `addiu sp,sp,8` (line 130) and compares
the remaining 130 lines to ours: **52 of 130 still differ.** Two clusters:
- **Prologue emission order.** Target: `move a3,zero`, then the `lui v1 / addiu
  v1 / addiu t1,v1,-2` chain, `li t0,1`, then the `a1`/`a2` globals. Ours emits
  the `v1`/`a2` chains first and `move a3,zero` sixth.
- **A `$v0`/`$v1` swap through both if/else arms** (`addu v0,v0,v1` vs
  `addu v1,v1,v0`, `lh v1,0(v1)` vs `lh v0,0(v0)`, `lhu v0,12(v0)` vs
  `lhu v1,12(v1)`, and the same at the `14(...)` arm).

So the earlier note that the stream "aligns 1:1 with target from the `andi`
onward" is true of instruction *kind and order*, not of registers.

**`perturb.py` on the `$v0`/`$v1` swap (spec `{78:3, 113:2}`), singles + pairs +
greedy: NO sufficient vector.** 78 carries an explicit hard-`$v0` preference and
113 hard-conflicts `$v0`, so the pair cannot be exchanged within the modeled
space. **Order of work: solve the frame first** — a phantom (unallocated) pseudo
changes the allocno set itself, so any register spec derived from the frame-0
body is provisional.

## The phantom slot — both producers found are ILLEGAL

Target reserves 8 bytes with **zero `($sp)` references and zero callee-saves**
(pure phantom, same class as `func_8003D9A0` / `func_8003DBE4`).

- **Hoisting the duplicated `v`/`ofs` out of the arms** → frame 8, but drops to
  123 insns (9 short), score 43. The arm duplication is load-bearing.
- **`if (idx2 > idx) { idx = idx2; }` in the tail** → frame 8, 133 insns, score
  34. **REJECTED AS A CHEAT:** `idx` is dead there — a dead conditional store,
  the exact forbidden family in [[no-new-park-categories]] (`func_8007B844`).
- **`(D_80102808 & ~idx) & 1`** → frame 8, 133 insns, score 41. **REJECTED:**
  changes the condition's semantics.

**Avenue 2 (a genuine stack temp) is closed BY CONSTRUCTION:** a leaf with no
calls, no aggregate assignments and no address-taken locals can never fire
`assign_stack_temp` / `assign_stack_local`. An unallocated pseudo is the only
route to those 8 bytes.

## Measured negative / inert (do not re-run)

- `if (i < 16)` guard around the `do`/`while` — inert; `i` is constant 0 so the
  compare folds at tree level. (The `func_8003DBE4` folded-guard lever needs a
  genuine variable comparison.)
- `s16` local for the loop's shift amount (37); guard + shift local (37);
  reusing `idx2` as the shift carrier (34, inert).
- No-op `if (idx2 >= idx) { }` relational probe (37).
- `s16 ofs` instead of `s32` (38, +2 insns); `u16 v` (34); splitting
  `v = v - 1` into its own statement (51, severe).
- `*(((u8 *)&D_800F65E0) + idx) |= 0x30` in the last store (46).
- `pc2 = pc - 1` spelling; `pc2` set later than right after `pc` — both inert.
- Seven spellings aimed at giving an s16 value the `tslLineG5Init` span (an
  SImode use either side of a CODE_LABEL) all leave frame 0: `s16 st` naming the
  loaded `D_80102808` (129 insns, 41); same read before the loop (129, 55);
  `idx` as s32 (130, 35); `idx2` as s32 (130, 35); `v` as s32 (130, 34);
  `while (i < 16)` form (130, 34). The body has no s16 value with a natural
  post-join use — `v` dies in the arms, `idx` before the `if`, `idx2` is born
  after it, and target re-reads `D_8010280A` rather than carrying a variable.

## Resume here — BANKED at 34, do not force

Start from `candidate.c` (34). The three instruction-count findings are solid.
The frame gap needs a phantom producer nobody has found, and the solver now
confirms independently that none exists in this body. Solve the frame before the
registers — the register residual (prologue order + the `$v0`/`$v1` arm swap) is
downstream of it and has no vector in the current model. Do not re-run any probe
above; do not resurrect either rejected producer.

## Instruments

`tmp/tgm.py` … `tmp/tgm4.py`, `tmp/tgm_bank.py`, `tmp/frame_probe.sh`,
`tmp/fdiff.sh`, `tmp/usehunt.py`, `tmp/orphan_probe.py`;
`tmp/ra/*` (generic harness), `tmp/ra/dropsp.py` (frame-adjust-blind compare).
