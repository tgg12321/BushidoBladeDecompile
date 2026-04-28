# exec_game -- TABLED at score 4425

## Function Profile
- **Location:** main.c, 194 instructions (target), 201 instructions (best GCC output)
- **Purpose:** 5-phase memory block manager for SPU voice allocation:
  - Phase 1: Merge adjacent allocated blocks (tag bit 0x80000000)
  - Phase 2: Mark zero-size blocks as sentinel 0x2FFFFFFF
  - Phase 3: Selection sort by address (tag bits 0x0FFFFFFF)
  - Phase 4: Compact first sentinel (swap with last block, truncate)
  - Phase 5: Backward sweep marking freed blocks (0x80000000 -> 0x40000000)
- **Frame:** Frameless leaf function, no callee-saved registers
- **Globals:** g_spu_voice_key_b (block count), g_spu_voice_key_c (block array base)
- **Block format:** 8-byte entries: [0]=tag (flags|address), [1]=size

## Score Trajectory
1. **Initial m2c + manual cleanup** -- score ~9530
2. **goto labels (phase3_exit, phase4_exit, phase5_exit)** -- score 4775 (prevented loop rotation)
3. **Permuter: `0 <= key_b`, `new_var` Phase 5** -- score 4470
4. **Permuter: pre-computed `new_var2 = key_b << 3`** -- score 4425 (best correct)
5. **Permuter output 4245** -- semantically wrong (pre-loads stale cur[0])
6. **Permuter outputs 4285-{1,2,3}** -- semantically wrong (base[1] instead of cur[1])

## Permuter
- 150+ outputs explored, many long runs
- Best semantically correct: 4425
- All sub-4425 outputs have semantic errors (permuter doesn't check correctness)

## Root Cause: GCC 2.7.2 Split Pointer Optimization (INTRACTABLE)

The target was compiled by SN Systems cc1psx, which does NOT apply pointer strength reduction. GCC 2.7.2 does.

When GCC sees `ptr[0]` and `ptr[1]` in a loop with `ptr += stride`:
- Creates a **derived induction variable** `ptr_hi = ptr + 4`
- Loads `[0]` from `ptr` and `[1]` from `ptr_hi` (both offset 0)
- Advances BOTH: `ptr += 8; ptr_hi += 8;`

The target uses a single register with offsets:
```
lw $2, 0($reg)    # [0]
lw $3, 4($reg)    # [1]
addu $reg, $reg, 8
```

This adds ~2 extra instructions per loop (ptr_hi creation + advancement). With 5 loops, that's ~7 extra instructions (201 vs 194).

### C-level attempts to prevent split pointers:
- **Cast arithmetic** `(s32 *)(((s32)cur) + 8)` -- still creates split pointers
- **Native pointer arithmetic** `cur += 2` -- identical split pointers
- Both paths produce exactly the same tree-level representation

### Why regfix can't bridge it:
- gen-regfix: 146 structural diffs, 21 matches, 17 register diffs
- The split pointers cascade into different register allocation, different constant materialization, different loop structure
- Would need 100+ regfix rules to rewrite essentially the entire function
- Practical regfix limit is ~40-75 rules for functions with isolated diffs

## 5 Compound Blockers
1. **Split pointer optimization (primary):** GCC creates derived IVs for ptr[0]/ptr[1], adding 7 extra instructions. No C-level technique prevents this.
2. **LICM constant hoisting:** GCC hoists 0x80000000, 0x2FFFFFFF, 0x0FFFFFFF, 0x40000000 into registers in loop preheaders. Target materializes them inline with lui/ori. Would need ~20 LICM unhoist rules across 5 loops.
3. **Loop counter duplication:** GCC maintains separate `i` counter alongside pointer, while target derives loop bounds differently.
4. **addu operand order:** Target uses `addu a1,v0,t0` (offset-first), GCC generates opposite order.
5. **Label renumbering cascade:** Integration shifts ~50 GCC labels (.L446-.L495), breaking func_8008BC60 regfix rules (.L705 -> .L754). Fixable but adds fragility.

## Best Code
Archived at: archive/tabled_attempts/exec_game_score4425.c

## Possible Future Approaches
- **Different compiler version:** If a cc1psx binary without strength reduction becomes available
- **Struct-based access pattern:** Untested -- typedef struct { s32 tag; s32 size; } MemBlk might prevent split pointer optimization if GCC handles COMPONENT_REF differently from ARRAY_REF
- **Full inline asm:** Write entire function in asm (defeats purpose of decompilation)
