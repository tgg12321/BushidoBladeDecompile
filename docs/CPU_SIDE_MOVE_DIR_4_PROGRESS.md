# cpu_side_move_dir_4 — sandbox 20 → 7 progress (2026-05-30)

Building on the previous agent's session-1-through-10 work documented in
`.claude/rules/register-alloc-pure-c.md`, a NEW chain lever this session
brings cpu_side_move_dir_4 from sandbox 20 to sandbox 7 — with build_insns
matching target 160 and ALL 6 callee-saves at target's allocation. Documented
here so the next iteration doesn't re-derive.

## The new lever (`&D_800A1494 + 1` chain)

```c
idx_1495 = (u8 *)((u8 *)tbl_125c + ((s32)&D_800A1494 - (s32)D_800A125C) + 1);
```

Three things distinguish this from the previous session's "chain via tbl_125c"
attempts (which gave +1 insn or wrong allocation):

1. **Uses `&D_800A1494` (not `&D_800A1495`) as the symbol** — combine's
   symbol-fold path here keeps the trailing `+ 1` as a `(plus reg const)`
   pattern, so MIPS emits the natural `addiu $s4, $s2, 1` (1 insn) — the
   SAME bytes as the original `idx_1495 = idx_1494 + 1;` form.
2. **Still bumps tbl_125c's ref count via the chain** — flow.c counts
   tbl_125c at the chain's RTL use site, the priority shift lands idx_1495
   in `$s4` (target's allocation) instead of `$s6`.
3. **No D_800F19C0 chain needed** — surprisingly, adding the previous
   D_800F19C0-via-tbl_125c chain ON TOP regresses the allocation. The new
   lever alone is the optimum.

ALLOCDBG verification (instrumented `tmp/gccdbg/cc1` with `BB2_ALLOC_DEBUG`):

```
ord=9  pseudo=80 hardreg=17 ($s1) nrefs=2 livelen=21 pri=952  ← saved ✓
ord=10 pseudo=77 hardreg=18 ($s2) nrefs=7 livelen=154 pri=909 ← idx_1494 ✓
ord=11 pseudo=79 hardreg=19 ($s3) nrefs=5 livelen=152 pri=657 ← tbl_125c ✓
ord=12 pseudo=78 hardreg=20 ($s4) nrefs=2 livelen=74 pri=270  ← idx_1495 ✓
ord=13 pseudo=72 hardreg=21 ($s5) nrefs=2 livelen=78 pri=256  ← arg0 ✓
ord=14 pseudo=73 hardreg=22 ($s6) nrefs=2 livelen=81 pri=246  ← arg1 ✓
```

All 6 callee-saves match target exactly. Build count = 160 = target.

## The residual 7 (Levenshtein edit-distance)

After the chain lever, `engine score.py`'s masked Levenshtein reports 7 edits
in the debug_printf prep block:

| edit | target | mine |
|------|--------|------|
| reg substitution at idx 49 | `lbu a0, 0(s2)` | `lbu v1, 0(s2)` |
| sched/reg at idx 51-52 | `lui a1; lw a1` (D_800F19C0) | `sll v1; addu v1+s3` |
| sched/reg at idx 55 | `sll a0, a0, 2` | `lw a3, 0(v1)` |
| sched/reg at idx 59 | `addu a0, a0, s3` | `lui a1; lw a1` |
| delete at idx 65 | `lw a3, 0(a0)` | (mine: this insn happens earlier as lw a3,0(v1)) |

These trace to ONE root cause: target's first lbu (`*idx_1494`) lands in
`$a0`; mine lands in `$v1`. The byte-chain "carrying register" cascades
through the rest of the chain to the final `lw arg4` — so target threads
through $a0, mine threads through $v1. The D_800F19C0 lui+lw then schedules
DIFFERENTLY in the load-delay-slot.

### Why GCC picks $a0 in target, $v1 in mine

Both my build and target reserve `$v0` for the C `v0` local (used in
`v0 = -1; ... if (v0 != 0)` after debug_printf). So `$v0` is unavailable.

In mine, `$v1` is FREE — so the first lbu picks `$v1` (lowest available
scratch).

In target, `$v1` MUST also be unavailable for the first lbu — otherwise
`$v1` would be the natural pick. Something else is reserved in `$v1`.

The previous agent's session-7 PRIODBG suggested D_800F19C0's `lw $a1`
pseudo binding to `$a1` is GCC's call-arg pre-binding heuristic. If a
similar pre-binding exists for `$v1` (e.g., a pseudo destined for some
later arg-register use), the byte chain would pick `$a0` next.

### What was tried this session that didn't close it

15 source variants in `tmp/sweep_csmd4.py`:
- `void *a1_v = D_800F19C0;` preload (CSE folded) → 7
- Swap arg4/arg5 compute order → 8
- Inline arg5 in call (matches marionation_Exec form) → 7 (same code,
  different intermediate diff)
- `arg4` via `idx_1495[-1]` (semantically equivalent) → 7
- `tbl_via_chain` routing through D_800F19C0 to extend dep chain → 12
- Type changes (u32 instead of s32) → 7
- `register` keyword on locals → 7
- Renaming the C `v0` to `ret_state` (broke naming coincidence) → 7
- Adding `s32 v1_holder = v0;` to occupy $v1 (emits real bytes) → 14

And several chain-form variants in `tmp/sweep_chain.py`:
- `&D_800A1495 - D_800A125C` (no +1) → 9 (build 161, +1 insn)
- `&D_800A1494 - D_800A125C + 2 - 1` → 7 (same as +1, combine folds)
- `((char *)tbl_125c + ...)` cast form → 7
- `&((u8*)tbl_125c)[idx]` array form → 7
- `(s32)tbl_125c + delta + 1` int-arith form → 7

The +1 form is the structural minimum within this lever class.

## What remains genuinely untried

1. **Permuter `PERM_*`-directed** — random permuter hit ~495 floor in 5 min
   from the sandbox-7 base (vs sandbox=7 corresponding to lower permuter
   score). A PERM_GENERAL or PERM_TYPECAST run from this base might find a
   structural mutation the random walk missed.

2. **Source-structure pattern that pre-binds $v1** — if mode/call analysis
   can identify what pre-binds $a1 in target (D_800F19C0's call-arg
   binding) and a similar mechanism for $v1, we can engineer the source to
   trigger it.

3. **C `v0` local restructure** — currently `v0 = sys_VSync(-1); ...
   v0 = -1; goto check; v0 = 0; if (v0 != 0)`. The single C variable
   serves both timeout return state AND vsync count. Splitting these into
   two separate variables might shift allocation.

## Committed state

The source for cpu_side_move_dir_4 was reverted to the pre-session
committed state (with `register asm` pins + 5 regfix rules). The chain
lever is NOT committed because the chain shifts maspsx indices →
`verify-oracle --rebuild` reports SHA1 `33433dc69b...` ≠ oracle. To commit
the chain lever, the residual 7 must close to 0 first (then `retire`
drops all 5 rules).

## Related
- `.claude/rules/register-alloc-pure-c.md` — the previous agent's
  session-1-through-10 analysis. This session's findings extend that.
- `.claude/rules/no-compiler-divergence.md` — the standing HARD RULE
  that constrains the search to C source structure only.
- `tmp/sweep_csmd4.py`, `tmp/sweep_chain.py` — the variant sweepers
  used this session.
- `tmp/gccdbg/cc1` — instrumented cc1 (ALLOCDBG/SCHEDDBG/PRIODBG hooks);
  canonical `tools/gcc-2.7.2/build/cc1` SHA1 `045c9543d3...` is UNTOUCHED.
