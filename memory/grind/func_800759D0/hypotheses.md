# func_800759D0 — hypotheses / levers (2026-09-25)

## Status
Honest floor **25/364** (candidate.c: per-site `q0..q3`, one local per write).
The 0/364 form (one function-scope `q` written at 4 sites) was FAILED by layer-2 on
2026-09-25: rejected/function-scope-q-multiwrite-0.c (reasoning in its header). The owner
question is logged in docs/grind/borderline.md 2026-09-25 "one role, differing constant
offsets".

## What the 25 is (read from RTL dumps, tmp/f759d0/rtl.py)
- Head (3): target `addiu a1,v1,12` for `table[0] + 0xC`. Per-site, the sum is a
  single-block pseudo; local-alloc ties it to the dying table[0] pseudo -> `addiu v1,v1,12`.
  Moving the sp1C store past the sp2C if/else (so the sum spans blocks) = 29 (the store
  then leaves the beqz delay slot). The store must precede the branch.
- Loop 1 (22): target seats q in $a1, which pushes the table value to $a2, arg1*4 to $a3
  and the cell address to $t0. Per-site, q1 is local-alloc'd to $v0 (it sits inside the
  D_800A36A0-load pseudo's life and wins on priority), so everything shifts down one
  register. Worked through local-alloc: with q1 single-block, $v1 is free during its life,
  so local-alloc can never give it $a1; the target's q must be a pseudo referenced in
  several blocks (global.c). In loop 1 the `s.sp1C = q` store has a true dependence on the
  following `lh` reads through `state` (unknown pointer vs the address-taken `s`), so it
  cannot be moved past the grid branch to make q span blocks.
- Conclusion so far: every multi-block reference to loop 1's / the head's q found is a
  reuse of one variable across sites, which the current rules refuse. Loop-2 and loop-3 q
  are naturally multi-block (computed before the highlight if, stored after).

## Confirmed / refuted
- CONFIRMED: `s32 zero` constant-holder needed (literal 0 = 30, 362 insns). Layer-2 judged
  it OK.
- CONFIRMED: `(D_8009BCF8 + i)->unk0` (layer-2 OK). `D_8009BCF8[i].unk0` = 41.
- CONFIRMED: grid read `((u8 *)D_8009BCF8)[index]` with `index` local (layer-2 OK).
- REFUTED: rec pointer held across the call (123); struct-index grid (67).
- REFUTED (rule): function-scope q reuse (0) and any partial reuse (loops only = 3,
  still a multi-write carrier; extension (B) fails because the record is picked with
  `table[entry + 1]` / `table[arg2[i] + 1]`, not a constant or loop-counter subscript).
- REFUTED: head store after the if/else (29).

## Permuter (2026-09-25, carrier-free chassis = candidate.c, 8 jobs, 3,883 iterations,
## stopped early for host memory pressure)
Base 335 (permuter-weighted). Finds 90/140/180/238/265/285: EVERY one makes the head's or
loop 1's sum a multi-block pseudo by reusing a q local across blocks (best 90:
`q0 = (q1 = s.sp18 + 0xC);` making q1 a head+loop-1 carrier; others reuse q0 for the
arg1 byte term, or store q0 before assigning it). All banned (multi-write carrier /
staged-value borrow); they confirm the data flow the target needs, nothing more.
Workspace: tools/decomp-permuter/nonmatchings/func_800759D0 (built by tmp/f759d0/mkperm.sh).

## Frontier
- No honest single-write mechanism found that makes the head/loop-1 sum a multi-block
  pseudo. Untried ideas: a real record type for the sprite header (so s.sp1C is spelled as
  `&hdr->frame[k]` from a pointer local shared under Ruling 6's exclusive-region shape —
  check whether the regions qualify), or a restructure where loop 1's descriptor fill
  happens in a block that ends before the grid lookup.
- If the owner allows the one-role reuse (borderline.md), the 0 form lands as-is
  (re-review with the ruling cited; rename q to a role name for R5 1(f)).
