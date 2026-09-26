# func_8002DE20 — hypotheses (ruled out / open)

## Ruled out (measured 2026-09-26)
- u8* offset access for the point array (any spelling: `obj + i*12 + 0x118`, `obj + 0x118 + i*12`,
  `((s32 *)(obj+0x118))[i*3+k]`, `(s32(*)[3])(obj+0x118)`, `(Vec3i *)(obj+0x118)`): operand order
  of the index addu stays idx-first (102/99). Struct member array is required.
- A `pts` pointer local for obj+0x118: CSE keeps obj+0x118 live; loop base becomes obj+0x124 (159).
- Final group as `return A && B;` (88), `if (<0) return 0; return X>=0;` (99),
  `if (>=0) { ...; return X>=0; } return 0;` (99): wrong ret-0 layout or result pseudo in $a0.
- Init order / zero-block order / `3 - (max+min)` for the min/max seat swap: only
  `mid_i = 3 - min_i - max_i` fixes it without other damage.
- Island clobbers "$12","memory" (the func_8002EA24 spelling): floor 8, four reload regs in
  $t6/$t7 instead of $s0.
- Every no-multi-write spelling of the cross products: see evidence.md receipts (90/90/72/54).

- Permuter from the carrier-free body (one fresh pair per test, sandbox 90): workspace
  tmp/func_8002DE20/perm (built by tmp/func_8002DE20/mkperm.py: minimal-TU base.c with the asm
  statements as `#pragma _permuter b64literal`, target.o from asm/funcs with mvmva -> .word).
  2026-09-26, -j 2, 2106 iterations, base 1080 -> best 1060 (last novel find at 207 s). Both
  1060 finds borrow the function-scope `dz` or add a `new_var` spanning blocks: the same
  multi-block-carrier mechanism as the shared pair, i.e. the banned class, not an ordinary lever.
- Allocation dumps (tmp/func_8002DE20/d_*: instrumented cc1 BB2_ALLOC_DEBUG + -dlg): the cross
  temps and xor results are local; only a pseudo referenced in more than one block avoids the
  local-alloc tie, which is what the target shows at every test.

## Open
- An ordinary spelling that makes the xor operands non-local without one reused pair. Mechanism
  needs the cross values to be global pseudos (referenced in >1 block or dying more than once).
  Not found; only the reused pair (the original's apparent idiom, same as func_8002E6B0) does it.
- Admission of the islands: census member of the 2026-08-17 cluster; needs an operator-added
  owner_cluster_grants.txt row (Ruling 3 terms, 2026-09-15) or the 2026-09-23 verbatim-macro route
  (inline_o.h form, so owner-instructed rows have been used: 80018300/CD58/DAD0). Provenance pinning
  (4.3 copy sha + second copy) not done yet.
