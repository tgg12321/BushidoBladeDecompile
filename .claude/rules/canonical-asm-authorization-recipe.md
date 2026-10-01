---
name: canonical-asm-authorization-recipe
paths: ["inline_asm_canonical.txt"]
description: "How to author a whole-body __asm__(\"glabel ...\") block for a canonical-asm grant: symbol-form globals, named labels, sibling label-shift check, TAB+SPACE .set pairs, oracle + layer-2 before commit."
metadata:
  type: reference
---

# Writing a whole-body `__asm__("glabel ...")` block

Only for a function the `canonical` gate / grant path qualifies (`tools/scan_hand_coded.py`,
[[judge-sole-gate]]). These are the block-writing mechanics.

## 1. Symbol form for globals, never `%gp_rel(X)($gp)`

splat's `asm/funcs/*.s` disassembly form `lw $v0, %gp_rel(D_xxx)($gp)` does not assemble inside `__asm__`.
Write `lw $v0, D_xxx` — maspsx converts it to gp-relative when the symbol is gp for that file (today
`sdata_syms.txt`; under [[per-file-gp-model]], the file's own definition). For a non-gp symbol use
`lui $at, %hi(D_xxx); lw $v0, %lo(D_xxx)($at)`. Check per symbol before writing.

## 2. Named labels for intra-function branch targets

cc1 numbers `.L<N>` per TU in source order; a literal `.L987:` can collide. Use function-scoped names:
`.L_func_8006BD28_loop:`.

## 3. Check the label-shift cascade

Removing a C body drops its auto `.L<N>` labels and shifts every later function's numbers in that `.c` file.
After writing the block, run `verify-oracle --rebuild`; if SHA1 mismatches, diff `build/src/<file>.o` to find
the drifted sibling (`python3 tools/probe_func_labels.py <sibling>`). Never commit a broken oracle — fix or
revert.

## Supporting rules

- **Decimal displacements** `0(sp)`, `4(sp)` — maspsx parses `disp($reg)` as base 10; hex immediates are fine.
- **TAB and SPACE forms of every `.set`**, and restore modes after `endlabel` ([[maspsx-noreorder-stripping]]):

```c
__asm__(
    ".set\tnoat\n" ".set\tnoreorder\n"
    ".set noat\n"  ".set noreorder\n"
    "glabel func_name\n"
    "    ...body...\n"
    "endlabel func_name\n"
    ".set\treorder\n" ".set\tat\n"
    ".set reorder\n"  ".set at\n"
);
```

  Without the restore, noreorder/noat leaks into the next compiled function in the TU (symptom: the function
  scores 0 but the full build differs in the NEXT function's delay-slot fill).
- **Strip every cheat from the C-side declarations** (pins, frame-coercion pads, now-unused typedefs/externs).

## `inline_asm_canonical.txt` entry

Match existing entries. 1-2 line rationale citing the hand-coded signal(s) (S1/S2/S6/S7/S8) and provenance:
pipeline grants cite "pipeline grant YYYY-MM-DD: scan_hand_coded tier=STRONG, judge ESCALATE
canonical-asm-grant — owner ruling 2026-08-18" and have a matching docs/grind/borderline.md entry.

## Gate before commit

1. `& tools/wteng.ps1 main verify-oracle --rebuild` → SHA1 == `62efab4f73f992798c43e8c730aa43baa10bb4fa`.
2. Fresh layer-2 `cheat-reviewer`, then `& tools/wteng.ps1 main layer2 record <func> --verdict PASS --reviewer
   <id> --scope auth --expect-hash <reviewer's layer2 hash>` (owner ruling Q39).
3. `& tools/wteng.ps1 main queue done <func>`; commit with `memory/grind/<func>/layer2.jsonl`, subject
   `auth: <func> (<file>.c) — COMPLETED-INLINE-ASM-CANONICAL`.

Related: [[inline-asm-policy]] · [[review-discipline-before-commit]] · [[packed-multiply-cluster]]
