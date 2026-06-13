# func_80064E90 — WIP

## TL;DR
Pure-C state-init helper (entry in function-pointer table at `8009BAA4`). Reads a pointer
from `D_800A347C`, mirrors three words to `D_800F0CA0/A4/A8`, sets `D_800F10E0 = 1`,
clears `D_800F0BA8`. 18 insns target / 18 insns build. **Floor lowered 8 → 7** by
removing the volatile-coercion HEAD cheat and adding a named-intermediate `s32 t`.

## Resume here
1. Apply `candidate.c` body in place of the volatile-coercion HEAD body at
   `src/text1b.c:14391-14405`. Confirm `sandbox func_80064E90 --disable all` → 7.
2. First lever to try: **directed permuter from this base** (build target.o from
   `asm/funcs/func_80064E90.s` + `tools/decomp-permuter/prelude.inc` with `.set gp=64`
   stripped — function is 18 insns so convergence is cheap).
3. Or: **instrument cc1** (`BB2_ALLOC_DEBUG`, `BB2_PRIO_DEBUG` via `tmp/gccdbg/cc1`) on
   the candidate's `.i` to see which pseudo's priority blocks `$v0` for the load values.

## The 7 remaining diffs (build vs target)
- 3 register renames: target uses `$v0` for loaded `p[0]/p[1]` values (and the
  constant `1`); build uses `$a0` for the loads and `$v0` only for the constant.
  **Target uses LOWER-numbered reg → my build is the anomaly** per
  [[register-alloc-pure-c]] Step 0.
- 4 schedule shifts: target keeps `lui at,hi(F0CA4); sw v0,lo(F0CA4)` directly after
  `lw v0, 4(v1)`; build hoists `lw v1, 8(v1)` above that pair and pushes the F0CA4
  store all the way to the end. This is downstream of the RA choice (the long-chain
  `lw v1,8` gets higher priority when the F0CA4 store's value is in `$a0` because
  `$a0`'s caller-saved short lifetime makes the dependency look less urgent).

## Ruled out at floor 7 (no new info — don't re-derive)
- `extern volatile` on the 5 globals + `volatile s32 *p` — **cheat**, removed
- block-local `{ s32 ti; ti = p[i]; D = ti; }` — score 8
- `D_800F0CA8 = p[2]` (no p reassign) — score 10
- `struct {s32 a,b,c;} *p` — score 12
- statement-order permutations (const stores first / last / between) — 7 or 8
- post-increment `*p++` — 10 (extra `addiu`)
- two distinct locals (`s32 t0, t1`) — 8
- `s32 *q` for the new pointer — 9
- `u32` type swap / `void **` / `(s32 (*)[3])` array-ref — 7-10
- comma expression + named const + decl-order permutations — all 7 (syntactic)

## Sibling cluster (important context — closing one likely unblocks many)
The HEAD source has the SAME volatile-coercion cheat on ~10+ near-identical functions
in the same file. Each writes to a distinct `D_800F0Cxx` slot but the shape is identical:

```
read p = (s32 *)D_800A347C;
D_800F0Cxx = p[0]; D_800F0Cxx+4 = p[1]; p = p[2];
D_800F10Ex = 1; D_800F0BAy = 0;
D_800F0Cxx+8 = (s32)p;
```

Confirmed members (from queue + `grep "volatile s32 *p = (volatile s32 *)D_800A347C"`):
- `func_80064E90` (this one, slot `0CA0`)
- `func_80064ED8` (slot `0CAC`)
- `func_80064F20` (slot `0CB8`)
- siblings using slots `0CC4`/`0CD0`/`0CDC` (one uses `D_800A3468`)/`0CE8`/`0CF4`/`0D18`/`0D24`/...

Cluster entries in the function-pointer table `8009BA**`: `func_80064E90`, `func_800652F4`,
`func_80064ED8`, `func_80065344`, `func_80064F20`, `func_80065394`, ... interleaved with
other state handlers.

A single closing form here likely unblocks every cluster member. Before another solo
session, investigate the cluster: an identifiable RA technique for ONE will likely
generalize to all (and the parking decision should be made at the cluster level, not
function-by-function).

## Pointers
- Target asm: `asm/funcs/func_80064E90.s` (18 insns; `lw v1,gp_rel;nop;lw v0,0(v1);lui;sw;lw v0,4(v1);lui;sw;lw v1,8(v1);li v0,1;lui;sw;lui;sh zero;lui;sw v1;jr;nop`)
- HEAD body: `src/text1b.c:14391-14405` (with the volatile-coercion cheat)
- `scan_hand_coded.py --single func_80064E90` → tier LOW (no canonical-asm signals; pure-C target)
- 25+ candidate forms swept via `tools/sweep_variants.py` — all plateau at 7
- Cheat-reviewer verdict: PASS (`meta.json.reviewer`)
