---
name: loop-counter-address-delay-slot
description: Express a loop load address from the loop counter, not only from a rolling byte offset, when target puts the counter increment in the load delay slot.
paths: ["src/*.c"]
---

# Counter-derived address can keep `i++` in a load delay slot

## Symptom

Target loop:

```asm
lui   at, %hi(table)
addu  at, at, v1
lw    v0, %lo(table)(at)
addiu s0, s0, 1      # load delay slot
addu  s1, s1, v0
slt   v0, s0, a0
bnez  v0, loop
addiu v1, v1, 0x10   # branch delay slot
```

Natural C with a rolling byte offset can let GCC treat the counter increment as
independent of the load address:

```c
sum += *(s32 *)((u8 *)&table + offset);
i++;
offset += 0x10;
```

GCC may then hoist `i++` before the address materialization, leaving a `nop` in
the load delay slot.

## Lever

Write the load address in terms of the loop counter value whose old value must
feed the access, while still maintaining the rolling offset if later code needs
it:

```c
s32 addend = *(s32 *)((u8 *)&table + (i << 4));
i++;
sum += addend;
offset += 0x10;
```

This gives the scheduler a real dependency: `i++` cannot move before the address
calculation, but it can fill the load delay slot.

## Confirmed case

`func_80045294` (`src/text1a_c.c`, 2026-06-05): changed the first accumulation
loop's load from `+ v1` to `+ (i << 4)`, with a named `addend`. Sandbox
`--disable all --keep-cheat-asm` dropped from distance 2 to 0 for the
cheat-clean function; `retire func_80045294` removed three regfix rules and
the full build SHA1 stayed at the oracle.
