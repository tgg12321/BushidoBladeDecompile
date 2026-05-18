---
name: store-before-jal
paths: ["src/*.c"]
description: "C structure that lets GCC schedule a sh into the jal's delay slot, avoiding a callee-save register for the stored value. The unlock for the calc_fc_frame_800203B4 match."
metadata:
  type: reference
---

# The pattern

Target asm:

```mips
lhu  $v0, %lo(D_8008D59E)($at)    # v0 = lookup_table[arg1*N]
addu $s1, $a2, $zero               # save arg2 callee-side
jal  game_GetPlayerData
sh   $v0, 0x352($s0)               # delay slot: store v0 BEFORE $v0 is
                                   #             overwritten by return
lh   $v1, 0x352($s0)               # post-jal: reload the stored value
nop
sll  $v1, $v1, 2                   # *4
addu $v1, $v1, $v0                 # + return value (pd_result)
lw   $a0, 0($v1)                   # final dereference
```

What's happening:
1. Pre-jal: `$v0` holds the lookup value (`v0 = lookup_table[...]`).
2. `jal` issues. Its delay slot fires BEFORE the call's return overwrites `$v0` — so the `sh $v0` stores the *pre-jal* value (the lookup).
3. After return, `$v0` holds `game_GetPlayerData`'s return.
4. The lookup gets reloaded via `lh` for further use.

This avoids spilling the lookup value to a callee-saved `$s` register across the call. Compared to the naïve scheduling (keep lookup in `$s0` across the jal, restore at end), it saves:
- One `sw $s0` in prologue + `lw $s0` in epilogue
- The `$s0` reservation (frees it for another live value)
- Net: 2 fewer instructions per function + 1 fewer callee-save

GCC 2.7.2's scheduler emits this pattern naturally when the C structure makes the store's *only* dependency the pre-jal lookup value, AND the post-jal expression reads the stored value back from memory (not from the register).

# The C structure that triggers it

```c
*(s16 *)(arg0 + 0x352) = *(u16 *)((u8 *)&D_8008D59E + arg1 * 20);
src = *(s32 *)((((s32) *(s16 *)(arg0 + 0x352)) << 2)
               + game_GetPlayerData(*(s16 *)(arg0 + 4)));
func_8002EECC(src, mat);
```

The key shape elements:
1. **Store the lookup in its own statement**, BEFORE the call statement that consumes it.
2. **Inside the next statement, the call is INSIDE a larger expression** that also reads back from the just-stored memory location. The reload-from-memory is what tells GCC "the lookup register is dead after the store; you can let the call clobber it."
3. The post-call read is via `*(s16 *)(arg0 + 0x352)` (not via a local variable holding the lookup). A local would invite GCC to CSE-back to the register form and undo the delay-slot fill.

# The C structure that DOESN'T trigger it

```c
s16 lookup_val = *(u16 *)((u8 *)&D_8008D59E + arg1 * 20);
pd_result = game_GetPlayerData(*(s16 *)(arg0 + 4));
*(s16 *)(arg0 + 0x352) = lookup_val;
src = *(s32 *)((((s32)lookup_val) << 2) + pd_result);    // reads lookup_val!
```

This is semantically equivalent but **uses the local `lookup_val` directly in `src`'s computation**. GCC sees `lookup_val` is live after the call and assigns it to `$s0` (callee-saved). Result: extra prologue/epilogue, no delay-slot fill, +1 callee-save vs target.

This is the trap I (and prior agents) fell into for 20+ rounds on `calc_fc_frame_800203B4`. The local variable looks innocuous but tells GCC's allocator the wrong thing.

# When to suspect this is the unlock

Run `dc.sh diff-summary <func>`. If you see:

```
[callee-save] target saves [s0, s1], mine saves [s0, s1, s2]  (+1)
   hint: +1 callee-save vs target. Likely cause: a value lives across a
         jal in your C that target stores via delay-slot fill...
```

…that's this pattern. Restructure the C as above.

Also: re-read the m2c base.c top section of `agent-brief`. m2c often reads the original C correctly here (it computed expression nesting from data flow, not from variable lifetimes), so if m2c shows the call INSIDE the surrounding expression, that's strong evidence for this pattern.

# Variants

The same idea applies to ANY value computed pre-jal, used as a store target in the delay slot, and reloaded post-jal. Common shapes:

- **Lookup → store → reload** (this case)
- **Constant materialization → store → reload** (rarer; usually GCC folds the constant)
- **Address arithmetic → store-of-pointer → reload-of-pointer** (when the post-jal code needs the pointer)

Each is the same fundamental optimization: hand GCC a value it can spill to memory via delay-slot fill instead of via a callee-save reservation.

# Cost

The pattern requires:
- The store and reload to go through memory (extra `sh` + `lh`/`lhu` vs. register-only).
- That cost (~2 cycles round-trip) is less than the cost of an extra callee-save (`sw` + `lw` + reserved register lifetime).

GCC chose this tradeoff intentionally; reproducing it matches bytes.
