---
name: store-before-jal
paths: [".claude/rules/store-before-jal.md"]
description: "Target stores a pre-call value in the jal delay slot and reloads it after (no callee-save): store it in its own statement and read it back from memory inside the call's expression, not via a local."
metadata:
  type: reference
---

# Store in the jal delay slot, reload after the call

## Symptom

Target:

```mips
lhu  $v0, %lo(D_8008D59E)($at)    # lookup
jal  game_GetPlayerData
sh   $v0, 0x352($s0)              # delay slot: store BEFORE $v0 is overwritten
lh   $v1, 0x352($s0)              # reload after the call
sll  $v1, $v1, 2
addu $v1, $v1, $v0                # + return value
```

Ours keeps the lookup in a callee-save across the call: an extra `$sN` save/restore, no delay-slot fill
(`diagnose` shows "+1 callee-save vs target").

## The C shape that produces it

```c
*(s16 *)(arg0 + 0x352) = *(u16 *)((u8 *)&D_8008D59E + arg1 * 20);
src = *(s32 *)((((s32) *(s16 *)(arg0 + 0x352)) << 2)
               + game_GetPlayerData(*(s16 *)(arg0 + 4)));
```

1. Store the value in its own statement BEFORE the call statement.
2. The call sits INSIDE a larger expression that reads the value back from the stored memory location.
3. The post-call read goes through memory, not a local — a local (`s16 lookup_val = ...; ... lookup_val << 2`)
   keeps the value live across the call in `$s0`.

The same applies to any value computed pre-call, stored, and reloaded after (constants, pointers). m2c often
shows the call nested inside the surrounding expression — strong evidence for this shape. Example:
calc_fc_frame_800203B4.

## Related

[[defer-store-past-later-compute-into-jal-delay]] · [[hoist-call-arg-local-flips-jal-delay]] ·
[[register-alloc-pure-c]]
