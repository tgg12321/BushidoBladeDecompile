# DispPracticeMenuTex_A (src/code6cac.c) — WIP checkpoint 2026-08-06

**Honest floor: 32 (unmoved). Body is now cheat-clean.**
Wiring: `asmfix.txt` `DispPracticeMenuTex_A: replace_with_asmfile "asm/funcs/DispPracticeMenuTex_A.s"`.

## Done this session

Removed an un-annotated dead self-assign `inv_s1 = inv_s1;` from the draft body. Measured
**inert** (32 before and after), so it was pure catalog debt with no codegen effect — it
would have failed `queue done` regardless. `volatile_cheat_count` is now 0.

## The residual — one uniform RA exchange plus two scheduling placements

**A. $v0/$v1 exchange, 6 sites, all the same shape.** Every "delta then round-shift" block
puts the intermediate in `$v0` for us and `$v1` for target, with the co-live value taking
the other register. Representative (indices 112-119):

```
ours   : sra v0,v0,0xc | lh v1,0(gp) | subu v0,v0,a0 | slti v1,v1,11 | bgez v0 | xori s3,v1,1 | addiu v0,v0,15 | sra v0,v0,4
target : sra v1,v0,0xc | lh v0,0(gp) | subu v1,v1,a0 | slti v0,v0,11 | bgez v1 | xori s3,v0,1 | addiu v1,v1,15 | sra v0,v1,4
```

Two quantities per block (the `dx` delta chain and the `use_high`/`D_800A3310` read),
exchanged. This is a textbook local-alloc (`local-alloc.c block_alloc`) residual.

**B. `sh zero,0(gp)` placement** (the `D_800A3310 = 0;` store in the early-return path).
Target emits it at index 56; we emit it at 64. Moving the statement above `new_var = ...`
in source moved it to 53 — overshoot, score unchanged. Target's slot is between the first
`mult` and the strength-reduced `frac_s1 * 0x9C4` chain.

**C. `lw a0,24(s0)` placement** in the final `dst+0x18` block. Target loads it LATE and
eats a genuine load-delay `nop`; we hoist it into the multiply shadow. Rewriting the source
as `dx = expr; cur = *(s32*)(dst+0x18); dx = dx - cur;` did NOT move it — the scheduler
re-hoists.

## Measured negatives (do not re-run these)

1. `D_800A3310 = 0;` moved above `new_var` — store went 64 -> 53, score stayed 32.
2. Load-placement reorder in the final block — load stayed at index 197, score stayed 32.
3. Dead self-assign removal — inert.

## Solver state (extracted, not yet exploited)

`tools/ra_solver/local_extract.py code6cac --func DispPracticeMenuTex_A` succeeded ->
`tmp/ra_solver_work/code6cac.local.json`. Running
`inverse.py local ... --block 12 --swap 0,1 --depth 1` yields **11 distinct single-atom
vectors**, cheapest being `refs_down` on qty 0 (4->3) and `live_extend` on qty 0. Levers
mapped: `hoist-shared-arm-computation-defeats-copy-pref`, `defeat-licm-hoist-var-reuse`.

**Caveat before trusting this:** block 12 was a GUESS. The QTYDBG block numbering was not
correlated to asm indices, so the contested block may be a different one. Correlate first
(dump QTYDBG alongside the emitted stream) before spending iterations on these vectors.
Heed the tool's own local-mode caveat: birth/span vectors are NECESSARY, not SUFFICIENT.

## Tooling note

`inverse_compose.py classify` is unusable for this function — see
[[func_80089F3C notes]] for why (`replace_with_asmfile` targets are unreadable to
`goalmap.asm_body`). Use `tools/pairdiff.py code6cac DispPracticeMenuTex_A`.
