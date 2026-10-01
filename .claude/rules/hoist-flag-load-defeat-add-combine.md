---
name: hoist-flag-load-defeat-add-combine
description: "Two consecutive `p += K1; p += K2` merged by combine into one addiu: hoist the if-test's flag-word load into a named local BEFORE the first add so a real use of p sits between them."
paths: [".claude/rules/hoist-flag-load-defeat-add-combine.md"]
metadata:
  type: reference
---

# Hoist a flag-word load BEFORE its `+=` to split a combine-folded address add

## Symptom

A pointer-walking function where target keeps `addiu $sN,$sN,K1; addiu $sN,$sN,K2` but your build
emits one `addiu $sN,$sN,K1+K2`. Small distance, build_insns = target − 1.

```c
if ((*(s32 *)arg0 & 8) == 0) return;   /* load result only feeds the test */
arg0 += 4;                              /* [A] */
arg0 += 8;                              /* [B] — combine merges A+B */
```

## The lever

```c
flags = *(s32 *)arg0;                   /* real use of arg0 before [A] */
arg0 += 4;
if ((flags & 8) == 0) return;
arg0 += 8;
```

The hoisted load gives combine a use of `arg0` between the two defs, so it does not substitute; the
target's split emits with no extra instructions. `flags` names the header's flag word — the
idiomatic spelling (the matched sibling `efc_buki_draw_zanzou` in text1b.c uses the same
hoist-then-advance idiom). Example: func_800484A0 (text1b.c), 2 → 0.

## Does NOT apply when

- the load must stay in place for other reasons (another path reads that address; over-long
  `flags` live range);
- the two adds are not both unconditional;
- no matched sibling supports the idiom — still possible, but vet against the cheats-by-any-spelling
  checklist ([[no-new-park-categories]]).

Related: [[defeat-combine-symbol-fold]] · [[register-alloc-pure-c]]
