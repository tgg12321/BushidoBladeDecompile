# func_8007CE0C — WIP (opened 2026-08-04)

`src/display.c:754`. Verdict **C**, honest distance **48**, target 143 insns /
ours 142. **23 regfix rules, 0 asmfix, 0 prologue_config, not canonical.**
Source carries cheat-asm: `register s32 var_s5 asm("s5")` + an
`__asm__ volatile("" : "=r"(var_s5) : "0"(var_s5))` barrier.

## Both entry cautions discharged — measured, not assumed

1. **No sibling label landmine.** `tmp/ce0c_landmines.py` resolves every
   regfix/asmfix rule that hardcodes an absolute `.L<N>` to its owning function
   and file. All nine holders (`CalcHiraNormal`, `SetPacketData`,
   `func_80017848`, `func_8002CA8C`, `func_80056CB8`, `func_80070C70`,
   `marionation_Exec`, `mk_leaf_newpos`, `tslPrintScreen`) live in **other**
   translation units. cc1's `.L` counter is per-TU, so a label-count change in
   func_8007CE0C cannot drift any of them. CE0C's own splices already use
   `{lbl#N}`. **Restructure freely.**
2. **Not GTE.** The body is a GPU packet/DMA routine (`g_gpu_stat_reg`,
   `g_gpu_data_reg`, `g_gpu_dma_madr/bcr/chcr`) — zero cop2. The gte-3x3 /
   scratchpad-gte / packed-multiply families do not apply.

## The 48 splits into three independent groups

| group | insns | what |
|---|---|---|
| **A frame** | **16** | every prologue/epilogue offset, frame 64 vs target 80 |
| B body regalloc | ~31 | register naming + 3 extra/missing moves in the clamp + multiply blocks |
| C arg load | 1 | idx 66: target `nop`, ours `lui a0,0xa000` |

Group A is exactly the 16 frame regfix rules (`regfix.txt:3419-3434`); the other
7 rules are the body splices/substs. **Fixing the frame retires 16 of 23 rules
in one move** and is mechanically independent of B.

## Group A — the frame is a phantom-locals problem, quantified

Target reserves `0x00..0x2F` (48 bytes) below its register saves and **touches
none of it** — the only sp-relative accesses in `asm/funcs/func_8007CE0C.s` are
the 7 saves and their 7 restores. Both calls (`motion_LoadPreCalcData_8007DC68`,
`func_8007DC9C`) take at most one argument, so `args` is the 16-byte o32
minimum in target as it is in ours.

Frame equation (`mips.c:compute_frame_size`, MIPS_STACK_ALIGN=8):
`frame = ALIGN8(vars) + ALIGN8(args) + ALIGN8(gp_regs)`

|  | vars | args | gp_regs | frame |
|---|---|---|---|---|
| ours | 16 | 16 | 28→32 | **64** |
| target | **32** | 16 | 28→32 | **80** |

So the spec is: **raise `vars` from 16 to 32** (anything in [25,32] rounds to 32).
Per [[phantom-frame-slots-gcc272]] these are ordinary LIVE locals GCC allocates a
stack temp for and then register-allocates away — a padding array is FORBIDDEN
([[dead-vars-local-array]]); the live-locals form has to be found.

**The instrument is `bash tmp/frame_probe.sh func_8007CE0C display`** — cc1
prints `vars=` (which IS `get_frame_size()`) without running the pipeline. It is
a direct gradient on group A, strictly better than the score, which cannot
separate "wrong frame" from "wrong codegen". `tmp/ce0c.py` / `tmp/ce0c2.py`
report `vars` and score together per variant.

## Measured this session

- **The cheat-asm is score-inert.** Replacing the `s5` pin with a plain local and
  deleting the barrier scores **48** — identical to the committed source. The
  next session should start from that pin-free base; it costs nothing and drops
  the cheat-asm debt. (It is NOT independently committable — same gate as
  hirahira_w_ctrl: with the 23 rules ENABLED the emission shifts. It retires
  with the completion.)
- **`vars` did not move off 16 for ANY of 13 variants.** Named locals of every
  kind are register-allocated and contribute nothing: hoisting the block-scope
  temps (`y_ext`/`prod`/`rounded`) to function scope, explicit `x_clamped` /
  `y_clamped`, `s16 xc`/`yc`, packet-word locals, `s16 xlim`/`ylim` holding the
  two clamp globals (the memory's own minimal HImode trigger), and `s16`
  struct-field temps feeding the multiply. All `vars=16`.
- Region bisect (what reserves the current 16): deleting the rounding block, the
  push loop, the DMA block or the x-clamp all leave `vars=16`. Deleting the loop
  or the DMA drops the frame to 56, but via `gp_regs` (7 saved regs → 6), not
  `vars`. **The 16 bytes are not attributable to any one region.**

So "declare more locals" is a closed door. The open question is which
*computation shape* makes cc1 call `assign_stack_local` twice as often.

## Group B — shape of the body residual (from `tmp/adiff.py`)

- **x-clamp (target 11-19):** target loads `lh a1,4(s1)` once and uses `a1` for
  both the `bltz` and the `slt`, threading `move v1,a1` into the branch delay
  slot. We emit `lh v1` + a load-delay `nop` + `move v0,v1`, and our `slt`
  operands are swapped. Our `coord = (v1_tmp = arg0->x);` double-assign is the
  likely cause of the extra move.
- **y-clamp (25-41):** same story one register over (`a1`/`v1` vs our `a2`/`v0`),
  plus an extra `move a0,v0`.
- **multiply/rounding (44-50):** pure register naming — target `a2/v1/v0/s0`
  where we use `a3/v0/v1/s4`. No structural difference. **This is the ALLOCDBG
  lever-sizing case** (`tmp/allocone.sh` from the hirahira work generalises: it
  isolates one function into its own TU so the allocno table needs no
  block-alignment guesswork).
- **53-59:** target holds `big_size` in `s0` and copies it through `v1` into
  `s4` (`move v1,s0` … `move s4,v1`); we keep it in `s4` throughout.

## Group C — one instruction

Index 66: target has `nop` in the call's delay slot where we emit
`lui a0,0xa000`. Our `func_8007DC9C((u32 *)0xA0000000)` passes an argument the
target does not. The committed rule `subst "lui\s+\$4,..." "nop" @ 63` confirms
it. Try `func_8007DC9C()` with no argument (the other call site already does).

## Resume here

Group A first — it is 16 of 48, mechanically independent, and has a direct
cheap gradient (`vars=`). Then group C (1 insn, source-level). Group B last,
with ALLOCDBG sizing once the frame is right, since frame changes will
re-shuffle the allocation anyway.

`src/display.c` is at HEAD; nothing was committed.

## Instruments

`tmp/ce0c.py`, `tmp/ce0c2.py` (sweeps: vars + frame + insns + score),
`tmp/ce0c_landmines.py` (absolute-label sibling audit, reusable), `tmp/adiff.py`
