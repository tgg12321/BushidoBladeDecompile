# func_80037A20 — WIP (memcard file-count via firstfile/nextfile)

## TL;DR (2026-08-06) — floor 13 -> 1, the s0<->s1 swap is SOLVED in plain C
`candidate.c` is cheat-free (no pins, no barrier) and now sits at honest
distance **1** at the correct 33 instructions. Its disassembly is
instruction-for-instruction AND register-for-register identical to target
except ONE insn: we emit `li s1,1` where target has `addiu $s1,$s1,0x1`.
cheat-reviewer PASS 2026-08-06. HEAD still byte-matches only via two
`register asm()` pins + an `__asm__("")` barrier, so the function is INCOMPLETE.

## What closed the register swap (this is the reusable finding)
Two ordinary-C changes, together:
1. the hand-written `loop:`/`goto loop` became a real `do { ... } while (v0_val);`
2. the loop body NAMES the next entry before consuming it:
   `next = (s32*)((u8*)var_s0 + 0x28); v0_val = bios_nextfile_B(next); var_s0 = next;`
   instead of `var_s0 += 0x28; v0_val = bios_nextfile_B(var_s0);`
Change (2) is the load-bearing one: naming the intermediate moves the pointer
pseudo's references/live range, which flips the global.c allocno order so the
pointer takes $s0 and the counter takes $s1 — target's assignment. This
REFUTES the s1-s8 conclusion that the structural axis was exhausted and that
the swap was cc1-internal; it was a named-intermediate away.
Reviewer classed (2) as the SOTN-resolved named-intermediate-declaration-order
family (the `w_037.c randy` precedent), which carries no FAKE/exhaustion
prerequisite. It is also the shape target actually performs (advance, then call).

## The sole residual: the cse REG_WAS_0 fold (1 insn)
`var_s1 = 0;` dominates the entry `var_s1++`, so cse's FIRST pass
constant-folds `0 + 1` and emits `li 1` instead of an increment.
Mechanism (cse.c): a cse basic block runs until a `CODE_LABEL` or, pre-loop,
a `NOTE_INSN_LOOP_END` (`cse_end_of_basic_block`, cse.c:8038-8055). Nothing
separates the zero-init from the entry `++` — the conditional branch does not
end the block — so the zero stays in the equivalence class and folds.
Target's bytes prove its cc1 did NOT fold under the same visible structure.

## Fold spellings measured DEAD this session (all still 1, do not re-derive)
- `var_s1 += 1;` / `var_s1 = var_s1 + 1;` instead of `var_s1++`
- firstfile result staged into `v0_val` before the zero-init and the branch
- zero-init hoisted above the func_80079A30 call (5) and into both arms (16)
- explicit two-way `goto have_first;` to place a CODE_LABEL immediately before
  the increment, and the `goto`-flag variant — jump.c collapses both before cse
- inverted guard (`== 0 goto end`), `while(1)/break`, `for(;;)`, do/while
- jump-into-loop so the `++` has two reaching definitions: 20 @ 37 insns (worse)
NOT attempted, deliberately: `do { } while (0)` around the zero-init. Its rule
sanctions it only for the LABEL_OUTSIDE_LOOP_P / reorg.c interaction and
explicitly refuses to be a precedent for other wrappers or effects; s2 also
measured it collapsing here.

## Next session
The remaining question is narrow and mechanical: what C shape puts a
`CODE_LABEL` or `NOTE_INSN_LOOP_END` between `var_s1 = 0` and the entry
`var_s1++`, or otherwise removes 0 from the counter's cse equivalence class,
WITHOUT changing the 33-instruction shape. Everything else already matches.

## Floor
- HEAD honest distance: 13 (carries 2 pins + 1 opt-barrier cheat)
- candidate.c honest distance: **1** (zero cheats), 33/33 insns
