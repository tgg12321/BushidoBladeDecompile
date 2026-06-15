# func_8002BC68 (code6cac_b.c) — BLOCKED: GTE-LZCS sibling of func_8002BEA0 (documented wall)

## TL;DR
**Direct sibling of func_8002BEA0** (same file, identical structure: GTE-LZCS
distance lookup + `(X - var_t0)*0x50/100` scaling, just `arg0` here vs `0x44C`
there, different globals). func_8002BEA0 was deeply studied 2026-06-13 — see
`memory/wip/func_8002BEA0/notes.md`. Its findings apply directly:

1. **The residual is a `(a-b)*c/100` magic-constant ORDERING wall** coupled to
   register pressure (mfhi $t-reg choice). The /100 reciprocal magic-const and
   the subtraction const materialize in swapped order vs target; GCC's fixed RTL
   for `(a-b)*c/d` locks the ordering.
2. **Random permuter EXHAUSTED:** the sibling ran ~24,000 iters / 4 workers with
   ZERO improvement over base. Do NOT re-run vanilla random permuter on this
   family — the random mutation space contains no improvement.
3. **GTE block is canonical-asm** (raw `.word 0x488CF000`/`0xE99F0000` = mtc2/swc2
   LZCS cop2 ops + a `register asm("t4")` pin). `canonical` routes ASM-PARTIAL
   (2/130 canonical). Authorizing it is a user/orchestrator decision (3rd LZCS
   sibling per the func_8002BEA0 plan), NOT worker pure-C work.

## This function's 6 rules (same shape as the sibling residual)
insert `srl $3,$3,1` @34 + delete @37 (the >> shift codegen around the LZCS
result), reorder @44-46, mfhi $13->$24 subst @52-53 (the magic-divide mflo/mfhi
register), $2<->$3 reg-swap @56-60. All are the magic-const-ordering +
register-pressure residual the sibling pinned precisely.

## Endpoint (per func_8002BEA0 NEXT section)
DIRECTED permuter (PERM_* macros around the /100 expression — random space
exhausted) OR canonical-asm authorization of the GTE block + the documented
unsolved /100 magic-ordering lever. Both are non-worker modalities. Blocked.
Resolve func_8002BEA0 FIRST (it's the simpler `0x44C`-const variant with full
RTL diagnosis already done); the solution transfers to this `arg0` variant.
