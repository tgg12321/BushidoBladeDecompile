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

## 2026-08-06 session 2 — block correlation DONE, and it CORRECTS the guidance below

Ran the correlation the old note said was needed, with `tools/blockmap.py code6cac
DispPracticeMenuTex_A 112` (splits the emitted stream into basic blocks and aligns them by
ordinal with the QTYDBG block numbering, which GCC 2.7.2 emits in order).

**The block-12 vectors recorded below are aimed at the WRONG BLOCK — do not spend
iterations on them.** Block 12 is `asm[174:181]`. The contested hunk at asm 112-119 lands
in ordinal 3 = model **block 3**, which spans `asm[98:118]`.

**And block 3 has only ONE local quantity** (`q0 -> $v0`, refs 4, birth 2 death 6). The
contested region uses two values, so the other one is NOT a local quantity at all:

- the `use_high` / `lh 0(gp)` value is born and dies inside the block -> local quantity q0
- the `dx` delta chain is born at 112 and still live at 118-119, which the block split puts
  in ordinal 4 -> it CROSSES the block boundary, so local-alloc never sees it; it is a
  **global allocno**

So the $v0/$v1 exchange is **global-vs-local, not local-vs-local**. `inverse.py local`
cannot express it by construction — it permutes quantities within one block, and one of
the two participants is not one. This is exactly the caveat the previous note flagged
("block 12 was a GUESS"), now resolved: the guess was wrong.

Global model extracted for the next step: `tools/ra_solver/extract.py
DispPracticeMenuTex_A code6cac` -> `tmp/ra_solver_work/DispPracticeMenuTex_A.model.json`
(order = 27 pseudos, 133 dispositions).

**Remaining blocker for the global backend: goal derivation.** `inverse.py global` needs a
goal (`{pseudo: hardreg}`), and `goal_from_asm.py` derives it by aligning our stream with
the target's — which is precisely what does NOT work for a `replace_with_asmfile` function
(same root cause as the classify limitation; see docs/grind/inverse-compose-2026-08-06.md).
The goal must therefore be built by hand: identify which pseudo in `dispositions` is the
`dx` chain (currently `$v0` = 2, wanted `$v1` = 3) and which is its partner, then
`inverse.py global tmp/ra_solver_work/DispPracticeMenuTex_A.model.json --swap A,B`.
Mapping pseudo -> asm position is the un-done piece; the model's `allocdbg` / `flow`
fields are the place to start.

## Solver state — SUPERSEDED, see the correction above

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
