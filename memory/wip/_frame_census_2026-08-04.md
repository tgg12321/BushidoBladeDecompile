# Frame-mismatch census — 2026-08-04

Reconnaissance only; no source was edited. Data: `tmp/frame_census.csv`
(302 rows = 265 active + 37 parked queue items). Generator:
`tmp/frame_census.py` (`--all` includes parked).

## Method

- **our_frame** = cc1's own `.frame $sp,N # vars= V, regs= R, args= A` comment
  for the CURRENT source, harvested one TU at a time (~31 cc1 runs for the whole
  tree instead of one per function). cc1 runs BEFORE prologue_fix / frame_fix /
  regfix / asmfix, so this is the honest native frame for rule-carrying
  functions.
- **target_frame** = the prologue `addiu $sp, $sp, -N` in `asm/funcs/<func>.s`
  (0 for leaves); target save count from the `sw $sN/$ra,…($sp)` lines.
- Same pass records combine orphan-USE pseudos per function (the phantom-slot
  producer from `memory/wip/func_8003D9A0/notes.md`) plus source heuristics
  (`loop`, `relguard`, `hi16`, `srcheat`).

**Two caveats that limit the data** (both visible in the CSV):
1. `our_frame` is NOT honest for functions whose SOURCE carries cheat-asm
   (register pins / `__asm__`) — flagged `srcheat`. Re-measure after pins come out.
2. 32 items have an empty C body (`empty-c-body`) — the whole body arrives from
   an asmfix splice, so the frame comparison is meaningless. Most of the very
   large deltas (≥64 B) are the same story in a milder form: `vars=0/regs=0/args=0`
   means cc1 compiled a stub, not a real body. Do not read those as phantom slots.

## Distribution (302 rows)

| class | n | reading |
|---|---|---|
| `equal` | 149 | frame already correct — the gap is RA/scheduling, not frame |
| `empty-c-body` | 32 | asmfix whole-body splice; not comparable |
| `ours-larger` | 7 | we reserve MORE than target — inverse phantom (one slot too many) |
| `phantom-8` | 6 | exactly one missing slot |
| `phantom-16 / 24 / 32` | 5 / 2 / 2 | 2-4 missing slots |
| `phantom-40`…`phantom-584` | ~95 | mostly stub-body artifacts; low signal |

The headline: **for roughly half the active queue the frame is already right**,
so the phantom-slot lever is not the bottleneck there.

## Shortlist (drives the next per-function assignments)

Ranked by honest distance among items with a real frame delta and a real C body.

1. **`func_8007C7A0` + `func_8007C86C`** (display) — dist **20** each, 21 rules
   each, delta **-8** (ours 24, target 16), `orphan=2` already present.
   *Inverse* case: we allocate one phantom slot too many. Twins with identical
   shape, so one fix closes two. Best active lead in the tree.
2. `title_mv_exec2` (main) — dist 27, 18 rules, delta -8.
3. `InitHiraRmd_800480C0` (text1b) — dist 36, 33 rules, delta **+32** — the only
   ACTIVE member of the 32-byte dead-vars cluster below.
4. `tslGlobalMemFree_800861BC` (main) — dist 37, 30 rules, delta +8.
5. `saTan4FireDisp` (text1a) — dist 41, 35 rules, delta +16.
6. `func_8007CE0C` (display) — dist 48, 23 rules, delta +16, `orphan=2`.
7. `hirahira_w_ctrl_2` (text1a_c) — dist 58, 63 rules, delta -8.
8. `decBs0` (text1a) — dist 58, 39 rules, delta +8 (`srcheat`, re-measure first).

## The 32-byte dead-vars cluster — correction to my 2026-08-04 claim

In `memory/wip/func_8003DBE4/notes.md` I wrote that the escalated
`AddTbpOfst_80047EE8` / `InitHiraRmd_80047FBC` cluster should be re-opened
because the "no instruction touches the bytes ⇒ must be a dead declaration"
inference is unsound. **That inference is still unsound, but measurement shows
re-opening is not the easy win I implied:**

| func | status | honest dist | our frame | target | note |
|---|---|---|---|---|---|
| `InitHiraRmd_80047FBC` | parked | **1** | 80 | 80 | frame already EQUAL — supplied by its `s32 buf[8]` |
| `AddTbpOfst_80047EE8` | parked | **8** | 72 | 72 | EQUAL via `s32 unused_slack[8]`; also carries 2 register pins |
| `func_800481E8` | parked | 14 | 40 | 72 | delta +32 |
| `InitHiraRmd_800480C0` | active | 36 | 56 | 88 | delta +32 |

So their frames currently match *because of* the arrays, and the residual 1 / 8
are register-allocation diffs (`addu s0,s4,v0` vs `addu s0,a0,v0`; a missing
`move s0,a0` / `move s2,s0` pair). Removing the arrays honestly needs **32 bytes
= 4 phantom slots**, which is a materially bigger ask than the single slot the
`func_8003D9A0` / `func_8003DBE4` lever demonstrates. Multi-orphan functions do
exist in-tree (`func_8007C2A0` vars=32 from 3 orphans, `tslPrintScreen` vars=16),
so it is not ruled out — but it is a research task, not a transfer.

**One literal dead array remains in the whole tree** (grep): `s32
unused_slack[8];` at `src/text1b.c:23` with `(void)unused_slack;` at :29.
`InitHiraRmd_80047FBC`'s own comment cites an `s32 buf[8]`. `tools/frame_fix_funcs.txt`
is now empty (only comments) — nothing else coerces a frame.

## Instruments

`tmp/frame_census.py` (this census), `tmp/frame_probe.sh` (single-function
`.frame` gradient), `tmp/orphan_census.py` (combine orphan-USE producers),
`tmp/fdiff.sh`. The `deadarr` note-flag in the census under-fires when a
function's comments mention the array name — use grep, not that column.
