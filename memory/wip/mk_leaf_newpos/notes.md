# mk_leaf_newpos — WIP (round 2, 2026-08-05) — 65 -> 58, primary rename CLOSED

`src/code6cac_b.c:3374`, stem **`code6cac_b`** (note: `code6cac.c` only declares it
`extern`; callers are in `code6cac.c:1839/1944` and `code6cac_c2.c:873`).
Measure: `& tools/wteng.ps1 main sandbox mk_leaf_newpos --disable all`;
frame `wsl bash tmp/csz/gn_frame.sh code6cac_b mk_leaf_newpos`.

## Baseline — this is a PURE register-allocation residual

| quantity | ours | target |
|---|---|---|
| honest `sandbox --disable all` | **58** (`candidate_58.diff`; HEAD 65) | 0 |
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

## ROUND 3 REFRAME: at 58 the residual is NOT renames — it is two LICM hoists

Re-derived the ours->target register map from the aligned streams at the 58 baseline
(`tmp/csz/mk_renames.py <ref.s> <sbx.s>`, both 176 insns). Result: **ZERO same-shape
register substitutions.** The three `$16<->$20` / `$5<->$3` leads read off `regfix.txt`
describe the OLD (pre-candidate_58) allocation and are now stale — do not build specs
from them.

What the alignment actually shows is two loop-invariant values that **we hoist into
callee-saves and target materialises inside the loop**:

| value | ours | target |
|---|---|---|
| the constant `1` | `li s5,1` before the loop, `bne v1,s5` in it | `addiu $v0,$zero,0x1` at idx 28 INSIDE the loop, reused by the `bne v1,v0` at idx 29/41/53 |
| `&D_800A391E` (the loop bound) | `lui s4,%hi / addiu s4,%lo` hoisted | rematerialised (this is what regfix's `insert "la $2,D_800A391E"` was papering over) |

Both hoists cost us two extra live callee-saves (`$s4`, `$s5`), which forces the prologue
`sw s4,32(sp)` / `sw s5,36(sp)` pair to sit where target has none, and everything the old
rules called a "rename" is downstream of that. Note target's `addiu $v0,$zero,1` sits
INSIDE the `if (*slot != 0)` guard (target idx 22 `beqz $v0` jumps past it), so the
original's shape kept the constant conditionally-executed where ours is hoisted to the
preheader.

**So the primary remaining axis is LICM, not RA** — the triage listed it as secondary at
15%; at this baseline it is what is left. `defeat-licm-hoist-var-reuse` is the relevant
recipe, but read it before spelling anything, and prefer a shape where the loop bound and
the comparison constant are naturally where target has them over a scratch-variable reuse.
The `(s32)slot < (s32)&D_800A391E` pointer-to-s32 cast in the loop condition is itself a
decomp artifact worth revisiting first — a natural pointer comparison may change what
`loop.c` treats as invariant.

## Next

1. The LICM axis above. Re-derive renames with `mk_renames.py` after ANY change; the
   regfix rule list is stale from candidate_58 onward.
3. `defeat-licm-hoist-var-reuse` (LICM 15% in triage) is the secondary axis; treat it as
   downstream of the RA fix.
4. **Do NOT commit src.** Owner runs the gate; layer-2 before any completion claim.

Tools: `tmp/csz/mk_parity.sh`, `mk_findreg.sh`, `mk_perturb.sh`.
