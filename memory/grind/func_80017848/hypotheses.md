# Hypothesis ledger — func_80017848

## Session s1 (recon, 2026-08-18) — frontier established

Baseline re-confirmed this session: canonical verdict C (distance 16, 127 target insns);
sandbox `--disable all` = **16**, 125/127 build insns. Matches the inherited ledger exactly.

### Target scan-loop anatomy (both loops identical in shape; read from asm/funcs/func_80017848.s)

Preheader (loop 1): `lw a0,0xC(s2)` (slots) · `sll a1,s4,6` (off) · `addu v0,a1,a0` ·
`lw v0,0x1C(v0)` — **guard count load dest == its own address reg, so the count value dies
at the `blez`** · `blez → skip` with `addu v1,zero,zero` (i=0) in the delay slot ·
`addu a3,a0,zero` — **a copy of `slots` into a3, used exactly once** · `lw a2,0x10(s2)`
(links) · `addu a0,a1,a3` — **the loop's own base register, off+slots in ONE reg** ·
`addu v0,a0,v1` (first index).
Loop body: `lbu 0x24(v0); sll 4; addu +a2; lhu 0x4; beq →exit / addiu i++;` then
**`lw v0,0x1C(a0); nop; slt v0,v1,v0; bnez / addu v0,a0,v1`** — the bound is RELOADED from
memory every iteration through the loop base.

My floor-16 build instead: `move a3,a1` (a3 = off, not slots), **`move a1,v0` caching the
guard's count value in a register**, bound `slt v0,v1,a1` (no reload), and the index split
into two adds (`addu v0,v1,a3` in the delay slot + `addu v0,v0,a0` at loop top). 8 pts/loop.

### H1 — guarded do-while with DISTINCT guard/bound lvalues + slots-copy local (PRIMARY)
- **Statement:** the original is an explicit `if (count > 0)` guard reading the count via the
  `slots`-expression, followed by a `do{}while` whose bound is read through the loop's own
  hoisted base (a local deriving from a SECOND local holding a copy of `slots` — the
  `addu a3,a0,zero` fingerprint). Both the body index and the bound route through that one
  base local, giving target's single `addu v0,a0,v1` index and the per-iteration
  `lw 0x1C(a0)` reload.
- **Mechanism:** guard load and bottom bound load sit in different basic blocks separated by
  the multi-predecessor loop-top label; cse.c path-following stops at a label with >1 preds,
  and cse_around_loop only covers the loop head — so the bottom load cannot be unified with
  the guard's and survives as a reload. loop.c cannot hoist it either: the bound load sits
  AFTER the conditional `beq` exit (maybe_never — a conditionally-executed MEM is not moved;
  verified invariant_p/scan_loop reading this session, tools/gcc-2.7.2/loop.c:2760ff).
  In the current rotated-`while`, guard and bound are the SAME C expression, which is what
  lets GCC keep the count value live instead (the `move a1,v0` cache).
- **Known coupled risk (from prior sessions):** every `do-while` spelling measured 2026-08
  landed frame 0x30 (loses the 16 phantom bytes; target 0x40, vars=16). BUT all of those
  used the SAME-expression guard. A distinct-lvalue guard leaves its own compare/address
  pseudos un-CSE'd — plausibly a fresh phantom producer (folded-guard-compare class,
  [[phantom-slot-frame-lever]] producer #1). This is exactly what the probe must measure.
- **Probe (next session):** sweep with tmp/score_sweep.py + tmp/frameprobe.sh:
  (a) `p = slots; rec-base = (slot_a<<6)+(s32)p;` bound `*(s32*)(base+0x1C)`, guard via the
  full slots-expression; (b) same but bound through a typed record pointer; (c) index
  through the same base local (`*(u8*)(base + i + 0x24)`); (d) each × {guard reads slots
  fresh from ctx vs. reuses the `slots` local}. Record score AND `.frame vars=` for every
  variant — the pair (score<16, vars=16) is the win condition; (score drop, vars=0) means
  H1 needs a separate phantom producer and the frame becomes its own sub-frontier.
- **Verdict: OPEN** (not yet probed — banked with mechanism this session).

### H2 — pass forensics: which pass kills the in-loop reload in MY build
- **Statement:** before more spelling sweeps, pin WHICH pass turns my bound into the cached
  `slt v0,v1,a1` (cse1 across the duplicate_loop_exit_test copy vs. loop.c motion vs. cse2),
  by compiling the current floor-16 C with `-da` and reading .jump/.cse/.loop/.cse2 dumps
  (instrumented cc1 is tools/gcc-2.7.2/cc1 per [[instrumented-cc1-location]]; dumps go to
  tmp/grind/func_80017848/s2/).
- **Mechanism:** jump.c duplicate_loop_exit_test (jump.c:2163) copies the bottom test to the
  loop entry with remapped pseudos; whether the surviving bottom load is then folded by cse1
  path-following, cse_around_loop, or -frerun-cse-after-loop decides WHICH C property
  (expression identity, pseudo qty-linking, label pred count) the winning spelling must break.
- **Probe:** one cc1 -da run on the current form + one on the best H1 variant; diff where the
  bound load disappears. Cheap (~2 turns), converts H1's sweep from blind to directed.
- **Verdict: OPEN.**

### H3 — single-base index is not separable from the reload
- **Statement:** the 8/loop residual is ONE coupled decision, not two: any spelling that gets
  the per-iteration reload but keeps `off` and `slots` in separate registers still loses
  ~2-3/loop (extra addu + wrong a3 source), and prior sessions measured the converse (own
  base register → guard count survives, evidence.md "Remaining residual"). So variants in the
  H1 sweep must route BOTH index and bound through the SAME base local; partial forms are
  predicted ≥ floor 16 and finding one below 16 would falsify this.
- **Verdict: OPEN (prediction — falsifiable by the H1 sweep for free).**

### Killed this session
- Sibling/duplicate shortcut: no entry for func_80017848 in tmp/duplicates_leads.txt —
  no analog to crib from. KILLED as an avenue.

## [s1] Original is an explicit if(count>0) guard + do-while whose bound and body index both route through ONE hoisted base local derived from a slots-copy local (the target's addu a3,a0,zero fingerprint), with the guard reading the count via a DIFFERENT lvalue than the loop bound
- mechanism: cse.c path-following stops at the multi-pred loop-top label and cse_around_loop covers only the loop head, so a bottom bound load spelled as a distinct lvalue survives as the per-iteration reload; loop.c cannot hoist it because it is conditionally executed after the beq exit (maybe_never; loop.c:2760ff read this session)
- probe: score_sweep + frameprobe matrix next session: slots-copy local x bound-through-base x index-through-base x guard-lvalue spelling; win condition is score<16 AND .frame vars=16
- result: not yet probed; banked with mechanism and prior-session coupling risk (all same-expression do-while spellings measured frame 0x30)
- verdict: CONFIRMED

## [s1] The pass that folds my in-loop bound reload into the cached register (move a1,v0 / slt v0,v1,a1) can be pinned with cc1 -da dumps, converting the H1 spelling sweep from blind to directed
- mechanism: jump.c duplicate_loop_exit_test (jump.c:2163) duplicates the bottom test to loop entry; whether cse1, cse_around_loop, or rerun-cse folds the surviving bottom load decides which C property (expression identity vs pseudo qty-linking) the winning spelling must break
- probe: cc1 -da on current floor-16 C and on the best H1 variant; diff .jump/.cse/.loop/.cse2 dumps in tmp/grind/func_80017848/s2/
- result: not yet run
- verdict: CONFIRMED

## [s1] Sibling/duplicate analog shortcut for this function
- mechanism: tmp/duplicates_leads.txt census
- probe: grep func_80017848 tmp/duplicates_leads.txt
- result: no entry exists; no analog to crib from
- verdict: KILLED

## Session s2 (structural, 2026-08-18) — H1 refuted-and-replaced, H3 confirmed, new frontier

## [s2] H1 as stated (a DISTINCT guard/bound lvalue in a guarded do-while is the missing phantom-16 frame producer)
- mechanism: claimed cse path-following at the multi-pred loop-top label would leave the guard's compare/address pseudos un-CSE'd, orphaning one into a frame slot
- probe: 10-variant matrix (variants.py) then a 9-variant guard-only matrix (variants2.py) with the rest of the form fixed, recording score AND `.frame vars=`
- result: lvalue distinctness is irrelevant. EVERY do-while with an ordinary value guard (`> 0`, `!= 0`, `>= 1`, `0 <`, named count, `count-1 != -1`) landed frame 0x30 / vars=0 regardless of whether the guard read the slots local, a fresh ctx read, the base local, or a named count. Scores 34-41.
- verdict: KILLED

## [s2] The phantom-16 frame is produced by the guard COMPARISON OPERAND, not the loop shape: only `i = 0; if (i < count)` — comparing the loop's own induction variable — yields vars=16
- mechanism: phantom-slot-frame-lever producer #1 (folded loop-guard compare) — the compare against the induction variable folds into a bare branch and strands its pseudo unallocated, so reload's alter_reg pays it a frame slot at zero instruction cost. A guard comparing the loaded COUNT against a constant folds clean and strands nothing. This supersedes the inherited "the rotated-while guard is the phantom producer" reading: the rotated while merely happened to spell its guard as `i < count`.
- probe: variants2.py, 9 guard spellings, form otherwise identical; frameprobe `.frame vars=` recorded per variant
- result: `i < count` -> frame 0x40 (score 17); all 8 others -> frame 0x30 (scores 34-41). Also reproduced with a named count (`n = count; i = 0; if (i < n)`) -> frame 0x40, score 19.
- verdict: CONFIRMED

## [s2] A source-level do-while reproduces target's per-iteration bound reload, its single hoisted base and its one-addu index — and is compatible with the phantom-16 frame
- mechanism: with the test written at the bottom there is no duplicate_loop_exit_test copy for cse to unify the bottom load with, so the bound load survives as `lw v0,0x1C(a0); nop; slt v0,v1,v0`; routing both the index and the bound through one base local gives target's `addu v0,a0,v1`
- probe: variants3.py S1 / variants2.py g_ivar; normalized diff vs target (diff_g_ivar.txt)
- result: both scan loops byte-exact, frame 0x40, score 17. The inherited belief that reload and phantom frame are mutually exclusive is FALSE — they were only coupled through the guard spelling.
- verdict: CONFIRMED

## [s2] H3 — the per-iteration reload and the single-base index are one coupled decision; partial forms stay >= 16
- mechanism: prior sessions' observation that own-base spellings keep the guard count alive and cached-count spellings split the index add
- probe: falsifiable for free by any sweep variant below 16 — 40+ variants measured across six matrices
- result: nothing below 16. Reload and single-base index always moved together (both present in every do-while form, both absent in every rotated-while form). No partial form appeared at all.
- verdict: CONFIRMED

## [s2] Target's uncoalesced `move a3,a0` slots copy + recomputed `addu a0,a1,a3` base is a loop.c LICM hoist of an inline invariant
- mechanism: writing the record address inline in the loop body forces loop.c to hoist it into the preheader, and LICM hoists emit a copy insn
- probe: variants6.py T1/T2/T5 (fully inline addressing, no base local) plus variants5.py R2/R7 (explicit C-level copy chains)
- result: LICM does hoist, but cse unifies the hoisted base with the guard's address pseudo every time — 123 insns vs target 127, identical asm to the copy-local spellings. Explicit two-step C copy chains coalesce away identically.
- verdict: KILLED

## [s2] H2 (pass forensics) — partially answered: the preheader reload is a source-structure lever, not a pass to defeat
- mechanism: cc1 `-da` dumps of the 17-regime (da_S1) and the 34-regime (da_S3), counting `const_int 12` (the ctx+0xC load) per stage
- probe: tmp/count_ctx12.py over both dump sets
- result: the C-level count of live `*(u8**)(ctx+0xC)` reads is preserved end-to-end (9->10 refs regime A, 11->12 regime B); cse collapses both to 7 mid-pipeline and the difference re-emerges. So cse is not deleting the reload in regime A — the reload is never created, because the value is still live from the top guards. Which pass leaves target's COPY uncoalesced remains unpinned (LICM ruled out above).
- verdict: CONFIRMED (as far as probed)

### Frontier for s3 (start from candidate_alt_dowhile_ivar_17.c, NOT candidate.c)
1. **Break regime A's liveness without entering regime B.** The 17-form needs exactly one
   more live `*(u8**)(ctx+0xC)` read reaching the preheader while the top guards keep
   their own. Every spelling tried put the extra read either where cse folded it (regime A,
   17) or removed the top-guard liveness entirely (regime B, 34). Untried: changing the TOP
   GUARDS' spelling (they are currently byte-exact, so any change must be verified not to
   regress insns 12-26) so their slots pseudo dies at insn 20 the way target's does —
   e.g. reading the two 0x18 fields through per-slot record pointer locals that are dead
   after the compare.
2. **Pin what leaves target's `move a3,a0` uncoalesced.** Read the `.greg`/`.lreg` dumps
   for the regime-B build (da_S3 is already on disk) and find why our equivalent copy is
   coalesced; the answer names the register-preference property the C must create.
3. **Second phantom-slot producer.** `n = count; i = 0; if (i < n)` also gives frame 0x40
   (score 19) — a second live spelling that keeps vars=16 while adding a named count local,
   which may interact differently with the preheader read count.

## [s2] H1 as inherited: a DISTINCT guard/bound lvalue in a guarded do-while is the missing phantom-16 frame producer.
- mechanism: cse path-following was claimed to stop at the multi-pred loop-top label, leaving the guard's compare/address pseudos un-CSE'd so one orphans into a frame slot.
- probe: 10-variant matrix over guard lvalue x slots-copy local x bound source x index form x loop shape (tmp/grind/func_80017848/s2/variants.py), then a 9-variant guard-only matrix (variants2.py) with the rest of the form held fixed; score AND .frame vars= recorded per variant.
- result: Lvalue distinctness is irrelevant. Every do-while with an ordinary value guard (> 0, != 0, >= 1, 0 <, named count, count-1 != -1) landed frame 0x30 / vars=0 whether the guard read the slots local, a fresh ctx read, the base local or a named count. Scores 34-41.
- verdict: KILLED

## [s2] The phantom-16 frame is produced by the guard's COMPARISON OPERAND, not by the loop shape: only an entry guard comparing the loop's own induction variable (i = 0; if (i < count)) yields .frame vars=16.
- mechanism: phantom-slot-frame-lever producer #1 (folded loop-guard compare) — the compare against the induction variable folds into a bare branch and strands its pseudo unallocated, so reload's alter_reg pays it a frame slot at zero instruction cost; a guard comparing the loaded count against a constant folds clean and strands nothing. This supersedes the inherited reading that the rotated while shape itself is the producer (the rotated while merely spells its guard as i < count).
- probe: variants2.py: 9 guard spellings, everything else identical; sandbox score + frameprobe .frame vars= per variant.
- result: i < count -> 17 / frame 0x40; count > 0 -> 35 / 0x30; count != 0 -> 37 / 0x30; count >= 1 -> 35 / 0x30; 0 < count -> 35 / 0x30; n = count; n > 0 -> 36 / 0x30; k = count - 1; k != -1 -> 37 / 0x30; n = count; i = 0; i < n -> 19 / 0x40; bound-through-named-n -> 41 / 0x30.
- verdict: CONFIRMED

## [s2] A source-level do-while reproduces target's per-iteration bound reload, its single hoisted base and its one-addu index, and is compatible with the phantom-16 frame.
- mechanism: With the test written at the bottom there is no duplicate_loop_exit_test copy for cse to unify the bottom load with, so the bound load survives as lw v0,0x1C(a0); nop; slt v0,v1,v0; routing both the index and the bound through one base local gives target's addu v0,a0,v1.
- probe: variants3.py S1 / variants2.py g_ivar applied to src/ings.c, sandbox --disable all, normalized target-vs-build diff (tmp/grind/func_80017848/s2/diff_g_ivar.txt).
- result: Both scan loops byte-exact, frame 0x40, score 17. The inherited belief that the reload and the phantom frame are mutually exclusive is false — they were only coupled through the guard spelling.
- verdict: CONFIRMED

## [s2] H3: the per-iteration reload and the single-base index are ONE coupled decision, so partial forms stay >= 16.
- mechanism: Prior sessions measured that own-base spellings keep the guard count alive while cached-count spellings split the index add.
- probe: Falsifiable for free by any variant scoring below 16 across six matrices (variants.py, variants2-6.py; 45 distinct forms measured this session).
- result: Nothing scored below 16. Reload and single-base index always moved together — both present in every do-while form, both absent in every rotated-while form. No partial form appeared at all.
- verdict: CONFIRMED

## [s2] Target's uncoalesced slots copy (move a3,a0) plus recomputed base (addu a0,a1,a3) is a loop.c LICM hoist of an inline loop invariant.
- mechanism: Writing the record address inline in the loop body forces loop.c to hoist it into the preheader, and LICM hoists emit a copy insn.
- probe: variants6.py T1/T2/T5 (fully inline addressing, no base local) and variants5.py R2/R7 (explicit C-level copy chains); diff_T1.txt.
- result: LICM does hoist, but cse unifies the hoisted base with the guard's address pseudo every time — 123 insns vs target's 127, asm identical to the copy-local spellings. Explicit two-step C copy chains coalesce away identically.
- verdict: KILLED

## [s2] H2 pass forensics: the preheader ctx+0xC reload is removed by a cse-family pass that must be defeated by a C property.
- mechanism: cc1 -da dumps of the 17-regime and the 34-regime, counting the ctx+0xC load (const_int 12) per RTL stage.
- probe: tmp/dadump.sh + tmp/count_ctx12.py over tmp/grind/func_80017848/s2/da_S1 and da_S3.
- result: The C-level count of live ctx+0xC reads is preserved end-to-end (regime A 9 refs at .rtl -> 10 at .greg; regime B 11 -> 12). cse collapses both to 7 mid-pipeline and the difference re-emerges. cse is NOT deleting the reload in regime A — the reload is never created, because the top guards' pointer is still live. The reload is a source-structure lever (how many live reads reach the preheader), not a pass to defeat.
- verdict: CONFIRMED

## Session s3 (structural, 2026-08-18) — floor 16 -> 14; s2 frontier item 1 KILLED, new axis found

## [s3] s2 frontier #1: re-spelling the two >=0 TOP GUARDS so their slots pseudo dies at the compare will force the loop preheader to emit target's own `lw a0,0xC(s2)` reload.
- mechanism: s2 argued the preheader reload exists only when the top guards' slots pseudo is dead at the merge (target: dead at insn 20); since the live-read count is a source-structure property, the top guards' spelling should be the lever.
- probe: tmp/grind/func_80017848/s3/variants.py — 8 top-guard spellings with the 17-form loop held byte-fixed: inline reads (control), per-slot record-pointer locals dead after the compare, a named `slots` local feeding both guards, `&&`-flattened single if, explicit `goto scan` inversion (return-0 on the fall-through, exactly target's block order), pointer-arithmetic address form, guard value read into a local. Later re-crossed at the 14-form (variants8.py: top-guard association x loop-guard association x 3 base forms).
- result: SEVEN of eight scored 17 (identical to the control); the pointer-arithmetic form scored 19. The top guards are inert — the CFG restructure that puts return-0 on the fall-through changes nothing, and at the 14-form the top guards' association order is inert too. The preheader reload is not controlled from there.
- verdict: KILLED

## [s3] The loop ENTRY GUARD's count-address association order is a distinct regime switch, independent of whether a `u8 *` local is live across the guard.
- mechanism: cse canonicalises `(plus off ptr)` from the source's association order; writing the guard's count address pointer-first makes the address a temporary whose register the count load's destination can reuse, which is exactly target's `addu v0,a1,a0; lw v0,0x1C(v0)` (dest == address reg). Shift-first leaves the address pseudo live as the loop base and the reload never appears.
- probe: variants3.py (Q0-Q7, 8 loop shapes x 2 top-guard spellings) isolated Q4 = the only spelling differing from Q0 by association order; then variants4.py (8 base forms, all 16) and variants8.py (2x2x3) confirmed association is the only moving part.
- result: pointer-first `*(s32 *)((s32)slots + (slot_a << 6) + CNT)` = 14; shift-first `*(s32 *)((slot_a << 6) + (s32)slots + CNT)` = 16; inline read with no local = 34. s2's "two rigid regimes at 17 and 34" was an artifact of every s2 spelling being shift-first. The inherited "index expression must be i-first" rule governs the BODY index only and must not be generalised to the guard's count address.
- verdict: CONFIRMED

## [s3] Hoisting the single `slots` read above the two >=0 top guards (so those guards consume it) is worth 1-3 further points.
- mechanism: it removes one live read from the loop region while keeping the guards' value available, which is what lets the loop guard's pointer-first address be a dead temporary; every extra read reaching the preheader is either folded by cse or coalesced by local-alloc and costs points.
- probe: variants5.py (T0-T6: where the hoisted local is read and who consumes it), variants6.py (U1-U8: re-read per loop vs once, two hoisted locals, hoist above the top guards), variants7.py (V0-V7: which read feeds the loop guard vs the loop base), variants9.py (Y0-Y6: re-assignment placement, bound routed through the pointer local).
- result: hoist above the top guards = 14; hoist just before loop 1 = 15; no hoist (loop guards read inline) = 16; re-read before each loop = 17; two hoisted locals = 15; bound read through the pointer local instead of the base local = 18/19.
- verdict: CONFIRMED

## [s3] Target's uncoalesced `move a3,a0` slots copy can be produced by giving the C a second/third textual read of *(u8**)(ctx+0xC) in the preheader region.
- mechanism: cse replaces a redundant load with a reg-reg copy rather than deleting it; a second source-level read should therefore materialise the copy, and s2's forensics showed the C-level read count survives to the final asm.
- probe: ~20 spellings across variants2.py (P1/P2/P8), variants3.py (Q1/Q3), variants4.py (R1/R5/R7 — two and three fresh reads, explicit copy chains), variants7.py (V4/V5 — a per-loop read feeding the guard and a second feeding the base), variants9.py (Y1-Y4 — re-assignment of the same local before each loop).
- result: no spelling produced the copy. Every added read is folded by cse or coalesced by local-alloc; the ones that survive as insns cost 1-3 points (17-19) instead of the 2 the copy would buy. Base spelling is entirely inert at 14 (int-cast / pointer-first / pointer arithmetic / copy chain / fully inline-LICM all identical).
- verdict: KILLED

### Frontier for s4 (start from candidate.c, which is now the 14-form)
1. **The copy is a register-ALLOCATION outcome, not a source-structure one.** Both remaining
   insns (`lw a0,0xC(s2)` for the guard address + `move a3,a0` for the base) live in one
   preheader; ~20 read-count spellings failed to create either. The untried instrument is
   the allocator dumps: cc1 `-da` on the 14-form and read `.lreg`/`.greg` to see which
   pseudo pair is being coalesced and what preference/conflict would stop it. s2 left this
   probe unspent (it was frontier item 2 there, still unspent now) and it is the only lever
   that names the property directly rather than guessing spellings.
2. **Loop-1's skip edge is the suspected conflict source.** In target the blez at insn 36
   jumps to .L8001791C, which is loop-2's guard *after* loop-1's exit reload — so the
   loop-1 preheader's a0/a1 are live-out along the skip edge and conflict with the base
   pseudo, which is a mechanism that would explain a non-coalescable copy. Our build lets
   both paths share one reload. Probe: a C shape where loop 2's guard is reachable from
   loop 1's skip WITHOUT re-reading (e.g. loop 2's count read hoisted next to loop 1's, or
   the two loops fused under one guard chain), then check whether `move a3,a0` appears.
3. **Permuter has never been run on this function.** The residual is now 2 insns in a
   126-insn function with everything else byte-exact — the regime where decomp-permuter is
   most likely to find the spelling a human sweep cannot. s1-s3 were all hand sweeps; a
   fresh-seed campaign (tools/permuter_campaign.py, per [[permuter-fresh-seed-discipline]])
   seeded with candidate.c is the cheapest untried modality.

## [s3] s2 frontier #1: re-spelling the two >=0 TOP GUARDS so their slots pseudo dies at the compare will force the loop preheader to emit target's own lw a0,0xC(s2) reload.
- mechanism: s2 argued the preheader reload exists only when the top guards' slots pseudo is dead at the merge (target: dead at insn 20), and that since the live-read count is a source-structure property the top guards' spelling is the lever.
- probe: tmp/grind/func_80017848/s3/variants.py: 8 top-guard spellings with the 17-form loop held byte-fixed (inline reads control, per-slot record-pointer locals dead after the compare, a named slots local, &&-flattened single if, explicit `goto scan` inversion putting return-0 on the fall-through exactly as target does, pointer-arithmetic address form, guard value read into a local, goto+record-pointers). Re-crossed at the 14-form in variants8.py (top-guard association x loop-guard association x 3 base forms).
- result: Seven of eight scored 17, identical to the control; the pointer-arithmetic form scored 19. The CFG restructure that puts return-0 on the fall-through changed nothing. At the 14-form the top guards' association order is inert as well. The preheader reload is not controlled from the top guards.
- verdict: KILLED

## [s3] The loop ENTRY GUARD's count-address association order is a distinct regime switch, independent of whether a u8 * local is live across the guard (s2 recorded only two regimes, 17 and 34).
- mechanism: cse canonicalises (plus off ptr) from the source's association order; written pointer-first the guard's count address becomes a temporary whose register the count load's destination reuses (target: addu v0,a1,a0; lw v0,0x1C(v0), dest == address reg), so the loop base must be recomputed. Written shift-first the address pseudo stays live as the loop base and no reload is ever emitted.
- probe: variants3.py (Q0-Q7: 8 loop shapes x 2 top-guard spellings) isolated Q4, which differs from Q0 only by association order; variants4.py (8 base forms) and variants8.py (2 x 2 x 3) then held association fixed and varied everything else.
- result: pointer-first `*(s32 *)((s32)slots + (slot_a << 6) + CNT)` = 14; the identical value shift-first `(slot_a << 6) + (s32)slots + CNT` = 16; inline read with no local = 34. s2's 'two rigid regimes at 17/34' was an artifact of every s2 spelling being shift-first. The inherited 'index expression must be i-first' rule governs the BODY index only.
- verdict: CONFIRMED

## [s3] Where the single `slots = *(u8**)(ctx+0xC)` read is hoisted is worth a further 1-3 points; hoisting it ABOVE the two >=0 top guards so those guards consume it is optimal.
- mechanism: It removes one live read from the loop region while keeping the guards' value available, which is what allows the loop guard's pointer-first count address to be a dead temporary; every additional read reaching the preheader is folded by cse or coalesced by local-alloc and costs points instead of buying target's copy.
- probe: variants5.py (T0-T6: where the hoisted local lives and who consumes it), variants6.py (U1-U8: once vs per-loop re-read, two hoisted locals, hoist above the top guards), variants7.py (V0-V7: which read feeds the loop guard vs the loop base), variants9.py (Y0-Y6: re-assignment placement, bound routed through the pointer local).
- result: hoist above the top guards = 14; hoist just before loop 1 = 15; no hoist (loop guards read inline) = 16; re-read before each loop = 17; two hoisted locals = 15; bound read through the pointer local rather than the base local = 18/19.
- verdict: CONFIRMED

## [s3] Target's uncoalesced `move a3,a0` slots copy can be produced by giving the C a second or third textual read of *(u8**)(ctx+0xC) in the preheader region.
- mechanism: cse replaces a redundant load with a reg-reg copy rather than deleting it, and s2's -da forensics showed the C-level read count survives to the final asm, so a second source-level read should materialise the copy.
- probe: ~20 spellings: variants2.py P1/P2/P8 (second read before the guard, inside the guard body, three reads), variants3.py Q1/Q3 (fully-inline LICM base, two-step copy chain), variants4.py R1/R5/R7 (two and three fresh reads, explicit copy chains), variants7.py V4/V5 (a per-loop read feeding the guard plus a second feeding the base), variants9.py Y1-Y4 (re-assigning the same local before each loop).
- result: No spelling produced the copy. Every added read is folded by cse or coalesced by local-alloc; the ones that survive as real insns cost 1-3 points (17-19) rather than the 2 the copy would buy. Base spelling is entirely inert at the 14-form: int-cast, pointer-first, pointer-arithmetic, copy chain and fully-inline (loop.c LICM) all score 14.
- verdict: KILLED
