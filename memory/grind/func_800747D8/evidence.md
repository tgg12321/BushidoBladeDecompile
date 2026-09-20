# func_800747D8 — evidence

## Provenance of candidate.c (2026-09-20, operator salvage)

`candidate.c` was recovered from an **uncommitted working-tree edit** to
`src/text1b.c` found at session start (prior session ended without banking it).
It was reverted out of `src/` per [[asm-until-matched]] — `main` carries
`INCLUDE_ASM("asm/funcs", func_800747D8);` — and banked here intact.

**Measured, not claimed** — run by the operator while the body was still live in
`src/text1b.c`, before the revert:

```
engine sandbox func_800747D8 --disable all
{"score": 6, "target_insns": 208, "build_insns": 208, "scorable": true,
 "cheat_asm_stripped": 132, "rules_dropped": 0}
```

- **Honest pure-C distance: 6** (queue's pinned floor was 208 = the bare
  `INCLUDE_ASM` baseline, so the pin is stale-high — re-measure on first session).
- `build_insns == target_insns == 208` — instruction *count* already matches;
  the residual 6 is spelling/allocation, not missing or extra work.
- `cheat_asm_stripped: 132` is **file-wide** (other `text1b.c` functions), not
  this body. Scanned: the banked block contains no `__asm__`, no register pins,
  no scheduling barriers — it is pure C.

## Caveats for the first session

- This candidate was **never cheat-reviewed and never self-vetted** — no
  `self_vet.md` exists. Its constructs have not been cleared by the Judge.
  Treat the spelling as unaudited: re-measure it, then vet it before submitting.
- The struct `S_800747D8` and the `MENU_800747D8` macro are this candidate's
  own invention (declaration puns over `D_800A36A0`), not established project
  types — they are exactly the kind of construct the Judge scrutinizes.
- No `state.json` yet; the pre-existing `pre-include-asm-body.c` (2026-08-26)
  predates this work.
