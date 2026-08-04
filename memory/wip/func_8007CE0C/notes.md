# func_8007CE0C — WIP (opened 2026-08-04)

`src/display.c:754`. Verdict **C**. HEAD distance **48**, `candidate.c` **47**.
Target 143 insns / ours 142. **23 regfix rules**, 0 asmfix, 0 prologue_config.
HEAD source carries cheat-asm (`register s32 var_s5 asm("s5")` + an
`__asm__ volatile("" : "=r"(var_s5) : "0"(var_s5))` barrier) — both **measured
score-inert** and dropped in `candidate.c`.

## Both entry cautions discharged — measured, not assumed

1. **No sibling label landmine.** `tmp/ce0c_landmines.py` resolves every rule
   hardcoding an absolute `.L<N>` to its owning function+file. All nine holders
   (`CalcHiraNormal`, `SetPacketData`, `func_80017848`, `func_8002CA8C`,
   `func_80056CB8`, `func_80070C70`, `marionation_Exec`, `mk_leaf_newpos`,
   `tslPrintScreen`) are in **other** TUs; cc1's `.L` counter is per-TU, and
   CE0C's own splices already use `{lbl#N}`. **Restructure freely.**
2. **Not GTE.** GPU packet/DMA routine, zero cop2. gte-3x3 / scratchpad-gte /
   packed-multiply do not apply.

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
`subst "lui\s+\$4,…" "nop" @ 63` was papering over. **`func_8007DC9C()` with no
argument → 47.** (`func_8007DC9C(0)` and `(u32 *)0` both stay at 48 — it must be
no argument at all, not a null one.)

## Group A — the phantom-slot PRODUCER is now identified exactly

Target reserves `0x00..0x2F` (48 B) below its saves and **touches none of it**;
both calls take ≤1 arg so `args` is the o32 minimum 16 on both sides. With
`frame = ALIGN8(vars)+ALIGN8(args)+ALIGN8(gp_regs)` and 7 saved regs (28→32):
ours `vars=16` → 64, target `vars=32` → 80.

**Each phantom slot is one combine orphan-USE pseudo, and the producer is an
s16→s32 sign-extension of a value ALREADY IN A REGISTER.** GCC expands it as a
shift pair (`tmp/ce0c_rtl.py` on the flow dump):

```
(set (reg:SI 96) (ashift:SI (subreg:SI (reg/v:HI 75) 0) (const_int 16)))
(set (reg:SI 95) (ashiftrt:SI (reg:SI 96) (const_int 16)))
```

combine merges the pair into one `sign_extend`, killing 96's def but leaving
`(use (reg:SI 96))` orphaned; greg reports it UNALLOCATED and it takes an 8-byte
slot. `tmp/orphan_probe.py` on our body: `UNALLOCATED: ['96','100']` — exactly
two strands, exactly `vars=16`. **A value loaded straight from memory does NOT
count**: `lh` sign-extends in the load, so there is no shift pair and no orphan.

**So the spec is: go from 2 orphan strands to 4.**

## Measured negatives — the naive levers are closed

- **Extra s16 casts do not create strands** (round 3, 6 variants). `s16 xf =
  arg0->x` + `(s32)xf`, `s16 yf = (s16)(v0_ext>>16)` + `(s32)yf`, both together,
  the clamped x carried as `s16 xc`, and both clamp limits as `s16 xlim/ylim`
  with explicit `(s32)` widening — **every one still reports exactly 2
  unallocated pseudos and `vars=16`.** Re-casting a value that is already s16 is
  a no-op GCC folds; only a genuinely distinct HImode value that must be widened
  emits the pair. Three of those variants also collapsed the multiply
  (142→131 insns, score 59).
- **Declaring more locals of any kind does nothing** (round 1, 6 variants):
  hoisting the block temps `y_ext`/`prod`/`rounded` to function scope, explicit
  `x_clamped`/`y_clamped`, `s16 xc`/`yc`, packet-word locals. All `vars=16`.
- **Region bisect** (round 2): deleting the rounding block, the push loop, the
  DMA block or the x-clamp all leave `vars=16`; deleting the loop or DMA drops
  the frame to 56 via `gp_regs` (7 saved regs → 6), not via `vars`.

## Group B — shape (from `tmp/adiff.py`)

- **x-clamp (tgt 11-19):** target loads `lh a1,4(s1)` once and uses `a1` for
  both the `bltz` and the `slt`, threading `move v1,a1` into the branch delay
  slot. We emit `lh v1` + a load-delay `nop` + `move v0,v1` and our `slt`
  operands are swapped. The `coord = (v1_tmp = arg0->x);` double-assign is the
  likely cause of the extra move.
- **y-clamp (25-41):** same one register over (`a1`/`v1` vs our `a2`/`v0`), plus
  an extra `move a0,v0`.
- **multiply/rounding (44-50):** pure register naming — target `a2/v1/v0/s0`
  vs our `a3/v0/v1/s4`. No structural difference: **the ALLOCDBG lever-sizing
  case** (`tmp/allocone.sh` generalises to any function).
- **53-59:** target holds `big_size` in `s0` and copies it through `v1` into
  `s4`; we keep it in `s4` throughout.

## Resume here

Group A remains the big prize (16 insns, 16 rules) and the spec is now one
sentence: **find two more genuinely-distinct HImode values that must be widened
to SImode.** The closed levers say it cannot come from re-casting existing
values — it needs a decomposition of the clamp/multiply arithmetic in which two
more distinct s16 quantities exist. Note target emits two `sll …,0x10` and one
`sra …,0x10` exactly as we do, so the extra strands are merged (invisible) ones,
not extra emitted shifts.

Then Group B by ALLOCDBG sizing — **after** the frame is right, since a frame
change re-shuffles the allocation.

`src/display.c` is at HEAD; `candidate.c` (47) carries the Group-C fix and the
cheat-asm removal. Neither is independently committable: with the 23 rules
ENABLED the emission shifts (same gate as hirahira_w_ctrl).

## Instruments

`tmp/ce0c.py` / `ce0c2.py` / `ce0c3.py` / `ce0c4.py` (sweeps; ce0c3 reports the
greg UNALLOCATED set = the direct Group-A gradient), `tmp/ce0c_rtl.py` (RTL for
named pseudos), `tmp/ce0c_apply.py`, `tmp/ce0c_landmines.py`, `tmp/adiff.py`,
`tmp/orphan_probe.py`, `tmp/rtldump.sh`, `tmp/frame_probe.sh`, `tmp/allocone.sh`.
