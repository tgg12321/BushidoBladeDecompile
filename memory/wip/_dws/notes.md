# func_8007CE0C — WIP (current state 2026-08-04, ra_solver round)

`src/display.c:754`. Verdict **C**. HEAD **48**, `candidate.c` **30**.
**143/143 insns and frame 80 == target 0x50** — both correct.
**23 regfix rules**, 0 asmfix, 0 prologue_config. HEAD carries cheat-asm (an
`asm("s5")` pin + an `__asm__ volatile("" : "=r"…)` barrier) — both **measured
score-inert** and dropped in `candidate.c`.

**Entry cautions discharged (measured).** No sibling label landmine:
`tmp/ce0c_landmines.py` resolves every absolute-`.L<N>` rule to its owner; all
nine holders are in **other** TUs, cc1's `.L` counter is per-TU, and CE0C's own
splices use `{lbl#N}` — restructure freely. Not GTE: GPU packet/DMA, zero cop2.

## The 48 splits three ways

| group | insns | status |
|---|---|---|
| **A frame** | 16 | **CLOSED (47→30)** — frame 80, `vars=32`, 143/143 insns |
| **B body regalloc** | ~30 | OPEN — the whole residual |
| **C arg load** | 1 | **CLOSED (48→47)** |

**Group C — CLOSED.** Target idx 66 is a `nop` where we emitted `lui a0,0xa000`:
the source passed `(u32 *)0xA0000000` to `func_8007DC9C`, which the rule
`subst "lui\s+\$4,…" "nop" @ 63` papered over. **`func_8007DC9C()` with no
argument → 47.** (`func_8007DC9C(0)` and `(u32 *)0` both stay at 48.)

## Group A — MECHANISM (round 7-9), CLOSED

Target reserves `0x00..0x2F` (48 B) below its saves and touches none of it. With
`frame = ALIGN8(vars)+ALIGN8(args)+ALIGN8(gp_regs)` and 7 saved regs: ours
`vars=16` → 64, target `vars=32` → 80. Each 8-byte slot is one combine
orphan-USE pseudo. The producer is an s16→s32 widening of a register-resident
value, expanded as a shift pair that combine then merges:

```
(set (reg:SI 96) (ashift:SI (subreg:SI (reg/v:HI 75) 0) (const_int 16)))
(set (reg:SI 95) (ashiftrt:SI (reg:SI 96) (const_int 16)))
```

**The SITE COUNT was never the problem** — the body already has five sites and
combine folds all five; only two leave an orphan. Three outcomes per site, only
the middle buys a slot: merges with a single-use intermediate (no slot); merges
with a MULTI-use intermediate → keeps a `(use)` (**8-byte slot**); cannot merge
→ real `sll`+`sra` (no slot). That is `combine.c:1458`
`added_sets_2 = !dead_or_set_p(i3, i2dest)`, the mechanism
[[packed-multiply-cluster]] documents.

A site orphans only when the `reg:HI` being widened has a second use **as an
HImode value**. Narrowing `v1_tmp` to `u16` and mirroring the y side's shape
converts both x sites: orphans 2→4, vars 16→32, frame 64→80, 142→143 insns,
**47→30**. Partial forms convert exactly one site (orphans=3, vars=24, frame=72,
score 49), confirming the mechanism is graded.

## ra_solver round (2026-08-04) — Group B quantified

Model extracted; simulate matches the dump **23/23** (free regression check).
Callee-save queue, in allocation order:

| pseudo | reg | nrefs | livelen | pri | role |
|---|---|---|---|---|---|
| 78 | `$s0` | 10 | 39 | 7692 | |
| 72 | `$s1` | 8 | 70 | 3428 | |
| 73 | `$s2` | 8 | 89 | 2696 | |
| 118 | `$s3` | 3 | 14 | 2142 | |
| **77** | `$s4` | 4 | 52 | **1538** | **`big_size`** |
| 74 | `$s5` | 2 | 120 | 166 | |

Target holds `big_size` in `$s0`. The `$s` registers are handed out **strictly in
descending priority order**, so `big_size` must be **first** in this queue —
pri > 7692 against its current 1538, a **5x lift**. At livelen 52 that needs
nrefs ≈ 8; at nrefs 4 it needs livelen ≤ 10.

**`perturb.py` (spec `{77:16}`), 133 single atoms + greedy: NO sufficient
vector** — no refs, live-length, conflict, preference or birth-order atom gets
there, because the required lift is far outside single-atom range. This settles
the r8 note's suggestion that "priority framing is the wrong model": for CE0C the
callee-save assignment **is** a pure priority queue, and the non-priority atoms
the solver adds do not help either.

**Group B is also not purely naming.** 51 of 143 lines differ, and from ~line 26
the clamp region is **misordered**, not just renamed (target `sh v1,4(s1)` at 27
vs ours at 30; target `lh a1,6(s1)` vs our `lhu v1,6(s1)` + a separate
`lh a2,6(s1)`). Group B is a shape problem in the clamps plus the callee-save
queue, not a ~30-insn rename.

**Accumulator type sweep (new):** `s16 v1_tmp` gives 51 differing lines, same as
the banked `u16`; `s16 a0_tmp` and both-`s16` are worse at 53. The `u16` choice
is not costing anything measurable, but it is also not what produces target's
`lh` at the clamp read.

## Resume here — BANKED at 30

Start from `candidate.c` (30). Group A and C are closed. Group B needs the clamp
**shape** fixed before any register work: target loads `lh a1,4(s1)` once and
uses `a1` for both the `bltz` and the `slt`, threading `move v1,a1` into the
branch delay slot; we emit a load + load-delay `nop` + `move` with the `slt`
operands swapped — the `coord = (v1_tmp = arg0->x);` double-assign is the likely
cause. Fixing that may re-price the queue; re-derive the model afterwards rather
than working the current priorities.

Two hard constraints: the multiply must keep re-reading `arg0->x` from memory
(target splice `regfix.txt:3417` is `lh $3,4($17)` … `mult $3,$2`), and the
visible `sll`/`sll`/`sra` census must stay identical — a new multi-use must NOT
push a site into the unmergeable row (what `lim_top` did: real shifts, score 55).
Re-run `tmp/widen.py` after any Group B change: a shape change can push a site
out of the orphan row and silently cost the frame.

**Discipline note:** no synthetic-strand spelling has been tried and none should
be. Everything banked here is measurement, not a policy call.

`src/display.c` is at HEAD. Neither `candidate.c` nor the Group-C fix is
independently committable: with the 23 rules ENABLED the emission shifts (same
gate as hirahira_w_ctrl).

## Instruments
`tmp/ce0c{,2,3,4,5,6}.py`, **`tmp/widen.py`** (per-pass site census, fold vs
orphan), **`tmp/rtlseg.py`**, `tmp/ce0c_rtl.py`, `tmp/ce0c_apply.py`,
`tmp/ce0c_landmines.py`, `tmp/adiff.py`, `tmp/orphan_probe.py`,
`tmp/rtldump.sh`, `tmp/frame_probe.sh`; `tmp/ra/*` (generic solver harness).
