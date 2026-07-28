/* REJECTED (s1, measured): candidate form WITHOUT the `arg0 = 0;` dead store.
 * Prologue staging ($s0=$a0; $s2=$s0) still emits, but the second pointer
 * rebinds to the live $a0: cc1 emits `addu $16,$4,$2` where target needs
 * `addu $s0,$s2,$v0`. The dead store is load-bearing — it breaks GCC's
 * $a0==base value association (identical to InitHiraRmd_80047FBC technique 2).
 * Diff vs candidate.c: delete the line `arg0 = 0;`. Do not re-propose. */
