# Hypothesis ledger — func_80021A98

## Frontier (after session 1, recon)

### H1 — v1 must lose $2 to the table-pointer chain (cluster 1, ~16 of 20)
**Statement:** the entire cluster-1 diff closes if the `u16 v1` pseudo is allocated $3 instead of $2, with the table-pointer temporaries taking $2.
**Mechanism:** GCC 2.7.2 allocation order/priority between v1 (multi-block pseudo, global-alloc) and the arm-local table temporaries. In target, the table-load temps own $2 through both arms and v1 sits in $3; in ours v1 wins $2. Suspect either (a) v1's ref count / live length differs in the original spelling (e.g. v1 dies earlier, or the shift `v1*4` was a SEPARATE named pseudo so v1's pseudo is short-lived), or (b) the table-pointer chain had MORE refs (e.g. spelled through a reused variable) lifting its priority.
**Next probe (drill session):** run the `-da` greg dump (register-alloc-pure-c Step-0) on the sandbox .i to read the actual allocno order/priorities for v1 vs the table temps — stop guessing which side to nudge. Then try: (i) separate named local for the shifted index (`s32 sc = v1 * 4;` per arm — named-intermediate declaration order, sanctioned) so v1's pseudo dies at the sh 0x5C/shift; (ii) narrow v1's live range by re-reading `*(u16*)(arg1+4)`... NO — that changes bytes (extra load); prefer (i) + reuse-variable spellings for the table chain.
**Status:** OPEN — mechanism confirmed by diff map, lever not yet found. P1 (inline fold) measured byte-neutral, P2 (shared v0/store) measured WORSE (25) — both banked.

### H2 — a0_58/a1_val $4↔$5 tie flip (cluster 2, 4 of 20)
**Statement:** target gives $4 to the longer-lived a0_58; plain priority (refs/live-length) predicts our allocation, so the original C carried some extra edge for a0_58.
**Mechanism:** global.c allocno_compare tie-break. Candidate original spellings: a0_58's pseudo unified with the SECOND 0x58 load (v1_58, line 62) — one variable `p58` loaded twice would have 4 refs and win priority outright; or a1_val typed/placed so its priority drops.
**Next probe:** merge `a0_58` and `v1_58` into one reused variable (variable-reuse-for-codegen-control, SOTN-sanctioned family) — reload it between uses exactly as target does (`p = *(s32*)(s0+0x58); ... use *(u8*)p ...; p = *(s32*)(s0+0x58); use *(u8*)(p+2)`); verify bytes stay 158 and cluster 2 flips. P3 (decl-order swap) already measured DEAD.
**Status:** OPEN. P3 killed.

### H3 — cascade, not independent
**Statement:** else-arm idx ($3 ours vs $4 target) and all addu operand orders are downstream of H1; no separate lever needed.
**Status:** believed CONFIRMED by register-identity analysis of the diff map (see evidence.md); will be verified automatically when H1 lands.

## Killed / measured-dead forms
- P1 inline-fold of second u16 read — byte-neutral (equivalent spelling, kept in src for m2c-shape parity).
- P2 shared v0 + single post-if 0x58 store — WORSE (25/157); breaks cross-jump tail. rejected/shared-v0-shared-store.c.
- P3 a1_val declared before a0_58 — byte-neutral; decl order alone does not move the $4/$5 tie.

## [s1] Folding the second u16 table read inline (m2c shape) instead of reusing v1 changes v1's allocation
- mechanism: reduce v1 pseudo ref count so it loses $2 to the table-pointer temps
- probe: P1: v0 = tbl2 + *(u16*)(v0+2) in both arms; sandbox + normalized diff
- result: byte-identical (20, same diff) — CSE unifies both spellings; kept in src as it matches m2c
- verdict: KILLED

## [s1] Single shared v0 across arms + one shared 0x58 store after the if/else (literal m2c structure) fixes cluster 1
- mechanism: raise the pointer pseudo's refs/live-length priority over v1 for $2
- probe: P2: restructure and sandbox
- result: WORSE: 25, build 157 insns — breaks the cross-jump tail; per-arm stores are required
- verdict: KILLED

## [s1] Declaring a1_val before a0_58 flips the cluster-2 $4/$5 tie via pseudo creation order
- mechanism: global.c allocno tie-break by allocno number
- probe: P3: hoist a1_val declaration above a0_58's init; sandbox + diff
- result: byte-identical (20, cluster-2 diff unchanged) — declaration order alone does not move the tie
- verdict: KILLED
