# mk_leaf_newpos — WIP (round 4, 2026-08-06) — 65 -> 51, BANKED (spec-grade)

`src/code6cac_b.c:3374`, stem **`code6cac_b`** (note: `code6cac.c` only declares it
`extern`; callers are in `code6cac.c:1839/1944` and `code6cac_c2.c:873`).
Measure: `& tools/wteng.ps1 main sandbox mk_leaf_newpos --disable all`;
frame `wsl bash tmp/csz/gn_frame.sh code6cac_b mk_leaf_newpos`.

## Baseline — this is a PURE register-allocation residual

| quantity | ours | target |
|---|---|---|
| honest `sandbox --disable all` | **51** (`candidate_51.diff`; HEAD 65) | 0 |
| `build_insns` | **176** | **176** |
| frame | `.frame $sp,48 vars=0 regs=7/0 args=16` | `addiu $sp,$sp,-0x30` = 48 |

**Instruction count and frame already match exactly**, and FRAMEDBG shows `vars=0` — no
stack temps, no spill slots, no phantom slots. So none of the frame/phantom machinery
applies here; every point of the 65 is register choice or ordering. That is consistent
with the heavy-12 triage row (43 rules, RENAME 39 = 60%, STRUCT 9, class MIXED RA 60% /
LICM 15%) and makes this the cleanest `ra_solver` case in the queue.

## ra_solver is EXACT here (18/18)

Round 1's 17/18 miss (pseudo 89) was a real solver bug, not the reload-retry gap
(`BB2_FINDREG_DEBUG=89` → `retry=0`, zero retries in the function). `simulate.py` merged
the copy- and plain-preference sets; GCC's `find_reg` upgrades in two stages, copy prefs
first with `goto no_prefs` on a hit. Fixed in commit `f527fae3`; `validate.py` still 9/10
(baseline unchanged). Per-function instrumented-cc1 parity here is **OK** — the TU-level
`parity=False` is confined to `func_80030900`.

The primary rename lead (`$16 <-> $17`) came from allocnos **84 = `cat`** (`$16`, pri
16363) and **72 = `slot`** (`$17`, pri 16190), only 173 points apart; `perturb.py` gave
ten single-atom solutions and `84: refs-1` was the semantically-motivated one.

## The primary rename is CLOSED by removing an m2c artifact (65 -> 58)

Pseudo identities from the `.flow` dump: **72 = `slot`** (`u8 *slot = &D_800A3918;`, the
loop pointer, `*slot` read at insns 30/42/120) and **84 = `cat`**
(`s32 cat = func_80033498();`, the call return at insn 39, used as `cat * 2` at insn 46 and
as `(&D_8008EBF4)[cat]` at insn 59).

The C carried a genuine decomp artifact: `idx = cat * 2;` is computed, then **clobbered
with `0`**, then the same value is **recomputed** into a variable literally named
`new_var` (m2c's generated name), while the clobbered `idx` is only used to store that 0
back. Reusing the already-correct `idx` and storing `0` explicitly is both the natural C
and exactly the solver's `84: refs-1` atom:

```c
s32 val = *slot + 1;
*slot = val;
if ((u32)(&D_8008EBFD)[idx] < (u32)(val & 0xFF)) {   /* was new_var = cat * 2 */
    *slot = 0;                                        /* was idx, after idx = 0 */
}
```

Banked as `candidate_58.diff`: **58 / 176 insns**, and `ra_solver` re-extracted confirms
**pseudo 72 (`slot`) now lands in `$16`** — target's choice — with dispositions still
**18/18 exact**. This is an artifact removal, not a codegen construct: it deletes a
declaration, a dead store, and a redundant recomputation.

## ROUND 4 (2026-08-06): 58 -> 51 by removing the second m2c artifact

`candidate_51.diff` (supersedes `candidate_58.diff`, includes it) adds two changes, both
artifact removals that ADD nothing:

1. **The `goto do_call` dispatch becomes a plain `if / else if / else if` chain**, and the
   `a0_val` holder disappears — each arm just calls
   `func_800325E0((&D_8008EBF4)[cat] + K, (s32 *)pos)`. 58 -> **51**, insns still 176.
2. **`(s32)slot < (s32)&D_800A391E` becomes `slot < &D_800A391E`** — a plain pointer
   comparison, dropping two casts. Score-inert (58 either way) but plainly better C;
   included because it is a cleanup, not a lever.

Probes that did NOT move it: a named `u8 *end = &D_800A391E;` compared as pointers (58).

## THE REMAINING RESIDUAL AT 51 — an exact, unchanged spec

Re-derived at 51 with `mk_renames.py`: still **ZERO register renames**; both sides 176
insns. The entire residual is the two loop-invariant hoists, unchanged by rounds 3-4:

| value | ours | target |
|---|---|---|
| the constant `1` | `li s5,1` in the preheader; `bne v1,s5` in the loop | `li v0,1` INSIDE the loop, then `bne v1,v0` |
| `&D_800A391E` | `lui s4,0x0` / `addiu s4,s4,0` hoisted | rematerialised in-loop |

Those two hoists are the whole gap: they keep `$s4`/`$s5` live across the loop's call, so
we emit a `sw s4,32(sp)` / `sw s5,36(sp)` prologue pair target places differently, and the
alignment's other 80-odd STRUCT lines are that displacement rippling through. Target's
`li v0,1` sits INSIDE the `if (*slot != 0)` guard, so in the original the constant was
conditionally executed where ours reaches the preheader.

## Next

1. **The LICM axis is the only remaining work.** Read [[defeat-licm-hoist-var-reuse]]
   before spelling anything; prefer a shape where the constant and the loop bound are
   naturally where target has them over a scratch-variable reuse. Two reviewers have
   already rejected a codegen-motivated construct elsewhere in this queue — same bar.
2. Worth checking first: why `loop.c` treats the `li 1` as always-executed here when
   target's is inside the `*slot != 0` guard (`scan_loop`'s `maybe_never` logic). That is
   a diagnosis, not yet a lever.
3. Re-derive renames with `mk_renames.py` after ANY change; **`regfix.txt`'s rule list is
   stale from `candidate_58` onward** and its `$16<->$20` / `$5<->$3` leads are dead.
4. **Do NOT commit src.** Owner runs the gate; layer-2 before any completion claim.
3. `defeat-licm-hoist-var-reuse` (LICM 15% in triage) is the secondary axis; treat it as
   downstream of the RA fix.
4. **Do NOT commit src.** Owner runs the gate; layer-2 before any completion claim.

Tools: `tmp/csz/mk_parity.sh`, `mk_findreg.sh`, `mk_perturb.sh`.
