# coli_HitPauseKatana_2 Session 2 Notes (2026-04-21)

Prior archive: commit 5404958 (tabled @ permuter score 595 using leaf-bypass C form).

## Session 2 approach: match target insn count, bridge with regfix

Reached 178-insn match (size 606208) using intermediate-var C form with
register asm hints, but internal scheduling/register cascade prevents byte match.

## Key finding: GCC allocates `base` to $v1, target uses $a0

This is the ROOT-CAUSE divergence. Everything else cascades from it.

- Target: `base` in $a0, `arg3` in $a3, `move t0, a1` (mask init) fills beqz delay slot
- Mine: `base` in $v1, `arg3` materialized to $a0 (`move a0, a3` in beqz delay slot),
  mask init deferred to idx 15 (after converge label)

### Progress notes

1. **Inner-block mask** (`register u32 mask asm("t0") = arg1;` after if/else):
   moved mask init out of the very start but didn't get it into the delay slot.
   Mask init ended up at idx 15 (after converge).

2. **`__asm__ volatile("" : : "r"(mask))` force** before branch:
   moved mask init to idx 0 (before branch). +1 extra insn at start, still
   not in delay slot. Delay slot still filled with `move a0, a3`.

3. **`register volatile u16 *base asm("a0")`** hint:
   SUCCESS for base placement — base goes to $a0. But cascades:
   arg3 now materializes to $v1 instead (`move v1, a3` in delay slot).

4. **Remove intermediates** (`new_var2 = arg3; new_var3 = new_var2;`):
   GCC dedupes the 2 `base[arg3]` reads → -24 byte mismatch. The intermediates
   are LOAD-BEARING to force 2 reads of volatile pointer.

5. **Base asm(a0) + direct arg3** (rewrite7): miscompile — `sll a3, a3, 1` 
   destroys arg3 in place, wrong global (0x800a2cbc instead of 0x800a2cd4).
   Not viable.

## Remaining blockers

Under the intermediate-var + inline-asm form (rewrite5):
- `move t0, a1` at idx 0 (extra, wants to be in delay slot)
- `move a0, a3` in delay slot (wants `move t0, a1`)
- `base` in $v1 (wants $a0)
- `arg3` materialized to $a0 (wants direct $a3)
- First `lhu` loads into $t2 (wants $v1)
- `li t3, 1` scheduled at idx 19 (wants idx 21, beq delay slot)
- Case 0 tail has 4 insns (wants 5 with cross-jump un-merge)
- Case 8 uses $t2 for mask construction (wants $v0)

## Conclusion

Compound-regfix territory but with deep register cascade. Estimate 15-25
regfix rules needed (cascade rename $v1→$a0, $t2→$v1, $a0→$a3, plus
scheduling reorders). Maspsx doesn't pull `move t0, a1` into delay slot
despite eligibility — investigation into maspsx's delay-slot fill logic
may reveal a different trigger.

## Files preserved in tmp/

- rewrite_coli.py — first (hi intermediate) attempt (REGRESSION: -24 bytes)
- rewrite2_coli.py — revert from hi
- rewrite3_coli.py — inner-block mask
- rewrite4_coli.py — direct arg3 no intermediates (REGRESSION: -24 bytes)
- rewrite5_coli.py — inline asm force mask materialization
- rewrite6_coli.py — base asm(a0) hint
- rewrite7_coli.py — base asm(a0) + direct arg3 (miscompile)
- trace_regs.py — trace $6/$11 usage in cc1 output
