# Evidence bank — cpu_set_move_command_and_dir_for_no_action

## Session 1 (2026-07-30, modality: recon) — CLOSED AT DISTANCE 0

### Baseline as inherited
- `engine canonical` → verdict **C**, `asm_insns 0`, `total 189`, distance 18,
  reason "pure-C distance 18 <= 50 — pure-C target". No canonical-asm routing
  question exists for this function.
- `engine sandbox --disable all` → score **18**, `target_insns 189`,
  `build_insns 188`, `rules_dropped 9`, `cheat_asm_stripped 334-335`.
  So the honest floor started 18 and the build was ONE instruction SHORT.
- `engine diagnose` → `CONTROL-FLOW (d42) 50/73 diffs are branch/jump`.
  **This triage was misleading and should not be trusted for this function.**
  The branch/jump "diffs" were almost entirely relative-offset noise caused by
  the single missing instruction shifting every subsequent branch target; once
  the deficit was closed the real diff set was 4 instructions.
- regfix.txt lines 2018-2021 hold this function's 4 own rules:
  `reorder 8,9,1,2,3,4,5,10,11,6,7 @ 1-11` (prologue) and three
  `subst "$4" "$2"` at idx 147 / 154 / 155. Both clusters turned out to be
  paperwork for exactly the two real diff clusters found below (A = prologue
  statement order, C = `val` register + branch sense), which is a useful
  general signal: the rule shapes were an accurate map of the C-side defects.
- asmfix.txt has no rules for this function.

### HEAD carried a cheat, and it was inert
`src/code6cac_b.c` HEAD had `s32 one; one = 1;
__asm__ volatile("" : "=r"(one) : "0"(one));` — a constraint-driven scheduling
barrier, i.e. cheat-asm. The cheat-invisible sandbox strips it, and the floor
WITH it in place was 18, so it was buying nothing. It was deleted, not
replaced. Net cheat-surface change for this function this session: **-1**.

### The "duplicate sibling" lead is a FALSE lead — do not chase it
`cpu_set_move_command_and_dir_for_no_action_2` exists in `src/ings.c:582` and
carries ~25 regfix rules, and BOTH functions carry the trailing comment
`/* kengo:HIGH | nm_cpu/cpu_set_move_command_and_dir_for_no_action[_2] | 189i |
x2 size collision */`. They are **not** duplicates of each other. The `_2`
function is a completely different routine: a top-level game frame loop
(`motion_Open` / `sys_GameInit` / `gpu_SetDispMask`, an OT double-buffer over
`D_800F7438 + idx*0x4090`, `sys_VSync`, `gpu_DrawOTag`). The shared name is a
splat/kengo auto-naming artifact — the "x2 size collision" annotation literally
says the name was matched by instruction COUNT (189i), which both happen to
have. There is no transferable C between them.

### The real diff map (produced by a purpose-built differ)
`tmp/grind/cpu_set_move_command_and_dir_for_no_action/s1/fdiff.py <func> <obj>`
objdumps the sandbox object, parses `asm/funcs/<func>.s`, canonicalizes both
(reloc operands → post-reloc rendering, `addu X,zero,zero`→`move`,
`addiu X,zero,N`→`li`, branch destinations blanked, hex→decimal) and prints an
aligned difflib opcode diff with target indices. It reduced an unreadable
"73 diffs" picture to three crisp clusters:

- **Cluster A — prologue statement order (6 slots).** Target order after
  `addiu sp,sp,-0x28` is: `lui a0`/`ori a0` (the 0x3EF3DF mask constant),
  `sw s2`, `move s2,zero`, `sw s1`, `move s1,zero`, `li a2,1`,
  `lui v0`/`lb v0` (D_8010277C), `lui a1`/`addiu a1` (D_801077B0), `sw ra`,
  `sw s0`. Our build put the mask constant AFTER the two zero-inits and put
  the D_801077B0 address-form BEFORE `li a2,1`.
- **Cluster B — the missing instruction (1 slot).** Target materializes the
  constant 1 TWICE: `li a2,1` at idx 7 for the loop's opaque `one`, and
  `li v0,1` at idx 17 (in the `lbu v1` load-delay slot) feeding
  `sllv v0,v0,v1; nor v0,zero,v0` for the mask. Our build emitted one
  materialization and reused it (`sllv v0,a2,v0`).
- **Cluster C — tail block (4 slots).** Target idx 162 `beqz v0,<end>` where
  we emitted `bnez v0,<work>`, and `val` allocated to `v0` in target vs `a0`
  in our build (idx 163 `li v0,26`, 171 `li v0,24`, 174 `sb v0,0(at)`).

### Measured lever chain — 18 → 12 → 8 → 4 → 0
All four measurements are `engine sandbox <func> --disable all` (rules dropped,
cheat-asm stripped), with the full normalized diff banked as an artifact at
each step.

1. **18 → 12.** Delete the `__asm__` barrier AND move `one = 1;` to after the
   `mask` / `bits` computation. `build_insns` went 188 → **189** and cluster B
   vanished completely (the mask's `li v0,1; sllv v0,v0,v1; nor` all matched,
   including the register choice `lbu v1` instead of `lbu v0`).
   Mechanism: with `one = 1;` textually before the mask expression, cse.c finds
   constant 1 already live in `one`'s pseudo when the mask shift is expanded
   and reuses that register, deleting the second `li`. Placing `one = 1;` after
   the mask means the mask's constant register has already been overwritten by
   its own `sllv` result, so cse has nothing to reuse. This is the general
   shape: **two required materializations of the same small constant are
   separated by clobbering the first one before the second is created, i.e. by
   statement order, not by a barrier.**
2. **12 → 8.** Order `one = 1;` before `ptr = &D_801077B0;` (was after).
   Fixes the `li a2,1` vs `la D_801077B0` ordering inside cluster A. sched1
   priority here follows source statement order directly.
3. **8 → 4.** Compute `mask` as the FIRST statement of the body — before
   `count = 0; i = 0;` — by declaring `bits` separately and assigning it after
   the two zero-inits. This hoists `lui a0`/`ori a0` above the callee-save
   stores to target's idx 1-2. Cluster A now empty.
4. **4 → 0.** Restructure the tail: hoist `append_last:` OUT of the else-arm's
   `if` body to a shared trailing block, assign `val` INSIDE each arm's `if`
   (not before it), give each arm `goto append_last;`, and add an explicit
   `goto set_count;` for the append-nothing path.
   Mechanism: with the label nested inside one arm, GCC lays the then-arm out
   as branch-TO-the-work (`bnez`) and `val` gets `a0`; with a shared tail both
   arms become branch-AWAY-from-the-work (`beqz` to the end, work reached by
   fallthrough in the else-arm, by `j` in the then-arm) and `val` reuses the
   dead `and` result register `v0`. All 4 remaining diffs closed together —
   branch sense and register allocation were ONE defect, not two.

### Final state
`sandbox --disable all` = **0**, `build_insns == target_insns == 189`, and the
normalized instruction-for-instruction diff has zero real differences. The 12
residual lines in `diff_s1_h3.txt` are artifacts of the differ itself (it does
not evaluate `(0x3EF3DF >> 16)`-style split-immediate expressions from the
splat asm, and objdump names an unresolved `jal` reloc after a neighbouring
symbol); each was verified arithmetically: 4125663>>16 = 62, 4125663&65535 =
62431, 2147483648>>16 = 32768, 65568>>16 = 1, 65568&65535 = 32, 65536>>16 = 1,
67108864>>16 = 1024, 16777216>>16 = 256.

Edits are in place in `src/code6cac_b.c` (lines ~3496-3609). The 9 rules
(4 own + 5 counted by the sandbox as droppable) are still in `regfix.txt` and
are now redundant; retiring them is the operator/driver step
(`engine retire cpu_set_move_command_and_dir_for_no_action` →
`verify-oracle`), since this session's contract forbids touching regfix.txt.

### Reusable findings for other functions
- `engine diagnose`'s CONTROL-FLOW verdict is unreliable when `build_insns !=
  target_insns`: a single missing instruction shifts every downstream branch
  offset and mass-reports as branch/jump diffs. **Check the insn-count delta
  first; if it is non-zero, find the missing instruction before believing any
  control-flow triage.**
- A `subst "$4" "$2"`-style register-rename cluster co-located with a
  branch-sense difference is likely ONE control-flow-shape defect, not two
  independent ones. Fix the shape and both go.
- `kengo:HIGH ... x2 size collision` comments name functions by instruction
  count, so identically-named `<name>` / `<name>_2` pairs in this tree can be
  entirely unrelated. Verify by reading the bodies before treating one as a
  duplicate lead.
- The differ `fdiff.py` is generic (takes func + object path) and is worth
  promoting out of `tmp/` if a future session wants it repo-wide.
