# func_8007CE0C — WIP (opened 2026-08-04)

`src/display.c:754`. Verdict **C**. HEAD **48**, `candidate.c` **30**.
**143/143 insns and frame 80 == target 0x50** — both now correct.
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
| **B body regalloc** | ~30 | OPEN — register naming; the whole residual now |
| **C arg load** | 1 | **CLOSED (48→47)** |

Group A was exactly the 16 frame rules (`regfix.txt:3419-3434`); with the frame
correct they should retire outright.

**Group C — CLOSED.** Target idx 66 is a `nop` where we emitted `lui a0,0xa000`:
the committed source passed `(u32 *)0xA0000000` to `func_8007DC9C`, which the
rule `subst "lui\s+\$4,…" "nop" @ 63` papered over. **`func_8007DC9C()` with no
argument → 47.** (`func_8007DC9C(0)` and `(u32 *)0` both stay at 48.)

## Group A — the frame arithmetic

Target reserves `0x00..0x2F` (48 B) below its saves and **touches none of it**;
both calls take ≤1 arg so `args` is the o32 minimum 16 on both sides. With
`frame = ALIGN8(vars)+ALIGN8(args)+ALIGN8(gp_regs)` and 7 saved regs (28→32):
ours `vars=16` → 64, target `vars=32` → 80. Each 8-byte slot is one combine
orphan-USE pseudo; `tmp/orphan_probe.py` reports `['96','100']` for our body.

## Group A — MECHANISM (round 7; supersedes the round-2..6 framing)

The producer is an s16→s32 widening of a register-resident value, which GCC
expands as a shift pair and combine then merges:

```
(set (reg:SI 96) (ashift:SI (subreg:SI (reg/v:HI 75) 0) (const_int 16)))
(set (reg:SI 95) (ashiftrt:SI (reg:SI 96) (const_int 16)))
```

**But the SITE COUNT was never the problem.** `tmp/widen.py` censuses sites per
pass: the body already has **five** (flow pseudos 83, 89, 96, 100, 109). combine
folds all five and only **two** leave an orphan. Rounds 1-6 were adding and
removing sites — a quantity that was never short, which is exactly why 25
variants moved nothing.

Three outcomes per site; only the middle one buys a frame slot:

| combine does | result | example |
|---|---|---|
| merges, intermediate single-use | no insns, **no slot** | base 83, 89, 109 |
| merges, intermediate MULTI-use → keeps a `(use)` | **8-byte slot** | base 96, 100 |
| cannot merge | real `sll`+`sra` emitted, no slot | `lim_top` 90, 99 |

That is `combine.c:1458` `added_sets_2 = !dead_or_set_p(i3, i2dest)` — the same
mechanism [[packed-multiply-cluster]] documents: combine preserves the dead
(full mechanism prose in git history: r7/r8 commits)

## Measured negatives — historical (Group A closed r9); details in git history
## Group B — shape (from `tmp/adiff.py`)

**x-clamp (tgt 11-19):** target loads `lh a1,4(s1)` once and uses `a1` for both
the `bltz` and the `slt`, threading `move v1,a1` into the branch delay slot; we
emit `lh v1` + a load-delay `nop` + `move v0,v1` with the `slt` operands
swapped — the `coord = (v1_tmp = arg0->x);` double-assign is the likely cause.
**y-clamp (25-41):** same one register over, plus an extra `move a0,v0`.
**multiply/rounding (44-50):** pure register naming (target `a2/v1/v0/s0` vs our
`a3/v0/v1/s4`) — the ALLOCDBG lever-sizing case. **53-59:** target holds
`big_size` in `s0` and copies it through `v1` into `s4`; we keep it in `s4`.

## Resume here

**GROUP A IS CLOSED (r8).** A site orphans only when the `reg:HI` being widened
has a second use **as an HImode value**. Sites 96/100 (`arg0->y`, `D_8009BE7A`)
had one — each is copied narrow into the 16-bit accumulator `a0_tmp`. Sites
83/89 (`arg0->x`, `D_8009BE78`) did not, because `v1_tmp` was `s32` so its
assignments took the *widened* value. The two clamps are the same algorithm
differing only in accumulator width, so **narrowing `v1_tmp` to `u16` and
mirroring the y side's shape converts both x sites**: orphans 2→4, vars 16→32,
frame 64→80, 142→143 insns, **47→30**. `u16` and `s16` both work. The second
consumer is the clamp's own accumulator, so this is the symmetric real spelling.
Partial forms convert exactly one site (orphans=3, vars=24, frame=72, score 49),
which confirms the mechanism is graded.

**Group B is now the whole residual** (~30 insns of register naming). Do it by
ALLOCDBG sizing with `tmp/allocone.sh` — safe now that the frame is correct and
will not re-shuffle under it. Re-run `tmp/widen.py` after any Group B change: a
shape change can push a site out of the orphan row and silently cost the frame.

Screen with `tmp/widen.py tmp/rtl_disp func_8007CE0C` (per-pass site census,
fold vs orphan), then `tmp/ce0c6.py` for orphan count + score. Two hard
constraints: the multiply must keep re-reading `arg0->x` from memory (target
splice `regfix.txt:3417` is `lh $3,4($17)` … `mult $3,$2`), and the visible
`sll`/`sll`/`sra` census must stay identical — a new multi-use must NOT push a
site into the unmergeable row (what `lim_top` did: real shifts, score 55).

**Discipline note:** no synthetic-strand spelling has been tried and none should
be. Everything banked here is on measurement, not a policy call. Group B by
ALLOCDBG sizing **after** the frame lands.

`src/display.c` is at HEAD; `candidate.c` (47) carries the Group-C fix and the
cheat-asm removal. Neither is independently committable: with the 23 rules
ENABLED the emission shifts (same gate as hirahira_w_ctrl).

## Instruments
tmp/ce0c{,2,3,4,5,6}.py **`tmp/widen.py`** **`tmp/rtlseg.py`** tmp/ce0c_rtl.py tmp/ce0c_apply.py tmp/ce0c_landmines.py tmp/adiff.py tmp/orphan_probe.py tmp/rtldump.sh tmp/frame_probe.sh`.