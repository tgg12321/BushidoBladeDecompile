# Rejected — array-index form of the base-address expr (s3, 2026-09-16)

```c
base = (u8 *)(((s32 *)&_ss_score)[a0] + (s16)a1 * 0xB0);
```

Measured: score 143, build_insns 233 (worse than both the manual-shift
inline form (130/213, banked in candidate.c) and the original 3-separate-
locals form (136/213)). Writing the bank index as an array subscript
instead of the explicit `(a0<<16)>>14` byte-offset expression makes GCC
2.7.2 emit a *wider* multiply-by-stride sequence (233 insns) — worse on
every axis. Not a cheat question, just measured worse; superseded by the
inline manual-shift form.
