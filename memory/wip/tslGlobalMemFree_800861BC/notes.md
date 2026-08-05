# tslGlobalMemFree_800861BC — WIP (current state 2026-08-05, sched goal-mapper)
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
   (`addiu t1,v1,-2`); we wrote it inline and GCC folded the `-2` into the load
   displacement. +1 insn, 37 → 36. Must be set immediately after `pc`.
2. **The tail reads the GLOBAL, not `*pc`.** Target rematerialises
   `&D_8010280C` late rather than keeping the pointer live; writing
   `D_8010280C` instead of `*pc` in the last two stores recovers both. → 35.
3. **The if/else condition is RELATIONAL.** `if (D_80102808 & 1)` emits `beqz`,
   but `if ((s16)(D_80102808 & 1) > 0)` emits target's `blez`. → 34. (`> 0` /
   `>= 1` without the `(s16)` cast are inert — the cast is load-bearing.)

All three are ordinary C.

## ra_solver round (2026-08-04)

Model extracted; simulate matches the dump **7/7**. Seven global allocnos:
78→`$v0` (pri 20000, pref `{v0}`), 113→`$v1`, 79→`$a1`, 74→`$a2`, 73→`$a3`,
108→`$t0`, 81→`$t1`. **`use_only` is EMPTY** — mechanical confirmation, from the
extractor's independent lreg path, of **no phantom-slot pseudo** (NO_REGS/orphan
class), corroborating round 3's `tmp/orphan_probe.py` (`UNALLOCATED: none`).

**The residual is not only the `sp` adjusts.** `tmp/ra/dropsp.py` drops target's
`addiu sp,sp,±8` and compares the remaining 130 lines to ours: **52 still
differ**, in two clusters — the prologue emission order (quantified in the
2026-08-05 section) and a **`$v0`/`$v1` swap through both if/else arms**
(`addu v0,v0,v1` vs `addu v1,v1,v0`, `lh v1,0(v1)` vs `lh v0,0(v0)`,
`lhu v0,12(v0)` vs `lhu v1,12(v1)`, same at `14(...)`). So "aligns 1:1 with
target from the `andi` onward" is true of instruction kind and order, not
registers. **`ra_solver/perturb.py` on that swap (spec `{78:3, 113:2}`), singles
+ pairs + greedy: NO sufficient vector** — 78 carries a hard-`$v0` preference
and 113 hard-conflicts `$v0`, so the pair cannot be exchanged in the modelled
space. **Solve the frame first**: a phantom pseudo changes the allocno set
itself, so any register spec from the frame-0 body is provisional.

## sched_solver goal-mapper round (2026-08-05)

`perturb.py --goal-from-target main`: **only block 0 has a scheduling residual;
blocks 1–5 are goal == identity** — their instruction ORDER is already target's,
so the `$v0`/`$v1` arm swap is no scheduling artefact, unreachable by reordering.

Block 0 (the prologue) **independently reproduces round 3's read of the two
listings** by a different code path. Emission slots 0–4: ours 12
`la $6,D_80107898`, 60 `move $7,$0`, 290 `li $9,1`, 18 `lh $5,D_8010280A`, 9
`la $8,D_8010280C`; target 60, 9, 290, 18, 12 — a three-way rotation of
`{a3 = 0, &D_8010280C, &D_80107898}`. (Round 3 counted the `move a3,zero` as
"sixth" in the objdump stream, where each `la` is two lines — same claim.)

No single-atom vector (950 atoms, post-`ready0`-fix). Depth 2 restricted to
spellable atoms (`--atoms luid,luid_move`) DOES return pairs — `luid swap
9 <-> 290` + `luid swap 12 <-> 30/31/35`. **But they are not C-reachable, and
this was tested:** every placement of `p = &D_80107898[0];` (after the
`D_8010280E` store, after the `0x7FFF` store, either side of `i = 0;`, with
`pc`/`p` swapped) leaves honest-vs-target differing at **30, exactly HEAD** —
bit-identical emission — except `p` right after `i = 0;`, giving **29 (-1)**.

**Why it does not survive contact with C:** the other half is uid 290, the
**LICM-hoisted constant `1`** from `1 << (*(pc-2))` inside the loop. No source
statement controls that insn's LUID, so `swap 9 <-> 290` is unspellable however
the surrounding statements move. General rule: a LUID atom is spellable only
when its insn belongs to a statement you can move.

**Tooling correction:** `sched_solver/perturb.py` never re-sorted `ready0`, so
every LUID atom was inert for exactly the opening picks it should decide; any
SCHEDULING "no vector" from before 2026-08-05 was searched under that defect.
Fixed, falsification-tested (8664/8664). Does **not** affect `{78:3, 113:2}` —
that is `ra_solver`'s perturb.py, which has no `ready0`.

## The phantom slot — both producers found are ILLEGAL

Target reserves 8 bytes with **zero `($sp)` references and zero callee-saves**
(pure phantom, same class as `func_8003D9A0` / `func_8003DBE4`). Hoisting the
duplicated `v`/`ofs` out of the arms gives frame 8 but drops to 123 insns (9
short), 43 — the arm duplication is load-bearing. `if (idx2 > idx) { idx =
idx2; }` in the tail gives frame 8, 133 insns, 34, but is **REJECTED AS A
CHEAT**: `idx` is dead there, a dead conditional store, the forbidden family in
[[no-new-park-categories]] (`func_8007B844`). `(D_80102808 & ~idx) & 1` gives
frame 8, 133 insns, 41 — **REJECTED**, it changes the condition's meaning.
**A genuine stack temp is closed BY CONSTRUCTION:** a leaf with no calls, no
aggregate assignments and no address-taken locals can never fire
`assign_stack_temp` / `assign_stack_local`; an unallocated pseudo is the only
route to those 8 bytes.

## Measured negative / inert (do not re-run)

Frame stays 0 for all of: the `if (i < 16)` guard (`i` is constant 0, folds at
tree level — the `func_8003DBE4` lever needs a real variable compare); an `s16`
shift local ± the guard; `idx2` as shift carrier; a no-op `if (idx2 >= idx) {}`;
`s16 ofs` (+2 insns); `u16 v`; splitting `v = v - 1` (51); `pc2 = pc - 1`;
setting `pc2` late; `*(((u8 *)&D_800F65E0) + idx) |= 0x30` (46).
Plus **seven** spellings aimed at giving an s16 value the `tslLineG5Init` span
(an SImode use either side of a CODE_LABEL): `s16 st` naming the loaded
`D_80102808` (129 insns, 41), that read hoisted before the loop (129, 55),
`idx`/`idx2`/`v` as s32 (130; 35/35/34), `while (i < 16)` (130, 34). The body
has no s16 value with a natural post-join use — `v` dies in the arms, `idx`
before the `if`, `idx2` is born after it, and target re-reads `D_8010280A`
rather than carrying a variable at all.

## Resume here — BANKED at 34, do not force

Start from `candidate.c` (34). The three instruction-count findings are solid.
The frame gap needs a phantom producer nobody has found, and two independent
code paths now confirm none exists in this body. Solve the frame before the
registers — the register residual (prologue order + the `$v0`/`$v1` arm swap) is
downstream of it and has no vector in the current model. Do not re-run any probe
above; do not resurrect either rejected producer. Instruments: `tmp/tgm*.py`,
`tmp/frame_probe.sh`, `tmp/ra/*`, `tools/sched_solver/`.
