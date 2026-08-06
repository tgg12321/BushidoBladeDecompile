# mk_leaf_newpos — WIP (round 1 / orientation, 2026-08-05)

`src/code6cac_b.c:3374`, stem **`code6cac_b`** (note: `code6cac.c` only declares it
`extern`; callers are in `code6cac.c:1839/1944` and `code6cac_c2.c:873`).
Measure: `& tools/wteng.ps1 main sandbox mk_leaf_newpos --disable all`;
frame `wsl bash tmp/csz/gn_frame.sh code6cac_b mk_leaf_newpos`.

## Baseline — this is a PURE register-allocation residual

| quantity | ours | target |
|---|---|---|
| honest `sandbox --disable all` | **65** | 0 |
| `build_insns` | **176** | **176** |
| frame | `.frame $sp,48 vars=0 regs=7/0 args=16` | `addiu $sp,$sp,-0x30` = 48 |

**Instruction count and frame already match exactly**, and FRAMEDBG shows `vars=0` — no
stack temps, no spill slots, no phantom slots. So none of the frame/phantom machinery
applies here; every point of the 65 is register choice or ordering. That is consistent
with the heavy-12 triage row (43 rules, RENAME 39 = 60%, STRUCT 9, class MIXED RA 60% /
LICM 15%) and makes this the cleanest `ra_solver` case in the queue.

## ra_solver reproduces the allocation 17/18

`extract.py mk_leaf_newpos code6cac_b` + `simulate.py` → **sort order MATCH, dispositions
17/18**. The single miss is **pseudo 89: sim=$2, dump=$4, pri 40000, calls=0** — the
highest-priority allocno in the function and the only disagreement. Everything below it
(90, 126, 86, 151, 84, 72, 140, 138, 124, 122, 73, 74, 127, 121, 118, 91, 123) is exact.

Before searching C spellings, resolve that one miss: it is either a genuine model gap
(the README's known `retry_global_alloc` / reload-spill-retry path, observable via the
`retry=` field of `BB2_FINDREG_DEBUG`) or a real input difference. A solver that is wrong
about the top-priority allocno cannot be trusted to score perturbations for the rest.

## Next

1. Diagnose pseudo 89 (sim $2 vs dump $4). Check `retry=` in the FINDREGDBG stream; if the
   dump records a post-retry state, the simulator's answer is the pre-reload allocation and
   the miss is expected rather than a model error.
2. Build the target role-map (target register per pseudo) so `perturb.py` has a spec to
   solve toward — the diff is a rename cluster, so the mapping is the real work.
3. Then `perturb.py --atoms refs,live,birth,conf,pref`, and spell surviving vectors in C.
4. The triage also names `defeat-licm-hoist-var-reuse` (LICM 15%) as a secondary axis;
   treat it as downstream of the RA fix, not as its own target.
5. **Do NOT commit src.** Owner runs the gate; layer-2 before any completion claim.
