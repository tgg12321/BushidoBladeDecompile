# func_8008BB24 — TABLED (2026-04-24)

## Function
- 80 instructions, fixed-point attenuation/scaling function
- Frame: 16 bytes, volatile store at sp+8
- No Kengo match (PS1-specific)
- Loop with multiply pipeline overlap (two mflo per iteration)
- Multi-function stub: split into func_8008BB24 (80 insns) + func_8008BC60 (dead code)

## Algorithm
Computes a fixed-point power-of-two scaling:
1. Combine two 16-bit args into a 23-bit value (arg << 7 + arg)
2. Compute difference, divide by 1536 to get octave + remainder
3. Compute base = 0x1000 << quot (or >> quot for negative)
4. Interpolate within the octave using iterative multiplication by 0x103B >> 12
5. Clamp result to 14 bits

## Best C attempt (v2, 80 instructions matching target count)
```c
struct { s32 pad[2]; volatile s32 val; } sp8;
// ... standard code with volatile sp8.val stores ...
```
Frame=16 ✓, sp+8 ✓, instruction count=80 ✓
But: 52 structural diffs, 16 register diffs

## Core blocker: volatile scheduling vs multiply pipeline overlap

The target achieves this scheduling:
```
mult a2,a0        # start multiply
sll a3,a2,0xc     # 2 insns of independent work
addu a1,zero,zero
bgez v1,...       # abs_rem computation
addu v0,v1,zero   # 4 more insns
negu v0,v0
srl v1,v0,5
andi t0,v0,0x1f
mflo t1           # read multiply result (8 insns after mult!)
beqz v1,...       # branch
sw t1,0x8(sp)     # volatile store IN delay slot
mult a2,a0        # start next multiply for loop
```

Our GCC 2.7.2 produces:
```
mult v1,a1        # start multiply
sll t0,v1,0xc     # 2 insns
move a2,zero
move v0,a0
mflo t1           # read multiply (ONLY 4 insns after mult)
sw t1,8(sp)       # volatile store BEFORE branch
bgez v0,...       # abs_rem computation
srl a3,v0,5       # delay slot
negu v0,v0
srl a3,v0,5       # DUPLICATE srl
beqz a3,...
andi t2,v0,0x1f   # delay slot
move t0,a0        # extra register copy
```

### Why it's intractable

1. **Volatile forces early mflo+sw**: `volatile` makes GCC emit mflo+sw immediately after the multiply has enough pipeline stalls (~4 insns). Target defers mflo 8 insns, using abs_rem computation as fill.

2. **sw can't enter delay slot**: GCC doesn't move volatile stores into branch delay slots. Target has sw in beqz's delay slot (saves 1 insn) and bnez's delay slot (saves another).

3. **CSE prevents pre-loop mult**: The loop needs a fresh `mult a2,a0` before entry (target has it at BBE4). GCC CSE-eliminates it since the same product was just computed.

4. **Duplicate srl from abs_rem scheduling**: Target computes `abs_rem = rem` in bgez delay slot (one srl after). Our code puts srl in bgez delay slot (needs duplicate for the negu path).

## Attempts exhausted
1. No volatile → no stack store (score 2160)
2. volatile s32 sp8 → frame=8 (wrong), early mflo+sw (score 1605)
3. Permuter → best 1125, semantically incorrect (replaced atten with upper)
4. Array sp8[1] → optimized away
5. volatile +m constraint → GCC 2.7.2 doesn't support +m
6. struct pad + volatile → frame=16, sp+8 correct, 80 insns, 52 structural diffs
7. struct + intermediate lo_init → 78 insns (CSE removed pre-loop mult), worse scheduling
8. struct + asm barrier for CSE → +8 bytes, asm barrier generated extra code
9. struct + loop reorder (a3_val after mult_const) → different schedule but still wrong

## What would unblock it
- A way to defer volatile store past a branch without losing the store guarantee
- A way to force GCC to place sw in branch delay slots
- Potentially: handwritten inline assembly for the second half of the function
- Or: regfix with ~50+ rewrite rules (impractical and fragile)
