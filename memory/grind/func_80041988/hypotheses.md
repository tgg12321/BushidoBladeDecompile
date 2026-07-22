# func_80041988 — hypotheses

## H1 — structural respelling closes the $v1-vs-$v0 constant placement — KILLED
- literal if-goto chain: score 2 (baseline)
- switch(a0): score 2 (identical residual)
Neither moves the compiler-generated constant off the loop.c hoist path.

## H2 — named-local constant-holder biases RA to $v0 — CONFIRMED
`s32 one = 1; if (a0 == one)` → score 0 (byte-exact). REG_USERVAR_P defeats
move_movables hoisting; global RA assigns $v0. Sanctioned family
(named-local-fake-exception, SOTN `s16 three=3;`). Live store, FAKE-annotated.
Pending independent review.
