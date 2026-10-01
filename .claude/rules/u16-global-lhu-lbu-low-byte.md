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
extern u16 D_800A3578;
s32 state = *(u8 *)&D_800A3578;   /* lbu %gp_rel(D_800A3578) — low byte */
...
u16 word = D_800A3578;            /* lhu — sibling still needs the halfword */
```

Little-endian: offset 0 is the low byte; GCC keeps gp-rel addressing through the cast. Use `*(s8 *)&G` for
`lb`. Example: func_8006EC0C, 1 → 0.

## Related

[[narrow-stack-param-subword-offset]] · [[halfword-index-srl-sra]] · [[store-const-reload-cse]]
