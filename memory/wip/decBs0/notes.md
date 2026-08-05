# decBs0 — WIP (current state 2026-08-05, post-revert re-derivation)

`src/text1a.c:1252`. HEAD baseline: honest pure-C distance **58**, 39 regfix rules, plus a
`register s16 *fp_ptr asm("fp")` pin (cheat-asm — the sandbox strips it, which is why the
honest score is 58 while `--keep-cheat-asm` reads 52). Measure with
`wsl bash tmp/csz/d.sh decBs0 text1a` and `wsl bash tmp/csz/frame.sh decBs0 text1a`.

## LAYER-2 FAIL + REVERT (commit a6a83d99, reverting the self-committed 84054db1)

The FAIL is SPECIFIC and partial:
- **REJECTED**: the stride-3 pointer walk over `tbl[29]/[32]/[35]` (three unrelated one-off
  stores) — mechanism-motivated `MEM_IN_STRUCT_P` coercion outside
  [[walking-pointer-serializes-parallel-loads]]'s own scope section (that rule covers
  parallel-array element writes previously cheated with barriers/pins; this cluster's prior
  cheat was u16-coercion casts — different family). A human writes plain indexed stores
  here. Would need fresh SOTN evidence + an owner ruling.
- **CLEARED for standalone re-derivation**: the real `do {} while (outer < 2)` rewrite and
  the `fp_ptr` pin removal.
- Cleanup on next landing: stray comment at `regfix.txt:927`.

## Re-derived candidate — 58 → 11, UNCOMMITTED in `src/text1a.c`

Cleared subset only: **pin removal + real `do`-loop + PLAIN indexed stores** (the three
`*(u16 *)&tbl[N] = *(u16 *)&GLOBAL;` casts dropped to `tbl[N] = GLOBAL;` — strictly better,
11 vs 13 with the casts, and anti-cheat: an HImode→HImode copy emits `lhu` either way, so
the casts were inert coercion). **Frame and register allocation are now EXACT** (72,
`vars=16`, both params homed at 0x10/0x18, `$fp`=fp_ptr, `$s3`=tbl). 132 insns vs 134.

### Why the loop rewrite is the frame/RA fix (measured, not inferred)

`flow.c` accumulates `reg_n_refs[regno] += loop_depth` (flow.c:2081/2329/2515/2725), and
`loop_depth` comes from `NOTE_INSN_LOOP_BEG/END` notes (flow.c:440-471, 1385-1449), which
the front end emits only for real loop statements — never for a backward `goto`. So the
`goto` form counted fp_ptr's in-loop use as 1 ref instead of 2. As a real loop, fp_ptr's
allocno priority goes 182 → **487** (`floor_log2(refs)*refs*size/livelen*10000`), past both
incoming-param pseudos at 246; fp_ptr takes the last callee-save `$fp` and BOTH params
spill — target's frame exactly. `ra_solver/perturb.py` independently returned exactly ONE
single-atom solution for "pseudo 74 → hardreg 30": `refs+1`.

NB the earlier "ra_solver is not exact here (9/11)" caveat is **not disqualifying**: the two
misses (pseudos 107/108) are caller-save-class allocnos; the tail cluster that decides `$fp`
(72/73/74) simulates exactly, and the prediction was confirmed empirically.

## The whole remaining residual: the three colour LOADS batch (exactly 2 nops)

```
target:  lhu v0,g_anim_select ; nop ; sh v0,58(s3) ; lhu v0,D_800A323A ; nop ;
         sh v0,64(s3) ; lhu v0,D_800A323C ; sh v0,70(s3)          (8 insns)
ours:    lhu v1 ; lhu a0 ; lhu a1 ; sh v1,58 ; sh a0,64 ; sh a1,70 (6 insns)
```
Ours batches the three independent loads into three registers; target serialises them into
load/store pairs, paying 2 load-delay nops. Everything else in the function matches.

**Mechanism (confirmed by RTL dump, not theory).** `sched.c:true_dependence` (815-838) drops
a store→load dependence when the store is `MEM_IN_STRUCT_P` at a varying address and the
load is non-in-struct at a fixed address. `expr.c:4567-4577` sets `MEM_IN_STRUCT_P` on an
`INDIRECT_REF` whose address tree is a `PLUS_EXPR`, and `c-typeck:build_array_ref` lowers
`tbl[N]` on a pointer base to exactly that. Dump confirms: every `tbl[N]` store is `mem/s`,
every colour load is bare `mem` at a `symbol_ref`. So the exclusion fires and the loads are
free to hoist. **To restore the dependence the LOAD must become in-struct OR address-varying**
(the store side is now off the table per the FAIL).

### Measured on the load side

- **`extern s16 g_anim_select[3]` (one array for the consecutive triple 0x800A3238/A/C):**
  the loads become `ARRAY_REF`s, the dependence returns and **the block order matches target
  exactly** — but GCC then CSEs the shared base into a callee-save (`$s7`), which displaces
  fp_ptr back to rematerialisation. Score **16**, 138 insns, frame still correct (72/16).
  So the array is right for the *scheduling* and wrong for the *RA*. Three separate symbols
  can't CSE a base, which is why the current form doesn't pay that cost.
- Note the use sites do support the triple being ONE object: `func_80041E10` writes all
  three as R/G/B from one packed colour, `func_800420D0` sets `[0] = -1` as a sentinel, and
  decBs0 copies them into a 3-halfword-stride matrix column (`tbl[29]/[32]/[35]`).

## Next

1. **Search the load side with `sched_solver`** (`extract.py text1a` → `mkasm.sh` →
   `perturb.py --func decBs0 --pass 1 --goal-from-target text1a --atoms luid,luid_move`).
   A prior run flagged the block as "goal is not a topological order (5 violations)", but it
   also reported the target alignment mis-paired duplicate instruction text there, so that
   verdict is unreliable — redo it against this score-11 source before trusting it.
2. **If a dependence is genuinely required**, the only load-side spellings that create one
   are in-struct (aggregate declaration) or address-varying (read through a pointer). The
   aggregate is measured above and costs the base CSE; a pointer read would likely be folded
   back to a direct symbol access by cse. If neither lands clean, document exhaustion
   honestly rather than reaching for a store-side respelling — that door is closed.
3. **Do NOT commit.** At 0, stop and report; the owner runs the gate.
