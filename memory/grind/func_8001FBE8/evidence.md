# func_8001FBE8 — evidence (manual session 2026-09-23)

## State
- **COMPLETED-C 2026-09-23** (Match commit on main; `queue done` ok, SHA1 == oracle). The shared `rec`
  is admitted under owner Ruling 6 (ef4f8cdad); the layer-2 landing review PASSed every prong (A)-(G).
  The bullets below are the pre-ruling history.
- `candidate.c` = sandbox `--disable all` **0 (289/289)**; the earlier form of this
  body (with `off` + `other`) also reached full-build SHA1 == oracle 62efab4f when spliced
  (verify-oracle --rebuild, 2026-09-23). The cleaned candidate differs only by
  byte-neutral simplifications (both measured 0/289).
- NOT committed: two layer-2 cheat-reviewer FAILs on 2026-09-23 (see hypotheses.md).
  src/code6cac.c stays `INCLUDE_ASM`.

## Levers that closed it (from the per-hunk diff; first draft = 17)
1. `(D_800A38AE == 0 ? 0x44C : 0)` record offset in the D_800A376E branch: the target's
   sltiu/negu/andi store-flag form appears only when the offset is a block-local
   variable `s32 o = ...;` (jump.c store-flag conversion). Inline in the address it
   expands to a branch: 17 (author), 8 (reviewer measurement on the later body).
2. func_8001FAE4's return typed as `StatusEvt {u16 flags; u16 id; u8 b[4];}`:
   COMPONENT_REF/ARRAY_REF loads are MEM_IN_STRUCT_P, so the `ent->id` load schedules
   above the fixed-address D_800A38E8 store as in the target. Raw `*(u16 *)(ent + 2)` = 9.
3. `data = ent->b;` (target keeps ent+4 live in $s7): without it = 19.

## The remaining blocker: one `rec` across two exclusive paths
- The D_800A3758 != 0xFF path (call-free, ends in `return`) and the loop path share one
  `u8 *rec`. The target holds that block's pointer in **$s1**, a callee-saved reg, in a
  block with no calls. That only happens when it shares a pseudo with the loop's
  call-crossing record pointer.
- Separate local per path = **14** (reviewer: tmp/rv_fbe8/B_seplocal.c, B2_fnsel.c,
  B3_fnsel_first.c). Every scored hunk is operand-only, `s1` vs `a1`, in that block alone.
