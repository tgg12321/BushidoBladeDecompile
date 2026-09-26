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

## Frontier after the layer-2 FAIL (2026-09-26) — next pure-C session starts here
- Start from rejected/reuse-per-group-scope-score54.c (per-group pair, 54) — but note it is still
  a multi-write pair inside each group; the landable baseline is the one-pair-per-test body (90).
- Try an inline static helper for the same-side test (per-call parameter locals), e.g.
  `static s32 same_side(s32 ax, s32 ay, s32 bx, s32 by, s32 px, s32 py, s32 qx, s32 qy)`; GCC 2.7.2
  inlining gives per-call pseudos and may change local/global classification. Watch the return
  value: a store-flag result then `bnez` differs from the target's direct `xor; bltz`.
- Try a sides array (`s32 side[2]` written per test) — check whether it stays in registers.
- Islands: independent of the C question, landing needs an owner-instructed registry row and
  pinned inline_o.h provenance (a second independent header copy).

## slotE 2026-09-26 (after the Ruling 11 + inline_o.h class rulings)
- CONFIRMED: separate-statement verbatim islands byte-match (keep-asm 0). The joined form is not
  needed. Minimal residual = D1 `0($12)` (6 stmts, byte-neutral) + D2 post-DMPSX word (3 stmts).
- CONFIRMED: fully verbatim `($12)` builds and byte-matches under a 2-line maspsx prototype
  (tools/maspsx_empty_offset_prototype.diff) — D1 could be removed by a maspsx fidelity fix.
- KILLED: `static inline` cross-product helper as a way around sharing (90 per-value, 90 inline
  calls; 0 only with the shared pair) — inlined params/temps are single-block like any per-value
  local. Inline-expression form 90. KILLED: sides array `s32 side[2]` for the shared pair
  (tmp s_sides_array.c): 265, 578 insns — GCC 2.7.2 keeps it on the stack (and it would be the
  same multi-value reuse anyway).
- KILLED: sharing cross_b's test-11 value is unnecessary (X ablation b11 = 0) -> split out.
- Frontier: (1) owner: per-function owner_cluster_grants.txt row for D1+D2 (or D2 + maspsx fix);
  (2) engine: PINNED gte_ldv0/gte_rtv0/gte_stlvnl/gte_ApplyRotMatrix + recognizer (adjacent
  macros, DMPSX word) so default-strip sandbox reaches 0; (3) landing build + fresh layer-2 on
  candidate.c walking R11 (A)-(H) and the class/row prongs.
