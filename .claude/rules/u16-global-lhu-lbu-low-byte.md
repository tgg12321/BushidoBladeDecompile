---
name: u16-global-lhu-lbu-low-byte
paths: [".claude/rules/u16-global-lhu-lbu-low-byte.md"]
description: "Ours `lhu` where target `lbu` on a u16 global whose dispatch read uses only the low byte while a sibling read needs the halfword: keep it u16, read the low byte via `*(u8 *)&G` at the dispatch site."
metadata:
  type: reference
---

# `lhu` vs target `lbu` on a u16 global — read the low byte explicitly

## Symptom

A `u16` global's entry/dispatch read compiles to `lhu`; target reads it with `lbu` (the dispatch compares only
small constants). The global can't be retyped `u8` because a sibling read in the same function needs the full
halfword (e.g. `(word >> 8)` plus a `word + 1` write-back).

## Fix

```c
extern u16 g_entry_menu_slide_phase;
s32 state = *(u8 *)&g_entry_menu_slide_phase;   /* lbu %gp_rel(g_entry_menu_slide_phase) — low byte */
...
u16 word = g_entry_menu_slide_phase;            /* lhu — sibling still needs the halfword */
```

Little-endian: offset 0 is the low byte; GCC keeps gp-rel addressing through the cast. Use `*(s8 *)&G` for
`lb`. Example: StepSlideProgressCommitPendingStateAndQueueSound, 1 → 0.

## Related

[[narrow-stack-param-subword-offset]] · [[halfword-index-srl-sra]] · [[store-const-reload-cse]]
