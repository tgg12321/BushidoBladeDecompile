# func_8007CE0C — WIP (opened 2026-08-04)

`src/display.c:754`. Verdict **C**. HEAD **48**, `candidate.c` **47**. Target 143
insns / ours 142. **23 regfix rules**, 0 asmfix, 0 prologue_config. HEAD carries
cheat-asm (an `asm("s5")` pin + an `__asm__ volatile("" : "=r"…)` barrier) —
both **measured score-inert** and dropped in `candidate.c`.

**Entry cautions discharged (measured).** No sibling label landmine:
`tmp/ce0c_landmines.py` resolves every absolute-`.L<N>` rule to its owner; all
nine holders are in **other** TUs, cc1's `.L` counter is per-TU, and CE0C's own
splices use `{lbl#N}` — restructure freely. Not GTE: GPU packet/DMA, zero cop2.

## The 48 splits three ways

| group | insns | status |
|---|---|---|
| **A frame** | 16 | OPEN — frame 64 vs 80; needs `vars` 16 → 32 |
| **B body regalloc** | ~30 | OPEN — register naming + 3 extra/missing moves |
| **C arg load** | 1 | **CLOSED (48→47)** |

Group A is exactly the 16 frame rules (`regfix.txt:3419-3434`), so fixing the
frame retires 16 of the 23 rules in one move.

**Group C — CLOSED.** Target idx 66 is a `nop` where we emitted `lui a0,0xa000`:
the committed source passed `(u32 *)0xA0000000` to `func_8007DC9C`, which the
rule `subst "lui\s+\$4,…" "nop" @ 63` papered over. **`func_8007DC9C()` with no
argument → 47.** (`func_8007DC9C(0)` and `(u32 *)0` both stay at 48 — no
argument at all, not a null one.)

## Group A — the phantom-slot PRODUCER is identified exactly

Target reserves `0x00..0x2F` (48 B) below its saves and **touches none of it**;
both calls take ≤1 arg so `args` is the o32 minimum 16 on both sides. With
`frame = ALIGN8(vars)+ALIGN8(args)+ALIGN8(gp_regs)` and 7 saved regs (28→32):
ours `vars=16` → 64, target `vars=32` → 80.

**Each phantom slot is one combine orphan-USE pseudo, and the producer is an
s16→s32 sign-extension of a value ALREADY IN A REGISTER**, which GCC expands as
a shift pair (flow dump):

```
(set (reg:SI 96) (ashift:SI (subreg:SI (reg/v:HI 75) 0) (const_int 16)))
(set (reg:SI 95) (ashiftrt:SI (reg:SI 96) (const_int 16)))
```

combine merges the pair into one `sign_extend`, killing 96's def but leaving
`(use (reg:SI 96))` orphaned; greg reports it UNALLOCATED and it takes an 8-byte
slot. `tmp/orphan_probe.py`: `UNALLOCATED: ['96','100']` — two strands, exactly
`vars=16`. **A value loaded straight from memory does NOT count**: `lh`
sign-extends in the load, so there is no pair to merge.

**So the spec is: go from 2 orphan strands to 4.**

**Cross-checked against the in-tree 3-orphan witness.** `func_8007C2A0`
(display.c:376, `vars=32`, `UNALLOCATED ['128','137','140']`) uses the **same**
producer — `tmp/rtlseg.py tmp/rtl_disp func_8007C2A0 flow` shows all three are
`ashift`/`ashiftrt` subreg pairs — and its source has exactly three distinct
multi-use `s16` locals. So it is **one strand per HImode→SImode WIDENING SITE**.

## Measured negatives — 25 variants, `vars` never rose above 16

- **Extra s16 casts** (r3, 6 variants): `s16 xf = arg0->x` + `(s32)xf`, `s16 yf
- **Splitting/retyping existing s16 locals** (r5, 6 variants): `coord` split
- **More locals of any kind** (r1, 6 variants); **region bisect** (r2): deleting
- **The real-s16-quantity family** (r6, 6 variants — clamp limits as real `s16`

### `vars` IS controllable — proven, but so far only DOWNWARD

`lim_top` (both clamp globals read once into `s16 xlim`/`ylim` at the top, used
by both compares) gives **orphans=1, vars=8, frame=56 at the same 142 insns**
(score 55). Reproduced. It refutes any "the frame is stuck at 16" reading — the
count moves with source shape. Hoisting the load AWAY from its compare *removed*
a strand, so a strand needs the widening where combine cannot fold it into the
load. Nothing yet pushes the count upward.

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

Group A is the big prize (16 insns, 16 rules): **the body needs FOUR
HImode→SImode widening sites; it has two.** 25 variants across five rounds show
the count is a property of the *arithmetic*, not the declarations. The named
real-s16-quantity family is now measured and does not deliver it.

**The live handle is `lim_top`**: it proves the count moves (2→1, `vars` 16→8,
same 142 insns). Work that lever in reverse — find the placement that makes a
widening un-foldable into its load. Two hard constraints on any candidate: the
multiply must keep re-reading `arg0->x` from memory (target splice
`regfix.txt:3417` is `lh $3,4($17)` … `mult $3,$2`), and target emits the same
two `sll …,0x10` + one `sra …,0x10` we do, so the extra strands must be
widenings combine folds away entirely.

**Discipline note:** no synthetic-strand spelling was tried and none should be.
Nothing reaches 3 or 4 orphans by *any* means, real or not, so there is no "only
works with a semantically empty local" trade-off to adjudicate — banked on
measurement, not a policy call. Then Group B by ALLOCDBG sizing, **after** the
frame is right (a frame change re-shuffles the allocation).

`src/display.c` is at HEAD; `candidate.c` (47) carries the Group-C fix and the
cheat-asm removal. Neither is independently committable: with the 23 rules
ENABLED the emission shifts (same gate as hirahira_w_ctrl).

## Instruments

`tmp/ce0c{,2,3,4,5,6}.py` (sweeps; ce0c3/5/6 print the greg UNALLOCATED set =
the direct Group-A gradient), **`tmp/rtlseg.py`** (section-safe RTL reader — its
predecessor `tmp/ce0c_rtl.py` silently scanned the WRONG function on a name-
lookup miss; prefer rtlseg), `tmp/ce0c_apply.py`, `tmp/ce0c_landmines.py`,
`tmp/adiff.py`, `tmp/orphan_probe.py`, `tmp/rtldump.sh`, `tmp/frame_probe.sh`.
