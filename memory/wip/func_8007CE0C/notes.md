# func_8007CE0C — WIP (opened 2026-08-04)

`src/display.c:754`. Verdict **C**. HEAD **48**, `candidate.c` **47**. Target 143
insns / ours 142. **23 regfix rules**, 0 asmfix, 0 prologue_config. HEAD carries
cheat-asm (an `asm("s5")` pin + an `__asm__ volatile("" : "=r"…)` barrier) —
both **measured score-inert** and dropped in `candidate.c`.

## Both entry cautions discharged — measured, not assumed

1. **No sibling label landmine.** `tmp/ce0c_landmines.py` resolves every rule
2. **Not GTE.** GPU packet/DMA routine, zero cop2. gte-3x3 / scratchpad-gte /

## The 48 splits three ways

| group | insns | status |
|---|---|---|
| **A frame** | 16 | OPEN — frame 64 vs 80; needs `vars` 16 → 32 |
| **B body regalloc** | ~30 | OPEN — register naming + 3 extra/missing moves |
| **C arg load** | 1 | **CLOSED (48→47)** |

Group A is exactly the 16 frame rules (`regfix.txt:3419-3434`), so fixing the
frame retires 16 of the 23 rules in one move.

## Group C — CLOSED

Target idx 66 is a `nop` where we emitted `lui a0,0xa000`: the committed source
passed `(u32 *)0xA0000000` to `func_8007DC9C`, which the rule
`subst "lui\s+\$4,…" "nop" @ 63` papered over. **`func_8007DC9C()` with no
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

## Measured negatives — 19 variants, `vars` never left 16

- **Extra s16 casts do not create strands** (round 3, 6 variants): `s16 xf =
  Re-casting an already-s16 value is a no-op GCC folds; three also collapsed the
  multiply (142→131, score 59).
- **Splitting/retyping existing s16 locals does not either** (round 5, 6
  variants): `coord` split into `cx`/`cy` leaves ONE HI pseudo (disjoint live
  ranges merge); `a0_tmp` `u16`→`s16` = 50; clamped x as a multi-use `s16 xres`
  = 144 insns / 63; plus the pairwise combinations.
- **More locals of any kind do nothing** (round 1, 6 variants); **region
  bisect** (round 2): deleting the rounding block, push loop, DMA block or
  x-clamp all leave `vars=16` — deleting the loop or DMA drops the frame to 56
  via `gp_regs` (7 regs → 6), not `vars`.

## Group B — shape (from `tmp/adiff.py`)

- **x-clamp (tgt 11-19):** target loads `lh a1,4(s1)` once and uses `a1` for
  both the `bltz` and the `slt`, threading `move v1,a1` into the branch delay
  slot; we emit `lh v1` + a load-delay `nop` + `move v0,v1` with the `slt`
  operands swapped. The `coord = (v1_tmp = arg0->x);` double-assign is the
  likely cause. **y-clamp (25-41):** same one register over, plus an extra
  `move a0,v0`.
- **multiply/rounding (44-50):** pure register naming (target `a2/v1/v0/s0` vs
  our `a3/v0/v1/s4`) — the ALLOCDBG lever-sizing case. **53-59:** target holds
  `big_size` in `s0` and copies it through `v1` into `s4`; we keep it in `s4`.

## Resume here

Group A is the big prize (16 insns, 16 rules) and the spec is exact: **the body
needs FOUR HImode→SImode widening sites; it has two.** 19 variants across three
rounds show the count is a property of the *arithmetic*, not the declarations —
re-casting, splitting and retyping existing s16 values all fail, because GCC
folds a re-cast of an already-s16 value and merges disjoint live ranges into one
HI pseudo.

NOT yet tried: a decomposition in which two genuinely new s16 quantities exist
and are each needed in SImode — e.g. carrying the clamp globals
(`D_8009BE78`/`D_8009BE7A`) through s16 locals ALSO used after the clamp, or an
s16 intermediate in the `half_size`/`big_size` rounding chain. Target emits two
`sll …,0x10` and one `sra …,0x10` exactly as we do, so the two extra strands are
merged (invisible) ones — the widening must be one combine folds away entirely.

Then Group B by ALLOCDBG sizing — **after** the frame is right, since a frame
change re-shuffles the allocation.

`src/display.c` is at HEAD; `candidate.c` (47) carries the Group-C fix and the
cheat-asm removal. Neither is independently committable: with the 23 rules
ENABLED the emission shifts (same gate as hirahira_w_ctrl).

## Instruments

`tmp/ce0c{,2,3,4,5}.py` (sweeps; ce0c3/ce0c5 print the greg UNALLOCATED set =
the direct Group-A gradient), **`tmp/rtlseg.py`** (section-safe RTL reader —
its predecessor `tmp/ce0c_rtl.py` silently scanned the WRONG function when its
name lookup missed, which briefly suggested a bogus producer family; prefer
rtlseg), `tmp/ce0c_apply.py`, `tmp/ce0c_landmines.py`, `tmp/adiff.py`,
`tmp/orphan_probe.py`, `tmp/rtldump.sh`, `tmp/frame_probe.sh`, `tmp/allocone.sh`.
