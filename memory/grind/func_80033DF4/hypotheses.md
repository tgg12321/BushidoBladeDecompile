# HYPOTHESES — func_80033DF4

## Session 1 (2026-08-13, recon)

### H1 — CONFIRMED. D_8008EC24 / D_8008E908 are `u8 [][5]` tables, not scalars.
**Mechanism:** target CSEs a single `flag*5` row offset across both table reads and
adds it to a materialized base GPR — the signature of array-of-array indexing, not of
a flat `(&sym)[i]` access (which GCC folds into `%lo(sym)($at)`).
**Probe:** changed the two file-local externs at src/code6cac_b.c:88-89 to
`extern u8 D_XXXXXXXX[][5];` and wrote `T[row][entry]`.
**Result:** floor 23 → 11. CONFIRMED.

### H2 — CONFIRMED. The residual 11 is the table-base materialization's LUID/schedule
position, not a register-allocation wall.
**Mechanism:** cc1's first-pass scheduler fills the D_8008D538 `lbu` load-delay window
with whatever independent insn has the best priority. Target puts the
`lui/addiu %hi/%lo(D_8008EC24)` pair there; our build put the `andi` + `$at` arithmetic
there and materialized the base later, which cascaded into a $v0/$v1 swap over the
whole tail. Giving the base its own named local at the top of the block assigns the
address-materialization an earlier LUID.
**Probe:** `u8 (*ranks)[5] = D_8008EC24; u8 (*moves)[5] = D_8008E908;` declared with the
other else-block locals; identical code otherwise.
**Result:** floor 11 → **0**. CONFIRMED. Negative control (same code, direct
`D_8008EC24[row][entry]`, no named locals) re-measured at 11.

### H3 — CONFIRMED (KILLS the pin hypothesis). All 8 `register asm("$N")` pins in this
function were inert; none of them was load-bearing for any byte.
**Mechanism:** n/a — the pins never steered anything the corrected declarations do not
already produce.
**Probe:** deleted every pin, rewrote both if-branch bodies with plain named locals
(`flags`/`word`/`mask`) and stopped reusing `tableIndex` as the bitmask holder.
**Result:** score stayed 0. CONFIRMED.

## Frontier for the next session
The pure-C body is DONE (sandbox 0). The only outstanding work is integration, which
is outside a grind session's allowed surface:
1. Retire the 3 dead asmfix.txt rules (asmfix.txt:17-19) for func_80033DF4 and confirm
   the full-build SHA1 still equals the oracle
   (`62efab4f73f992798c43e8c730aa43baa10bb4fa`).
2. `queue done func_80033DF4`.
