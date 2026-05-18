---
name: satan2-kabuto-pin
paths: ["src/code6cac.c", "src/code6cac_b.c"]
description: "For saTan2KabutoWareMove (asmfix"
metadata:
  type: recipe
---


# Context

`saTan2KabutoWareMove` is item #2 in the asmfix retirement queue (215 insns, code6cac_b.c). The function is bridged via `replace_with_asmfile`. When the bridge is removed via `dc.sh retire`, the existing C body produces 22 structural diff regions vs target.

# Discovery (2026-05-17)

The first structural diff at the top of `dc.sh diff-align` output:
- Target T[3-4]: `sw $s2, 0x20($sp); addu $s2, $a1, $zero` — saves s2 + loads arg1 into s2
- Mine M[4-5]: `sw s3,36(sp); move s3,a1` — uses s3 instead of s2

cc1 picks s3 for `arg1` because of register allocation pressure, but target uses s2.

## Fix: pin arg1 to $s2

```c
s32 saTan2KabutoWareMove(u8 *arg0, u8 *arg1) {
    register u8 *arg1_s2 asm("s2") = arg1;
    // ... rest of function, replace all `arg1` references with `arg1_s2` in body
}
```

Then rename every `arg1` in the function body to `arg1_s2` (7 occurrences). The pin assignment + body renaming forces cc1 to allocate s2 for the variable throughout.

## Result

- Before pin: 22 structural diff regions, mine 4 insns short
- After pin: 20 structural diff regions, mine still 4 insns short
- Prologue (T[3-4]) now matches structurally

## Remaining work (not done in 2026-05-17 session)

The 20 remaining regions involve:
- T[6]: target `addiu $a2, $zero, 0x4`; mine `li a1, 4` (constant 4 register choice)
- T[12]: `lhu` register choice (v1 vs a1)
- T[14-15]: 2-insn pattern (andi+beq) vs mine's single beq — needs intermediate variable
- T[71-75]: 5-insn block missing from mine (lh, nop, bnez, sll, addu) — needs additional condition or loop structure
- T[96-110]: 15-insn block compressed to 1 in mine — major structural divergence in the inner switch flow

Each region needs:
- Either a register pin (risky due to cascading)
- Or a C-source restructure to match cc1's scheduling
- Or non-wildcard regfix rules (debt per user directive)

Estimated 1-2 hours of focused work to close. Not done in 2026-05-17 session due to context budget.

## Second attempt 2026-05-17 (a2-constant pin added)

Added on top of s2 pin: `register s32 four_a2 asm("a2"); __asm__ volatile ("addiu %0, $zero, 4" : "=r"(four_a2));` to force constant-4 into $a2 register. Result:
- Mine now uses `a2` for constant 4 in compares (matches target's register choice for T[15], T[22], T[23])
- But mine puts `li a2,4` at idx 11 instead of target's idx 6 (5-position drift)
- Total: still 20 regions, 4 insns short (same as s2-only attempt)

Discovery: the BIG remaining gap is at target_idx=126..144 (n=19 insns) vs mine_idx=139..140 (n=2 insns) — mine is missing 17 instructions in this region. This is a major code path missing from the current C body (likely involves D_800A38A8=1, D_800A3876=-1 side effects). Not just register-rename; actual missing logic. Won't be closed with register pins alone.

To make further progress, would need:
- Read the target.s for idx 126-144 carefully to understand the missing code path
- Identify which switch case / conditional in the C source produces (or fails to produce) this block
- Either restructure the C, or accept that the m2c-generated body is incomplete and write the missing logic

## Third investigation 2026-05-17 (branch direction inversion)

Reading target asm idx 126-144 carefully showed the "17 missing" diff isn't actually missing logic — it's a **branch direction inversion** at src/code6cac_b.c:590:

```c
if (((u32)(*(u16 *)(arg0 + 0xE) - 6) < 2U) || ((u32)(*(u16 *)(temp_s4 + 0xE) - 6) < 2U)) {
    var_v0_2 = 0x19;
    if (var_s1 == 0) var_v0_2 = 0xB;
    goto block_48;
}
D_800A38A8 = 1;
D_800A3876 = -1;
return ret;
```

cc1 emits the D_800A38A8/D_800A3876 stores **right after** the conditional branch (fall-through of the inverted condition). Target emits them **at the end** (after a join label), with the var_v0_2 setup in the middle.

Try: invert the condition so D_800A38A8 path is the TAKEN branch:
```c
if (!(((u32)(*(u16 *)(arg0 + 0xE) - 6) < 2U) || ((u32)(*(u16 *)(temp_s4 + 0xE) - 6) < 2U))) {
    D_800A38A8 = 1;
    D_800A3876 = -1;
    return ret;
}
var_v0_2 = 0x19;
if (var_s1 == 0) var_v0_2 = 0xB;
goto block_48;
```

cc1 may then emit the D_800A38A8 path off-path (branch-taken) and the var_v0_2 path inline. This MIGHT close the 17-region diff but not certain — GCC 2.7.2's branch prediction heuristics could re-invert.

NOT attempted in 2026-05-17 session due to context budget; each iteration cycle (retire → modify → build → verify-c → restore on regression) is ~10-15 min and the rest of the 20 regions still need separate analysis.

## Fourth attempt 2026-05-17 (branch inversion tested)

Applied s2 pin + inverted the if-else at line 590-600 (D_800A38A8 path becomes the taken branch, var_v0_2 becomes fall-through). Result: still 20 regions, 4 short — same as s2-only attempt. cc1 re-inverted the branch direction internally, preserving its scheduling.

Discovered the deeper issue: target_idx=96..110 (15 insns) vs mine_idx=96..96 (1 insn) shows mine collapses 15 instructions into ONE visible diff line, with the actual code DUPLICATED at idx 98-110 in mine's M[98-110]. This is cc1 emitting both branches of the if-else INLINE rather than sharing code through a join label. To fix would require breaking the C structure to share the call sequence (e.g., extract to a separate scope or use a goto into the common block).

Cleaner C body decomposition is needed — likely 1-2 more hours of focused work to identify the precise structural transform that lets cc1 share the call sequence. Not achievable in current session budget.




# Reproducing the partial progress

1. `bash tools/dc.sh retire saTan2KabutoWareMove` (sets active marker, comments out bridge)
2. Edit src/code6cac_b.c line 517 to add the pin (see fix above)
3. `python3 -c "import re; ..."` to rename `arg1` to `arg1_s2` inside function body
4. `bash tools/dc.sh build` — confirm builds without errors
5. `bash tools/dc.sh diff-align saTan2KabutoWareMove` — should show 20 regions, 4 insns short

To restore HEAD state:
1. `sed -i "s/^# RETIRE: saTan2KabutoWareMove: replace_with_asmfile/saTan2KabutoWareMove: replace_with_asmfile/" asmfix.txt`
2. `git show HEAD:src/code6cac_b.c > tmp/code6cac_b_head.c && python3 -c "..."` to restore source
3. `bash tools/dc.sh build` to confirm match
4. `bash tools/dc.sh refresh-queue` to clear active marker

# Related

- [[bridge-signature]] — 41/209 bridged stubs have wrong arity; check audit before pinning
- [[register-asm-pins]] — pins are hints, not orders
- [[minimize-regfix]] — don't add regfix rules to close remaining gap
