# func_8001FBE8 — hypotheses / ruled out (2026-09-23)

## Layer-2 review history
1. FAIL #1: a local `u8 *other` was written 4x (`other = *(u8 **)rec;`) and changed no
   bytes (Ruling 5 / simplest form). Removed. Banked: rejected/other-multiwrite-0.c.
2. FAIL #2:
   - `off`, a second loop counter duplicating `i * 0x44C`, changed no bytes. Removed
     (candidate.c now uses `rec = &D_80101EC8 + i * 0x44C;`, 0/289).
   - The same bytes were read through both `ent->b[k]` and `data[k]`. Unified on `data[]` (0/289).
   - **`rec` written in two different paths with different templates** fails Ruling 5
     prongs 1(a)-(d) and 3. The bytes depend on it (split = 14). This is OPEN and needs
     an owner answer: see docs/grind/borderline.md 2026-09-23 func_8001FBE8.
   The reviewer cleared everything else: `o`, StatusEvt, the local prototypes, and the
   range, kind and `i == 0` tests.

## Ruled out (spellings of the ternary store)
Inline `?:` in any operand order, `(x == 0) * 0x44C` inline, `?:` with the inverted test,
and reusing the loop `off` as the offset local (18: it pins s5).
Loop forms with no effect: `i * 0x44C` vs `off`, declaration order, and the order of the
for-clauses (10).

## Frontier if the owner says no
Find a shape where the call-free D_800A3758 block's pointer is its own pseudo yet lands
in $s1. Also ruled out (2026-09-23, all 14/289, every scored hunk s1-vs-a1): a separate
function-scope `sel`, a block-local `sel`, and an if/else-if chain in place of
early returns combined with the split local. Not yet run: a permuter from the
split-local body (tmp/fbe8/v1.c).
