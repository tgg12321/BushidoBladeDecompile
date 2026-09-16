# Rejected key-variable spellings — s2 (2026-09-16)

Not cheats, just measured worse than the banked candidate.c (s16 cached
`key`, score 136 / build_insns 213). All three variants share the exact
same control-flow / field-offset logic as candidate.c; only the `key`
handling differs.

## Variant A — `s32 key;` (cached, computed once)
```c
s32 key;
...
key = a0 | (a1 << 8);
```
Measured: score 141 (s1 of this session, build_insns 202), then 142 after
splitting `shifted`/`addr`/`base_ptr`/`offset` into named locals
(build_insns unchanged at 202 in the first pass, 202→ same shape). Worse
than s16: promotes a0 via a FULL sign-extend pseudo that is then reused/
shared with the address-shift computation, producing extra materialization.

## Variant B — inline `(s16)(a0 | (a1 << 8))` at every call site (no shared var)
```c
_SsVmGetSeqVol((s16)(a0 | (a1 << 8)), &voll, &volr);
func_80087770((s16)(a0 | (a1 << 8)), voll + 1, volr + 1, 1);
/* ... repeated at all 6 call sites + the 2 _SsVmGetSeqVol calls ... */
```
Measured: score 143, build_insns 220 (worst of the three — recomputing the
OR+shift+truncate sequence at each of 8 use sites costs more than caching
it once, even though the target ALSO appears to recompute a `sll;or;sll;sra`
4-insn sequence at each call site in the asm). The asm recompute pattern is
NOT reproduced by a literal inline-expression rewrite; something else
(scheduling, or the parameter register `a0`/`a1` staying live in `$a0`/`$a1`
hardware regs rather than being copied to `s4`/`s5` at entry) drives the
target's repeated-materialization shape. Left as an open question for the
next register-alloc session (see hypotheses.md H4).

## Conclusion
`s16 key` computed once, near the top (candidate.c's form) is the best
measured of the three. None of the three variants closes the remaining
~13-instruction gap (213 built vs 200 target) — the residual is the
`(a0<<16)>>14` address-shift instruction-count difference (3 insns here vs
2 in the already-matched sibling _SsSndPause using textually the same
source idiom). See hypotheses.md H4.
