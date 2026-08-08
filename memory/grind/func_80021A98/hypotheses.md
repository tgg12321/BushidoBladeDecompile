# Hypothesis ledger — func_80021A98

## Frontier (after session 2, structural — floor 2/158)

### H5 — the last 2 insns: second-sum addu operand order vs the pref-3 delivery
**Statement:** the only remaining diff is the operand order of the two second-table addus
(target `addu $v0,$v0,$v1` base-first; ours `addu $2,$3,$2` lhu-first), a direct consequence of
the P12 operand flip that delivers hard-pref 3 to the v1/v0 preference web.
**Mechanism:** set_preference reads only XEXP(src,0) (first PLUS operand); GCC 2.7.2 emits reg-reg
PLUS operands in source order; find_reg's pref scan is ascending so pref set {3,5} -> $3. One
spelling controls BOTH the pref and the operand order — they currently fight.
**Candidate resolutions for a next session (in order of promise):**
1. Deliver pref-3 to the web from an insn whose emitted operand order does not matter, while
   spelling both second sums base-first. Enumerated-and-dead options this session: base-first
   spelling contributes pref-2 only (pruned by v1's $2 conflict, leaving {5}); split-init
   `v0 = lhu; v0 += base` puts the lhu value in $2 (wrong — target holds it in $3); locals never
   merge prefs (expand_preferences requires reg_allocno >= 0 on BOTH sides). UNEXPLORED: give the
   ELSE-arm only the flip (its base2 load is a GLOBAL pseudo — first-operand pref contribution is
   zero either way, so flipping the else arm costs nothing?? measure: flip if-arm only vs else-arm
   only — if ONE flipped arm suffices for pref-3 on the shared web, the other arm's addu emits
   base-first and the score may drop to 1 with a clean path to isolate the last insn).
2. The OTHER score-2 state (order-correct addus, sw/lhu transposed): needs v1 to lose the $5 pref
   with sw-first source. Dead ends measured/derived: dead self-assigns are flow-deleted (inert);
   arg1 cannot stay live past the lhu byte-neutrally; v1-reuse for v1_58 (P10) breaks block-1
   structure (15/159). UNEXPLORED: someone_prefers route — make any allocno that CONFLICTS with v1
   (candidates: arg0-idx(72), a3(85), a2(74)) carry pref 5 so find_reg's pass-0 avoids $5 for v1
   (prune_preferences also strips it from v1's own pref set). arg0-idx already prefers 4 via its
   home copy; no natural pref-5 source found yet.
3. Permuter sweep from the banked floor-2 candidate (both score-2 states as seeds) — the residual
   is a 2-insn operand-order/ordering knot, exactly the kind of micro-spelling a directed
   PERM_* run can crack. Target/base at offset 0 per the clean-single-function recipe.
**Status:** RESOLVED EMPIRICALLY (same session): the "flip one arm only" probe (option 1) measured
**sandbox 0** — if-arm offset-first + else-arm base-first is the zero form.  What remains open is
CLASSIFICATION, not search: is the mixed operand order of a 2-operand commutative `+`, derived from
set_preference mechanics, inside the or-tree-shape-shift forbidden family or ordinary spelling?
Session 2 returned ruling-request with the bytes-proven form banked in candidate.c (also live in
src/code6cac.c + the include/code6cac.h prototype).  If the owner rules it a cheat, the fallback
frontier is H5 options 2 (someone_prefers route for the transposed-sw/lhu score-2 state) and 3
(permuter sweep from both score-2 seeds).

## Historical frontier (after session 1, recon)

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
- [s2] P5 a0_58/v1_58 merged into one reused p58 loaded twice (the s1 H2 next-probe) — WORSE
  (31/160) and analytically impossible: both loads' values are simultaneously live in target
  (need two regs; one pseudo cannot split). rejected/p58-merged-single-var.c. H2's "4-ref merged
  variable" premise is DEAD — the actual fix was removing a1_val from local-alloc (P11).
- [s2] P7 named `s32 sc = v1*4` per arm — byte-neutral (20); named vs anonymous local temp is
  identical to local-alloc.
- [s2] P10 v1 reused for the v1_58 role (kill pref-5 via conflict) — WORSE (15/159); globalizing
  v1 across the join perturbs block-1. rejected/v1-reused-for-v1_58.c.
- [s2] dead self-assigns / dead stores for local ref-lifting — INERT (flow deletes them before
  local-alloc counts refs); confirmed matches the codegen-index note on motion_SetMotion.

## Resolved this session
- H1 (v1 loses $2 to the table chain) — CONFIRMED and CLOSED via P4+P12: in-place shift removes
  the $2-grabbing sll temp; pref-3 via first-operand spelling puts v1 in $3.
- H2 (a0_58/a1_val $4/$5 flip) — CONFIRMED mechanism was local-alloc qty priority
  (0.25 vs 0.167), CLOSED via P11 (arg1 reuse makes a1_val global with home-pref $5).
- H3 (else-arm idx / addu orders cascade) — PARTLY right: the addu operand orders inside the arms
  followed register identity as predicted, but the idx chain needed its own lever (arg0 reuse,
  P9/P9b), not just H1's cascade.
- H4 (new, confirmed): the two clusters were COUPLED through expand_preferences REG_DEAD
  preference inheritance — cluster-2's a0_58@$5 fed pref-5 into the entire v1/v0 web.

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
