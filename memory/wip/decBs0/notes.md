# decBs0 - WIP seed (2026-08-05)

**Baseline: honest pure-C distance 58; 132 insns against target's 134; frame 64
against target's 72.** `src/text1a.c:1252`. 39 regfix rules, 0 asmfix.
Measure with `wsl bash tmp/csz/d.sh decBs0 text1a` and
`wsl bash tmp/csz/frame.sh decBs0 text1a`.

## A register pin must be retired

```c
register s16 *fp_ptr asm("fp");   /* CHEAT - forbidden, must go */
```

`fp_ptr` holds `D_800F62E0` across the whole body including five calls. The pin
is cheat-asm ([[inline-asm-policy]]), so the function cannot complete while it
stands; the sandbox already strips it, which is why the honest score is 58 while
`--keep-cheat-asm` reads 52. The 6-point difference is the pin's whole value.

## Single root cause: target spills ONE pseudo that we keep in a register

Three independent numbers agree:

| | ours | target | delta |
|---|---|---|---|
| frame | 64 (`vars= 8`, regs 10/0, args 16) | 72 | **+8** |
| insns | 132 | 134 | **+2** |
| triage buckets | FRAME 21, RENAME 13 | | |

+8 bytes of locals is exactly one 4-byte slot rounded to alignment, and +2
instructions is exactly the `sw`/`lw` pair of one spill. So the whole FRAME
bucket (21 of the diff lines, per the heavy-12 triage) collapses to one
question: **which pseudo does target spill, and what makes it spill?**

This is the same shape as `gnd_land_hit_char_tsuba` (+16 = one spill plus 12
phantom bytes), so read that entry's round-2 notes before hunting: there the
pressure mechanism was confirmed but every spelling that produced the spill also
cost 1-2 instructions. Here the instruction budget says the right spelling costs
NOTHING - we are 2 insns short, and a spill is worth exactly 2.

## Model status: ra_solver is NOT exact here (9/11)

`simulate.py` sort order MATCHES but two allocnos miss:

```
XX pseudo 108: sim=65 dump=8  pri=6666
XX pseudo 107: sim=65 dump=3  pri=5000
```

The simulator puts both in the no-hard-reg state while real cc1 gives them `$t0`
and `$v1`. That is the opposite direction from the residual (the model over-
predicts spilling), so **do not drive a spec off this model until the gap is
understood** - most likely the reload/retry path the ra_solver README flags as
unmodelled (Phase 5 stage 2). `perturb.py` output would be unreliable here.

## Structure notes for the hunt

Two-iteration `goto` loop (`outer` 0 then 1) selecting `ptr` from `a0`/`a1` and
`tbl` from `fp_ptr`/`D_800F6340`; a dx/dy/dz range-check ladder; then a call
cluster (`single_game_getEnemyCharId`, `math_Cos`, `math_Sin`, a second
`single_game_getEnemyCharId`) whose results feed `tbl[4]`/`tbl[5]`. The values
live across that call cluster are the spill candidates - `angle`, `cos_val`,
`dx`, `dy`, `dz`, `tbl`, and `fp_ptr` itself.

The exec_game identity-split lever does NOT obviously apply: the reused locals
here are loop-carried across the two iterations, so splitting them changes
semantics rather than just identity.

## The spilled values are IDENTIFIED: both incoming params

`asm/funcs/decBs0.s` (rule-independent truth) homes BOTH parameters and reloads
them one per loop iteration:

```
sw  $a0, 0x10($sp)      sw  $a1, 0x18($sp)
lw  $a0, 0x10($sp)      lw  $a0, 0x18($sp)
```

Callee-saves occupy 0x20-0x47 (s0-s7, fp, ra = 40 bytes) and the outgoing-args
area is 0x00-0x0F, so 0x10-0x1F is target's 16-byte `vars` block holding exactly
these two 4-byte locals. Ours has `vars= 8` - **we home only one of the two
params**, and the other stays in a register. That is the entire +8 frame and the
entire 2-instruction shortfall (`sw` + `lw`).

The two reloads are the `ptr = (s32 *)a0;` / `ptr = (s32 *)a1;` arms of the
two-iteration loop. Note the slots are 8 apart, not 4, so this is NOT a
contiguous `s32 args[2]` array - an array spelling would land them at 0x10/0x14.

## Next

1. Retire the pin first and re-measure - the honest baseline is already pin-free
   but the committed source is not, and the pin constrains `$fp`, which is one of
   the ten callee-saves whose pressure decides whether a param stays in a reg.
2. Find the C spelling that makes BOTH params stack-resident and re-read per
   iteration, at the 0x10/0x18 offsets. The instruction budget says the right
   spelling is free (we are exactly 2 short). Do NOT reach for frame coercion:
   `dead-vars-local-array` is forbidden and an array spelling gives the wrong
   offsets anyway.
3. Only then look at RENAME (13 lines) - the base-reg RA the triage flagged.
