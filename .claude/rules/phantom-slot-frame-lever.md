---
name: phantom-slot-frame-lever
paths: [".claude/rules/phantom-slot-frame-lever.md"]
description: "DIAGNOSIS RECIPE: target frame reserves 8/16/24 bytes no instruction touches — unallocated pseudos reload's alter_reg pays for, reproducible from ordinary live C. Three producers + the .frame gradient. NOT a sanction."
metadata:
  type: reference
---

# Phantom frame slots — the honest lever family

## Symptom

Target prologue reserves N more frame bytes than the honest build (`addiu sp,sp,-X` deltas of 8/16/24/32), no
`($sp)` reference touches the extra bytes, and the whole score gap is sp-offset cascade through the
save/restore block.

## Mechanism (GCC 2.7.2)

An UNALLOCATED pseudo — refs but no hard register — reaches reload, and `alter_reg` gives it a stack slot that
costs zero instructions; `get_frame_size()` counts it. Measured producers:

1. **Folded loop-guard compare** — a guard comparison pseudo whose compare/jump fold into a bare branch, leaving
   it ref'd but dead. Ordinary C: `s2 = a1 - 1; if (s2 != -1)` (func_8003D9A0), or a rotated-while guard
   `if (i < limit)` reusing the loop's own exit test (func_8003DBE4). A constant-folding guard produces nothing;
   the comparison must involve a real variable.
2. **combine orphan-USE** (`combine.c:10836-10841`, `distribute_notes` REG_DEAD) — a death note with no home
   whose backward scan hits a CODE_LABEL/JUMP_INSN emits a bare `(use (reg))`. Classic shape: an HImode
   sign-extend intermediate stranding at a label; the narrow value needs a second use AS AN HIMODE VALUE that
   the algorithm genuinely consumes narrow (adding a use merely to trigger orphaning is forbidden).
3. **Live named locals on multi-read fields** — naming a thrice-read field in an `s16` local moves vars in +8
   steps at zero instruction cost when the local is genuinely live.

## Instruments

- **`.frame` gradient:** compile the TU with the project cc1 and read `# vars= N` for the function (cpp | cc1
  build flags | the `.frame` line after `.ent <func>`). Separates "wrong frame" from "wrong codegen"; better
  than the sandbox score for frame work.
- **Orphan detector:** the `-da` greg dump's unallocated set, or bare `(use (reg N))` in the combine dump.

## Boundaries (a recipe, NOT a sanction)

- Every spelling independently passes the ordinary cheat tests: locals REAL and LIVE, the guard comparison the
  function's own logic. The dead-conditional-store form the mechanism can also produce
  (`if (idx2 > idx) { idx = idx2; }` with `idx` dead after) is forbidden; finding a producer legitimizes
  nothing.
- The inverse (our frame one slot too big): find which pseudo orphans in OUR build and un-strand it.
- Screen with both the frame gradient and the orphan detector — a spelling can fix `vars` and still add real
  instructions.
- The recipe bounds the search; it does not guarantee a hit. Unused pad arrays are governed separately
  ([[dead-vars-local-array]], the frozen-list phantom-frame-slot pad family).
