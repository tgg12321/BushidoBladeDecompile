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

## s4 (2026-08-18) — permuter modality

### H-s4-1 — KILLED (and it kills a whole way of running this function)
**Statement:** "The remaining 2-insn gap is in the regime where decomp-permuter outperforms
hand sweeps, and permuter has never been run on this function" (the s3 frontier item).
Permuter has now been run, properly, on a workspace that reproduces the real build exactly.
**Probe:** built + validated a per-function workspace (`s4/mkws.sh`, self-checks at
125 vs 127 insns), launched a telemetered campaign from the s3 candidate
(`permuter_campaign.py launch --dir tmp/perm_ings_s4 -j 10`), waited in-turn, harvested
~25 saved outputs, and re-scored EVERY one with `sandbox --disable all`.
**Result:** best engine distance across all permuter output = **14** = exactly the starting
floor. The permuter's own best find (score 405 vs base 855) is engine **18**.
**Verdict:** KILLED as a *directed* lever. The premise "with 125/127 byte-exact the search
space a permuter explores is now tiny" was wrong in an instructive way: the space is tiny,
but the permuter's compass points out of it (see H-s4-2). A permuter campaign on this
function is a proposal generator whose ranking must be discarded and replaced with the
engine's, and at that point it is strictly weaker than the exhaustive directed sweep in
H-s4-3, which covers the same axes deterministically. Do not spend another session's
modality on an undirected campaign here.

### H-s4-2 — CONFIRMED (the session's most transferable finding)
**Statement:** The permuter's default scorer is anti-correlated with the engine's honest
distance for a function whose residual is a small instruction DELETION near the top of the body.
**Mechanism:** the permuter weights reorderings at 60 and ins/del at 100 and aligns the two
instruction streams globally. Two missing instructions early in the body displace the whole
tail, so the aligner charges ~14 reorderings (855 ≈ 14x60) even though 125 of 127 instructions
are byte-identical. The gradient therefore rewards mutations that re-sync the aligner by
CHANGING the already-correct tail, and punishes staying in the correct basin. The engine's
metric counts differing instructions with branch/jump targets masked and has no displacement
term, so the two metrics disagree by construction, not by accident.
**Probe:** the 20-row permuter-score vs engine-distance table in evidence.md E-s4-2, produced
by `batch_score.ps1` over every saved output of campaign A.
**Verdict:** CONFIRMED. Note the practical corollary: `permuter_campaign.py launch` does not
expose `--keep-prob`, so campaigns always run decomp-permuter's default 0.6 hill-climb, which
is precisely the behaviour that walks away from the basin here. A future session that wants a
usable campaign on a deletion-residual function needs either restart-from-base sampling or
engine-metric rescoring in the loop; the second is what s4 did by hand.

### H-s4-3 — KILLED (exhaustively, by measurement, not by sampling)
**Statement:** Some combination of the loop-preheader spelling axes that s1-s3 only swept
one-at-a-time reaches below 14.
**Mechanism:** the last three levers found on this function (do-while shape, guard comparison
operand, guard association order) were each invisible until someone wrote that exact spelling,
so combinations plausibly hid another.
**Probe:** generated the full symmetric cross-product of the five preheader axes
(guard-count base x preheader-p x base-spelling x body-index base x do-while-bound base =
72 variants, `s4/gen_variants.py`) and engine-scored all 72 in one pass (`s4/sweep.sh`,
raw results in `s4/sweep_results.txt`).
**Result:** min 14, max 19. Twelve cells tie at 14; zero cells go below.
**Verdict:** KILLED. The preheader spelling family is now closed by exhaustion rather than by
sampling. New sub-finding worth carrying: cells `a1_b0_*_{d0,d2}_e1` reach 14 with a FRESH
guard read, so s3's pointer-first/hoisted-guard rule is one sufficient route to 14 and not a
necessary condition — 14 is a plateau over the family, which is the signature of a constraint
living below the C source (register allocation), exactly as the s3 frontier predicted.

### H-s4-4 — CONFIRMED, and it is the floor drop (14 -> 12)
**Statement:** Randomization CONFINED to the residual region, with the permuter used purely as
a proposal generator and the ENGINE used as the objective, can find preheader structure that
neither the hand sweeps (s1-s3, ~105 forms) nor the full spelling cross-product (H-s4-3, 72
cells) could reach.
**Mechanism:** the winning form leaves the spelling family entirely — it introduces a SECOND
live pointer variable (`base = slots;`) that carries the hoisted pointer into the two scan-loop
entry guards, and is then re-pointed at the per-slot record inside each preheader. Every cell
of the H-s4-3 cross-product routed the guards through one of the two pointers that already
existed, so no amount of respelling inside that family could express this. It is the frozen-list
"variable reuse for codegen control" family; `base` is read by both loop guards before it is
reassigned, so it is a live variable, not a dead store or a holder.
**Probe:** chassis B = the validated workspace with the residual region wrapped in
`PERM_RANDOMIZE` (`s4/mk_perm_b.py`); 733 s / 21911 iterations / 130 saved outputs; every
output re-scored with `sandbox --disable all` (`s4/score_outputs.sh`); then a 4-cell cleanup
sweep to find the most readable spelling of the winner.
**Result:** two outputs at engine **12**, both at permuter score 845 — the permuter's own
WORST decile, ten points off base. Cleanup sweep: top guards through `base` = 12, through
`slots` = 12, mixed (as emitted) = 12, loop guards back onto `slots` = **19**. Chosen form
(both top guards through `slots`) re-verified in `src/ings.c` at **12**, 125/127 insns.
**Verdict:** CONFIRMED. Two compounding lessons for the pipeline: (i) scope the randomization
to the residual region so the randomizer cannot damage what is already exact — chassis B found
in 12 minutes what chassis A could not find at all; (ii) the engine metric must be the
objective, applied as a rescoring pass over every saved output. Trusting the permuter's own
ranking would have discarded both 12-cells as near-worst results.

## [s4] The remaining 2-insn gap is in the regime where decomp-permuter outperforms hand sweeps, and an undirected campaign from the s3 candidate will close or narrow it (the s3 frontier item).
- mechanism: s1-s3 were hand-authored spelling matrices (~105 forms) and each of the last three levers was invisible until someone wrote that exact spelling; with 125/127 insns byte-exact the space a permuter explores should be tiny.
- probe: Built and self-validated a per-function permuter workspace mirroring the real build exactly (tools/grind/func_80017848/s4/mkws.sh; prints 'base insns: 125  target: 127'), launched a telemetered campaign from the s3 candidate (permuter_campaign.py launch --dir tmp/perm_ings_s4 -j 10), waited in-turn, harvested ~25 saved outputs, and re-scored EVERY one with sandbox --disable all.
- result: Best engine distance across all campaign-A output = 14, exactly the starting floor. The permuter's own best find (score 405 vs base 855, a nominal 53% improvement) is engine distance 18  -  four instructions WORSE than where it started.
- verdict: KILLED

## [s4] decomp-permuter's default scorer is anti-correlated with the engine's honest distance for a function whose residual is a small instruction DELETION near the top of the body.
- mechanism: The permuter weights reorderings at 60 and ins/del at 100 and aligns the two instruction streams globally. Two missing instructions early in the body displace the whole tail, so the aligner charges ~14 reorderings (855 ~= 14 x 60) even though 125 of 127 instructions are byte-identical. The gradient therefore rewards mutations that re-sync the aligner by CHANGING the already-correct tail. The engine metric counts differing instructions with branch/jump targets masked and has no displacement term, so the two disagree by construction.
- probe: Re-scored every saved permuter output with sandbox --disable all and tabulated permuter score against engine distance (tmp/grind/func_80017848/s4/batch_score.ps1; 20-row table in evidence.md E-s4-2, full 130-row table for chassis B in tmp_perm_ings_s4b_engine_scores.txt).
- result: Ranking inversion is systematic, not noisy: perm 405 -> engine 18, perm 480 -> 16, perm 575 -> 14, perm 775 -> 39, perm 845 -> 12, perm 855 (base tie) -> 14. The two BEST forms found all session sit at permuter score 845, i.e. in the permuter's worst decile.
- verdict: CONFIRMED

## [s4] Some combination of the loop-preheader spelling axes that s1-s3 only swept one-at-a-time reaches below 14.
- mechanism: The last three levers found on this function (do-while shape, guard comparison operand, guard association order) were each invisible until someone wrote that exact spelling, so combinations plausibly hid another.
- probe: Generated the full symmetric cross-product of the five preheader axes (guard-count base x preheader-p x base-spelling x body-index base x do-while-bound base = 72 variants, s4/gen_variants.py) and engine-scored all 72 in one pass (s4/sweep.sh -> sweep_results.txt).
- result: Minimum 14, maximum 19. Twelve distinct cells tie at 14; zero cells go below. New sub-finding: cells a1_b0_*_{d0,d2}_e1 reach 14 with a FRESH guard read, so s3's 'the guard must consume the hoisted slots' is a sufficient route to 14, not a necessary one  -  14 was a broad plateau over the family, not a knife-edge.
- verdict: KILLED

## [s4] Randomization CONFINED to the residual region, with the permuter used purely as a proposal generator and the ENGINE used as the objective, can find preheader structure that neither the hand sweeps nor the full spelling cross-product could reach.
- mechanism: The winning form leaves the spelling family entirely: it introduces a SECOND live pointer variable (`base = slots;`) that carries the hoisted pointer into the two scan-loop entry guards and is then re-pointed at the per-slot record inside each preheader. Every cell of the 72-cross-product routed the guards through one of the two pointers that already existed, so no respelling inside that family could express it. Frozen-list 'variable reuse for codegen control' family  -  `base` is READ by both loop guards before it is reassigned, so it is a live variable, not a dead store or a constant-holder.
- probe: Chassis B = the validated workspace with the residual region wrapped in PERM_RANDOMIZE so the randomizer cannot touch the 125 byte-exact instructions (s4/mk_perm_b.py); 733 s / 21911 iterations / 130 saved outputs; every output re-scored with sandbox --disable all (s4/score_outputs.sh); then a 4-cell cleanup sweep to pick the most readable spelling of the winner.
- result: Two outputs at engine 12 (output-845-1, output-845-3), both at permuter score 845. Cleanup sweep: both top guards through `base` = 12, both through `slots` = 12, mixed as emitted = 12, loop entry guards moved back onto `slots` = 19. Chosen form re-verified in src/ings.c at sandbox distance 12, 125 build insns vs 127 target.
- verdict: CONFIRMED

## s5 frontier (2026-08-18, floor 10)

### KILLED this session
- **H-s5-A** "Hoist the ctx+0xC read above each entry guard so ONE pointer feeds the guard
  address and the loop base (target's preheader order)." KILLED — 14..35 over ~40 cells,
  best 17 (7 worse than 10). cse folds `p + (slot_a<<6)` to one pseudo and the second addu
  plus the copy both vanish; the build drops to 122 insns. See E-s5-3.
- **H-s5-B** "Write target's `move a3,a0` as a C-level intermediate pointer copy." KILLED —
  32 cells, `cp0` and `cp1` score IDENTICALLY in every pair. GCC 2.7.2 coalesces the copy
  unconditionally. See E-s5-4.
- **H-s5-C** "The top-guard pointer must not be a named variable, so it dies at the join and
  the loops re-read." KILLED — inline-expression top guards tie at 10, never beat it. See
  E-s5-5.
- **H-s5-D** (carried from s4 frontier) "A THIRD live pointer variable / a different
  assignment of pointers across the guard sites extends the s4 lever." KILLED as an
  independent lever — the `pv2`/`share`/`q` axes in all three s5 sweeps (a distinct pointer
  variable for loop 2, and a distinct copy variable per loop) are inert at the 10-cells and
  strictly harmful elsewhere (`_q_` cells reach 31-34). The s4 lever's value was the
  guard-vs-top-guard SPLIT, not the pointer count.

### Live frontier for s6
- **H-s5-E. The residual is one property: target computes `ptr + (slot_a<<6)` TWICE per
  scan loop (guard address, then loop base) with the pointer live across the entry-guard
  branch; we compute it once.**
  *Mechanism:* in target the guard address and the loop base are separate pseudos that cse
  never unified, and the pointer pseudo is live-out on the guard's taken edge, so
  local-alloc cannot coalesce the copy that bridges them. Both of the obvious C spellings
  are now measured dead: one shared pointer (H-s5-A) makes cse fold them, and an explicit
  copy (H-s5-B) is coalesced away. What has NOT been tried is forcing the two computations
  to be non-equivalent to cse — e.g. deriving the guard address from a pointer expression
  whose value cse cannot prove equal to the base's (a different base object, a different
  cast chain, or a read of `ctx+0xC` placed so the two live in different extended basic
  blocks).
  *Next probe:* the .lreg/.greg dump read that s1-s4 all banked and none has spent, now
  regenerable for the 10-form in seconds via `tmp/grind/func_80017848/s4/dump.sh`. Grep the
  func_80017848 region of `ings_pp.c.cse` FIRST (does cse fold the two addu's, and at which
  insn?), then `.lreg`/`.greg` for the pseudo pair and its conflict/preference records.
  That distinguishes "cse folded them" from "they were never separate" and names which pass
  to attack. Do NOT open another preheader spelling sweep — three sessions and ~230 cells
  say that family is a plateau.

- **H-s5-F. The 4 top-block differing instructions are downstream of H-s5-E, not an
  independent lever.**
  *Mechanism:* target's second `bltz` has a `nop` delay slot because the first insn of its
  target block is `lw a0,0xC(s2)`; ours has `sll a1,s4,6` there (the pointer is already
  live, so only the shift is needed), and `reorg` steals it into the slot and retargets the
  branch to label+4. Fix H-s5-E and the delay slot should follow for free — so do NOT spend
  a session attacking the delay slot directly.
  *Next probe:* after any H-s5-E movement, re-diff with `s5/dis.sh` and check whether the
  `bltz` delay slot became a `nop` without further work. If it did not, `reorg.c`'s
  `fill_slots_from_thread` non-own-thread path is the named mechanism to read.

- **H-s5-G. Chassis-relative rejection: re-measure cheap per-axis spellings after every
  structural lever lands.**
  *Mechanism:* s3 rejected shift-first guard spelling with a correct measurement on the s3
  chassis; on the s4 chassis the same spelling is worth 2 instructions (E-s5-1). Rejected
  forms in this ledger are conditional on the chassis they were measured against.
  *Next probe:* whenever s6+ lands a new structural lever, re-run `s5/gen.py`-style
  sweeps over the cheap axes (guard operand order, base spelling, guard source, read
  placement) on the NEW chassis before declaring any of them dead. One sweep is ~50 cells
  and one blocking call.


## s5 CORRECTION (supersedes the s5 frontier item H-s5-G and the E-s5-1 claim)

- **H-s5-G is WITHDRAWN.** It asserted that rejected forms are chassis-relative,
  citing shift-first guard spelling as worth 2 instructions on the s4 chassis. That
  measurement was contaminated by an unused local declaration in the sweep template.
  Clean: pointer-first 12, shift-first 13. s3's pointer-first conclusion stands.
- **H-s5-H (new, and the practical one for s6).** Generated-variant sweeps on this
  function must prune the declaration block per cell; a fixed declaration block
  makes dead declarations look like spelling wins and is worth up to 3 instructions
  of false signal here. Use `s5/gen_clean.py` as the template shape, and re-apply
  the banked candidate from a clean tree and re-score it before writing any outcome.
- The three KILLS (H-s5-A hoisted read, H-s5-B explicit copy, H-s5-C top-guard
  variable) and the H-s5-D kill all stand: each was measured as matched pairs or at
  margins far larger than the contamination, with the identical declaration block on
  both sides.
- The live frontier is unchanged and is H-s5-E: spend the cc1 -da dump probe
  (`s4/dump.sh`, then read `ings_pp.c.cse`, then `.lreg`/`.greg`) to find out whether
  cse folds the guard-address addu into the loop-base addu. Floor stays 12.

## [s5] On the s4 `base = slots;` chassis, the two scan-loop entry guards want their count address written SHIFT-FIRST rather than POINTER-FIRST, contradicting s3's banked conclusion.
- mechanism: Shift-first makes GCC emit `addu v0,a1,a0` with the `sll` result in the LEFT operand, which is target's operand order at both scan-loop preheaders; pointer-first emits `addu v0,a0,a1`.
- probe: First measured by the 56-cell s5/gen.py sweep (shift-first cells 10, pointer-first cells 12) and INITIALLY BANKED AS CONFIRMED. Then re-measured cleanly at the end of the session, because re-applying the banked candidate from a `git checkout`d tree scored 13 where the sweep log said 10. s5/gen_clean.py regenerates the four cells {pointer-first, shift-first} x {base spelling} with the declaration block pruned to exactly the variables each cell uses.
- result: KILLED, and the earlier CONFIRMED verdict is WITHDRAWN. The sweep templates (gen.py, gen2.py, gen3.py) emitted a FIXED declaration block including `u8 *q;` (and in gen3 `u8 *r;`) for every cell, so cells that never referenced them carried dead declarations. Clean numbers: pointer-first + no dead local = 12; shift-first + no dead local = 13; shift-first + `u8 *q;` = 10; pointer-first + `u8 *q;` = 12. The dead declaration is worth 3 instructions on the shift-first chassis and 0 on the pointer-first one; the operand order is worth nothing on its own and is a 1-instruction REGRESSION. s3's pointer-first conclusion stands and the honest floor is 12, unchanged. The dead declaration is not shippable (cheat-checklist T1/T2/T6, dead-local family) and is banked as rejected/unused_local_decl_q_contaminates_sweeps.c as an artifact to avoid, not a lever to spend.
- verdict: KILLED

## [s5] Hoisting the `*(u8 **)(ctx + 0xC)` read ABOVE each entry guard, so ONE pointer feeds both the guard's count address and the loop base, reproduces target's preheader order (`lw a0,0xC(s2)` before the guard) and closes the residual.
- mechanism: Target loads the pointer before the guard, uses it for the guard address, keeps it live across the `blez`, copies it (`move a3,a0`) and recomputes the loop base from the copy - so `ptr + (slot_a<<6)` is computed twice. The hypothesis was that a single C-level pointer read placed before the guard would produce that shape.
- probe: Axis G=1 in s5/variants (28 cells) and axis K=1 in s5/variants3 (24 cells), each engine-scored; plus a full instruction-level diff of the best such form (s5/probe3.sh on variants3/k11_c0_cp0_s1.c).
- result: Every cell scores 14-35; the best is 17, i.e. SEVEN instructions worse than the 10-form. The diff explains it: with one C pointer feeding both, cse folds `p + (slot_a<<6)` into a single `addu a0,v1,v0`, the guard address and the loop base become the same pseudo, and BOTH the second addu and the copy disappear - the build falls to 122 insns against target's 127. The form gets target's ORDER right and its instruction COUNT further wrong.
- verdict: KILLED

## [s5] Target's uncoalesced `move a3,a0` can be reproduced by writing the copy explicitly in C as a named intermediate pointer (`q = p; base = q + (slot_a << 6);`).
- mechanism: If GCC kept a C-level pointer copy as a distinct pseudo, the `move` would materialize and the loop base would be recomputed from the copy exactly as in target.
- probe: Axis `cp` in s5/gen3.py: all 32 cells generated in matched pairs (with/without the copy) across both the read-after-guard and read-before-guard chassis, with and without a distinct copy variable per loop; every pair engine-scored via s5/sweep3.sh.
- result: cp0 and cp1 score IDENTICALLY in every single pair (10/10, 17/17, 23/23, 31/31). GCC 2.7.2 coalesces the copy unconditionally, so a C copy statement is codegen-inert here. There is no C-level handle on target's copy via this route.
- verdict: KILLED

## [s5] Target's top-guard pointer dies at the join (it is v1 and is never reused) because no C variable holds it; removing the `slots` variable and spelling the top guards as two inline `*(u8 **)(ctx + 0xC)` reads will make the pointer die and force the loops to re-read it.
- mechanism: A named live variable keeps the value in a register across the join, so the loop preheaders never need their own load; an inline expression consumed only by the two top guards should let cse fold the pair inside the top-guard extended basic block and then let the value die.
- probe: s5/gen2.py generated all 40 cells of the no-`slots`-variable family (top guards inline) across the same G/o/c/pv2/base-seed axes; s5/sweep2.sh engine-scored them (s5/sweep2_results.txt).
- result: Best cells TIE at 10 and none beats it; the 11-cells mirror the 10-cells exactly one operand-order step away. Whether the top-guard pointer is a named variable or an inline expression is codegen-inert - the `base` seed keeps the value live either way.
- verdict: KILLED

## [s5] (Carried from the s4 frontier) A THIRD live pointer variable, or a different assignment of the existing pointers across the four guard sites, extends the s4 two-pointer lever the rest of the way.
- mechanism: s4's lever worked by giving the loop entry guards a pseudo distinct from the one the top guards use, which changed what local-alloc could coalesce; the obvious next notch is routing more of the six sites through more named pointers.
- probe: Covered as the `pv2` axis in s5/gen.py and s5/gen2.py (loop 2 takes its own pointer `q`) and the `share`/`cp` axes in s5/gen3.py (a distinct source pointer AND a distinct copy variable per loop) - 128 engine-scored cells in total across the three sweeps.
- result: Inert at every 10-cell and strictly harmful elsewhere: the `_q_` cells reach 31-34 in sweep 1 and 34 in sweep 2. The s4 lever's value was the guard-vs-top-guard SPLIT, not the number of pointer variables; there is no further notch on this axis.
- verdict: KILLED

## [s5] A directed permuter campaign scoped to the residual region of the 10-form finds a body below distance 10.
- mechanism: The residual is now 10 differing instructions concentrated in the two scan-loop preheaders and the top-guard block; PERM_RANDOMIZE over exactly those regions lets the randomizer explore the residual without disturbing the 125 already-byte-exact instructions.
- probe: Two campaigns on the validated s4 workspace built from the 10-form, launched via tools/permuter_campaign.py with telemetry: chassis C (s4/mk_perm_b.py region - the two scan loops only) and chassis D (new s5/mk_perm_d.py region - widened to include the top-guard block, because 4 of the 10 differing insns live there and every prior chassis froze it). 633 s each, 13447 + 13293 iterations, 88 outputs, every one re-scored with `sandbox --disable all` (s5/score_outputs.sh). A third campaign, chassis E, was run on the structurally different read-before-guard base as the fresh-seed reseed. All campaigns harvested with --stop. Chassis E was built from the read-before-guard form (variants3/k11_c0_cp0_s1.c, 122 insns, engine 17) with the widened region - a structurally different base, per the fresh-seed rule - and ran 31600 iterations to 205 outputs.
- result: Nothing below 10. Distributions - C: 3x10, 1x11, 4x12, 5x13, 5x14, 6x15, 7x16, 3x17, tail to 27; D: 4x10, 7x12, 6x13, 4x14, 6x15, 7x16, 2x17, 2x18, tail to 35. E-s4-2's anti-correlation reproduced exactly: each campaign's best permuter-ranked find (620) is not among the engine-best cells, while chassis D's mid-ranked output-685-1 ties the base at 10 with a structurally different body. 2 of 88 outputs could not be scored (apply_find.py could not locate the function in the permuter's reformatted source) - a bounded, recorded gap. Chassis E: 1x10, 7x11, 1x12, 15x13, 13x14, 15x15, 20x16, 22x17, tail to 121 (4 apply-fails, 1 unscorable); its single best cell TIES 10 and came from permuter score 540, mid-ranking again. Across s4+s5 that is 4 chassis, ~80k iterations and 423 engine-scored outputs with zero finds below the running floor; both of this session's floor drops came from directed hand sweeps, not the randomizer. permuter_campaign.py status reports 0 live campaigns.
- verdict: KILLED

## s6 (2026-08-18) — forensics

### H-s6-1 — CONFIRMED (and it retires the chassis)
**Statement.** The inherited 12-form's `base = slots;` reuse makes loop 2's entry guard
address `ptr + 2*slot_a*64 + 0x20` on the loop-1-ran path, where target addresses
`ptr + slot_a*64 + 0x20`; the form is therefore semantically divergent and cannot reach 0.
**Mechanism.** `base` is re-pointed to `(slot_a << 6) + (s32)p` in loop 1's preheader and
loop 2's guard still reads through it. Target reloads the pointer at `0x80017914` in loop
1's exit tail precisely to avoid that.
**Probe.** `cc1 -da` cse dump (insn 146 consumes reg/v79 set at insn 86) plus the build /
target instruction diff (`s6/build.txt` vs `s6/target.txt`).
**Verdict.** CONFIRMED divergent. Chassis retired; `rejected/base_reuse_loop2_guard_is_semantically_divergent.c`.

### H-s6-2 — CONFIRMED
**Statement.** cse.c's extended-basic-block equivalence table is what collapses the
E-s5-3 "hoisted read" family, and the collapse is defeatable by sourcing the base's addend
from a pseudo that was live before the guard block's join label.
**Mechanism.** The loop preheader is the fall-through successor of the guard block, so it
is in the same EBB; `(plus shift p)` is already in the hash table from the guard-address
insn and cse rewrites the preheader insn to a register copy (hoist dump insn 83). A
pre-join pseudo (`slots`) has no known value in the post-join EBB, so no fold occurs.
**Probe.** `s6/hoist/ings_pp.c.cse` insn 83 vs `s6/G/ings_pp.c.combine` insns 71/86.
**Verdict.** CONFIRMED. Variants D/G reproduce target's pre-guard `lw` and both addus; the
cost is keeping `slots` live (15 vs B's 14).

### H-s6-3 — CONFIRMED (and it corrects E-s5-4's attribution)
**Statement.** No C-level copy statement can materialize target's `addu a3,a0,zero`,
because combine.c deletes it before register allocation — not because the allocator
coalesces it.
**Mechanism.** `try_combine` substitutes a single-use register copy into its use and the
dead copy insn is removed; the copy is `NOTE_INSN_DELETED` already in the `.combine` dump.
**Probe.** Variant G (`q = slots;` on the fold-defeated chassis, i.e. the one regime where
E-s5-4's kill does not apply by construction) — `s6/G/ings_pp.c.combine` note 82; score 15
with 125 build insns, identical to D.
**Verdict.** CONFIRMED. Do not re-propose a copy statement in any spelling.

---

## Live frontier for s7+

1. **The last 2 instructions are the two `addu a3,a0,zero` copies, and they are a
   whole-function live-range property, not a preheader spelling.**
   *Mechanism.* In target the loop-1 pre-guard pointer `a0` is live-out on the entry
   guard's **taken** edge: the `blez` at `0x800178C8` jumps to `.L8001791C`, which consumes
   `a0` as loop 2's guard operand. The preheader therefore cannot overwrite `a0` with the
   loop base, so the allocator emits the copy and then `addu a0,a1,a3`. Every form we have
   built lets the guard pointer die at the branch, so the allocator overwrites it and the
   copy never appears.
   *Next probe.* Build a form where ONE pointer variable is read fresh after each join,
   used by that loop's entry guard, AND still read by the NEXT guard on the skip path — so
   that its live range spans the taken edge. Concretely: a single `p` assigned before loop
   1's guard and re-assigned only in loop 1's exit tail (i.e. after the `do/while`, not
   before loop 2's guard), which is exactly the data flow the target asm shows. Start from
   variant D (`rejected/base_from_pre_join_pseudo_defeats_cse_fold_costs_1.c`), not from B,
   because D already has the pre-guard read and the un-folded addus. Read
   `tmp/grind/func_80017848/s6/G/ings_pp.c.greg` for reg/v77's conflict and preference
   records before writing C, and re-dump with `s6/probe.sh` after each attempt.

2. **Every s3/s4/s5 conclusion that was measured ON the divergent chassis must be treated
   as chassis-relative and re-measured before it is spent.**
   *Mechanism.* The divergence deleted two pointer reloads, which shifted register
   pressure and scheduling for the whole preheader region; rankings measured against a
   125-insn build that was 2 insns short for a semantic reason do not transfer to a
   correct 125-insn build. s5 already learned the same lesson in a weaker form (its
   "shift-first" conclusion was chassis-relative to s4's `base = slots;`).
   *Next probe.* Before re-using ANY banked spelling conclusion, re-run it as a matched
   pair on the current candidate with `s6/score.sh`. Cheap (one blocking call per ~6 cells)
   and it is the only thing that makes an inherited conclusion trustworthy. The kills that
   DO survive untouched are the pass-level ones (H-s6-2, H-s6-3) because they are RTL
   facts, not score comparisons.

3. **The top-block 4-instruction shadow is still downstream, and still should not be
   attacked directly.**
   *Mechanism.* Unchanged from s5: target's second `bltz` has a `nop` delay slot because
   the first instruction of its target block is `lw a0,0xC(s2)`; ours has `sll a1,s4,6`
   there and `reorg`'s `fill_slots_from_thread` steals it, retargeting the branch to
   label+4. Variant D already puts a `lw` first in that block, so this may fall for free.
   *Next probe.* After ANY movement on frontier item 1, re-diff with `s6/dis.sh` and check
   whether the `bltz` delay slot became a `nop` on its own. Only if it did NOT should
   `reorg.c`'s non-own-thread `fill_slots_from_thread` path be read as its own mechanism.

## [s6] The s3/s4/s5 chassis (base = slots; base re-pointed in loop 1's preheader; loop 2's entry guard still reading its count through base) is semantically divergent from the target and therefore cannot produce target's bytes at any distance.
- mechanism: On the path where loop 1's guard passed, `base` holds ptr + slot_a*64, so loop 2's guard addresses ptr + 2*slot_a*64 + 0x20. Target addresses ptr + slot_a*64 + 0x20 on that path: at 0x80017914, in loop 1's exit tail, it does `lw a0,0xC(s2)` then `sll a1,s4,6` before falling into .L8001791C's `addu v0,a1,a0 ; lw v0,0x20(v0)`. Byte-identical code implies identical semantics, so a divergent C body cannot close.
- probe: cc1 -da cse dump sliced to the function (tmp/grind/func_80017848/s6/dump/ings_pp.c.cse): insn 86 sets reg/v79 = (plus reg92 reg/v78) in loop 1's preheader and insn 146, loop 2's guard address, is (set reg111 (plus reg/v79 reg110)) consuming that stale pseudo. Cross-checked against the build-vs-target instruction diff (s6/build.txt vs s6/target.txt): our loop-2 guard is `sll a1,s4,6 ; addu v0,a0,a1 ; lw v0,32(v0)` with a0 = loop 1's base; target's is `lw a0,0xC(s2) ; sll a1,s4,6 ; addu v0,a1,a0 ; lw v0,0x20(v0)`.
- result: Divergence confirmed twice over. The 12 score was 2 instructions of false credit: the two 'missing' insns (125 vs 127) ARE the two pointer reloads the divergence deleted. Seven semantically-correct forms measured; best is 14, and HEAD's untouched form is 16.
- verdict: CONFIRMED

## [s6] cse.c's extended-basic-block equivalence table is what collapses the E-s5-3 'hoisted read' family, and the collapse is defeatable by sourcing the loop base's addend from a pseudo that was live BEFORE the guard block's join label.
- mechanism: The loop preheader is the fall-through successor of the guard block, so it lies in the same extended basic block; (plus shift p) is already in cse's hash table from the guard-address insn, so cse rewrites the preheader insn to a register copy and the second addu never reaches the allocator. A pre-join pseudo has no known value in the post-join EBB (cse's table is reset at a join), so no fold occurs.
- probe: Dumped the E-s5-3 hoisted form (s6/hoist/ings_pp.c.cse): insn 71 `reg93 = reg92 + reg/v78` is the guard address, insn 83 is `(set reg/v79 reg93)` — a copy, not an addu. Then built variants D and G (guard through the fresh post-join read, base through the pre-join `slots` pseudo) and dumped G: s6/G/ings_pp.c.combine shows insn 71 `reg94 = reg/v78 + reg93` and insn 86 `reg/v80 = reg93 + reg/v77` as two independent addsi3_internal insns.
- result: Fold defeated. D/G's build contains BOTH of target's structural features the 12-form was faking: the pre-guard `lw v0,12(s2)` and two separate addus. Cost is keeping `slots` live across both loops, so D/G score 15 vs B's 14.
- verdict: CONFIRMED

## [s6] No C-level copy statement can materialize target's `addu a3,a0,zero`, and s5's attribution of this to register-allocator coalescing (E-s5-4) is wrong — combine.c deletes the copy before allocation.
- mechanism: combine.c's try_combine substitutes a single-use register copy into its one use and the now-dead copy insn is deleted; local-alloc never sees it. E-s5-4 measured only on chassis where cse had already folded the base addu away, so it could not distinguish the two passes.
- probe: Variant G writes `q = slots; base = (u8 *)((slot_a << 6) + (s32)q);` on the fold-defeated chassis — the one regime where E-s5-4's kill does not apply by construction. In s6/G/ings_pp.c.combine the copy's insn is already `(note 82 ... NOTE_INSN_DELETED)`. Scored end-to-end: G = 15 with 125 build insns, byte-for-byte the same shape as D.
- result: Copy statement is completely inert. Two named passes now cover the whole 'make a copy appear' family: cse.c when the two expressions are equal, combine.c when they are not.
- verdict: CONFIRMED

## [s6] Repairing the divergence by giving loop 2's guard its own fresh pointer read is cheap and keeps the 12-form's other levers.
- mechanism: A fresh `base = *(u8 **)(ctx + 0xC);` immediately before loop 2's guard restores target's data flow without touching the do-while shape, the `i = 0; if (i < count)` guard shape, or the hoisted slots read.
- probe: Variant A scored from a clean tree (s6/score.sh, s6/scores.txt).
- result: A = 15, one worse than variant B's 14. Reading both scan-loop guards through the never-re-pointed `slots` (B) is the cheaper repair.
- verdict: KILLED

## s7 (2026-08-18) - forensics

### H-s7-1. CONFIRMED. The loop-1 exit-tail reload is C-expressible and produces target's skip-past-the-reload branch.
Mechanism: one pointer variable `p` read before loop 1's guard and re-read only in loop 1's
exit tail; loop 2's guard reads it too, so on the skip path it consumes the pre-guard value
and GCC lets the `blez` branch into the middle of loop 2's guard block.
Probe: variant J, `s7/v/J.c`, built and disassembled with addresses (`blez v0,1444` skips
`lw a0,12(s2)` at `1440`).  Result: control flow matches target exactly.  Verdict: CONFIRMED.

### H-s7-2. CONFIRMED. The fold-defeat lever's SIGN was chassis-relative; on the corrected chassis it is worth 3 instructions.
Mechanism: with the exit-tail reload in place, the entry guard's address and the loop base
are the same expression in the same cse EBB unless the base's addend is a pseudo that was
live before the join.  Sourcing it from `slots` leaves both `addsi3`s alive.
Probe: J (both through `p`) = 14 / 124 insns; O (base through `slots`) = 11 / 125 insns;
s7/O/ings_pp.c.combine insns 146 and 158.  Verdict: CONFIRMED - floor 14 -> 11.

### H-s7-3. KILLED. The cse fold can be defeated by WHERE the base is written.
Probe: variant M (base assigned inside the do-loop body) and N (no base local at all).
Both build to 124 insns / 14, identical to J.  loop.c hoists, cse2 folds.  Verdict: KILLED.

### H-s7-4. KILLED. An explicit `shift` local carried across loop 1's guard edge buys the skipped `sll`.
Probe: K (J chassis) = 16, X (O chassis) = 17, against O = 11.  Verdict: KILLED.

### H-s7-5. KILLED. The preheader's lw/addu order is source-controllable.
Probe: Y1 (explicit `links` local read before the base) = 15; Y2 (after) = 11 = O.
Verdict: KILLED - scheduling artifact.

### H-s7-6. KILLED. The guard/base roles are interchangeable.
Probe: V (guards through the pre-join `slots`, bases through the fresh `p`) = 16; W (only
loop 2's base through `p`) = 14; S (only loop 1's base through `slots`) = 14.  The guard
must consume the fresh read and the base must consume the pre-join read, in that assignment
and in BOTH loops.  Verdict: KILLED (asymmetric and swapped assignments).

### FRONTIER FOR s8
1. The residual is exactly one dead register copy per scan loop (`addu a3,a0,zero`,
   read once by the next `addu` and never again).  s6 closed both C-level routes to a copy
   by name (cse.c folds it when the expressions are equal, combine.c's try_combine
   propagates and deletes it when they are not), so the copy has to come from a pass.  The
   forensics question for s8 is narrow and answerable: WHICH GCC 2.7.2 pass emits a
   redundant reg-reg move into a loop preheader?  Read `tools/gcc-2.7.2/loop.c`
   `move_movables` (the `m->move_insn` path emits `(set new old)` at the loop start) and
   `local-alloc.c` `block_alloc`/`combine_regs` (the tie-the-output-to-a-dying-input path)
   against `s7/O/ings_pp.c.loop`, `.lreg` and `.greg`, and ask what source shape makes the
   base's addend NOT die at the base's own insn.  In O it dies there
   (`REG_DEAD reg/v77` on insn 158).
2. Register naming: ours keeps `slots` live (slots=a2, links=a1); target lets its top-guard
   `slots` read die after the two `>=0` guards (slots=v1) and uses links=a2.  If the copy in
   frontier item 1 is solved by giving the base's addend its own short live range, the
   naming should follow for free - do not attack it separately first.
3. Do NOT re-open: source-level copy statements (s6, pass-level kill), base placement
   (H-s7-3), an explicit `shift` local (H-s7-4), explicit `links` locals (H-s7-5), swapped
   guard/base roles (H-s7-6).  And do not re-measure a banked spelling conclusion without
   re-running it on the CURRENT chassis first - s7 is the second session in a row where a
   banked verdict flipped sign after the chassis changed.

## [s7] The pointer reload that target executes between the two scan loops is a SOURCE-LEVEL reload in loop 1's exit tail, and writing it there reproduces target's control flow, where loop 1's entry-guard branch skips the reload and lands in the middle of loop 2's guard block.
- mechanism: Target's blez at 0x800178C8 targets .L8001791C = instruction index 53, skipping both `lw a0,0xC(s2)` and `sll a1,s4,6` (indices 51/52) and landing on `addu v0,a1,a0`, the second insn of loop 2's guard. That is only expressible if ONE pointer variable feeds both entry guards and is re-read only in loop 1's exit tail (after the do/while, inside the if): on the skip path loop 2's guard consumes the pre-guard value, on the loop-ran path the reload. Semantically the reload is a no-op because the scan loops contain no stores, so this is plain equivalent C.
- probe: Variant J (tmp/grind/func_80017848/s7/v/J.c) built and disassembled with addresses: `blez v0,1444` where 1440 is `lw a0,12(s2)` and 1444 is the `sll` - the reload is skipped on the guard's taken edge, exactly target's shape.
- result: Control flow matches target; J itself scores 14 at 124 build insns (the preheader is wrong for a separate reason, see H-s7-2).
- verdict: CONFIRMED

## [s7] s6's cse-fold-defeat lever (source the loop base's addend from the PRE-JOIN `slots` read rather than from the post-join fresh read) was banked with the wrong sign: it cost 1 instruction on s6's chassis but is worth 3 on the chassis that carries the exit-tail reload.
- mechanism: With the exit-tail reload in place, the entry guard's address and the loop base are both `(plus shift p)` and sit in the SAME cse extended basic block (the preheader is the fall-through successor of the guard block), so cse.c rewrites the base into a copy of the guard address in loop 1 and reuses the guard's address register outright as the base in loop 2 - 124 insns. Making the base's addend a pseudo that was live BEFORE the join (`slots`) leaves the two addends un-equatable and both addsi3s survive.
- probe: J (guards AND bases through `p`) vs O (guards through `p`, bases through `slots`), both scored from a clean tree with s7/score.sh; plus the RTL at tmp/grind/func_80017848/s7/O/ings_pp.c.combine, insn 146 `reg111 = reg110 + reg/v78` (guard) and insn 158 `reg/v79 = reg110 + reg/v77` (base) as two independent addsi3_internal insns.
- result: J = 14 at 124 insns; O = 11 at 125 insns. O re-verified end to end by re-applying memory/grind/func_80017848/candidate.c to a git-checkout'd src/ings.c and re-scoring: 11.
- verdict: CONFIRMED

## [s7] The cse fold can be defeated by WHERE the base assignment is written (inside the loop body instead of the preheader) rather than by the addend's live range.
- mechanism: If the base is written inside the do-loop body, cse's first pass sees it in a different extended basic block from the guard (the loop top is a join), so the equivalence table should be empty for that expression.
- probe: Variant M (base assigned inside both do-loop bodies) and variant N (no base local at all, the full expression inlined in the loop), scored from a clean tree.
- result: M builds byte-identically to J (124 insns, 14) and N is also 14: loop.c hoists the invariant into the preheader and the second cse pass folds it exactly as cse1 would have. Placement is irrelevant; only the addend's live range matters.
- verdict: KILLED

## [s7] An explicit `shift` local assigned before loop 1's guard and recomputed in loop 1's exit tail buys the `sll` that target also skips on the guard's taken edge.
- mechanism: Target's skip edge bypasses both `lw a0,0xC(s2)` and `sll a1,s4,6`, so the shift must be live across that edge and recomputed on the loop-ran path - the same shape as the pointer.
- probe: Variant K (that shift local on the J chassis) and variant X (the same on the O chassis), scored from a clean tree against O = 11.
- result: K = 16, X = 17. The extra live pseudo costs far more than the one `sll` it saves; letting GCC recompute the shift at the join is correct.
- verdict: KILLED

## [s7] The preheader's instruction ORDER (target: copy / lw links / addu base; ours: addu base / lw links) is controllable from the source by hoisting the links read into an explicit per-loop local.
- mechanism: If the links pointer is read into a named local placed ahead of the base assignment, the RTL order should follow the source order into the preheader.
- probe: Variant Y1 (links local read BEFORE the base) and Y2 (AFTER), scored from a clean tree.
- result: Y1 = 15, Y2 = 11 (identical to O without the local). The order is a scheduling artifact and the local is inert at best.
- verdict: KILLED

## [s7] The guard / base roles are interchangeable - it does not matter which of the two pointer reads (pre-join `slots` vs fresh post-join `p`) feeds the entry guard and which feeds the loop base.
- mechanism: Both reads load the same memory and the same value, so either assignment should defeat the cse fold equally.
- probe: Variant V (guards through `slots`, bases through `p`), W (only loop 2's base through `p`), S (only loop 1's base through `slots`), scored from a clean tree against O = 11.
- result: V = 16, W = 14, S = 14. The guard must consume the FRESH read (it is what target's cross-jump edge carries into loop 2) and the base must consume the PRE-JOIN read, in that assignment and in BOTH loops.
- verdict: KILLED

## [s8] s7's "the base's addend must be the PRE-JOIN `slots` read" conclusion is chassis-relative and inverts on the s7 chassis itself.
- mechanism: s7 measured the addend routing on the s6 chassis and banked "guard = fresh read, base = `slots`" (variant O = 11) as settled. But the fold cse actually performs depends on which BASIC BLOCK the base add sits in relative to the guard's address expression, not on which C variable supplies the addend. Loop 2's preheader is a cse JOIN - target's loop-1 `blez` branches to .L8001791C, which sits AFTER both `lw a0,0xC(s2)` and `sll a1,s4,6` - so cse's hash table is reset there and a redundant read in loop 2's base block survives as a real `lw` instead of being folded into the guard's value.
- probe: B_basefresh - variant O with `base = (u8 *)((slot_a << 6) + (s32) * (u8 **)(ctx + 0xC));` in BOTH loops instead of `+ (s32)slots`. Scored from a clean tree with s8/score.sh, then disassembled with s8/dis2.sh.
- result: 11 -> 9. Loop 2's preheader becomes instruction-count-exact with target (9 insns: lw/sll/addu/lw/blez/addu/lw/lw/addu). Loop 1 still folds (7 insns vs target's 9... 8 vs 9 after the branch). Confirmed by the disassembly diff, not just the score.
- verdict: CONFIRMED (and s7's opposite conclusion is retired)

## [s8] Reusing ONE local for both a scan loop's guard ADDRESS and the count it loads defeats cse's fold of the loop base against the guard address.
- mechanism: cse.c hashes the guard's address expression `(plus shift p)` and records the pseudo that holds it. Writing the loaded count back into that SAME pseudo makes cse_insn invalidate the hash entry on the SET_DEST. When the loop base later recomputes `(plus shift p)` in the preheader, cse has no available register for it and must emit a real `addu`. Without the reuse the base collapses to `addu a0,a1,zero` - a copy of the still-live guard address - and the preheader is one instruction short of target.
- probe: Q_treuse_l1 - loop 1's guard rewritten as `t = sh + (s32)p; t = *(s32 *)(t + 0x1C); if (i < t)`, on the B_basefresh chassis. Also P_treuse_both (the same on both loops), R_treuse_l2 (loop 2 only), S_treuse_O (the same on the s7 O chassis).
- result: Q = 6 (9 -> 6, and loop 1's base is now genuinely recomputed as `addu a0,a1,a0`). P = 32, R = 9, S = 37. The lever is LOOP-1-ONLY and chassis-specific: on loop 2 it costs 26 points, on the s7 chassis 31.
- verdict: CONFIRMED (loop 1) / KILLED (loop 2, and on the O chassis)

## [s8] s7's kill of the explicit `slot_a << 6` shift local is chassis-relative and reverses on the Q chassis.
- mechanism: s7 measured K = 16 and X = 17 and banked "the extra live pseudo costs far more than the one `sll` it saves". With the guard-address/count reuse in place, loop 1's register pressure is different: our links read was landing in a1 (target: a2) because a1 was free after `addu a0,a1,a0`. A shared `sh` local keeps the shift live into loop 2's guard, which pushes the links read to a2 and fixes the dependent `addu v0,v0,a2` in loop 1's body.
- probe: Z1_shiftlocal - `sh = slot_a << 6;` assigned once before loop 1's guard and used by both loops' guards and bases, on the Q chassis. Plus Z4 (loop 1 only), Z5 (reassigned before loop 2), Z7 (two shift locals).
- result: Z1 = 5 (6 -> 5). Z4 = 6, Z5 = 6, Z7 = 11. The local must be assigned ONCE and SHARED by both loops; every other spelling of it loses the point.
- verdict: CONFIRMED (and s7's opposite conclusion is retired)

## [s8] A source-level copy statement for target's `addu a3,a0,zero` is still dead on the Q chassis (s6's pass-level kill survives two chassis changes).
- mechanism: s6 proved at RTL level that `q = p; base = shift + q;` is deleted before local-alloc (cse folds it when the expressions are equal, combine's try_combine propagates and deletes it when they are not). The frontier flagged that every s3-s6 spelling conclusion is chassis-relative, so the kill was worth one re-measurement now that the chassis has changed twice more.
- probe: W1_qcopy (the copy feeding both loops' bases on the Q chassis) and W2_qcopy_t2 (the same plus a loop-2 guard reuse).
- result: W1 = 11 (5 points WORSE than Q = 6), W2 = 34. The copy is not merely inert, it actively perturbs allocation. s6's kill is re-confirmed on the current chassis.
- verdict: KILLED (re-confirmed)

## [s8] Placing the redundant `ctx+0xC` read in the GUARD block (before the branch) puts the cse-created copy in a different basic block from the base add, where combine.c cannot reach across to propagate and delete it.
- mechanism: combine.c only combines insns within one basic block. If cse folds a redundant load in the guard block into `(set q p)` and the base add lives in the post-branch preheader block, try_combine cannot substitute q -> p, so the copy should survive as target's `addu a3,a0,zero`.
- probe: ZA_readinguard (both loops), ZB_readinguard_l1 (loop 1 only), ZC_readinguard_l2 (loop 2 only), on the Z1 chassis.
- result: ZB = 5 with a disassembly BYTE-IDENTICAL to Z1 - the read is folded away entirely, no copy is ever created, so there is nothing for combine to fail to delete. ZA = 16, ZC = 8. The block-boundary reasoning is sound but unreachable from this source position.
- verdict: KILLED

## [s8] loop.c is the pass that emits target's dead preheader copy.
- mechanism: The frontier's next probe named `move_movables` as the suspect. loop.c has exactly two preheader-copy emitters.
- probe: Read tools/gcc-2.7.2/loop.c: the `m->move_insn` path (lines 653-672, 1673-1712) and the `m->partial && m->match` path (lines 1290-1336, 1639-1662).
- result: `m->move_insn` is set ONLY when the insn carries a REG_EQUIV note or a REG_EQUAL note with a CONSTANT operand (or a REG_RETVAL libcall block). Our addend is a plain memory load - none of those apply. The `m->partial && m->match` path is the zero/sign-extension movable combiner: it requires a `reg = 0` movable whose NEXT insn sets a SUBREG of that reg, which is not our shape. loop.c is eliminated as the emitter.
- verdict: KILLED

## [s8] Structurally different loop shapes (variable reuse of the pointer as the base, walking-pointer body, for-loop guard, count-into-local guard) reach target's preheader.
- mechanism: The rederive mandate: produce a shape that is not a tweak of the inherited one.
- probe: A_preuse (`p` reused as the base in both loops), D_walkptr (`rec_a` walked through the byte array instead of `base + i`), E_countlocal (the guard count hoisted into a local BEFORE the `if`), U8_l1_condfresh (the do/while condition through the fresh read expression), Y3_condassign (the do/while condition assigning into the count local), C_twobases (per-loop base locals).
- result: A = 14, D = 45, E = 32, U8 = 21, Y3 = 29, C = 11 (identical to O). All KILLED except C, which is a pure renaming with no effect. The winning shapes were NOT loop-shape changes but the read-routing and variable-reuse levers above.
- verdict: KILLED

## [s8] s7's banked conclusion 'the loop base's addend must be the PRE-JOIN `slots` read, in BOTH loops' is chassis-relative and inverts: a per-loop FRESH read of ctx+0xC for the base addend is worth 2 points on the s7 chassis itself.
- mechanism: The fold cse performs depends on which BASIC BLOCK the base add sits in relative to the guard's address expression, not on which C variable supplies the addend. Loop 2's preheader is a cse JOIN (target's loop-1 `blez` branches to .L8001791C, which sits AFTER both `lw a0,0xC(s2)` and `sll a1,s4,6`), so cse's hash table is reset there and a redundant read in loop 2's base block survives as a real `lw` instead of being folded into the guard's value. With `slots` as the addend the whole preheader was 2 insns short.
- probe: B_basefresh - variant O with `base = (u8 *)((slot_a << 6) + (s32) * (u8 **)(ctx + 0xC));` in both loops instead of `+ (s32)slots`. Scored from a clean tree with tmp/grind/func_80017848/s8/score.sh, then disassembled with s8/dis2.sh to confirm the mechanism rather than just the number.
- result: 11 -> 9. Loop 2's preheader becomes instruction-count-exact with target (9 insns). Loop 1 still folds its base into a copy of the guard address and stays 1 insn short.
- verdict: CONFIRMED

## [s8] Reusing ONE local for both loop 1's guard ADDRESS and the count it loads through that address defeats cse's fold of the loop base against the guard address.
- mechanism: cse.c hashes the guard's address expression `(plus shift p)` and records the pseudo holding it. Writing the loaded count back into that SAME pseudo makes cse_insn invalidate the hash entry on the SET_DEST, so when the loop base later recomputes `(plus shift p)` in the preheader cse has no available register for it and must emit a real `addu`. Without the reuse the base collapses to `addu a0,a1,zero` - a copy of the still-live guard address.
- probe: Q_treuse_l1 - loop 1's guard rewritten as `t = sh + (s32)p; t = *(s32 *)(t + 0x1C); if (i < t)` on the B_basefresh chassis. Controls: P_treuse_both (both loops), R_treuse_l2 (loop 2 only), S_treuse_O (the same lever on the s7 O chassis).
- result: Q = 6 (9 -> 6), and the disassembly confirms loop 1's base is now genuinely recomputed as `addu a0,a1,a0`. Controls: P = 32, R = 9, S = 37 - the lever is loop-1-only and chassis-specific.
- verdict: CONFIRMED

## [s8] s7's kill of the explicit `slot_a << 6` shift local (K = 16, X = 17) is chassis-relative and reverses on the s8 chassis.
- mechanism: With the guard-address/count reuse in place, loop 1's register pressure differs: our links read was landing in a1 (target: a2) because a1 fell free after `addu a0,a1,a0`. A single shared `sh` local keeps the shift live into loop 2's guard, pushing the links read to a2 and fixing the dependent `addu v0,v0,a2` in loop 1's body.
- probe: Z1_shiftlocal - `sh = slot_a << 6;` assigned once before loop 1's guard and used by both loops' guards and bases, on the Q chassis. Controls: Z4 (loop 1 only), Z5 (reassigned before loop 2), Z7 (two shift locals).
- result: Z1 = 5 (6 -> 5). Z4 = 6, Z5 = 6, Z7 = 11. The local must be assigned ONCE and SHARED by both loops; every other spelling loses the point.
- verdict: CONFIRMED

## [s8] A source-level copy statement for target's `addu a3,a0,zero` might survive on the new chassis even though s6 killed it at pass level.
- mechanism: s6 proved at RTL level that `q = p; base = shift + q;` is deleted before local-alloc (cse folds it when the expressions are equal; combine's try_combine propagates and deletes it when they are not). Since the frontier flags every pre-s7 spelling conclusion as chassis-relative and the chassis has now changed twice more, the kill was worth exactly one re-measurement.
- probe: W1_qcopy (the copy feeding both loops' bases on the Q chassis) and W2_qcopy_t2 (the same plus a loop-2 guard reuse), scored from a clean tree.
- result: W1 = 11, five points WORSE than its base Q = 6. W2 = 34. The copy is not merely inert - it actively perturbs allocation. s6's pass-level kill is re-confirmed on the current chassis and needs no further re-testing.
- verdict: KILLED

## [s8] Placing the redundant ctx+0xC read in the GUARD block (before the branch) puts the cse-created copy in a different basic block from the base add, where combine.c cannot reach across to propagate and delete it, so target's `addu a3,a0,zero` survives.
- mechanism: combine.c only combines insns within one basic block. If cse folds a redundant load in the guard block into `(set q p)` and the base add lives in the post-branch preheader block, try_combine cannot substitute q -> p and the copy would have to survive.
- probe: ZA_readinguard (both loops), ZB_readinguard_l1 (loop 1 only), ZC_readinguard_l2 (loop 2 only), on the Z1 chassis, each scored and ZB disassembled.
- result: ZB = 5 with a disassembly BYTE-IDENTICAL to Z1 - the extra read is folded away entirely, so no copy is ever created and there is nothing for combine to fail to delete. ZA = 16, ZC = 8. The block-boundary reasoning is sound but unreachable from this source position.
- verdict: KILLED

## [s8] loop.c is the pass that emits target's dead preheader copy (the frontier's named next probe).
- mechanism: loop.c has exactly two preheader-copy emitters: `move_movables`'s `m->move_insn` path, and the `m->partial && m->match` movable-matching path.
- probe: Read tools/gcc-2.7.2/loop.c:653-672 and :1673-1712 (the move_insn flag and its emission), and :1290-1336 and :1639-1662 (the partial/match path).
- result: `m->move_insn` is set ONLY when the movable's insn carries a REG_EQUIV note, or a REG_EQUAL note whose operand is CONSTANT_P, or a REG_RETVAL libcall block - our addend is a plain memory load and qualifies for none. The `m->partial && m->match` path is the zero/sign-extension movable combiner and requires a `reg = 0` movable whose NEXT insn sets a SUBREG of that reg - not our shape. loop.c is eliminated; the emitter must be a pass AFTER cse2 that combine does not undo.
- verdict: KILLED

## [s8] A structurally different loop shape (the mandated rederive axis) reaches target's preheader where read-routing tweaks could not.
- mechanism: Fresh shapes rather than tweaks: reuse the pointer variable itself as the base, walk a pointer through the byte array instead of indexing `base + i`, hoist the guard count into a local before the `if`, run the do/while condition through the fresh-read expression or assign into the count local, give each loop its own base local.
- probe: A_preuse, D_walkptr, E_countlocal, U8_l1_condfresh, Y3_condassign, C_twobases - all scored from a clean tree against the then-current best.
- result: A = 14, D = 45, E = 32, U8 = 21, Y3 = 29, C = 11 (a pure rename with no effect). Every loop-shape change loses. The session's three wins were read-routing and variable-reuse levers inside the existing shape, not loop shape.
- verdict: KILLED

## Session 9 (2026-08-18, rederive) — floor 5 -> 3

### H-s9-1  CONFIRMED — the dead preheader copy survives COMBINE; it is not created after it
Statement: target's `addu a3,a0,zero` / `addu a0,a1,a3` preheader pair is a copy that
existed at combine time and that combine.c was FORBIDDEN to delete, not an artifact
manufactured by local-alloc / global-alloc / reload as s6, s7 and s8 concluded.
Mechanism: `can_combine_p` refuses to combine I2 into I3 when I2's destination is used
after I3. cse.c folds a redundant `*(u8 **)(ctx + 0xC)` read in the preheader into a
plain reg-reg copy of the guard's pointer; give that copy's destination a second use
downstream of the base add and combine cannot substitute it away. The addend then loses
its REG_DEAD note at the base add, so local-alloc's tie-output-to-a-dying-input path
does not fire and the base stops collapsing into the addend's own hard register.
Probe: variant R2_q_live_past_base — `q = *(u8**)(ctx+0xC); base = (u8*)(sh + (s32)q);
... ; p = q;` in loop 1, scored with the engine and disassembled.
Result: loop 1's preheader became byte-exact with target for the first time in nine
sessions (both instructions, both register numbers). Score 5 -> 6 in isolation (the tail
`p = q` costs 2), then 6 -> 4 -> 3 once E-s9-3 and E-s9-2's `sh2` were stacked.
Verdict: CONFIRMED.

### H-s9-2  KILLED — s8's frontier prediction that the shift should be shared by the two GUARDS and recomputed for loop 2's BASE
Statement: decoupling the shared `sh` so both guards use it while loop 2's base
recomputes `(slot_a << 6)` inline would recover target's `sll a1,s4,6` without losing
loop 1's links register.
Mechanism: the two effects were believed coupled only because one local fed all four
sites.
Probe: the complete 16-cell sweep of {loop1 guard, loop1 base, loop2 guard, loop2 base}
x {sh, inline} — s9/v/M0000.c .. M1111.c, one batched score.sh call.
Result: the score depends ONLY on loop 2's pair; loop 1's two sites are exactly neutral
in all four combinations. (L2G,L2B) = (1,0) — the predicted cell — is 31, the worst
value in the space. (1,1) = 5, (0,0) = (0,1) = 6.
Verdict: KILLED. The correct spelling is a SECOND shift local `sh2` covering both of
loop 2's sites (V1 = 3); guard-only = 5 and base-only = 7.

### H-s9-3  KILLED — a source-level `r = p;` copy can give loop 2 the same copy loop 1 now has
Statement: writing the copy explicitly in C, with a second use, should reproduce for
loop 2 what the cse-folded redundant load reproduces for loop 1.
Mechanism: same can_combine_p survival condition as H-s9-1.
Probe: W1 (copy before loop 2's guard, guard+base both read r), W2 (copy in the
preheader, `p = r` tail), W3 (copy in the preheader, no second use).
Result: 8 / 8 / 8 — all five points worse than V1's 3. cse registers p and r as
equivalent and flow.c dead-store-eliminates the `p = r` tail before combine runs, so r
is back to a single use and the copy is deleted exactly as before.
Verdict: KILLED. The copy must be MANUFACTURED BY cse FROM A REDUNDANT LOAD, and its
second use must feed a variable that is genuinely live afterwards.

### H-s9-4  KILLED — loop 2's base addend can get its required second use from the function tail
Statement: routing one of the tail's three ctx+0xC reads through the loop-2 base addend
local would supply the missing downstream use.
Mechanism: same as H-s9-1; any live use after loop 2's base add suffices.
Probe: T1 / T3 / T4 (math_Distance3D's two arguments taken from the live local),
plus T2 / T5 (base addend = `p`, tail left as fresh reads).
Result: 22 / 22 / 22 and 8 / 8, versus V1's 3. Target emits `lw a1,12(s2)` for
math_Distance3D and `lw v0,12(s2)` twice more for rec_a / rec_b (s9/T.txt:77, 94, 104);
every one of those disappears when a live local is substituted, and the register
pressure change cascades through the whole tail.
Verdict: KILLED. Loop 2 has no post-loop second-use site.

### H-s9-5  KILLED — loop 2's base can borrow loop 1's already-live `q` if `q` is pre-initialised before loop 1's guard
Statement: pre-initialising `q` makes it legal on the loop-1-skipped path, so loop 2's
base could read it and inherit loop 1's liveness.
Mechanism: *(ctx+0xC) is never written by this function, so all reads of it are the
same value and the substitution is semantically sound.
Probe: X1 (pre-init by copy), X3 (pre-init by its own read), X2 / X4 (same, with the
loop-1 tail restored to a fresh read).
Result: 14 / 14 / 13 / 13 versus V1's 3.
Verdict: KILLED.

### H-s9-6  CONFIRMED — the links pointer wants its own local, positioned between the addend read and the base assignment
Statement: an explicit `lnk = *(u8 **)(ctx + 0x10);` local fixes `lw a2,16(s2)`'s
position relative to the copy/base pair.
Probe: S1 (lnk read after the `q` read, before `base = ...`) vs S2 (lnk read first).
Result: S1 = 4, S2 = 6. Purely positional; the whole 2-point gain is in the ordering.
Verdict: CONFIRMED. Note s7 had killed the links local outright (X1/X2/X3 = 12/10/7 on
the s8 chassis) — the fourth banked chassis inversion on this function.

### H-s9-7  CONFIRMED — symmetric per-loop locals are neutral, shared locals are catastrophic
Statement: s8's "t-reuse costs 26 on loop 2" was a property of SHARING the `t` local,
not of the lever.
Probe: P1 (loop 2 t-reuse through the same `t`) vs P2 (loop 2 t-reuse through its own
`t2`); W4 (full loop-2 mirror of loop 1's preheader on the V1 chassis).
Result: P1 = 31, P2 = 5 (exactly neutral), W4 = 3 (exactly neutral).
Verdict: CONFIRMED — every "loop 2 mirror costs N" entry in the s8 bank must be re-read
as "SHARED-LOCAL loop 2 mirror costs N".

## [s9] Target's `addu a3,a0,zero` / `addu a0,a1,a3` preheader pair is a copy that existed at combine time and that combine.c was forbidden to delete, not an artifact manufactured by local-alloc / global-alloc / reload as s6, s7 and s8 concluded.
- mechanism: combine.c's can_combine_p refuses to combine I2 into I3 when I2's destination register is still used after I3. cse.c folds a redundant `*(u8 **)(ctx + 0xC)` read in the preheader into a plain reg-reg copy of the guard's pointer; giving that copy's destination a second use downstream of the base add makes combine unable to substitute it away. The addend then loses its REG_DEAD note at the base add (visible as `(expr_list:REG_DEAD (reg/v:SI 78))` on insn 85 of the .lreg dump), so local-alloc's block_alloc/combine_regs tie-output-to-a-dying-input path does not fire and the base stops collapsing into the addend's own hard register.
- probe: Variant R2_q_live_past_base: `q = *(u8**)(ctx+0xC); base = (u8*)(sh + (s32)q); ... ; p = q;` in loop 1, scored with engine sandbox --disable all and disassembled against the normalised target.
- result: Loop 1's preheader became byte-exact with target (both instructions, both register numbers - `addu a3,a0,zero` then `addu a0,a1,a3`). Score 5 -> 6 in isolation because the tail `p = q` costs 2, then 6 -> 4 -> 3 once the links local and the second shift local were stacked.
- verdict: CONFIRMED

## [s9] s8's frontier prediction that the shift should be shared by the two GUARDS and recomputed for loop 2's BASE recovers target's `sll a1,s4,6` without losing loop 1's links register.
- mechanism: The two effects were believed coupled only because one `sh` local fed all four sites (loop1 guard, loop1 base, loop2 guard, loop2 base).
- probe: The complete 16-cell sweep of those four sites x {shared `sh`, inline `(slot_a << 6)`} - variants M0000..M1111, one batched score.sh call.
- result: Score depends ONLY on loop 2's pair; loop 1's two sites are exactly codegen-neutral in all four combinations. The predicted cell (L2G=sh, L2B=inline) is 31 - the worst value in the whole space. (1,1)=5, (0,0)=6, (0,1)=6.
- verdict: KILLED

## [s9] A source-level `r = p;` copy can give loop 2 the same surviving copy loop 1 now has.
- mechanism: Same can_combine_p survival condition as the confirmed hypothesis above.
- probe: W1 (copy before loop 2's guard, guard+base both read r), W2 (copy in the preheader with a `p = r` tail), W3 (copy in the preheader, no second use).
- result: 8 / 8 / 8 - all five points worse than V1's 3. cse registers p and r as equivalent and flow.c dead-store-eliminates the `p = r` tail before combine runs, so r is back to a single use and the copy is deleted exactly as before.
- verdict: KILLED

## [s9] Loop 2's base addend can get its required downstream second use from one of the function tail's three ctx+0xC reads.
- mechanism: Any live use after loop 2's base add would remove the REG_DEAD note and preserve the copy.
- probe: T1 / T3 / T4 (math_Distance3D's two arguments taken from the live local) plus T2 / T5 (loop-2 base addend = `p`, tail left as fresh reads).
- result: 22 / 22 / 22 and 8 / 8, versus V1's 3. Target emits `lw a1,12(s2)` for math_Distance3D and `lw v0,12(s2)` twice more for rec_a / rec_b; each disappears when a live local is substituted and the register-pressure change cascades through the whole tail.
- verdict: KILLED

## [s9] Loop 2's base can borrow loop 1's already-live `q` if `q` is pre-initialised before loop 1's guard (legal because this function never writes *(ctx+0xC), so all reads of it are the same value).
- mechanism: Pre-initialisation makes `q` defined on the loop-1-skipped path, so loop 2's base could read it and inherit loop 1's liveness for free.
- probe: X1 (pre-init by copy), X3 (pre-init by its own read), X2 / X4 (same two, with the loop-1 tail restored to a fresh read).
- result: 14 / 14 / 13 / 13 versus V1's 3.
- verdict: KILLED

## [s9] An explicit links-pointer local `lnk = *(u8 **)(ctx + 0x10);` is dead on this function (s7's X1/X2/X3 kill).
- mechanism: s7 measured the links local at 12/10/7 on its own chassis and banked it as closed.
- probe: S1 (lnk read after the `q` read and before `base = ...`) vs S2 (lnk read first), both on the R2 chassis.
- result: S1 = 4, S2 = 6, R2 = 6. The local is worth 2 points and the entire gain is POSITIONAL - it fixes `lw a2,16(s2)`'s placement between the copy and the base add. s7's kill is inverted; this is the fourth banked chassis inversion on this function.
- verdict: KILLED

## [s9] s8's bank that mirroring loop 1's guard-address/count-reuse lever onto loop 2 costs 26-31 points is a property of the lever.
- mechanism: s8 measured P_treuse_both and R_treuse_l2 and concluded the lever was loop-1-specific.
- probe: P1 (loop 2 t-reuse through the SAME `t` local) vs P2 (loop 2 t-reuse through its own `t2`), plus W4 (full loop-2 mirror of loop 1's preheader: own read, own links local, own tail copy) on the V1 chassis.
- result: P1 = 31 but P2 = 5 (exactly neutral) and W4 = 3 (exactly neutral). The cost was entirely due to SHARING the local, not to the lever.
- verdict: KILLED

## [s10] Target's two preheader copies have the same origin, so the spelling that produced loop 1's will produce loop 2's.
- mechanism: s6-s9 all read the two `addu a3,a0,zero` instructions (0x800178E4 and 0x80017930) as one symmetric construct, and s9 explained loop 1's as a cse-folded redundant `*(u8 **)(ctx + 0xC)` read surviving combine because its destination has a downstream use.
- probe: read the branch targets straight out of asm/funcs/func_80017848.s (never done before - the scoring normaliser masks them) to establish the real CFG, then confirm with the -da combine dump of the current 3-form.
- result: loop 1's skip branch targets .L8001791C = loop 2's guard, so loop 2's guard block is a JOIN and cse's EBB cannot reach loop 2's preheader. Loop 1's preheader IS single-predecessor. The two copies therefore cannot share the cse-fold origin.
- verdict: KILLED

## [s10] Loop 2's copy can be produced by a source-level C copy (`q2 = p;`) of the guard's pointer, in some spelling.
- mechanism: since cse cannot fold a redundant read at that point, the only remaining C-level way to emit a reg-reg copy there is to write one.
- probe: M1 (shared `q`), M3 (shared `q`, `p = q` tail retained), M4 (private `q2`), M5 (private `base2`), Q3/Q4 (a pre-initialised second pointer with the guard and base roles split between it and `p`).
- result: 9 / 8 / 9 / 9 / 13 / 9 versus the 3-form's 3. M4's disassembly shows the copy propagated away by cse and deleted by flow, taking the loop-1 exit-tail re-read with it (function 12 bytes short).
- verdict: KILLED

## [s10] A second use of the base addend that sits INSIDE the loop can buy the preheader copy, so the exit tail can be spent on target's fresh re-read instead.
- mechanism: s9's frontier probes (a) and (b) - carry the liveness on the do/while count reload or the element read rather than on a post-loop assignment.
- probe: loop 1 (whose target bytes we already match, so the mechanism is directly observable): P4 count-through-addend, P5 element-through-addend, P6 both, all with the tail restored to a fresh read. Same carriers on loop 2: P1, P2, P3.
- result: P4 = 6, P5 = 7, P6 = 4, P1 = 5, P2 = 6, P3 = 3 - never better than the 3-form, and P6 is no better than having no second use at all (M6 = 4). loop.c hoists the invariant `sh + q` into the preheader and cse2 folds it into `base`, so combine sees a single-use addend again.
- verdict: KILLED

## [s10] One of target's three post-loop-2 ctx+0xC re-reads can carry loop 2's second use cheaply, even though s9 measured math_Distance3D at 19-22.
- mechanism: rec_a and rec_b each consume the pointer once; if either could be fed from the loop-2 addend, the addend would gain the downstream use combine needs.
- probe: R2 (rec_a via the live local), R3 (rec_b), R4 (both), R5 (rec_a via a separate p2 local).
- result: 32 / 32 / 52 / 14. Every one of target's three tail re-reads is now measured; all are fatal.
- verdict: KILLED

## [s10] The sanctioned duplicated-statement-into-arms family can make loop 2's guard block single-predecessor, restoring the cse fold that produces loop 1's copy.
- mechanism: duplicate loop 2 into both arms of loop 1's `if`; the fall-through copy is then reached only from loop 1's exit tail, so cse's EBB extends into its preheader and folds the redundant read to a copy, and jump2's cross-jumping was expected to re-merge the two identical arms afterwards.
- probe: D1 (fresh-read tail) and D2 (`p = q` tail), whole loop 2 duplicated into both arms.
- result: 35 and 35. Cross-jumping does not merge, precisely because the fold makes the two arms textually different (copy in one, load in the other).
- verdict: KILLED

## [s10] Loop 2's addend can be made to behave differently by giving it a different identity (a reused existing local, a different type, its own name).
- mechanism: variable reuse for codegen control is the sanctioned family that has repeatedly moved this function; s9's frontier item (c) proposed reusing a still-live local so flow would not DSE the assignment.
- probe: S1 (reuse the s32 `t`), S2 (reuse `slots`), S3 (reuse loop 1's `q`), S4 (reuse `p`), plus P0 (its own `q2`) and the inline baseline.
- result: 3 / 8 / 13 / 3 / 3 / 3. Everything that does not cross loops is exactly neutral; everything that crosses loops is worse. Identity is not a lever here - only use-count is.
- verdict: KILLED

## [s10] Loop 2's shape (if + do/while + `base` local) is still optimal on the V1 chassis, six chassis changes after s2 established it.
- mechanism: every s2-era conclusion is chassis-relative and none of loop 2's shape had been re-measured since.
- probe: W1 (base precomputed + `while`), W2 (plain `while`, address recomputed in the condition), W3 (do/while with the count re-read through shift+pointer), W4 (base recomputed inside the body).
- result: 9 / 14 / 7 / 10 versus 3.
- verdict: CONFIRMED (the s2 shape survives; the alternatives are killed)

## [s10] Target's two preheader copies (0x800178E4 and 0x80017930) have the same origin, so the spelling that produced loop 1's will produce loop 2's.
- mechanism: s6-s9 all read the two `addu a3,a0,zero` instructions as one symmetric construct, and s9 explained loop 1's as a cse-folded redundant `*(u8 **)(ctx + 0xC)` read that survived combine because its destination has a downstream use.
- probe: Read the branch targets straight out of asm/funcs/func_80017848.s (never done before - the scoring normaliser masks branch targets), then confirm the block structure against the sandbox object and the -da combine dump of the current 3-form.
- result: Loop 1's skip branch is `blez $v0, .L8001791C` and .L8001791C is loop 2's guard `addu $v0,$a1,$a0`. Loop 2's guard block therefore has two predecessors, so cse's EBB stops there and cannot fold a redundant load in loop 2's preheader. Loop 1's preheader block IS single-predecessor. Confirmed in RTL: combine-dump insn 162 is a full `(mem (plus (reg 72) (const_int 12)))` load, and insn 164's base add carries REG_DEAD for both the shift and the pointer.
- verdict: KILLED

## [s10] Loop 2's copy can be produced by a source-level C copy (`q2 = p;`) of the guard's pointer, in some spelling.
- mechanism: Since cse cannot fold a redundant read at a join, the only remaining C-level way to emit a reg-reg copy in loop 2's preheader is to write one explicitly.
- probe: M1 (shared `q`), M3 (shared `q` with the `p = q` tail retained), M4 (private `q2`), M5 (private `base2`), Q3/Q4 (a pre-initialised second pointer with the guard and base roles split between it and `p`).
- result: 9 / 8 / 9 / 9 / 13 / 9 versus the 3-form's 3. M4's disassembly shows the copy propagated into the base add by cse and deleted by flow, which also makes the loop-1 exit-tail re-read redundant - the function comes out 12 bytes short.
- verdict: KILLED

## [s10] A second use of the base addend INSIDE the loop can buy the preheader copy, freeing the exit tail for target's fresh re-read (s9 frontier probes a and b).
- mechanism: Carry the liveness on the do/while count reload or on the element read instead of on a post-loop assignment.
- probe: Applied to loop 1, whose target bytes we already match so the mechanism is directly observable: P4 (count through the addend), P5 (element through the addend), P6 (both), each with the tail restored to a fresh read. Same carriers on loop 2: P1, P2, P3.
- result: P4 = 6, P5 = 7, P6 = 4, P1 = 5, P2 = 6, P3 = 3. P6 is no better than having no second use at all (M6 = 4). loop.c hoists the invariant `sh + q` into the preheader and cse2 folds it into the existing `base`, so combine sees a single-use addend again. The carrier class is dead, not just these spellings.
- verdict: KILLED

## [s10] One of target's three post-loop-2 ctx+0xC re-reads can carry loop 2's second use cheaply (s9 only measured math_Distance3D).
- mechanism: rec_a and rec_b each consume the pointer once; feeding either from the loop-2 addend would give the addend the downstream use combine needs.
- probe: R2 (rec_a via the live local), R3 (rec_b), R4 (both), R5 (rec_a via a separate p2 local).
- result: 32 / 32 / 52 / 14. All three of target's tail re-reads are now measured and all are fatal; there is no free downstream use of loop 2's base addend anywhere in the function.
- verdict: KILLED

## [s10] The sanctioned duplicated-statement-into-arms family can make loop 2's guard block single-predecessor, restoring the cse fold that produces loop 1's copy.
- mechanism: Duplicate loop 2 into both arms of loop 1's `if`; the fall-through copy is then reached only from loop 1's exit tail, so cse's EBB extends into its preheader and folds the redundant read to a copy, with jump2's cross-jumping expected to re-merge the arms afterwards.
- probe: D1 (fresh-read tail) and D2 (`p = q` tail), whole loop 2 duplicated into both arms.
- result: 35 and 35. Cross-jumping does not merge - precisely because the fold makes the two arms textually different (copy in one arm, load in the other).
- verdict: KILLED

## [s10] Loop 2's addend responds to its identity (a reused existing local, a different type, its own name) - s9 frontier probe (c).
- mechanism: Variable reuse for codegen control is the sanctioned family that has repeatedly moved this function, and s9 predicted flow would not DSE an assignment into a still-live local.
- probe: S1 (reuse the s32 `t`), S2 (reuse `slots`), S3 (reuse loop 1's `q`), S4 (reuse `p`), P0 (its own `q2`), versus the inline baseline.
- result: 3 / 8 / 13 / 3 / 3 / 3. Everything that does not cross loops is exactly neutral; everything that crosses loops is worse. Identity is not a lever here - only use-count is, and use-count cannot be raised for free.
- verdict: KILLED

## [s10] Loop 2's shape (if + do/while + a `base` local) is still optimal on the V1 chassis, six chassis changes after s2 established it.
- mechanism: Every s2-era conclusion on this function is chassis-relative and loop 2's shape had never been re-measured.
- probe: W1 (base precomputed + `while`), W2 (plain `while` with the address recomputed in the condition), W3 (do/while with the count re-read through shift+pointer), W4 (base recomputed inside the body).
- result: 9 / 14 / 7 / 10 versus 3.
- verdict: CONFIRMED

## [s11] Every banked s3-era spelling conclusion still standing is chassis-relative and must be re-measured on V1 before it is spent
- mechanism: this function has inverted a banked conclusion in five consecutive sessions (s5 inverted s3/s4, s8 inverted two s7 conclusions, s9 inverted three s7/s8 conclusions); s3's pointer-first rule and its slots-hoist rule predate the s6 chassis retirement and had been carried unexamined through four chassis changes.
- probe: matched-pair re-runs of every s3-era and s8-era conclusion against V1 with tmp/grind/func_80017848/s11/score.sh - association order at all five address sites, the slots hoist, the loop-1 two-step guard local, the t-reuse kill, the shared shift local.
- result: BOTH s3 conclusions fall. Pointer-first is now a strict regression (top guards 5 vs 3; loop-1 guard 4; loop-1 base 4; loop-2 guard 4) and the slots hoist is inert (inline reads in both top guards = 3, tied). Both s8 loop-1 conclusions re-confirm (two-step guard 3 vs inline 7 vs count-local 7; t-reuse for loop 2 = 36/18; separate shift local worth 1).
- verdict: CONFIRMED (the meta-hypothesis: banked spelling conclusions are chassis-relative and half of them invert)

## [s11] The lever for loop 2's missing preheader copy is somewhere in loop 2's preheader C
- mechanism: s10 mapped 28 hand-written variants across six families and found none below 3, but had not swept the axes independently, so a cross-product cell could still have been missed.
- probe: three independent cross-products holding loop 2's guard on the carried `p` - the E cube (guard source x base addend x links x second use, 12 cells), the G cube (loop-1 guard shape x loop-2 guard shape x guard temp variable, 15 cells) and the H cross (4 loop-1 tails x 5 loop-2 preheader spellings, 20 cells) - plus the association-order pair on loop 2's base.
- result: every carried-guard cell scores exactly its loop-1-tail baseline. The loop-2 preheader is C-INERT: named local vs inline read, links local vs inline, second use vs none, inline vs two-step-t2 vs count-local guard, and both association orders all produce the same score.
- verdict: KILLED

## [s11] Declaration order of the function's locals can flip local-alloc's tie decision at loop 2's base add
- mechanism: pseudo-register numbers follow declaration order, and local-alloc's allocno ordering and combine_regs tie-breaking are sensitive to pseudo numbering, so a permutation could change which allocno wins the tie without changing any semantics. This is the sanctioned named-intermediate-declaration-order family.
- probe: 56 permutations of the 13-local declaration block - 12 rotations, 13 move-to-front, the full reverse, and 30 random shuffles (seed 1234) - each engine-scored on a clean tree.
- result: all 56 score exactly 3. Not one permutation moved a single instruction.
- verdict: KILLED

## [s11] Writing loop 2 as an exact statement-for-statement mirror of loop 1 (fresh read + two-step guard local + redundant read + links local) reproduces target's identical-preheader shape
- mechanism: target's two preheaders are byte-identical instruction sequences with identical register numbers (`lw a0,12(s2); sll a1,s4,6; addu v0,a1,a0; lw v0,CNT(v0); blez; addu v1,zero,zero; addu a3,a0,zero; lw a2,16(s2); addu a0,a1,a3`), which is the fingerprint of two identical source constructs, so the C should be symmetric too.
- probe: the S family (12 cells: fresh-name locals p2/q2/lnk2/t2 vs reuse of p/q/lnk/t, crossed with 3 loop-1 tails and 2 loop-2 tails) and the R family (54 cells: fresh read reusing `p` with an inline guard, crossed with base addend, links, second use and loop-1 tail).
- result: fresh names = 10 uniformly, variable reuse = 36 uniformly, R family >= 9 uniformly. Loop 2's guard reading ctx+0xC freshly is a hard regime boundary worth 6 to 28 points regardless of every other axis; the carried `p` is mandatory. E_fresh's disassembly shows the whole function's allocation shifting (124 insns vs target 127, a0 and v1 swapping roles from loop 1's first instruction).
- verdict: KILLED

## [s11] Target's preheader copies are produced by the s9 second-use mechanism
- mechanism: s9 showed a copy survives combine iff its destination has a second use downstream of the base add, and used `p = q;` in loop 1's exit tail to buy loop 1's copy.
- probe: read target's own tail out of tmp/grind/func_80017848/s10/T.txt and look for any live carrier of ctx+0xC out of either loop.
- result: there is none. Target re-reads ctx+0xC three separate times after loop 2 (T.txt:77 for the math_Distance3D args, :94 for the rec_a update, :104 for the rec_b update) and each loop's addend register a3 is dead immediately after its base add. Target's copies exist with NO second use.
- verdict: KILLED (as an account of TARGET; the lever itself still works on our chassis and is still V1's best form - it is a coincidental reproduction of loop 1's bytes, and it cannot be replicated for loop 2 because no downstream purchase site exists)

## [s11] Every banked s3-era and s8-era spelling conclusion still standing is chassis-relative and must be re-measured on the V1 chassis before it is spent (frontier item #2).
- mechanism: This function has inverted a banked conclusion in five consecutive sessions (s5 inverted s3/s4; s8 inverted two s7 conclusions; s9 inverted three s7/s8 conclusions). s3's pointer-first rule and its slots-hoist rule predate the s6 chassis retirement and had been carried unexamined through four chassis changes.
- probe: Matched-pair re-runs against V1 with tmp/grind/func_80017848/s11/score.sh: association order at all five address sites (top guards, loop-1 guard, loop-1 base, loop-2 guard, loop-2 base), the slots hoist, the loop-1 two-step guard local, the t-reuse kill for loop 2, and the shared shift local.
- result: BOTH s3 conclusions fall. Pointer-first is now a strict regression at four of five sites (top guards 5 vs 3, loop-1 guard 4, loop-1 base 4, loop-2 guard 4; loop-2 base inert), and the slots hoist above the two >=0 top guards is INERT (deleting the local and reading inline in both guards = 3, exactly tied). Both s8 loop-1 conclusions re-confirm: two-step guard local 3 vs inline 7 vs count-into-local 7; t-reuse for loop 2 = 36 (two-step) / 18 (count-local) while a separate t2 is inert; one shared shift local = 4, so s9's second shift local is still worth 1.
- verdict: CONFIRMED

## [s11] The lever for loop 2's missing preheader copy is somewhere in loop 2's preheader C (s10 frontier item #1, C-level half).
- mechanism: s10 measured 28 hand-written variants across six families and found none below 3, but had not swept the axes independently as a cross-product, so a cell could still have been missed.
- probe: Three independent cross-products holding loop 2's guard on the carried `p`: the E cube (guard source x base addend x links x second use, 12 cells), the G cube (loop-1 guard shape x loop-2 guard shape x guard temp variable, 15 cells) and the H cross (4 loop-1 tails x 5 loop-2 preheader spellings, 20 cells), plus the association-order matched pair on loop 2's base address.
- result: Every carried-guard cell scores exactly its loop-1-tail baseline. Base addend inline-fresh-read vs named local `q2` vs reuse of `q`, links inline vs named `lnk2`, a second use of the addend after the loop vs none, guard shape inline vs two-step-through-t2 vs count-into-local, and both association orders ALL produce the identical score. 47 distinct spellings, one score.
- verdict: KILLED

## [s11] Declaration order of the function's 13 locals can flip local-alloc's tie decision at loop 2's base add (the sanctioned named-intermediate-declaration-order family).
- mechanism: Pseudo-register numbers follow declaration order, and local-alloc's allocno ordering and combine_regs tie-breaking are sensitive to pseudo numbering, so a permutation could change which allocno wins the tie with zero semantic change.
- probe: 56 permutations of the declaration block, each engine-scored on a clean tree: 12 rotations, 13 move-to-front, the full reverse, and 30 random shuffles (seed 1234).
- result: All 56 score exactly 3. Not one permutation moved a single instruction.
- verdict: KILLED

## [s11] Writing loop 2 as an exact statement-for-statement mirror of loop 1 reproduces target's identical-preheader shape, because target's two preheaders are byte-identical instruction sequences with identical register numbers.
- mechanism: Target's loop-2 preheader is a register-for-register copy of loop 1's (`lw a0,12(s2); sll a1,s4,6; addu v0,a1,a0; lw v0,CNT(v0); blez; addu v1,zero,zero; addu a3,a0,zero; lw a2,16(s2); addu a0,a1,a3`), which is the fingerprint of two identical source constructs, so the C should be symmetric too - including loop 2 reading ctx+0xC freshly for its guard, which is what target's asm literally does.
- probe: The S family (12 cells: fresh-name locals p2/q2/lnk2/t2 vs reuse of p/q/lnk/t, crossed with 3 loop-1 tails and 2 loop-2 tails) and the R family (54 cells: fresh read reusing `p` with an inline guard, crossed with base addend, links spelling, second use and loop-1 tail).
- result: Fresh names = 10 uniformly; variable reuse = 36 uniformly; the R family is >= 9 in all 54 cells. Loop 2's guard reading ctx+0xC freshly is a HARD REGIME BOUNDARY worth 6 (reusing `p`) to 28 (a new local p2) regardless of every other axis - the carried `p` is mandatory. E_fresh's disassembly shows the whole function's allocation shifting (124 insns vs target's 127; a0 and v1 swap roles from loop 1's first instruction).
- verdict: KILLED

## [s11] Target's two dead preheader copies are produced by the s9 second-use mechanism (a copy survives combine iff its destination has a use downstream of the base add).
- mechanism: s9 established the rule from the .lreg/.combine dumps and used `p = q;` in loop 1's exit tail to buy loop 1's copy; s10 priced the purchase at 2 in and 2 out. If that were target's mechanism, target must contain a live carrier of ctx+0xC out of at least one loop.
- probe: Read target's own tail out of the normalized listing tmp/grind/func_80017848/s10/T.txt and look for any live carrier of ctx+0xC out of either loop, and for any use of the addend register after each base add.
- result: There is none. Target re-reads ctx+0xC three separate times after loop 2 (T.txt:77 `lw a1,12(s2)` for the math_Distance3D args, T.txt:94 for the rec_a update, T.txt:104 for the rec_b update) and each loop's addend register a3 is dead immediately after its base add. Both of target's preheader copies therefore exist with NO second use.
- verdict: KILLED

## [s12] The parameter-homing / prologue region sets the allocno priority that decides loop 2's combine_regs tie (s11 frontier item #3).
- mechanism: Target's prologue moves all four parameters into callee-saved registers before the first compare, and our prologue is byte-identical - but the ORDER in which those pseudos are created and the number of references each accumulates feeds reg_n_refs and live-length, which are local-alloc's allocno ordering inputs, and therefore the tie-break at each base add. No session had probed it, because the prologue always looked settled.
- probe: 12 cells on V1 - each parameter homed into a named local used throughout (slot_a, slot_b, both in either assignment order, ctx, arg1), plus 6 top-block spellings (fused &&, the de Morgan two-arm form, `sh` hoisted above the guards, guard values staged through `t`, slot_b's shift staged early, the `slots` read moved above the equality early return), plus 4 tail spellings (the ctx+6 index hoisted into `t`, link store order, math_Distance3D argument association, `dist * 3` written as `(dist << 1) + dist`).
- result: Every parameter-homing cell is a strict regression (6, 6, 6, 7, 7). Every top-block change is inert (3) or a regression (6, 7, 18, 41). The tail spellings are inert (3) or regressions (5, 9, 34, 39). The prologue stays byte-exact throughout - it is not a lever.
- verdict: KILLED

## [s12] The loop-body / loop-shape / exit-form spelling changes register pressure enough to flip the preheader tie.
- mechanism: The s11 frontier named "loop-body spelling" and "tail spelling" as the two untouched regions outside loop 2's preheader. Both loops' bodies keep `base`, `lnk`, `i` and the induction temp live simultaneously, so a spelling that shortens or lengthens one of those lifetimes changes the allocno ordering local-alloc consumes.
- probe: 7 loop-body cells applied symmetrically to both loops (association order, the byte staged into `t`, an element-pointer local, `* 16` for `<< 4`, `(i + 0x24)` grouping, the `!=`-else form), 5 loop-shape cells (reversed while test, count reloaded into a local, for-loop form, a second counter local `j` for loop 2, a redundant `slots` re-read between the top guards) and 3 exit-form cells (all-returns-goto shared label, mixed goto/inline, an `res` accumulator with a single exit).
- result: No cell below 3. The ties (`* 16`, `!=`-else, byte-into-`t`, reversed test, the `j` counter, separate base1/base2) produce byte-identical output. Everything else regresses, most sharply the exit forms (15 / 9 / 8) - itself informative: jump.c already cross-jumps our four inline `return 0`s into target's single shared exit block, so spelling it explicitly only adds work.
- verdict: KILLED

## [s12] Loop 2's missing preheader copy can be bought by removing the join - duplicate loop 2 into both arms of the loop-1 guard so cse sees a single-predecessor block, fold the redundant load there, and let jump.c cross-jump the two copies back into target's shared shape.
- mechanism: s10/s11 proved loop 2's copy cannot be a cse fold BECAUSE its guard block is a two-predecessor join and cse in GCC 2.7.2 works on extended basic blocks. If the C removes the join, cse's equivalence for ctx+0xC survives into loop 2's preheader and folds the redundant load to a copy - exactly loop 1's mechanism. jump.c's cross-jumping would then have two identical tails to merge, which is how target's single join could still appear in the final asm.
- probe: Two cells - loop 2 duplicated verbatim into both arms of `if (i < t)`, and the same with the taken arm carrying `q` instead of `p` as the guard source.
- result: 35 in both cells. jump2 does not merge the duplicated blocks; the output carries two full copies of loop 2 and diverges by ~32 instructions.
- verdict: KILLED

## [s12] On the symmetric chassis (target's own fresh-reload loop-1 exit tail) the two preheaders are a single repeated construct, so one spelling should fix both at once.
- mechanism: Target's two preheaders are byte-identical instruction sequences with identical register numbers, and with the fresh-reload tail our residual becomes perfectly symmetric too (4 = 2 + 2: each preheader missing `addu a3,a0,zero` and reading the wrong register in its base add). A single symmetric spelling that produced the copy would be worth all 4 points at once, and would be the first form whose mechanism is shared between the loops rather than bought per-loop.
- probe: 12 one-axis cells on the symmetric chassis (loop-1 and loop-2 preheader addend spellings crossed with the presence of an `lnk` local), plus 11 fully symmetric rewrites where both loops receive the identical preheader construct.
- result: The symmetric chassis floor is exactly 4 and completely flat: every spelling whose base addend is a FRESH ctx+0xC read scores 4 (6 loop-1 spellings, 5 loop-2 spellings), and every spelling whose addend is a live pointer (`p`, `q = p`, `slots`) scores 9. The fully symmetric rewrites are 17 uniformly, because they also force loop 1's guard inline and drop the second shift local - both already priced by s8/s9. The symmetric chassis is a better DESCRIPTION of the wall (one mechanism applied twice) but not a better score than V1's 3.
- verdict: KILLED

## [s12] The parameter-homing / prologue region sets the allocno priority that decides loop 2's combine_regs tie (s11 frontier item #3, never probed in eleven sessions because the prologue has always been byte-exact and therefore looked settled).
- mechanism: Target's prologue saves s0-s5,ra into a 0x40 frame and moves all four parameters into callee-saved registers before the first compare. The ORDER in which those pseudos are created and the number of references each accumulates feeds reg_n_refs and live-length, which are local-alloc's allocno ordering inputs and therefore the input to the combine_regs tie-break s9 identified at each base add.
- probe: 12 cells on V1: each parameter homed into a named local used throughout (slot_a; slot_b; both, in either assignment order; ctx; arg1), plus 6 top-block spellings (fused &&, de Morgan two-arm, sh hoisted above both >=0 guards and shared with loop 1, the guard VALUES staged through the existing `t`, slot_b's shift staged into `t` early, the `slots` read moved above the `slot_a == slot_b` early return), plus 4 tail spellings (ctx+6 index hoisted into `t`, link store order 0/4/8/C, math_Distance3D argument association, `dist * 3` as `(dist << 1) + dist`).
- result: Every parameter-homing cell is a strict regression: slot_a 6, ctx 6, arg1 6, both slots 7 in either order. Top-block: fused && 3, de Morgan 3, redundant slots re-read between the guards 3, sh hoisted 6, slots-above-the-equality-return 7, slot_b shift early 18, guard values through `t` 41. Tail: (dist<<1)+dist 3, d3d re-association 5, store reorder 9, index into `t` 34, rec_a/rec_b reusing `base` 39. The prologue stays byte-exact in every cell.
- verdict: KILLED

## [s12] The loop-body, loop-shape and exit-form spellings change register pressure outside loop 2's preheader enough to flip the tie (the other two regions s11's frontier named as untouched).
- mechanism: Both loop bodies keep `base`, `lnk`, `i` and the strength-reduced induction temp live simultaneously, so a spelling that shortens or lengthens any of those lifetimes changes the allocno ordering local-alloc consumes; and target's four `return 0` sites all share one exit block, so an explicit shared-exit spelling might reshape the whole function's pressure.
- probe: 7 loop-body cells applied symmetrically to both loops (address association order, byte staged into `t`, element-pointer local, `* 16` for `<< 4`, `(i + 0x24)` grouping, `!=`-else form), 5 loop-shape cells (reversed while test, count reloaded into a local, for-loop form, a second counter local `j` for loop 2, redundant `slots` re-read between the top guards), 3 exit-form cells (all four returns via `goto end0`, mixed goto/inline, an `res` accumulator with a single `end: return res;`), and 10 base-destination-identity cells (which local receives the base add).
- result: No cell below 3. Ties are byte-identical no-ops: `* 16` 3, `!=`-else 3, byte-into-`t` 3, reversed test 3, `j` counter 3, base1/base2 3, loop-2 base into `p` 3. Regressions: links-first association 5, loop-1 base into `q` 5, `(i+0x24)` grouping 7, loop-2 base into `q`/`slots` 8, mixed exit 8, accumulator 9, loop-2 base into `lnk` 10, element-pointer local 13, all-goto exit 15, for-form 21, count-into-local 30, loop-2 base into `rec_a` 41. The exit-form regressions are themselves informative: jump.c already cross-jumps our four inline returns into target's single shared block, so spelling it out only adds work.
- verdict: KILLED

## [s12] Loop 2's missing preheader copy can be bought by REMOVING THE JOIN - duplicate loop 2 into both arms of the loop-1 guard so its preheader has a single predecessor, let cse fold the redundant ctx+0xC load to a copy there (loop 1's mechanism), and let jump.c's cross-jumping re-merge the two copies into target's shared join.
- mechanism: s10/s11 proved loop 2's copy cannot be a cse fold BECAUSE its guard block is a two-predecessor join and cse in GCC 2.7.2 works on extended basic blocks. Removing the join is the only C-level way to make cse's ctx+0xC equivalence reach loop 2's preheader; jump2 having two identical tails to merge is how target's single join could still appear in the final asm.
- probe: Two cells: loop 2 duplicated verbatim into both arms of `if (i < t)`, and the same with the taken arm carrying `q` instead of `p` as the guard source.
- result: 35 in both cells. jump2 does NOT merge the duplicated blocks - the output carries two full copies of loop 2 and diverges by ~32 instructions. The join cannot be removed from C.
- verdict: KILLED

## [s12] On the symmetric chassis (target's own fresh-reload loop-1 exit tail) the two preheaders are ONE repeated construct, so a single symmetric spelling should fix both at once.
- mechanism: Target's two preheaders are byte-identical instruction sequences with identical register numbers - the fingerprint of two identical source constructs. With `p = *(u8 **)(ctx + 0xC);` as the loop-1 exit tail (target's own `lw a0,12(s2)`), our residual also becomes symmetric, so a spelling that produces the copy would be worth all 4 points at once and would be the first form whose mechanism is shared between the loops rather than bought per-loop.
- probe: 12 one-axis cells on the symmetric chassis (loop-1 and loop-2 preheader addend spellings crossed with the presence of an `lnk` local: addend from p / fresh q / `q = p` copy / staged `t` / pointer-add / inline fresh / re-associated / reusing `slots`), plus 11 fully symmetric rewrites where both loops receive the identical preheader construct.
- result: The symmetric chassis floor is exactly 4 and completely flat: all 11 fresh-read addend spellings score 4, all 3 live-pointer addend spellings score 9. Loop 2's `lnk` local is forbidden on this chassis (12 with it) and loop 1's is mandatory (6 without it). The fully symmetric rewrites are 17 uniformly, because they also force loop 1's guard inline and drop the second shift local - both already priced by s8/s9. The chassis is a better DESCRIPTION of the wall than V1, not a better score.
- verdict: KILLED

## [s12] The 3-scoring neutral cells found this session are individually inert but compound into a win when stacked (a mutually-cancelling set rather than a genuine plateau).
- mechanism: A structural change that is score-neutral on its own can still move pseudo lifetimes and allocno ordering; two such changes could reinforce rather than cancel, which is how s8 and s9 each found their multi-lever wins (three stacked levers apiece).
- probe: Eight stacked combinations of every 3-scoring lever found this session - fused top guard, `!=`-else loop body, `* 16` shift, reversed while test, second counter `j`, separate base1/base2 locals, `(dist << 1) + dist`, redundant slots re-read - including one cell (K3) stacking all seven at once.
- result: All eight score exactly 3. The neutral set is a true plateau: each of those cells produces byte-identical output, so there is nothing to reinforce.
- verdict: KILLED

## [s13] An automated permuter search over the V1 chassis, anchored OUTSIDE loop 2's preheader, finds a multi-region spelling combination below 3 that a hand sweep cannot enumerate (s10/s11/s12 frontier item #2, the highest-yield remaining modality after twelve sessions of hand sweeps).
- mechanism: s4's only permuter win (14 -> 12) was INVISIBLE to the permuter's own weighted ranking and appeared only when every output was re-scored with the engine, so the permuter's value on this function is breadth of spelling, not its own score. The V1 chassis is three levers deeper than the one s4/s5 permuted and had never been searched. s11 established that the search must be anchored outside loop 2's preheader (47 cells, one score); s12 additionally showed the top block, both loop bodies, the exit form and the tail are all INDIVIDUALLY inert, which is precisely the condition under which the remaining value is in multi-region COMBINATIONS - s12's own compound test covered only 8 of them by hand.
- probe: Three campaigns via tools/permuter_campaign.py with telemetry, all waited on IN-TURN and harvest --stop'ed before the session ended. (a) s13a: V1 chassis with two PERM_RANDOMIZE regions - the top guards + loop 1, and the whole tail - freezing loop 2 entirely; base perm score 405; 25,905 iterations over ~27 min. (b) s13c (fresh reseed after s13a went 12 min without a novel find): V1 chassis with NO PERM macros, i.e. whole-function randomization including loop 2's preheader; base perm score 405; 27,827 iterations over ~28 min. (c) s13b: the s12 symmetric chassis (target's own fresh-reload loop-1 exit tail, floor 4) with whole-function randomization; base perm score 310; 44,872 iterations over ~50 min. EVERY output dir was re-scored with `sandbox --disable all` rather than trusting the permuter score.
- result: 98,604 total iterations across the three campaigns produced 16 distinct perm-score finds and NOT ONE engine-scored improvement. s13a: 2 finds, engine 4 and 5. s13c: 7 finds, engine 3 (that one is the base itself, reformatted - perm score 405 = base score) then 4, 5, 7, 9, 11, 12. s13b: 7 usable finds, engine 4, 4, 5, 5, 6, 8, 15 (chassis floor is 4, so it too was never beaten) plus one that fails to re-apply. The permuter score is confirmed ANTI-CORRELATED with the engine distance on this function for the third time: s13b's BEST perm score (220, i.e. 90 better than base) re-scores to engine 5, while its engine-4 cells sit at perm 310 = the base score.
- verdict: KILLED

## [s13] The permuter's whole-function randomization reaches spellings of loop 2's preheader that s11's 47 hand cells missed.
- mechanism: s11 enumerated loop 2's preheader by hand along named axes (addend spelling, links local, second use, guard shape, association order). A randomizer that mutates the AST without a human's notion of "an axis" can produce combinations off every one of those axes, and it operates on the preheader jointly with the rest of the function rather than one region at a time.
- probe: Campaign s13c deliberately dropped the PERM_RANDOMIZE fences so the randomizer could touch loop 2's preheader together with every other region; 27,827 iterations.
- result: The best engine-scored output that is not the base itself is 4 - i.e. a strict regression - and the score distribution (3, 4, 5, 7, 9, 11, 12) is monotone in perm score with no outlier. Removing the fences did not find anything the fenced campaign missed; it only produced worse cells faster. Loop 2's preheader is now closed to automated search as well as to hand sweeps.
- verdict: KILLED

## [s13] An automated permuter search over the V1 chassis, anchored OUTSIDE loop 2's preheader, finds a multi-region spelling combination below 3 that a hand sweep cannot enumerate (the s10/s11/s12 frontier item #2, carried for three sessions as the highest-yield remaining modality).
- mechanism: s4's only permuter win (14 -> 12) was INVISIBLE to the permuter's own weighted ranking and appeared only when every output was re-scored with the engine, so the permuter's value on this function is breadth of spelling rather than its own score. The V1 chassis is three levers deeper than the one s4/s5 permuted and had never been searched. s11 established the search must be anchored outside loop 2's preheader (47 hand cells, one score); s12 further showed the top block, both loop bodies, the exit form and the tail are all INDIVIDUALLY inert - precisely the condition under which the remaining value is in multi-region COMBINATIONS, of which s12's own compound test sampled only 8 by hand.
- probe: Campaign s13a: tmp/perm_ings_s13a, V1 chassis with two PERM_RANDOMIZE regions (the top guards + loop 1; the whole tail), loop 2 frozen entirely. Base perm score 405, -j 6, 25,905 iterations over ~27 min, waited on in-turn with permuter_campaign.py wait, reseeded per the fresh-seed discipline after 12 min without a novel find. Campaign s13c (the reseed): same V1 chassis with NO PERM macros, i.e. whole-function randomization. Base perm 405, 27,827 iterations over ~28 min. Every output dir from both was re-scored with `sandbox --disable all` via tmp/grind/func_80017848/s13/score_outputs.sh rather than trusting the permuter score.
- result: s13a produced 2 finds: perm 380 -> engine 5, perm 310 -> engine 4. s13c produced 7 finds: engine 3 (that one is the base itself reformatted - its perm score is 405, exactly the base score), then 4, 5, 7, 9, 11, 12. Not one output of 53,732 iterations scored below the chassis floor of 3, and the engine distribution is monotone in perm score with no outlier. The fenced and unfenced campaigns agree.
- verdict: KILLED

## [s13] The permuter's whole-function randomization reaches spellings of loop 2's preheader that s11's 47 hand cells missed, because a randomizer mutates the AST without a human's notion of 'an axis' and can touch the preheader jointly with every other region.
- mechanism: s11 enumerated loop 2's preheader by hand along named axes (base-addend spelling, links local, second use, guard shape, association order) and found 47 cells at one score. A machine search operating on the preheader together with the rest of the function rather than one region at a time could produce combinations off every one of those axes.
- probe: Campaign s13c deliberately dropped the PERM_RANDOMIZE fences that s13a used, so the randomizer could mutate loop 2's preheader jointly with the top block, both loop bodies and the tail. 27,827 iterations, every output engine-re-scored.
- result: The best engine-scored output that is not the base itself is 4 - a strict regression - and removing the fences produced worse cells faster rather than reaching anything the fenced campaign missed. Loop 2's preheader is now closed to automated search as well as to hand sweeps.
- verdict: KILLED

## [s13] The s12 symmetric chassis (target's own fresh-reload loop-1 exit tail, floor 4) is worth permuting despite being one point worse, because its residual is ONE repeated mechanism rather than two, so a single find there is worth 4 points instead of 1 (the s12 frontier's explicit suggestion).
- mechanism: On the symmetric chassis both preheaders are missing the identical `addu a3,a0,zero` and both base adds read the wrong register, so any spelling that produces the copy would fix both loops at once. s12 measured the chassis flat at 4 across 23 hand cells, but a hand sweep cannot enumerate the combinations, which is exactly what a randomizer does.
- probe: Campaign s13b: tmp/perm_ings_s13b built from rejected/s12_symmetric_fresh_reload_tail_costs_4.c with whole-function randomization (no PERM macros). Base perm score 310, -j 5, 44,872 iterations over ~50 min across five wait windows, every output engine-re-scored.
- result: 8 finds, 7 of them re-appliable: engine 4, 4, 5, 5, 6, 8, 15. The chassis floor of 4 was tied but never beaten, so the symmetric chassis remains 1 worse than V1 after 44,872 iterations. It also produced the session's sharpest anti-correlation exhibit: its BEST perm score (220, a 90-point improvement over base) re-scores to engine 5, while both cells that tie the floor at engine 4 sit at perm 310 = exactly the base score.
- verdict: KILLED

## [s14] The deciding input is OUTSIDE the function body, in the TRANSLATION-UNIT context - the math_Distance3D prototype, the position of func_80017848 within src/ings.c, or the number and order of the file's preceding declarations (the s13 frontier's item #1, named as the highest-yield remaining axis and never probed in thirteen sessions).
- mechanism: The claim was that GCC 2.7.2's allocno ordering (local-alloc's reg_n_refs / live-length inputs) and cse's hash tables are affected by state that accumulates across the whole translation unit rather than per function, by analogy with the cc1-first-pass-scheduler-bug memory in which ~1400 lines of preceding declarations changed cc1's behaviour on a trivial function. The declared prototype of the callee additionally decides how its arguments are set up and therefore the register pressure of the tail that feeds back into loop 2's preheader tie.
- probe: Five TU mutations applied on top of the unmodified V1 candidate body and re-scored with `sandbox --disable all` (tmp/grind/func_80017848/s14/tu.py): M1 = 40 extern declarations immediately above the function; M2 = 200 extern declarations at the very top of the file; M3 = the math_Distance3D / math_Distance3D_16 definitions moved BELOW func_80017848 and replaced by prototypes; M4 = an extra complete function definition immediately above; M5 = ten file-scope static const declarations immediately above.
- result: M0 (baseline) 3, M1 3, M2 3, M3 3, M4 3, M5 3. Not one cell moved the score by a single instruction. GCC 2.7.2 resets pseudo-register numbering, cse's hash tables and local-alloc's allocno arrays per function; the only genuinely TU-accumulating state is label_num and varasm bookkeeping, neither of which is an input to allocation or scheduling. The cc1-first-pass-scheduler-bug analogy does not transfer - that is a first-pass-scheduler crash pathology, not a codegen-selection channel.
- verdict: KILLED

## [s14] A DIRECTED permuter campaign - a PERM_GENERAL cross-product over the levers the ledger measured individually at exactly 3, layered on top of PERM_RANDOMIZE - finds a compound below 3 that neither s12's 8 hand-stacked cells nor s13's pure-random campaigns could reach.
- mechanism: s12 established that the top block, both loop bodies, the exit form and the tail are all INDIVIDUALLY inert at 3, which is precisely the condition under which the remaining value lives in multi-region COMBINATIONS. s12 sampled only 8 compounds by hand; s13's campaigns were pure random mutation with no notion of the ledger's named alternatives, so they never systematically visited the product space. A PERM_GENERAL cross-product visits it exhaustively while randomization explores around each combination.
- probe: Campaign s14a (tmp/perm_ings_s14a, label s14a_V1_directed_crossproduct, -j 6, base perm 405): three PERM_GENERAL cross-products - top-guard shape (nested ifs vs fused &&), loop 1's element read (`<< 4` vs `* 16` vs byte staged into the named intermediate `t`), loop 2's block (base into `base` vs into `p` vs with an explicit `lnk` local) - inside PERM_RANDOMIZE regions covering loop 1, loop 2 and the tail. 47,221 iterations across two wait windows, stopped on a 547-second window with no novel find, per the fresh-seed discipline. Every output re-scored with `sandbox --disable all`.
- result: 14 finds. Engine distribution 3, 5, 5, 7, 7, 9, 9, 10, 12, 14, 16, 17, 19 (+1 apply-fail). The single engine-3 output sits at perm 405 = exactly the base score, i.e. it is the base chassis reformatted; every output the permuter actually preferred is a regression. The directed product space is empty below 3.
- verdict: KILLED

## [s14] m2c's decompilation of the target asm, transcribed faithfully into compilable C, is the structurally alien chassis the s13 frontier asked for ("a chassis that does not yet exist"), and searching from it reaches a different basin.
- mechanism: Every chassis in fourteen sessions descends from the same hand-written V1 ancestor, so all of them share its association order, loop shape and variable identity. m2c reads the target asm directly and produces rotated loops (increment at the top, `> 0` preheader guards), a walking element pointer, and a carried pointer local reused as the copy source in BOTH preheaders - the last of which is literally target's `addu a3,a0,zero` in both places. If the copy is a source-level fact, that chassis should show it.
- probe: `python3 tools/m2c/m2c.py --valid-syntax --target mipsel-gcc-c asm/funcs/func_80017848.s`, transcribed into compilable C against the ings.c context (M2C_FIELD expanded to casts) as tmp/grind/func_80017848/s14/chassis_M.c and scored with `sandbox --disable all`. Then the one placement idea it contributed - `sh` recomputed on loop 1's TAKEN path only, `sh2` deleted - was extracted as cells N1/N2/N3 on the V1 chassis, and campaign s14n was seeded on N1 with whole-function randomization (34,647 iterations, 11 finds, all engine re-scored).
- result: The faithful m2c transcription scores 49, sixteen times the V1 floor - m2c's output reflects the SCHEDULED, allocated asm rather than the source shape GCC was given, so it is not a usable chassis seed. The extracted `sh`-recompute cells: N1 (recompute after `p = q;`) = 3, N3 (dual-path `sh2`) = 3, N2 (recompute before `p = q;`) = 4. N1 is a genuinely new spelling at the floor, but its residual is BYTE-IDENTICAL to V1's - the same three instructions at insns 52/59/61 - so it lands in exactly the same place, and the campaign seeded on it found nothing below 3 (both engine-3 outputs sat at perm 405 = the base score).
- verdict: KILLED (the chassis is new; the basin is not)

## [s14] The deciding input is OUTSIDE the function body, in the TRANSLATION-UNIT context - the math_Distance3D prototype, the position of func_80017848 within src/ings.c, or the number and order of the file's preceding declarations (s13's frontier item #1, named the highest-yield remaining axis and never probed in thirteen sessions).
- mechanism: The claim was that GCC 2.7.2's allocno ordering (local-alloc's reg_n_refs / live-length inputs) and cse's hash tables are affected by state accumulating across the whole translation unit rather than per function, by analogy with the cc1-first-pass-scheduler-bug memory in which ~1400 lines of preceding declarations changed cc1's behaviour on a trivial function; and that the callee's declared prototype decides its argument setup and therefore the register pressure of the tail that feeds back into loop 2's preheader tie.
- probe: Five TU mutations applied on top of the unmodified V1 candidate body, each re-scored with `sandbox --disable all` (tmp/grind/func_80017848/s14/tu.py): M1 = 40 extern declarations immediately above the function; M2 = 200 extern declarations at the very top of the file; M3 = the math_Distance3D / math_Distance3D_16 DEFINITIONS moved below func_80017848 and replaced in place by prototypes; M4 = an extra complete function definition immediately above; M5 = ten file-scope static const declarations immediately above.
- result: M0 (baseline) 3, M1 3, M2 3, M3 3, M4 3, M5 3 - not one cell moved the score by a single instruction. Mechanistically consistent: GCC 2.7.2 resets pseudo-register numbering (init_emit), cse's hash tables and local-alloc's allocno arrays per FUNCTION; the only genuinely TU-accumulating state is label_num and varasm/obstack bookkeeping, neither of which is an input to allocation or scheduling. The cc1-first-pass-scheduler-bug analogy does not transfer - that is a first-pass-scheduler crash pathology, not a codegen-selection channel.
- verdict: KILLED

## [s14] A DIRECTED permuter campaign - a PERM_GENERAL cross-product over the levers the ledger measured individually at exactly 3, layered on top of PERM_RANDOMIZE - finds a compound below 3 that neither s12's 8 hand-stacked cells nor s13's three pure-random campaigns could reach.
- mechanism: s12 established that the top block, both loop bodies, the exit form and the tail are all INDIVIDUALLY inert at 3, which is exactly the condition under which remaining value lives in multi-region COMBINATIONS. s12 sampled only 8 compounds by hand; s13's campaigns were pure random mutation with no notion of the ledger's named alternatives, so they never systematically visited the product space. A PERM_GENERAL cross-product visits it exhaustively while randomization explores around each combination.
- probe: Campaign s14a (tmp/perm_ings_s14a, label s14a_V1_directed_crossproduct, -j 6, base perm 405): three PERM_GENERAL cross-products - top-guard shape (nested ifs vs fused &&), loop 1's element read (`<< 4` vs `* 16` vs the byte staged into the named intermediate `t`), loop 2's block (base into `base` vs into `p` vs with an explicit `lnk` links local) - nested inside PERM_RANDOMIZE regions covering loop 1, loop 2 and the tail. 47,221 iterations across two in-turn wait windows, stopped on a 547-second window with no novel find per the fresh-seed discipline. Every output re-scored with `sandbox --disable all`.
- result: 14 finds; engine distribution 3, 5, 5, 7, 7, 9, 9, 10, 12, 14, 16, 17, 19 (+1 apply-fail). The single engine-3 output sits at perm 405 = exactly the base score, i.e. it is the base chassis reformatted; every output the permuter actually preferred is a regression. The directed product space is empty below 3.
- verdict: KILLED

## [s14] m2c's decompilation of the target asm, transcribed faithfully into compilable C, is the structurally alien chassis the s13 frontier asked for ('a chassis that does not yet exist'), and searching from it reaches a different basin.
- mechanism: Every chassis in fourteen sessions descends from the same hand-written V1 ancestor, so all of them share its association order, loop shape and variable identity. m2c reads the target asm directly and produces rotated loops (increment at the top, `> 0` preheader guards), a walking element pointer, and one carried pointer local reused as the copy source in BOTH preheaders - the last of which is literally target's `addu a3,a0,zero` in both places. If the copy is a source-level fact, that chassis should show it.
- probe: `python3 tools/m2c/m2c.py --valid-syntax --target mipsel-gcc-c asm/funcs/func_80017848.s`, transcribed into compilable C against the ings.c context (M2C_FIELD expanded to casts) as tmp/grind/func_80017848/s14/chassis_M.c and scored. Then the one placement idea it contributed - `sh` recomputed on loop 1's TAKEN path only, `sh2` deleted - was extracted as cells N1/N2/N3 on the V1 chassis, and campaign s14n (tmp/perm_ings_s14n) was seeded on N1 with whole-function randomization: 34,647 iterations, 11 finds, all engine re-scored.
- result: The faithful m2c transcription scores 49 - sixteen times the V1 floor - so m2c's output reflects the SCHEDULED, allocated asm rather than the source shape GCC was given and is not a usable chassis seed (a thing eight sessions assumed in passing, now measured). The extracted cells: N1 (`sh = slot_a << 6;` appended after `p = q;`, sh2 deleted) = 3, N3 (dual-path sh2) = 3, N2 (recompute placed before `p = q;`) = 4. N1 is a genuinely NEW spelling at the floor but its residual is BYTE-IDENTICAL to V1's - the same three instructions at insns 52/59/61 - and the campaign seeded on it found nothing below 3 (both engine-3 outputs at perm 405 = the base score).
- verdict: KILLED

## s15 (2026-08-18, forensics) — hypotheses

### H-s15-1 CONFIRMED — the base-add operand swap is local-alloc's `optimize_reg_copy_1`
**Statement.** The pass that makes loop 1's base add consume the preheader copy's
destination (target `addu a0,a1,a3` rather than `addu a0,a1,a0`) is
`optimize_reg_copy_1` in local-alloc.c, not cse, combine, sched, global-alloc or
reload.
**Mechanism.** local-alloc.c:1002-1006 dispatches `optimize_reg_copy_1` for every
`(set (reg D) (reg S))` where S does NOT have a REG_DEAD note on the copy.  The
routine (local-alloc.c:700) scans forward inside the SAME basic block; if S dies
before either S or D is modified it rewrites every mention of S in that range to
D and moves S's death onto the copy, "to enable DEST to be tied to SRC".  It never
deletes the copy, and no DCE runs after local-alloc, so the copy is emitted.
**Probe.** Diff the same insn across the `-da` series for the candidate body
(`tmp/grind/func_80017848/s15/icand/F_*.txt`): insn 89 is
`(plus (reg 84) (reg 79))` in `.cse`, `.loop`, `.cse2`, `.flow`, `.combine` and
`.sched`, and `(plus (reg 84) (reg 80))` in `.lreg`; reg79's REG_DEAD moves from
insn 89 to insn 83 in the same step.
**Result.** The swap is localised to the cse→lreg boundary and matches
`optimize_reg_copy_1`'s precondition exactly (reg79 not dead at insn 83, dead at
insn 89, same block, nothing modifies either in between).  CONFIRMED.

### H-s15-2 CONFIRMED — a preheader reg-reg copy survives combine iff its destination is unused in that basic block
**Statement.** combine deletes a preheader copy only when a LOG_LINK reaches it,
and `flow.c:2102` builds LOG_LINKs only when the using insn is in the same basic
block as the setting insn (`BLOCK_NUM (y) == blocknum`).
**Mechanism.** Loop 1's copy (insn 83, `reg80 = reg79`) has one pre-lreg use,
insn 141 (`p = q`) in the loop-1 exit tail — a different block — so combine has
no link to it and leaves it alone.  Loop 1's base add still reads reg79 at
combine time, so it is not even a candidate for substitution.
**Probe.** F_combine.txt for the candidate (copy intact, base add unchanged) plus
the two-sided experiment C1/D1 in H-s15-3.
**Result.** CONFIRMED from both directions.

### H-s15-3 CONFIRMED (mechanism) / KILLED (as a scoring lever) — loop.c's `move_movables` can place a reg-reg copy in loop 2's preheader from pure C
**Statement.** s13/s14's frontier claim that loop 2's preheader "has no C-level
handle at all" is false: a loop-INVARIANT reg-reg copy written inside loop 2's
BODY is hoisted verbatim into the preheader by `move_movables`
(`tools/gcc-2.7.2/loop.c:1690-1710`), and survives combine by H-s15-2 because its
uses stay in the body.
**Mechanism.** loop.c hoists invariants by duplicating the insn before
`loop_start`; a `b2 = base;` invariant is therefore emitted as `move` into the
preheader.  Its uses remain inside the loop, in a different basic block, so no
LOG_LINK reaches the hoisted copy.
**Probe.** Cell C1 — `b2 = base;` as the first statement of loop 2's body, body
element read and loop condition rewritten onto `b2`.  Scored with
`sandbox --disable all` and dumped with the instrumented cc1
(`tmp/grind/func_80017848/s15/iC1/ings.s`).
**Result.** The copy IS emitted, in target's block: the preheader becomes
`lw $2,12($18) / lw $6,16($18) / addu $4,$5,$2 / move $5,$4`.  Score 6 — the copy
is of the BASE rather than of the ADDEND, so it lands after the base add instead
of before it and adds an instruction target does not have.  The mechanism is
CONFIRMED; the specific cell is KILLED.  Banked as
`rejected/s15_body_invariant_copy_of_base_hoisted_copy_survives_costs_6.c`.

### H-s15-4 KILLED — hoisting the copy AND its consumer together lets combine delete the copy again
**Statement.** Naming loop 2's addend (`q2`) and making both `b2 = q2;` and
`base = sh2 + b2;` loop-2-body invariants would reproduce target's
copy-then-base-add order in the preheader.
**Mechanism.** loop.c hoists both insns before `loop_start`, so the copy and its
only use end up in the SAME basic block — H-s15-2's survival condition is
violated, a LOG_LINK is created, combine substitutes and deletes the copy.
**Probe.** Cell D1 (both invariants) and D2 (copy invariant only, base add left
in the preheader).
**Result.** D1 = 3 with a preheader byte-identical to the candidate's — the copy
is gone.  D2 = 9.  KILLED; banked as
`rejected/s15_hoisted_copy_plus_base_combine_deletes_copy_costs_3.c` and
`rejected/s15_hoisted_copy_body_uses_b2_costs_9.c`.

### H-s15-5 KILLED — "loop 2's base addend is a copy/alias of the carried pointer" is structurally excluded, not merely expensive
**Statement.** s11/s12 recorded the reuse-p family as costing 8/9/19/28/41 without
a mechanism; the mechanism is cse merging the guard-address add with the base add.
**Mechanism.** If the base addend is provably equal to the guard's addend, cse
CSEs `sh2 + addend` and emits it ONCE; target emits it TWICE (`addu v0,a1,a0` at
0x8001791C for the guard, `addu a0,a1,a3` at 0x80017938 for the base).  So any
such spelling deletes an instruction target has.
**Probe.** Cells A1 (`r = p;` in the join block, guard on p, base on r), A2
(guard on r, base on p), A3 (base reuses p directly), A4 (`r = p;` inside the
preheader); A1 additionally dumped with the instrumented cc1.
**Result.** All four score 8.  A1's asm shows the collapse literally:
`.L171: sll $2,$20,6 / addu $4,$2,$4 / lw $2,32($4)` where `addu $4,$2,$4` serves
as both the guard address and the base.  KILLED with a mechanism; banked as
`rejected/s15_l2_copy_of_p_collapses_guard_and_base_adds_costs_8.c`.
Corollary for future sessions: **loop 2's base addend must be a value cse cannot
prove equal to the guard's addend — i.e. a fresh `*(u8 **)(ctx + 0xC)` read.**
The candidate already does this; do not re-probe the family.

### H-s15-6 OPEN — target's loop-2 copy must be created or made unfoldable AFTER combine, and `optimize_reg_copy_2` is the only remaining 2.7.2 candidate
**Statement.** Target's `addu a3,a0,zero` has a destination used exactly once, at
the base add, in the same basic block, dying there (a3 occurs only 5 times in the
whole target listing, twice per loop plus the prologue).  H-s15-2 says such a copy
is deleted by combine if it exists before combine; H-s15-4 says a hoisted one is
too; and any copy whose destination is used OUT of the block leaves that use in the
asm (our loop 1 pays `move $4,$7`, C1 pays inside the body) and target has no such
instruction.
**Mechanism (the untested one).** `optimize_reg_copy_2` (local-alloc.c:874) fires
on `(set (reg D) (reg S))` where S DIES in the copy, scans forward in the same
block until a CODE_LABEL / JUMP_INSN / NOTE_INSN_LOOP_BEG / NOTE_INSN_LOOP_END,
and if it finds a REVERSE copy `(set (reg S) (reg D))` with D dying there, it
replaces D with S over the whole range and removes both death notes — leaving
BOTH copy insns physically in place, because nothing after local-alloc deletes
dead insns.  No session has ever written C aimed at that predicate.
**Next probe.** See the frontier.

## [s15] The pass that makes loop 1's base add consume the preheader copy's destination (target `addu a0,a1,a3` rather than `addu a0,a1,a0`) is local-alloc.c's optimize_reg_copy_1, not cse, combine, sched, global-alloc or reload.
- mechanism: local-alloc.c:1002-1006 dispatches optimize_reg_copy_1 (local-alloc.c:700) for every `(set (reg D) (reg S))` where S does NOT carry a REG_DEAD note on the copy. The routine scans forward inside the SAME basic block; if S dies before either S or D is modified, it rewrites every mention of S in that range to D and moves S's death note onto the copy, explicitly 'to enable DEST to be tied to SRC, thus often saving one register in addition to a register-register copy'. It never deletes the copy, and no DCE pass runs after local-alloc, so the copy reaches the assembler.
- probe: Compiled the candidate body with the INSTRUMENTED cc1 (tools/gcc-2.7.2/cc1, re-proved codegen-identical to the frozen build/cc1 on the whole TU in the same run) with -da, sliced func_80017848 out of every pass dump, and diffed the same insn across the series: insn 89 is `(set (reg/v 81) (plus (reg/v 84) (reg/v 79)))` in .cse, .loop, .cse2, .flow, .combine AND .sched, and `(plus (reg/v 84) (reg/v 80))` in .lreg; reg79's REG_DEAD note moves from insn 89 to insn 83 in the same step.
- result: The operand swap is localised to the sched->lreg boundary and matches optimize_reg_copy_1's precondition exactly (reg79 not dead at the copy insn 83, dead at the base add insn 89, same basic block, neither modified in between). Six sessions of inference (s6/s7/s8 'a pass after combine'; s9 'combine survival'; s10-s14 'unidentified origin') are replaced by a named pass and a named routine.
- verdict: CONFIRMED

## [s15] A reg-reg copy in a loop preheader survives combine if and only if no insn in that same basic block uses the copy's destination.
- mechanism: combine only ever follows LOG_LINKS, and flow.c:2102 guards LOG_LINK creation with `if (y && (BLOCK_NUM (y) == blocknum) ...)` - links are never built across a basic-block boundary. Loop 1's copy (insn 83, `reg80 = reg79`) has exactly one pre-lreg use, insn 141 (`p = q`) in the loop-1 EXIT TAIL, a different block, so combine gets no link to it and never considers folding it; the base add still reads reg79 at combine time and is not a substitution candidate either.
- probe: Read flow.c:2085-2106 and combine's LOG_LINK dependence in the shipped 2.7.2 tree, then confirmed against F_combine.txt for the candidate (insn 83 intact, insn 89 unchanged), and tested the rule from both sides with cells C1 (dest used only in the loop body -> copy survives) and D1 (dest's use hoisted into the same block -> copy deleted).
- result: Confirmed in both directions. This is the general survival law for this whole class of construct and it supersedes s9's narrower 'the dest needs a second use downstream of the base add' formulation - what matters is that the use is in ANOTHER BLOCK, not merely downstream.
- verdict: CONFIRMED

## [s15] s13/s14's frontier claim that loop 2's preheader 'has no C-level handle at all' is false: loop.c's move_movables can be made to place a genuine reg-reg copy into that exact block from pure C.
- mechanism: loop.c hoists a loop-invariant insn by duplicating it before loop_start (loop.c:1690-1710), so a loop-invariant reg-reg copy written inside loop 2's BODY is emitted as a `move` into loop 2's preheader. Because its uses stay inside the loop body - a different basic block - the survival law above applies and combine cannot fold it.
- probe: Cell C1: `b2 = base;` as the first statement of loop 2's body, with the body's element read and the do-while condition rewritten onto b2. Scored with `sandbox --disable all` and dumped with the instrumented cc1.
- result: The copy IS emitted, in target's block: loop 2's preheader becomes `lw $2,12($18) / lw $6,16($18) / addu $4,$5,$2 / move $5,$4`. Score 6 - the copy is of the BASE rather than of the ADDEND, so it lands one slot after the base add instead of before it and adds an instruction target does not have. Mechanism CONFIRMED; the specific cell is dead and banked.
- verdict: CONFIRMED

## [s15] Naming loop 2's addend and making BOTH the copy and the base add loop-2-body invariants would reproduce target's copy-then-base-add order in the preheader.
- mechanism: loop.c hoists both insns before loop_start, so the copy and its only use land in the SAME basic block; the survival law is violated, flow builds a LOG_LINK, and combine substitutes the copy into the base add and deletes it.
- probe: Cell D1 (`q2 = *(u8 **)(ctx + 0xC);` in the preheader, then `b2 = q2;` and `base = (u8 *)(sh2 + (s32)b2);` both inside the loop body) and cell D2 (copy invariant in the body, base add left in the preheader), scored and D1 additionally dumped.
- result: D1 = 3 with a loop-2 preheader byte-identical to the candidate's (`lw $2,12($18) / lw $6,16($18) / addu $4,$5,$2`) - the copy is gone, exactly as the law predicts. D2 = 9. C2 (invariant RE-COMPUTE of the base rather than a copy) = 5. KILLED.
- verdict: KILLED

## [s15] The 'loop 2's base addend is a copy or alias of the carried pointer p' family is merely expensive (s11/s12 recorded 8/9/19/28/41 as bare numbers) and might be recoverable with the right chassis.
- mechanism: It is structurally excluded, not expensive. If loop 2's base addend is provably equal to the loop-2 guard's addend, cse CSEs `sh2 + addend` and emits it ONCE; target emits it TWICE (`addu v0,a1,a0` at 0x8001791C for the guard, `addu a0,a1,a3` at 0x80017938 for the base). Any such spelling therefore deletes an instruction target has, before register allocation is even reached.
- probe: Cells A1 (`r = p;` in the join block, guard on p, base on r), A2 (guard on r, base on p), A3 (base reuses p directly), A4 (`r = p;` inside the preheader); A1 additionally dumped with the instrumented cc1.
- result: All four score exactly 8, and A1's asm shows the collapse literally: `.L171: sll $2,$20,6 / addu $4,$2,$4 / lw $2,32($4)`, where `addu $4,$2,$4` serves as BOTH the guard address and the base. KILLED with a mechanism. Corollary for every future session: loop 2's base addend MUST stay a fresh `*(u8 **)(ctx + 0xC)` read, which is what the candidate already does.
- verdict: KILLED

## s16 (2026-08-18, forensics) — hypotheses

### H-s16-1 KILLED — combine refuses to fold a preheader copy whose destination has more than one SET (reg_n_sets > 1)

STATEMENT.  Target's loop-2 copy destination `a3` is SET twice (0x800178D0 and
0x80017930) and USED twice (0x800178D8 and 0x80017938) — one value with two
definitions, i.e. exactly what a single C local serving as the base addend in
BOTH preheaders would produce.  If combine declines to substitute a register with
two sets, that single fact would explain both of target's surviving copies at
once, with no out-of-block use needed and no extra instruction anywhere.

MECHANISM.  combine.c gates several rewrites on `reg_n_sets[regno] == 1`
(combine.c:1561, :1764, :1830, :4277, :6889, :7227, :9981, :10033); flow.c:2079
increments `reg_n_sets` per set.  A two-set pseudo would therefore be excluded
from the substitution that deletes the copy insn.

PROBE.  Cells E1 (V1 chassis) and E2 (symmetric chassis): one local `q` is the
base addend in loop 1's preheader AND in loop 2's preheader.  Scored, then E2
dumped with the instrumented cc1 (`s16/iE2/F_*.txt`).

RESULT.  E1 = 13, E2 = 4, E6 = 9, E7 = 33.  In E2's dumps `q` is reg80, set at
insn 83 (loop 1) and insn 162 (loop 2) — reg_n_sets = 2 — and combine STILL
deleted insn 83 (`(note 83 81 86 "" NOTE_INSN_DELETED)`, F_combine.txt:198) and
rewrote insn 89's addend back to reg79.  VERDICT: KILLED.  The two-set property
is not a defence; target's `a3` being multiply-set is a coincidence of allocation,
not the mechanism.

### H-s16-2 KILLED — `optimize_reg_copy_2` produces target's loop-2 preheader copy (the s15 frontier's item #1)

STATEMENT.  s15 named `optimize_reg_copy_2` (local-alloc.c:874) as the ONLY
remaining GCC 2.7.2 routine that can leave a physically redundant reg-reg copy in
the stream after combine, and asked for its predicate to be written from C: a
copy `D = S` where S dies, followed in the same block (before any label, jump,
NOTE_INSN_LOOP_BEG or NOTE_INSN_LOOP_END) by a reverse copy `S = D` where D dies.

MECHANISM.  It rewrites D to S over the range and removes both REG_DEAD notes,
and — because nothing after local-alloc runs DCE — leaves both copies in place.

PROBE.  (i) Read local-alloc.c:874-935 to establish exactly what it emits.
(ii) Cells G1 (`r = *(u8 **)(ctx + 0xC); q = r; r = q; base = sh2 + r;`) and G2
(the same round trip spelled through the live locals `p`/`q`, as the frontier
specified).

RESULT.  (i) It leaves BOTH copy insns, so it always emits TWO move instructions
in the block.  Target's loop-2 preheader has exactly ONE move.  The routine is
analytically incapable of producing target's shape regardless of the C.
(ii) G1 = 13, G2 = 13.  VERDICT: KILLED, twice over.  CONSEQUENCE: no named 2.7.2
routine creates a redundant copy after combine, so target's copy must be a
PRE-combine copy that combine declined to fold — i.e. arm (b) of the trichotomy
with the LOG_LINK absent or the substitution rejected.  Every future hypothesis
on this residual must explain a combine REFUSAL, not a post-combine creation.

### H-s16-3 KILLED — a loop-invariant copy of the ADDEND, hoisted by loop.c, lands before the base add and is re-pointed into it (the s15 frontier's item #2)

STATEMENT / MECHANISM as recorded in the s15 frontier: loop.c's `move_movables`
emits invariants into the preheader in discovery order, so an invariant copy of
the addend used earlier in the body than the base's uses would be emitted BEFORE
the base add, meeting `optimize_reg_copy_1`'s precondition.

PROBE.  The two body uses the frontier named: (a) the do-while limit rewritten as
`*(s32 *)(sh2 + (s32)q + 0x20)` (cell F1); (b) the element read rewritten as
`*(u8 *)(sh2 + (s32)q + i + 0x2C)` (cell F2).

RESULT.  F1 = 15, F2 = 16.  Both re-materialise `sh2 + q` rather than folding it
back into the existing giv.  VERDICT: KILLED.  With s15's C1 (6), C2 (5), D1 (3),
D2 (9) the loop.c lever on loop 2's preheader is fully enumerated and dead.

### H-s16-4 KILLED with a control — loop 1's guard-clobber lever, transplanted to loop 2, rescues the source-copy addend family

STATEMENT.  E-s15-6 killed the source-copy addend family (A1-A4 = 8) because cse
merges loop 2's guard-address add with the base add.  Loop 1 does not suffer that
merge because its guard reuses ONE local (`t = sh + (s32)p; t = *(s32 *)(t +
0x1C);`), whose clobber invalidates cse's hash entry (the s8 lever).  Transplant
the clobber into loop 2 and the source copy should survive.

PROBE.  K1/K2/K3/K5 (t-reuse guard in loop 2 + `q = p;` / `r = p;` source copy,
with and without a links local, on both chassis) plus K4, the CONTROL: the
t-reuse guard alone with the unchanged fresh-read addend.

RESULT.  K1 = 35, K2 = 32, K3 = 32, K5 = 36, K4 (control) = 36.  The control
carries essentially the whole cost, so the guard change alone is worth about -33
and the compounds never tested the copy question.  VERDICT: KILLED — loop 2's
guard must stay the inline one-expression form; the s8 lever is loop-1-only.

### H-s16-5 CONFIRMED — the preheader-copy survival trichotomy, and cse (not flow) is the DCE that kills the unused arm

STATEMENT.  A preheader reg-reg copy's fate is decided entirely by where its
destination is used: unused -> deleted; used in-block -> deleted by combine; used
out-of-block -> survives, and the use costs an instruction.

MECHANISM.  flow.c:2102 builds a LOG_LINK only when `BLOCK_NUM (y) == blocknum`,
and combine only follows LOG_LINKS (E-s15-2).  For the unused arm the ledger has
assumed flow.c's dead-store elimination; the dump shows cse removes it first.

PROBE.  Cell I1 (`q = p;` in loop 2's preheader, base add left on a fresh inline
read) scored and dumped; `reg/v:SI 80` counted per pass.

RESULT.  .rtl 4, .jump 4, .cse 3, .loop 3, .cse2 3, .flow 3, .combine 3 — the
copy disappears AT cse.  Score 3, identical to the base, i.e. completely inert.
VERDICT: CONFIRMED, with the pass corrected to cse.

### H-s16-6 CONFIRMED (and newly opened) — the loop-1 exit tail's spelling flips cse's canonicalisation of the base add's addend

STATEMENT.  Whether the candidate's loop-1 copy survives is decided by cse, not by
anything later: cse either leaves the ORIGINAL pseudo in the base add (copy
orphaned, no LOG_LINK, survives) or substitutes the COPY DEST there (LOG_LINK,
combine deletes it).  The C-level handle on that choice is the loop-1 EXIT TAIL —
a statement DOWNSTREAM of the insn whose operand changes.

MECHANISM.  cse canonicalises a register to `qty_first_reg` of its equivalence
class.  Both cells fold the redundant read into `(set reg80 reg79)` at insn 83 and
so merge reg80 into reg79's class; something about the tail's re-definition of
reg79 changes which member the class is named by at insn 89.  The precise cse
sub-rule is NOT yet pinned.

PROBE.  Diff of two instrumented dumps with identical insn numbering:
`s15/icand/F_cse.txt:177` (tail `p = q;`) vs `s16/iE2/F_cse.txt:178`
(tail `p = *(u8 **)(ctx + 0xC);`), with the downstream consequence read out of
`s16/iE2/F_combine.txt:196-210`.

RESULT.  Candidate: insn 89 = `(plus (reg 84) (reg 79))`, copy survives.
E2: insn 89 = `(plus (reg 84) (reg 80))`, copy deleted by combine, loop 1 emits
one instruction fewer than target.  VERDICT: CONFIRMED as a control channel; the
cse-internal reason is the next forensics question.

## [s16] Target's two preheader copies are explained by combine refusing to substitute a pseudo with two SETs (reg_n_sets > 1), which one shared addend local would produce.
  - KILLED.  Cell E2 has `q` set in both preheaders and combine deleted the loop-1 copy anyway (F_combine.txt:198).  E1 = 13, E2 = 4, E6 = 9, E7 = 33.

## [s16] optimize_reg_copy_2 (local-alloc.c:874) creates target's loop-2 preheader copy; its predicate has simply never been written from C (the s15 frontier's item #1).
  - KILLED analytically and by measurement.  The routine leaves BOTH copy insns, so it can only ever emit two moves where target has one; and both C spellings of its predicate score 13 (G1, G2).

## [s16] A loop-invariant copy of the loop-2 ADDEND, hoisted by loop.c's move_movables and consumed inside the body, lands before the base add and is re-pointed into it by optimize_reg_copy_1 (the s15 frontier's item #2).
  - KILLED.  F1 (limit through the copy) = 15, F2 (element read through the copy) = 16.

## [s16] Loop 1's guard-clobber lever (the s8 two-step `t`), transplanted into loop 2, blocks the cse merge that killed the source-copy addend family and lets a `q = p;` copy survive in loop 2's preheader.
  - KILLED with a control.  K1 = 35, K2 = 32, K3 = 32, K5 = 36 and the control K4 (guard change alone) = 36, so the guard change carries the entire cost.

## [s16] A preheader reg-reg copy whose destination is never used survives long enough for local-alloc to see it (the s15 frontier assumed flow.c's DCE was the gate).
  - KILLED / CORRECTED.  Cell I1's copy disappears at CSE (`reg/v:SI 80` counts .rtl 4 / .jump 4 / .cse 3 onward), three passes before flow, and the cell is inert at 3.

## [s16] The residual is decided by a single cse canonicalisation at the loop-1 base add, and the loop-1 exit tail's spelling is the C-level control channel for it.
  - CONFIRMED by a two-dump diff at identical insn numbers (s15/icand/F_cse.txt:177 vs s16/iE2/F_cse.txt:178).  The cse sub-rule that makes a DOWNSTREAM redefinition change `qty_first_reg` at an UPSTREAM insn is the open question this hands to the next session.

## [s16] Target's two surviving preheader copies are explained by combine refusing to substitute a pseudo that has more than one SET (reg_n_sets > 1) - which is exactly what ONE shared C local serving as the base addend in BOTH preheaders would produce, mirroring target's a3 (set at 0x800178D0 and 0x80017930, used at 0x800178D8 and 0x80017938).
- mechanism: combine.c gates several rewrites on reg_n_sets[regno] == 1 (combine.c:1561, :1764, :1830, :4277, :6889, :7227, :9981, :10033) and flow.c:2079 increments reg_n_sets per set, so a two-set pseudo would be excluded from the substitution that deletes the copy insn.
- probe: Cells E1 (V1 chassis) and E2 (symmetric chassis) make one local `q` the base addend in loop 1's preheader AND loop 2's preheader; E6/E7 add a loop-2 links local. Scored with sandbox --disable all, then E2 dumped with the instrumented cc1 (tmp/grind/func_80017848/s16/iE2/F_*.txt).
- result: E1 = 13, E2 = 4, E6 = 9, E7 = 33. In E2's dumps `q` is reg80, set at insn 83 (loop 1) and insn 162 (loop 2) - reg_n_sets = 2 - and combine STILL deleted insn 83 ((note 83 81 86 "" NOTE_INSN_DELETED), F_combine.txt:198) and rewrote insn 89's addend back to reg79. Sharing the addend local is neutral on the symmetric chassis and a regression on V1.
- verdict: KILLED

## [s16] local-alloc's optimize_reg_copy_2 creates target's loop-2 preheader copy; s15 named it the ONLY remaining GCC 2.7.2 routine able to leave a physically redundant reg-reg copy after combine, and its predicate had never been written from C.
- mechanism: optimize_reg_copy_2 (tools/gcc-2.7.2/local-alloc.c:874, dispatched at :1015) fires on (set D S) where S dies in the copy, scans forward in the same block (stopping at CODE_LABEL / JUMP_INSN / NOTE_INSN_LOOP_BEG / NOTE_INSN_LOOP_END) for a reverse copy (set S D) where D dies, replaces D with S over the range, removes both death notes, and deletes NEITHER copy insn - and nothing after local-alloc runs DCE.
- probe: (i) Read local-alloc.c:874-935 to establish exactly what the routine emits. (ii) Cells G1 (`r = *(u8**)(ctx+0xC); q = r; r = q; base = sh2 + r;`) and G2 (the same round trip spelled through the live locals p/q, exactly as the s15 frontier specified).
- result: (i) It leaves BOTH copy insns, so any function it fires on emits TWO move instructions in that block; target's loop-2 preheader contains exactly one move (addu $a3,$a0,$zero at 0x80017930). It is analytically incapable of producing target's shape regardless of the C. (ii) G1 = 13, G2 = 13. Consequence: no named 2.7.2 routine creates a redundant copy after combine, so target's copy must be a PRE-combine copy that combine DECLINED to fold.
- verdict: KILLED

## [s16] A loop-invariant copy of the loop-2 ADDEND, written inside loop 2's body so loop.c hoists it into the preheader AHEAD of the base add, is re-pointed into the base add by optimize_reg_copy_1 and reproduces target's copy-then-base-add order (the s15 frontier's item #2).
- mechanism: loop.c's move_movables (loop.c:1690-1710) duplicates each invariant before loop_start in discovery order, so an invariant copy of the addend written earlier in the body than the base's uses is emitted earlier in the preheader; optimize_reg_copy_1's precondition (SRC alive at the copy, dying at the base add, same block) would then be met.
- probe: The two body uses the s15 frontier named: cell F1 rewrites the do-while limit as *(s32 *)(sh2 + (s32)q + 0x20) so the body consumes the copy; cell F2 rewrites the element read as *(u8 *)(sh2 + (s32)q + i + 0x2C).
- result: F1 = 15, F2 = 16. Both re-materialise the sh2 + q add instead of folding it back into the existing giv. With s15's C1 (6), C2 (5), D1 (3) and D2 (9) the loop.c/move_movables lever on loop 2's preheader is now fully enumerated and dead: it can PLACE a copy there, but never a copy of the addend whose destination is free.
- verdict: KILLED

## [s16] Loop 1's guard-clobber lever - the s8 two-step local `t = sh + (s32)p; t = *(s32 *)(t + 0x1C);` - transplanted into loop 2 blocks the cse merge that killed the source-copy addend family (E-s15-6), letting a source-level `q = p;` copy survive in loop 2's preheader.
- mechanism: E-s15-6 measured that when loop 2's base addend is provably equal to the guard's addend, cse merges the guard-address add and the base add into ONE insn while target computes sh2+ptr twice. Clobbering the guard's address pseudo invalidates cse's hash entry for sh2+p, which is precisely why loop 1 does not suffer that merge.
- probe: Cells K1 (t-reuse guard + `q = p;`), K2 (+ loop-2 links local), K3 (separate local `r`), K5 (on the symmetric chassis), and critically the CONTROL K4: the t-reuse guard alone with the unchanged fresh-read addend.
- result: K1 = 35, K2 = 32, K3 = 32, K5 = 36, K4 (control) = 36. The control carries essentially the entire cost, so the two-step `t` guard local in loop 2 is worth about -33 by itself and the compounds never tested the copy question. Loop 2's guard must stay the inline one-expression form; the s8 lever is loop-1-only.
- verdict: KILLED

## [s16] A preheader reg-reg copy's fate is decided entirely by WHERE its destination is used - and for the unused arm the deleting pass is CSE, not flow.c's dead-store elimination as the ledger has assumed since s15.
- mechanism: flow.c:2102 builds a LOG_LINK only when BLOCK_NUM(y) == blocknum and combine only follows LOG_LINKS (E-s15-2), so an out-of-block use hides the copy from combine; a dest with no use at all is removed earlier still, by cse, which drops the set while building its equivalence classes.
- probe: Cell I1 writes `q = p;` into loop 2's preheader and leaves the base add on a fresh inline *(u8 **)(ctx + 0xC) read, so the copy's dest is never read. Scored, then dumped with the instrumented cc1 and `reg/v:SI 80` counted per pass (tmp/grind/func_80017848/s16/iI1/F_*.txt).
- result: Occurrences: .rtl 4, .jump 4, .cse 3, .loop 3, .cse2 3, .flow 3, .combine 3 - the copy disappears AT cse, three passes before flow ever sees it. Score 3, identical to the base, i.e. the construct is completely inert. Trichotomy now complete: unused -> cse deletes; used in-block -> combine deletes; used out-of-block -> survives and the use costs an instruction.
- verdict: CONFIRMED

## [s16] The whole 3-instruction residual is decided by ONE cse canonicalisation at loop 1's base add, and the C-level control channel for it is the loop-1 EXIT TAIL - a statement DOWNSTREAM of the insn whose operand changes.
- mechanism: cse canonicalises a register to qty_first_reg of its equivalence class. Both chassis fold the redundant read into (set reg80 reg79) at insn 83 and so merge reg80 into reg79's class; something about the tail's re-definition of reg79 changes which member the class is named by at insn 89. If cse leaves the ORIGINAL there, the copy is orphaned, gets no LOG_LINK, survives combine, and optimize_reg_copy_1 later re-points the base add onto it (E-s15-1). If cse substitutes the COPY DEST, combine deletes the copy.
- probe: Diff of two instrumented-cc1 dumps with identical insn numbering and character-identical loop-1 preheader C: tmp/grind/func_80017848/s15/icand/F_cse.txt:177 (tail `p = q;`) vs tmp/grind/func_80017848/s16/iE2/F_cse.txt:178 (tail `p = *(u8 **)(ctx + 0xC);`), with the downstream consequence read out of s16/iE2/F_combine.txt:196-210.
- result: Candidate: insn 89 = (set (reg 81) (plus (reg 84) (reg 79))) - the ORIGINAL - and the copy survives to the assembler as target's addu $a3,$a0,$zero. E2: insn 89 = (plus (reg 84) (reg 80)) - the COPY DEST - insn 83 becomes NOTE_INSN_DELETED, and loop 1 emits one instruction FEWER than target (lw $6,16($18) / addu $4,$5,$4 / addu $2,$4,$3), which is exactly s12's unexplained 126-vs-127 observation, now with the producing pass named.
- verdict: CONFIRMED

## s17 (escalation modality) — the last un-tried combine refusal path

### H-s17-A  KILLED (with two controls)
STATEMENT: target's loop-2 preheader copy survives combine because the copy's
SOURCE pseudo is re-set between the copy and the base add, tripping
`can_combine_p`'s `use_crosses_set_p (src, INSN_CUID (insn))` guard — a refusal
path never enumerated in sixteen prior sessions. The zero-cost C carrier is to
reuse the pointer local `p` to hold loop 2's links pointer, since the links load
(`lw $a2,0x10($s2)`) is the one instruction target already has between the copy
and the base add.
MECHANISM: combine.c:910-916. For a non-adjacent i2/i3 pair, combine refuses the
substitution if any register mentioned in i2's source is set between i2 and i3,
because the substituted value would no longer be the value that was copied.
PROBE: cells P1/P2/P3 (copy into `r` / `q` / `lnk`, each with `p` reused for the
links pointer) plus controls P4 (copy alone) and P5 (p-reuse alone), scored with
tmp/grind/func_80017848/s17/score.sh against a re-measured A_base = 3.
RESULT: P1 14, P2 14, P3 12, P4 8, P5 14, P7 12, P8 12.
VERDICT: **KILLED.** The control P5 shows the carrier alone costs 11 points; the
lever's maximum return is 2. The mechanism may well fire, but it can never be
bought on this chassis. Same failure shape as E-s16-6's guard-clobber transplant.

### H-s17-B  CONFIRMED (analytic, exhaustive)
STATEMENT: the combine.c refusal set for this copy is finite, enumerable, and now
fully classified — there is no eighth path for a future session to try.
MECHANISM / RESULT: see E-s17-1. Two paths are forbidden cheat families (volatile
coercion; REG_NO_CONFLICT/DImode chains), three are structurally unreachable (no
CALL in the preheader region, no autoinc on MIPS, no hard-reg/PARALLEL/libcall
forms for a plain pointer copy), one (out-of-block use) is priced dead at 2-for-2
with no free use site, and the last is H-s17-A.
VERDICT: **CONFIRMED.** s16 frontier item #2 is closed. Combined with E-s16-4
(no post-combine routine can create the copy) and E-s16-1 (the unused and
out-of-block arms are excluded by target's own asm), every named producer and
every named protector of target's loop-2 preheader copy is now measured or
analytically dead.

## [s17] Target's loop-2 preheader copy survives combine because the copy's SOURCE pseudo is re-set between the copy and the base add, tripping can_combine_p's use_crosses_set_p guard - a refusal path never enumerated in sixteen prior sessions. The zero-cost C carrier is to reuse the pointer local `p` to hold loop 2's links pointer, since the links load `lw $a2,0x10($s2)` is the one instruction target already has between the copy and the base add.
- mechanism: tools/gcc-2.7.2/combine.c:910-916. For a non-adjacent i2/i3 pair, can_combine_p returns 0 when use_crosses_set_p(src, INSN_CUID(insn)) holds, i.e. when any register mentioned in the copy's source is set between the copy and the insn being combined into - the substituted value would no longer be the value that was copied. E-s16-1(b) had shown combine otherwise ALWAYS deletes a same-block copy here.
- probe: Cells P1 (copy into a fresh local `r` + p reused for links), P2 (copy into `q`), P3 (copy into `lnk`), plus controls P4 (source copy alone, links left inline in the body) and P5 (p reused for links alone, fresh-read addend), and compounds P7/P8 (target-shaped loop-1 exit tail + the P1/P2 loop 2). Scored with tmp/grind/func_80017848/s17/score.sh against a re-measured A_base = 3.
- result: A_base 3, P1 14, P2 14, P3 12, P4 8, P5 14, P7 12, P8 12.
- verdict: KILLED

## [s17] The combine.c refusal set for this copy is finite, enumerable and exhaustively classifiable - there is no eighth path left for a future session to try.
- mechanism: can_combine_p (combine.c:803-970) was read end to end; every early `return 0` was classified against the shape (set D S) with D and S pseudos and D's only use the same-block base add.
- probe: Source enumeration cross-checked against target's own asm (asm/funcs/func_80017848.s:64-67 - copy / lw links / base add, no call, no autoinc, no multiword op) and against the ledger's priced measurements for the out-of-block-use arm.
- result: Seven paths. R5 (volatile src / ASM_OPERANDS / volatile insn between) and R6 (REG_NO_CONFLICT, emitted only for DImode multiword ops) are forbidden cheat families and would also add instructions target lacks. R4 (INSN_CUID < last_call_cuid) needs a CALL in the preheader - none exists and one is not C-reachable without changing semantics. R7 (PARALLEL/CLOBBER, FIND_REG_INC_NOTE, REG_RETVAL libcall, ZERO_EXTRACT/STRICT_LOW_PART, stack-pointer dest, hard-reg/REG_USERVAR_P) is unreachable for a plain pointer copy pre-RA. R1 degenerates to R2. R2 (dest live after i3) is the candidate's own loop-1 lever, priced dead for loop 2 across s9/s10/s16 at 2-spent-for-2-returned with every post-loop-2 use site measured at 19-22. R3 is the killed hypothesis above.
- verdict: CONFIRMED

## [s17] Writing C that mirrors target's instruction sequence one-for-one in the residual region is closer to the target bytes than the candidate.
- mechanism: Target's loop-1 exit tail is a fresh `lw $a0,0xC($s2)` (asm/funcs/func_80017848.s:56) and its loop-2 preheader is copy / lw links / base add. The candidate instead spends the loop-1 tail on an out-of-block use (`p = q` -> `move $4,$7`) and reads ctx+0xC freshly in loop 2's preheader.
- probe: Cells P7 and P8: loop-1 exit tail written as `p = *(u8 **)(ctx + 0xC);` (target's re-read) combined with a loop-2 preheader copy plus the links re-set.
- result: P7 = 12, P8 = 12, against the candidate's 3.
- verdict: KILLED

## s18 (modality `rederive`)

### H-s18-1 — KILLED
**Statement.** A different m2c structuring configuration yields a source-level
decomposition of the two scan loops that is not a descendant of the s8/s9
lineage, and that raw re-derivation scores at or below 12 (the s17 frontier's
own threshold for keeping the lineage question open).
**Mechanism.** m2c's structuring stage (control-flow reconstruction, &&/||
detection, pass count, register-variable mode) is configuration-driven, so a
different configuration should re-associate the loops and the guards differently
and hand us a whole-function chassis nobody has authored.
**Probe.** Five configurations run (`tmp/grind/func_80017848/s18/m2c.sh`):
default, `--no-andor`, `--passes 2`, `--gotos-only`, `--reg-vars`. Outputs
diffed against each other; the `--no-andor` output transcribed faithfully into
compilable BB2 C (cell R1) and scored; the CFG half and the loop-form half
isolated as cells R2 and R3.
**Result.** `--passes 2` is byte-identical to default; `--no-andor` and
`--gotos-only` differ only in guard/exit spelling; `--reg-vars` is semantically
broken. R1 = **53**, R2 = **15**, R3 = **49**, against the candidate's 3. The
50-point gap is +12 (goto-structured single exit) and +46 (walking-pointer
rotated loops), essentially additive.
**Verdict. KILLED** — and with it s17 frontier item #2. There is no second m2c
reading, and the one reading there is is 4-18x worse than the lineage chassis on
two independently priced axes.

### H-s18-2 — KILLED
**Statement.** Writing target's own walking element pointer explicitly at loop 2
(`e = base + i`, initialised in the preheader and updated after `i++`) fixes the
preheader's register identities, because target's preheader literally ends with
`addu $v0,$a0,$v1` and its loop closes with the same insn in the delay slot.
**Mechanism.** loop.c strength-reduces the candidate's `base + i + 0x2C`
addressing into exactly that walking pointer; hoisting the initialisation into C
should make the preheader's insn order match target's rather than being
scheduled from the folded form.
**Probe.** Cell W2 — candidate chassis, loop 2 only, explicit `u8 *e`.
**Result.** **14**, at the correct instruction count 127/127. All 11 points are
register identity, so the explicit pointer changes allocation, not shape.
**Verdict. KILLED.** With s12's loop-1 measurement (13) and this session's R3
(both loops, 49) the explicit-element-pointer family is closed at every site.

### H-s18-3 — CONFIRMED (prior art), but with no carrier here
**Statement.** The residual's idiom — a preheader reg-reg copy whose destination
is consumed by the following base add, with the source dead — is producible from
ordinary pure C by this compiler class, and a matched example exists in the
decomp.me corpus that names its origin.
**Mechanism.** Mine the 3,754-scratch local corpus for matched (score 0)
scratches whose target asm contains the copy→add pair, then read their C.
**Probe.** `tmp/grind/func_80017848/s18/mine_copy_idiom.py` and `mine_exact.py`.
**Result.** 163 copy→add hits in matched scratches, 50 with the source dead, 28
with target's exact copy-then-redefine-source pair. Most are call-return staging
or constant-multiply expansions. The structural twin is scratch **19TpT**
(`func_8009C6D8`, gcc2.7.2-cdk `-O2 -G0 -g2`, matching): the copy is an INNER
loop's induction-base initialised from an OUTER loop's live row-base, inside a
plain `for (row) for (col)` nest. The consumer is in the inner loop body — a
different block — so it survives combine by s16's out-of-block leg, and it is
free because the outer loop keeps the source live regardless.
**Verdict. CONFIRMED as prior art / NO CARRIER HERE.** func_80017848 has no
nested loop and nothing live across loop 2 (s11: three fresh `ctx+0xC` re-reads
in the tail; routing any of them through a live local = 19-22). Manufacturing an
enclosing loop is the duplicated-region family, priced at 35 by s12. The
positive value of this result is evidentiary: the idiom is compiler-producible
pure C, which independently corroborates `scan_hand_coded` = LOW 0/8 and rules
out "this is hand asm" as an explanation of the residual.

### H-s18-4 - KILLED
**Statement.** A transplantable sibling exists: either another BB2 function whose
whole shape is close enough to func_80017848's that its already-matched C can be
adapted, or a Kengo (PS2, shared "Marionation" engine) equivalent whose source
decomposition can be re-derived from. This is the third and last leg of the
rederive modality (fresh m2c = H-s18-1, external corpus = H-s18-3).
**Mechanism.** Sibling functions compiled from sibling source in the same TU
share the programmer's decomposition, so a matched sibling's spelling of the same
seam is proven-good C on this exact chassis; and Kengo, sharing the engine, would
carry the original source shape of the same routine.
**Probe.** (a) 5-gram opcode-sequence census of all 1,437 `asm/funcs/*.s` bodies
in the 0.6x-1.8x size band (`tmp/grind/func_80017848/s18b/sibling_scan.py`).
(b) Whole-binary scan for the residual's exact seam fingerprint, filtered to
functions that are pure-C-defined, rule-free and not queue-active (`fp_scan.py`,
`fp_exact.py`, `fp_exact2.py`). (c) Kengo channel: inspect `Kengo/`,
`kengo_matches.csv`, `Kengo/kengo_functions_full.txt`, and
`tools/kengo_match.py --bb2 func_80017848`.
**Result.** (a) Maximum whole-function overlap **0.120**; no sibling body exists.
(b) The exact fingerprint occurs in **3 of 1,437** functions - func_80017848
(both loops), func_800200DC (call-return staging, 14 regfix RA rules) and
func_8005E54C (still asm, distance 799). Widened to any consumer over matched
rule-free pure-C functions: 31 hits, every one call-return or shift staging, none
a loop preheader. (c) Kengo ships debug SYMBOLS ONLY - no C source anywhere in
the tree - and has no symbol resembling a pair/link registration in the plausible
modules; `kengo_matches.csv` has no row for this function.
**Verdict. KILLED**, and the rederive modality is now spent on all three of its
legs. The one positive by-product is evidentiary: the only in-tree spelling of
the seam idiom that a compiler demonstrably produces from pure C depends on a
preceding CALL, which is s17's R4 refusal leg - so the census independently
confirms E-s17-1's R4 classification instead of opening an eighth path.

### H-s18-5 - KILLED
**Statement.** Writing target's loop-1 exit edge exactly as target materialises
it - BOTH `lw $a0,0xC($s2)` and `sll $a1,$s4,6`, i.e. a fresh `ctx + 0xC` read
AND the shift recomputed on the exit edge, with `sh2` deleted so loop 2's guard
and base share the recomputed `sh` - puts the join block's inputs in target's
registers and lets loop 2's preheader emit target's copy.
**Mechanism.** The join `.L8001791C` is reached from two predecessors and its
guard address add reads whatever the exit edge left live. If both live values are
produced on that edge in target's order, the register identities entering the
copy/base pair should follow.
**Probe.** Cell X1 (`tmp/grind/func_80017848/s18b/X1.c`).
**Result.** **4 at 126/127 instructions** - loop 1 one instruction SHORT, the
E-s16-2 signature (a fresh read downstream of loop 1's base add makes cse
canonicalise the base add onto the copy dest; combine then deletes the copy).
Identical to s12's symmetric chassis (4/126).
**Verdict. KILLED**, with a durable side-result: the shift's PLACEMENT is inert
(exit edge vs just before loop 2's guard scores the same), so the symmetric
chassis's +1 over the candidate is entirely its tail read, never the shift site.

## [s18] A transplantable sibling exists for func_80017848: either another BB2 function whose whole shape is close enough that its already-matched pure C can be adapted, or a Kengo (PS2, shared Marionation engine) equivalent whose source decomposition can be re-derived. This is the third and last leg of the rederive modality (fresh m2c and the external decomp.me corpus were the first two, both spent).
- mechanism: Sibling functions compiled from sibling source in the same translation unit share the programmer's decomposition, so a matched sibling's spelling of the same seam would be proven-good C on this exact chassis; and Kengo, sharing the engine, would carry the original source shape of the same routine.
- probe: (a) 5-gram opcode-sequence census of all 1,437 asm/funcs/*.s bodies in the 0.6x-1.8x instruction-count band (tmp/grind/func_80017848/s18b/sibling_scan.py). (b) Whole-binary scan for the residual's exact seam fingerprint - `addu D,S,$zero` followed within three insns and no label by a three-register `addu` consuming D and redefining S - filtered to functions that are pure-C-defined, rule-free and not queue-active (fp_scan.py, fp_exact.py, fp_exact2.py). (c) Kengo channel: inspect Kengo/, kengo_matches.csv, Kengo/kengo_functions_full.txt, and tools/kengo_match.py --bb2 func_80017848.
- result: (a) Maximum whole-function overlap 0.120 (func_8005BA8C, func_8006BD28); the top fifteen all sit between 0.096 and 0.120 - no near-duplicate body exists in the game. (b) The exact fingerprint occurs in exactly 3 of 1,437 functions: func_80017848 itself (twice, once per scan loop), func_800200DC (NOT a preheader - call-return staging in the shadow of `jal SquareRoot0`, and it carries 14 register-allocation regfix rules at regfix.txt:628) and func_8005E54C (still an asmfix replace_with_asmfile body, honest distance 799, queue-active, so no C exists to transplant). Widened to any consumer of the copy over matched, rule-free, pure-C-defined functions: 31 hits, every one call-return staging ($s0 = $v0 after a jal, then $v0 reused) or shift staging, NOT ONE a loop-preheader base copy. (c) Kengo ships the retail PS2 disc plus debug SYMBOLS only (name + size + source path in kengo_functions_full.txt) - there is no Kengo C source anywhere in the tree; kengo_matches.csv has no row for func_80017848; and grepping the whole Kengo symbol table for link|pair|near|regist, plus reading the 32-symbol src/ishito/is_coli.c list, finds nothing resembling a pair/link registration in the 120-150-instruction band.
- verdict: KILLED

## [s18] Writing target's loop-1 exit edge exactly as target materialises it - BOTH `lw $a0,0xC($s2)` and `sll $a1,$s4,6`, i.e. a fresh ctx+0xC read AND the shift recomputed on the exit edge, with sh2 deleted so loop 2's guard and base share the recomputed sh - puts the join block's inputs in target's registers and lets loop 2's preheader emit target's copy.
- mechanism: The join .L8001791C has two predecessors and its guard address add reads whatever the loop-1 exit edge left live; if both live values are produced on that edge in target's own order, the register identities entering the copy/base pair should follow.
- probe: Cell X1 (tmp/grind/func_80017848/s18b/X1.c), one sandbox --disable all run.
- result: 4 at 126/127 instructions - loop 1 comes out one instruction SHORT, which is precisely E-s16-2's signature: a fresh read downstream of loop 1's base add makes cse canonicalise the base add onto the copy dest, combine then deletes the copy, and loop 1 loses target's `addu $a3,$a0,$zero`. The score is identical to s12's symmetric chassis (4/126), which isolates a variable s12 could not measure: the shift's PLACEMENT is inert (exit edge vs just before loop 2's guard), so the symmetric chassis's +1 over the candidate is entirely its tail read.
- verdict: KILLED

## s19 (modality `synthesis`) — hypotheses

### H-s19-1 — KILLED (with a control)

**Statement.** A body use of loop 2's ADDEND that `loop.c` cannot hoist (because
it varies with the induction variable `i`) creates a preheader reg-reg copy of the
addend that survives combine by E-s16-1's arm (c), and therefore reproduces
target's `addu $a3,$a0,$zero` in the one block eighteen sessions could not reach.
This is the only clause-A carrier the ledger had never written: s15's C1 copied the
BASE, s15's D1 and s16's F1/F2 used loop-INVARIANT expressions that `move_movables`
hoisted into the preheader (same block ⇒ combine deletes, or the address changes).

**Mechanism.** `loop.c`'s `move_movables` only hoists loop-invariant sets, so an
`i`-dependent use of the addend must stay in the loop body — a different basic
block from the preheader — which by flow.c:2102 gives the copy no LOG_LINK and by
E-s16-1 makes it survive combine. `optimize_reg_copy_1` would then re-point the
base add onto the copy's destination, exactly as it does for loop 1.

**Probe.** Cells Z2 (loop-2 body element read written as
`*(u8 *)((s32)q2 + sh2 + i + 0x2C)` with a named preheader addend `q2`) and Z1 (the
same construct in loop 1), Z3 (both), plus controls Z4 (an INVARIANT limit read
through `q2`) and Z5 (the named addend alone). Each applied to the V1 candidate
body and scored with `sandbox func_80017848 --disable all`; Z2 and Z4 additionally
disassembled and diffed against the normalised target listing.

**Result.** Z1 = 6 (128 insns), Z2 = 6 (128), Z3 = 9 (129), Z4 = 5 (128),
Z5 = 3 (127, inert control). The copy IS created and DOES survive — but it is a
copy of `base`, not of the addend, and it lands one slot AFTER the base add:
`addu $a0,$a1,$v0 / addu $a1,$a0,$zero`. cse folds the loop-invariant part of
`q2 + sh2 + i` back into the base add, so the value the body consumes is `base`;
the addend itself has no surviving body use to anchor a copy to.

**Verdict.** KILLED. And the kill generalises: it reproduces s15's C1 from a
structurally different C construct, which is what licences E-s19-1's CLAUSE A as a
general rule rather than a per-cell observation.

### H-s19-2 — CONFIRMED (analytic synthesis over the whole ledger, no new measurement of its own)

**Statement.** The 3-instruction residual is the fixed point of two independently
proven constraints (E-s19-1's CLAUSE A and CLAUSE B), not an unfound spelling:
loop 1 can satisfy both clauses and does, at a net cost of 1; loop 2 fails CLAUSE A
structurally (its preheader is downstream of the join `.L8001791C`, so cse's
extended basic block cannot fold the redundant `ctx + 0xC` read there) and has no
CLAUSE B consumer that lands on an instruction target already has.

**Mechanism.** cse's extended basic blocks terminate at any label that can be
branched to (`cse_end_of_basic_block`); loop 1's skip branch `blez $v0,.L8001791C`
makes loop 2's guard exactly such a label. Combined with E-s16-1's trichotomy and
E-s15-1's `optimize_reg_copy_1` re-pointing, this fixes both the position and the
survival condition of any preheader copy writable from C.

**Probe.** Re-derivation over the full ledger (E-s15-1..7, E-s16-1..7, E-s17-1..2,
E-s18-5..8) plus this session's five cells; the model was required to predict Z1..Z5
before they were run, and it predicted all five (copy created, copy of `base`, one
slot late, +1 instruction each, inert control).

**Verdict.** CONFIRMED. Its operational value is the exit list in E-s19-3: exactly
three clauses can be broken (kill the join, find a free clause-B consumer, or find a
combine refusal on an in-block single use), and two of the three are already
measured dead.

## [s19] A body use of loop 2's addend that varies with the induction variable creates a surviving preheader copy — but cse rewrites the address back through `base`, so it is a copy of `base` and lands one slot after the base add (Z2 = 6, Z4 = 5, control Z5 = 3), independently reproducing s15's C1.

## [s19] The residual is a fixed point of two proven constraints (CLAUSE A: only a cse-folded redundant read at the addend's read point emits a copy BEFORE the base add; CLAUSE B: the copy's destination needs an out-of-block consumer, which always costs one instruction), and loop 2 fails CLAUSE A structurally because its preheader sits downstream of the join that loop 1's skip branch creates.

## [s19] A body use of loop 2's ADDEND that loop.c cannot hoist (because it varies with the induction variable i) creates a preheader reg-reg copy of the addend that survives combine by E-s16-1's arm (c), reproducing target's `addu $a3,$a0,$zero` in the one block eighteen sessions could not reach. This is the only clause-A carrier never written: s15's C1 copied the BASE, and s15's D1 / s16's F1-F2 used loop-INVARIANT expressions that move_movables hoisted into the preheader (same block, so combine deletes them).
- mechanism: loop.c's move_movables only hoists loop-invariant sets, so an i-dependent use of the addend must stay in the loop body - a different basic block from the preheader - which by flow.c:2102 gives the copy no LOG_LINK and by E-s16-1 makes it survive combine; local-alloc's optimize_reg_copy_1 would then re-point the base add onto the copy's destination, exactly as it does for loop 1.
- probe: Cells Z2 (loop-2 body element read as *(u8 *)((s32)q2 + sh2 + i + 0x2C) with a named preheader addend q2), Z1 (same in loop 1), Z3 (both), Z4 (control: an INVARIANT limit read through q2), Z5 (control: the named addend alone), each applied to the V1 candidate body and scored with `sandbox func_80017848 --disable all`; Z2 and Z4 also disassembled and diffed against the normalised target listing.
- result: Z1 = 6 (128 insns), Z2 = 6 (128), Z3 = 9 (129), Z4 = 5 (128), Z5 = 3 (127, inert). The copy IS created and DOES survive combine, but it is a copy of `base`, not of the addend, and lands one slot AFTER the base add: Z2's preheader emits `lw $v0,12($s2) / lw $a2,16($s2) / addu $a0,$a1,$v0 / addu $a1,$a0,$zero / addu $v0,$a1,$v1`. cse folds the loop-invariant part of q2 + sh2 + i back into the base add, so the value the body consumes is `base` and the addend has no surviving body use to anchor a copy to.
- verdict: KILLED

## [s19] The 3-instruction residual is the fixed point of two independently proven constraints, not an unfound spelling. CLAUSE A (POSITION): a preheader copy is emitted BEFORE the base add only when it is a cse fold of a redundant memory read at the addend's own read point. CLAUSE B (SURVIVAL): the copy's destination needs a consumer outside the preheader block, which always materialises as one instruction. Loop 1 satisfies both at a net cost of 1; loop 2 fails CLAUSE A structurally and has no free CLAUSE B site.
- mechanism: cse's extended basic blocks terminate at any label that can be branched to (cse_end_of_basic_block), and loop 1's skip branch `blez $v0,.L8001791C` (asm/funcs/func_80017848.s:62) makes loop 2's guard exactly such a label - so the redundant ctx+0xC read in loop 2's preheader can never be folded and stays `lw $v0,12($s2)`, which is literally residual instruction #2. Combined with E-s16-1's survival trichotomy and E-s15-1's optimize_reg_copy_1 re-pointing, this fixes both the position and the survival condition of every preheader copy writable from C.
- probe: Re-derivation over the whole ledger (E-s15-1..7, E-s16-1..7, E-s17-1..2, E-s18-5..8) plus this session's five cells; the model was required to predict Z1..Z5 before they were run.
- result: The model predicted all five outcomes correctly (copy created, copy of `base`, one slot late, +1 instruction each, and an inert control), and it accounts for the whole rejected bank: every form is a failure of CLAUSE A (position) or CLAUSE B (price), including the previously unexplained spread 8/9/13/19/28/32/35/41/52.
- verdict: CONFIRMED

---

## s20 (structural) — hypotheses

**H-s20-1 — s19 frontier (b): a clause-B consumer for loop 2's addend that lands
on an instruction target already has (the single post-loop-2 `lw a1,0xC($s2)`
that feeds both `math_Distance3D` arguments).**
*Mechanism proposed by s19:* E-s16-1 arm (c) — an out-of-block use makes the
preheader copy survive combine, and `optimize_reg_copy_1` re-points the base add
onto the copy's destination; the purchase is free if the use site is an
instruction target already emits.
*Probe:* six cells on the V1 chassis — S3 (named `r2`, both math args derived from
it, `r2 = p` pre-init for the skip path), T1, T2, T3, W2, W3.
*Result:* 7 / 11 / 26 / 21 / 21 / 14. Best is S3 = 7 at 127/127.
**KILLED.** The consumer is not free: routing the math base through a live local
DELETES target's `lw a1,0xC($s2)`, so the purchase is +1 move and -1 load, and the
move lands in loop 2's exit block where target emits nothing. In W2 the move is
coalesced away entirely, so even the +1 does not appear (124 insns).

**H-s20-2 — s19 frontier (a): loop 2's preheader can be made cse-reachable by a
control-flow spelling that gives loop 2's guard exactly one predecessor.**
*Mechanism proposed by s19:* `cse_end_of_basic_block` terminates an EBB at any
label that can be branched to; loop 2's guard is such a label only because loop
1's skip branch targets it.
*Probe:* cell U1 — put a statement (`p = *(u8 **)(ctx + 0xC);`) between the two
loops so it occupies the join block, then read the `-da` `.jump` and `.cse`
dumps for the actual EBB behaviour before scoring.
*Result:* the premise is FALSE. The join label opens a new EBB, but loop 2's
guard block and its fall-through preheader are both INSIDE that EBB — `.cse`
shows the preheader read (insn 162) eliminated. **KILLED as stated (E-s20-3).**
The predecessor count is not the blocker, so no CFG enumeration is warranted.

**H-s20-3 (NEW, CONFIRMED) — cse deletes a folded redundant read outright and
only materialises a reg-reg copy when the destination has a use in a LATER EBB.**
*Mechanism:* cse substitutes the canonical pseudo at every use it can rewrite,
which is every use inside the EBB it is processing; a use in a later EBB cannot be
rewritten, so the destination must be materialised and cse leaves `DST = SRC`.
*Probe:* U1 `.jump` insn 83 `reg80 = mem(reg72+12)` → `.cse` insn 83 absent and
insn 89 rewritten to `reg81 = reg84 + reg79`.
**CONFIRMED.** This reframes clause B: it is not combine-survival insurance, it is
the sole reason the copy insn exists. Corollary: any future proposal must name a
later-EBB use, and that use always materialises as one instruction.

**H-s20-4 (NEW, CONFIRMED) — the guard's self-clobbering two-step
(`t = sh + p; t = *(s32 *)(t + K);`) is a necessary condition for the base add to
survive cse.**
*Mechanism:* when the guard load writes the SAME pseudo that held the guard
address, the address value is dead and cse cannot reuse it for the base add; when
the guard is written inline GCC picks a fresh destination, the address stays
available, and cse merges the base add into the guard add.
*Probe:* U1 `.jump` loop 2, insns 151/155/168 → `.cse` shows 162 and 168 both
gone; contrast loop 1 (insn 75 clobbers reg86) where insn 89 survives. Control
cell W1 (V1 + loop-2 guard two-step through a fresh `t2`) = 3 at 127/127.
**CONFIRMED.** This is the mechanism behind s15's bare "A1..A4 = 8" numbers and
behind the whole reuse-p exclusion. It is free, and W1 is the first loop-2 guard
spelling that is structurally symmetric with loop 1's rather than merely tied.

**H-s20-5 — the fully target-shaped spelling (join-block read feeding loop 2's
guard, redundant preheader read, source re-set downstream) reproduces target's
preheader in both loops.**
*Mechanism:* combining H-s20-3 and H-s20-4 with s16's E-s16-2 canonicalisation
flip (the copy's source re-set from the copy's destination downstream).
*Probe:* twelve cells U1..U11, S1, S2, T0.
*Result:* 10-37, all worse than 3; U7 (both guards two-step, join-block read) is
125 insns — exactly two short, i.e. both copies missing and nothing else wrong.
**KILLED.** The join-block read and the clause-B purchase compete for the same
instruction slot (target's `lw a0,0xC($s2)` at T.txt:52): it can be loop 1's
purchase (a move — the V1 candidate, residual 3) or loop 2's fold-enabler (a
load — U7, residual 2 short), never both.

## [s20] s19 frontier (b): loop 2's named preheader addend, carried out of the loop and used to derive BOTH math_Distance3D arguments the way target does from one base, is a clause-B consumer that costs zero net instructions and takes the residual to 1.
- mechanism: E-s16-1 arm (c) - a preheader copy whose destination is used in a different basic block survives combine, and local-alloc's optimize_reg_copy_1 then re-points the base add onto the copy's destination. The purchase price is one instruction at the use site; it is free only if the use site is an instruction target already emits (target's post-loop-2 `lw a1,0xC($s2)` feeds both math args).
- probe: Six cells on the V1 chassis (never tested there; s9's 19-point measurement predates it): S3 (loop-2 addend named `r2`, pre-initialised `r2 = p` so the skip path stays defined, both math args derived from r2), T1 (symmetric loop-1 tail + clause-B into math args), T2 (clause-B consumer = rec_a's base), T3 (V1 tail + clause-B into math args), W2 (self-clobbering loop-2 guard + named q2 + `p = q2;` exit tail + math args from p), W3 (W1 + q2 consumed by rec_a). Each scored with `sandbox --disable all`; S3 also diffed against target.
- result: S3 = 7 (127/127), T1 = 11 (125), T2 = 26 (128), T3 = 21 (124), W2 = 21 (124), W3 = 14 (127/127). Best is 7, against a floor of 3. S3's diff shows loop 1's exit tail expanded to `addu a0,a3,zero / addu a1,a0,zero`, target's tail `lw a1,12(s2)` deleted, and loop 2's preheader still emitting `lw a1,12(s2)` where target has the copy. In W2 the purchased move is coalesced away entirely, so even the +1 never appears.
- verdict: KILLED

## [s20] s19 frontier (a): loop 2's preheader can be made cse-reachable by a control-flow spelling in which loop 2's guard block has exactly one predecessor, because cse_end_of_basic_block terminates an extended basic block at any label that can be branched to.
- mechanism: The join label .L8001791C exists solely because loop 1's `blez` skip edge targets it; removing that join would let the extended basic block containing loop 1's exit tail extend into loop 2's preheader so the redundant ctx+0xC read there folds to a copy.
- probe: Cell U1 places a statement (`p = *(u8 **)(ctx + 0xC);`) between the two loops so it occupies the join block, then `pwsh tools/grinder/dump.ps1 func_80017848` and a direct read of the `.jump` and `.cse` RTL for func_80017848 (block boundaries and per-insn survival) BEFORE scoring, as the brief's pass-attribution rule requires.
- result: The premise is false. The join label opens a new EBB, but loop 2's guard block AND its fall-through preheader are both inside that EBB: `.jump` insn 162 `reg80 = mem(reg72+12)` (loop 2's preheader read) is ABSENT in `.cse`, i.e. cse did reach and eliminate it. Target's own listing has the same shape - `lw a0,0xC($s2)` at T.txt:52 is after the join label, in the guard block, not in loop 1's exit tail. The predecessor count of the guard label is not the blocker, so the CFG enumeration s19 specified is unnecessary.
- verdict: KILLED

## [s20] cse does not fold a redundant preheader read into a reg-reg copy; it DELETES the read and substitutes the original pseudo, and a copy insn is materialised only when the destination has a use in an extended basic block LATER than the one containing the preheader.
- mechanism: cse rewrites every use it can reach, which is every use inside the EBB currently being processed. A use in a later EBB cannot be rewritten, so the destination must be materialised and cse leaves `DST = SRC` behind.
- probe: Cell U1's `.jump` dump: insn 83 `(set (reg/v 80) (mem (plus (reg/v 72) (const_int 12))))` with base add insn 89 `(set (reg/v 81) (plus (reg/v 84) (reg/v 80)))`. Compare the same range in `.cse`.
- result: In `.cse` insn 83 is gone outright (not converted to a copy) and insn 89 reads reg79 - the ORIGINAL pseudo. In the V1 candidate, where the loop-1 exit tail `p = q;` puts a use of the destination in a later EBB, s16 already recorded insn 83 surviving as `(set (reg/v 80) (reg/v 79))`. This reframes s15/s16's clause B: it is not combine-survival insurance, it is the sole reason the copy insn exists, and it always costs exactly one instruction.
- verdict: CONFIRMED

## [s20] The base add survives cse only when the guard is written as a SELF-CLOBBERING two-step (`t = sh + p; t = *(s32 *)(t + K);`); with an inline guard, cse merges the base add into the guard's address add.
- mechanism: When the guard load writes the same pseudo that held the guard address, that address value is dead and cse cannot reuse it for the base add. With an inline guard GCC gives the load a fresh destination, the address pseudo stays available, and cse eliminates the base add as redundant.
- probe: U1's `.jump` loop 2: insn 151 `reg109 = reg85 + reg79` (guard address), insn 155 `reg111 = mem(reg109+32)` (fresh destination), insn 168 `reg81 = reg85 + reg80` (base add) - versus loop 1 where insn 75 `reg86 = mem(reg86+28)` clobbers the address. Then control cell W1 = the V1 body plus a loop-2 guard two-step through a FRESH local `t2`.
- result: In `.cse` loop 2's insns 162 AND 168 are both gone (add merged), while loop 1's insn 89 survives. W1 scores exactly 3 at 127/127 - the two-step is free for loop 2. This names the mechanism behind s15's bare 'A1/A2/A3/A4 all = 8' reuse-p exclusion, and gives the first loop-2 guard spelling that is structurally symmetric with loop 1's rather than merely tied.
- verdict: CONFIRMED

## [s20] Writing the ctx+0xC read into the join block between the two loops (target's own T.txt:52 position), so loop 2's preheader read folds, reproduces target's loop-2 preheader.
- mechanism: Combines the fold (a read earlier in the same EBB) with s16's E-s16-2 canonicalisation flip (re-setting the copy's source downstream from the copy's destination).
- probe: Twelve cells: U1, U2 (math args inline), U3 (loop-2 preheader read left inline), U4 (loop-2 guard two-step through the SHARED `t`), U5, U6 (loop-1 `p = q;` restored), U7 (loop-2 guard two-step through a fresh `t2`), U9 (shift recomputed for loop 2's base), U10 (+ loop-2 exit tail `p = q;`), U11 (both exit tails), plus controls T0 (symmetric chassis) and S1/S2 (fully target-shaped both loops).
- result: 11, 13, 11, 37, 36, 37, 10, 11, 12, 12, 4, 22 - all worse than 3. U7 is the sharpest: 125 instructions, exactly two SHORT, both preheader copies missing and nothing else wrong. The join-block read and the clause-B purchase compete for the SAME instruction slot (target's `lw a0,0xC($s2)`): it can be loop 1's purchase (a move - the V1 candidate at 3) or loop 2's fold-enabler (a load - U7 at two short), never both.
- verdict: KILLED

## s21 (2026-08-18, STRUCTURAL)

### H-s21-1 — KILLED
STATEMENT: On the W1 chassis (loop 2's guard written as loop 1's self-clobbering
two-step, so the base add survives per E-s20-2), a use of loop 2's named
preheader ADDEND inside loop 2's BODY — a later extended basic block, because the
loop top is a branch target — forces cse to materialise `DST = SRC` in the
preheader in front of the base add, i.e. target's shape.
MECHANISM PROBED: E-s20-1 (cse can only rewrite uses inside the EBB it is
processing) combined with E-s20-2 (the two-step keeps the base add alive).
PROBE: cells A1/A2/A3/A5 on W1 — element read, back-edge limit read, pointer-first
element read, pointer-first base add — plus control A4 (both body uses via the
addend so `base` dies).
RESULT: 6 / 5 / 10 / 6, all at 128 insns; A4 inert at 3 / 127.
VERDICT: KILLED. The copy is created and survives, but it is a copy of `base`
placed one slot AFTER the base add (A1/A2 residual diffs), reproducing s15's C1
and s19's Z1/Z2 on a chassis those cells never controlled for. By the time a copy
is placed, loop.c's strength reduction plus cse2 have re-expressed the body
address as `base + constant`, so the value needing a later-EBB carrier is always
`base`, never the addend. Body uses can never be target's clause-B consumer.

### H-s21-2 — CONFIRMED (partially; a chassis-relative retraction of s15 (4))
STATEMENT: s15's blanket prohibition "loop 2's base addend MUST stay a fresh
ctx+0xC read" was conditioned on a mechanism (cse merging the guard-address add
with the base add) that E-s20-2's self-clobbering two-step removes, so the
reuse-p family should score better on W1 than the 8 s15 recorded.
PROBE: cells D1 (reuse `p` as loop 2's base addend on W1), D2/D3 (plus a body
element / back-edge limit use of `p`), D4/D5 (association variants), D7 (named
links local).
RESULT: D1 = 5 at 126 insns; D3 = 4 at 127; D4 = D5 = 4; D2 = 7; D7 = 11.
VERDICT: CONFIRMED. The family improves from 8 to 5 and the guard/base merge does
not occur. D1 is one instruction SHORT and the missing instruction is exactly
loop 2's preheader copy — the fresh `lw v0,12(s2)` in that slot is gone. D3's
preheader is `addu a0,a1,a0 / addu a1,a0,zero` against target's
`addu a3,a0,zero / addu a0,a1,a3`: right instruction kinds, wrong order and
operands. This is the first change in the residual's SHAPE since s9 and it is one
point off the floor, so it is the chassis the next session should build on.

### H-s21-3 — KILLED
STATEMENT: On the reuse-p chassis (D1/D3), a post-loop consumer of `p` supplies
loop 2's clause-B use for free, because target reloads ctx+0xC after loop 2 for
the math_Distance3D arguments anyway.
PROBE: cells E1 (both math args from `p` on D1), E2 (`rec_a` from `p`), E3 (both
math args on D3), E4 (math args AND `rec_a`).
RESULT: 21 / 31 / 20 / 15, at 123 / 126 / 124 / **121** insns.
VERDICT: KILLED. Identical failure mode to s20 (4) on V1: consuming the pointer
from a live local deletes the reloads target keeps, so the purchase is net
NEGATIVE in instruction count. Across both chassis, nine post-loop consumers now
measure 7..31 and not one is free.

### H-s21-4 — KILLED
STATEMENT: Loop 1's clause-B consumer can be made FREE by letting loop 2's guard
address add — an instruction target already emits — be the later-EBB use, in
place of the `p = q;` exit-tail move that currently costs one point.
PROBE: cells B1 (outer/inner names swapped, tail deleted, loop 2's guard on the
top-level pointer) and B2 (`q = p;` pre-init before loop 1's guard so the name is
defined on the skip path, tail deleted, loop 2's guard on `q`).
RESULT: both 12, both at 125 insns — TWO instructions short, i.e. neither
preheader copy survives.
VERDICT: KILLED, with a clarifying corollary: in both cells the later-EBB use is a
use of the copy's SOURCE rather than its DESTINATION, so E-s20-1's predicate is
never triggered and cse deletes the redundant read outright. Renaming the
outer/inner pair cannot change which of the two the guard consumes; the guard
consumes whichever name is live across the join, and that name is always the
source.

### H-s21-5 — KILLED
STATEMENT (s20 frontier item #3): a post-loop expression reading loop 2's addend
can be folded onto the strength-reduced element pointer `addu v0,a0,v1` that
target already emits at loop 2's exit, making the clause-B purchase free.
PROBE: cell C1 — named `q2` for loop 2's addend plus
`end = (u8 *)((s32)q2 + sh2 + i);` after the loop, consumed only by the
immediately following math-arg base derivation.
RESULT: 20 at 127 insns.
VERDICT: KILLED. GCC rebuilds the address rather than folding it onto the
induction-variable update, and the surrounding reload pattern changes wholesale.

## [s21] On the W1 chassis (loop 2's guard written as loop 1's self-clobbering two-step, so E-s20-2 keeps the base add alive for free), a use of loop 2's named preheader ADDEND inside loop 2's BODY — a later extended basic block, because the loop top is a branch target — forces cse to materialise DST = SRC in the preheader in front of the base add, i.e. target's exact shape.
- mechanism: E-s20-1: cse can only rewrite uses inside the EBB it is processing, so a body use of the addend is un-rewritable and the copy must be left behind; E-s20-2 supplies the add-survival half so the copy lands before the base add rather than being merged away.
- probe: Cells A1 (element read as *(u8 *)((s32)q2 + sh2 + i + 0x2C)), A2 (back-edge limit as *(s32 *)((s32)q2 + sh2 + 0x20)), A3 (pointer-first element), A5 (pointer-first base add), plus control A4 (both body uses via the addend so `base` dies), all built on W1 and scored with sandbox --disable all; objdump-normalised residual diffs read for A1 and A2.
- result: A1 = 6, A2 = 5, A3 = 10, A5 = 6, all at 128 insns; A4 inert at 3 / 127. A1 and A2 emit `lw v0,12(s2) / lw a2,16(s2) / addu a0,a1,v0 / addu a1,a0,zero` where target has `addu a3,a0,zero / lw a2,16(s2) / addu a0,a1,a3` — the copy is created and survives combine, but it is a copy of BASE placed one slot AFTER the base add, and the un-folded addend load is the 128th instruction.
- verdict: KILLED

## [s21] s15 (4)'s standing prohibition — 'loop 2's base addend MUST stay a fresh *(u8 **)(ctx + 0xC) read, do not re-probe that family' — is chassis-relative and false on the W1 two-step chassis, because the mechanism s15 cited (cse merging the guard-address add into the base add when the two addends are provably equal) is exactly what the self-clobbering two-step prevents.
- mechanism: E-s20-2: writing the guard as `t2 = sh2 + (s32)p; t2 = *(s32 *)(t2 + 0x20);` makes the guard's address pseudo dead, so cse has no live address to reuse for the base add and cannot merge the two adds. s15's A1..A4 = 8 were all measured on the INLINE-guard chassis where the merge does happen.
- probe: Cells D1 (W1 + loop 2's base addend reuses the carried pointer `p`), D2 (D1 + body element read via `p`), D3 (D1 + back-edge limit read via `p`), D4/D5 (association variants of D3), D7 (D3 + a named links local); sandbox --disable all on each, residual diff read for D3.
- result: D1 = 5 at 126 insns (exactly one short, and the missing instruction is precisely loop 2's preheader copy — the fresh `lw v0,12(s2)` this chassis normally emits there is gone); D3 = 4 at 127 with preheader `addu a0,a1,a0 / addu a1,a0,zero` against target's `addu a3,a0,zero / addu a0,a1,a3`; D2 = 7, D4 = D5 = 4 (association inert), D7 = 11.
- verdict: CONFIRMED

## [s21] On the reuse-p chassis (D1/D3), a post-loop consumer of `p` supplies loop 2's clause-B use for free, because target reloads ctx+0xC after loop 2 for the math_Distance3D arguments anyway.
- mechanism: E-s20-1 requires only that some later-EBB insn read the copy's destination; loop 2's exit block is a later EBB and target already emits `lw a1,12(s2)` there, so routing the math args through the live pointer should trade that load for the copy at zero net cost.
- probe: Cells E1 (both math_Distance3D args derived from `p` on D1), E2 (`rec_a` derived from `p`), E3 (both math args on D3), E4 (math args AND `rec_a`); sandbox --disable all on each, instruction counts recorded.
- result: E1 = 21 at 123 insns, E2 = 31 at 126, E3 = 20 at 124, E4 = 15 at 121. Every post-loop consumer DELETES one to six `lw ...,12(s2)` reloads that target keeps, so the purchase is net negative in instruction count — the same failure mode s20 (4) measured on V1 (7/11/14/21/26). Nine post-loop consumers now measured across two chassis, none free.
- verdict: KILLED

## [s21] Loop 1's clause-B consumer can be made FREE by letting loop 2's guard address add — an instruction target already emits — be the later-EBB use of loop 1's copy, in place of the `p = q;` exit-tail move that currently costs one of the three residual points.
- mechanism: Target's loop-1 exit region contains `lw a0,12(s2) / sll a1,s4,6 / addu v0,a1,a0` — loop 2's guard address computation. If that add consumed loop 1's copy destination instead of a fresh read, E-s20-1's materialisation would be paid by an instruction target already has.
- probe: Cells B1 (outer/inner names swapped so `q` is read at the top and used by loop 1's guard while `p` is the redundant re-read inside loop 1's block, `p = q;` tail deleted, loop 2's guard two-step reading `q`) and B2 (W1 with `q = p;` inserted before loop 1's guard so the name is defined on the skip path, tail deleted, loop 2's guard on `q`).
- result: Both 12, both at 125 insns — TWO instructions short, i.e. NEITHER preheader copy survives. In both spellings the later-EBB use is a use of the copy's SOURCE rather than its DESTINATION, so cse simply deletes the redundant read and no copy is ever created. Renaming the outer/inner pair cannot change which of the two the guard consumes: the guard consumes whichever name is live across the join, and that is always the source.
- verdict: KILLED

## [s21] s20 frontier item #3: the clause-B purchase for loop 2 can be folded onto the strength-reduced element pointer `addu v0,a0,v1` that target already emits at loop 2's exit, by writing a post-loop expression that reads the preheader addend and feeds the math-arg base derivation.
- mechanism: GCC already materialises `base + i` at loop 2's exit; if the C expresses that exit value in terms of the preheader addend rather than base, the existing insn becomes the un-rewritable later-EBB use and no new instruction is needed.
- probe: Cell C1 — W1 plus a named `q2` for loop 2's addend (pre-initialised from `p` so it is defined on the skipped path) plus `end = (u8 *)((s32)q2 + sh2 + i);` after the loop, consumed only by the immediately following math_Distance3D first-argument base derivation.
- result: 20 at 127 insns. GCC rebuilds the address rather than folding it onto the induction-variable update, and the surrounding reload pattern changes wholesale.
- verdict: KILLED

## s22 (2026-08-18, FORENSICS)

### H-s22-A — KILLED
**Statement.** s21 frontier item #2: on the D3 chassis the loop-2 preheader
already contains target's two instruction KINDS (a base add and a reg-reg copy)
in the wrong order, so varying what the body-limit use looks like (offset split /
second named local / `while` instead of `do/while`) can flip the order into
target's `copy-then-add`.
**Probe.** Applied D3 (re-measured 4 at 127/127), ran `pwsh tools/grinder/dump.ps1
func_80017848` (canonical cc1, not instrumented), extracted the function region
from `.cse`, `.loop`, `.cse2`, `.combine`, `.lreg` and read the loop-2 preheader
insn chain.
**Result.** D3's loop-2 copy is `insn 338 (set (reg 122) (reg 81))`, ABSENT from
`.cse` and first present in `.loop`, placed after the LICM'd `ctx+0x10` load
(insn 337) and after the base add (insn 162). It is loop.c's own invariant
temporary and it copies BASE. Its source is defined by the immediately preceding
insn, so `optimize_reg_copy_1`'s precondition (source dies at the copy, earlier
uses to re-point) cannot hold for it under ANY spelling of the body-limit use.
**Verdict.** KILLED. The order is not a tunable on this chassis; all three
proposed variants would have been wasted sweeps.

### H-s22-B — CONFIRMED
**Statement.** The copy-before-base-add order that target has is produced by
local-alloc's `optimize_reg_copy_1`, not by cse or combine, and it needs exactly
two conditions: a copy insn textually before the base add, and the copy's SOURCE
dying at that copy.
**Probe.** Same dump set, loop 1 (which reproduces target's preheader byte-for-
byte): compared insn 89's operand across passes.
**Result.** `.cse` insn 89 = `(plus (reg 84) (reg 79))` — the ORIGINAL;
`.lreg` insn 89 = `(plus (reg 84) (reg 80))` — the COPY, with `REG_DEAD reg79`
carried on the copy insn 83. No intervening pass touches the operand.
**Verdict.** CONFIRMED. s15 (1)'s attribution is now RTL-proven and upgraded from
"the pass re-points the base add when the copy is orphaned" to the two-condition
predicate above. Any future cell that wants target's order must satisfy (i) and
(ii); a copy of BASE never can.

### H-s22-C — CONFIRMED (structural, and it renames the residual)
**Statement.** The V1 residual is a SITE-SWAP, not two independent defects:
target re-reads `ctx+0xC` where we emit a copy (loop 1's exit) and emits a copy
where we re-read (loop 2's preheader).
**Probe.** objdump-normalised diff of V1 against target, plus the `.lreg` slot
identification (V1's insn 162 is the loop-2 preheader load, carrying REG_EQUIV;
target's `addu a3,a0,zero` occupies that exact slot).
**Result.** Exactly three real instruction differences, listed in E-s22-3. Loop
1's preheader — including target's long-unexplained dead copy — is byte-exact
including hard-register assignment ($7/$4/$5/$6/$2/$3 = a3/a0/a1/a2/v0/v1).
**Verdict.** CONFIRMED.

### H-s22-D — KILLED
**Statement.** Performing the swap directly closes the gap: make loop 1's exit
tail a fresh `p = *(u8 **)(ctx + 0xC);` re-read (giving target's `lw`), and give
loop 2's base addend the loop-1 copy destination `q` (so `q` keeps its later-EBB
use and loop 1's copy survives), with `q = p;` pre-initialised before loop 1's
guard so `q` is defined when loop 1 is skipped.
**Probe.** Cell G1, built on V1, measured with `sandbox --disable all`; residual
diffed.
**Result.** 13 at 126 insns (banked
`rejected/s22_q_preinit_tail_reread_l2_addend_q_costs_13.c`). The pre-init
relocates loop 1's copy under a shifted register assignment, loop 2 still gets
NO copy, and block ordering is perturbed (`blez`/`j` pair moves), net -1 insn.
**Verdict.** KILLED. Combined with E-s22-4's EBB argument, loop 2's addend can
only be the carried variable or a fresh read; a loop-1-local name requires a
pre-init and the pre-init is itself the cost.

### Frontier after s22

1. The site-swap statement (H-s22-C) is the sharpest description of the residual
   in 22 sessions, and it makes ONE construction the obvious next target: a
   chassis where loop 1's exit tail is a FRESH READ (target's `lw`) and loop 2's
   preheader contains a C-LEVEL copy whose source dies there. E-s22-1's condition
   (ii) is the new discriminator: the copy's SOURCE must have no later use. In
   V1/D3 the carried pointer `p` is live into the post-loop-2 math args (via the
   re-reads), so a copy of `p` in loop 2's preheader would NOT have a dying
   source — unless loop 2's preheader is the LAST use of that particular name.
   Concretely untried: two DISTINCT carried names, `p` used only up to loop 2's
   guard and a second name `r` defined by loop 1's exit re-read and used only as
   loop 2's base addend, so that `r` dies exactly at loop 2's copy.
2. E-s22-2 says a copy of BASE can never be reordered; E-s22-1 says a copy of the
   ADDEND can. Every cell measured to date that produced a preheader copy in loop
   2 produced a copy of BASE (loop.c). No cell has yet produced a copy of the
   ADDEND in loop 2 at `.cse`. The single cheapest diagnostic for any future cell
   is therefore not the sandbox score but one `.cse` read: does the loop-2
   preheader contain `(set (reg X) (reg Y))` where Y is the addend? If not, the
   cell cannot reach target regardless of its score.
3. Not yet dumped: the `.lreg`/`.greg` of a cell in which loop 1's copy is
   DELETED (the s16 trichotomy's "in-block use" branch). Knowing whether combine
   or flow removes it, and whether local-alloc would have re-pointed it, would
   tell us whether the in-block-use branch is recoverable by making the use one
   `can_combine_p` refuses — s17 enumerated the refusals abstractly but never
   read a dump of the deletion itself.

## [s22] s21 frontier item #2: D3's loop-2 preheader already holds target's two instruction KINDS in the wrong order, so varying the body-limit use (offset split / second named local / while-instead-of-do-while) can flip it into target's copy-then-add order.
- mechanism: s15 (1) attributed the order to local-alloc's optimize_reg_copy_1 re-pointing the base add onto the copy when the copy is orphaned; D3's copy is not orphaned (its use is the body limit read), so removing/reshaping that use should flip it.
- probe: Applied D3 (re-measured 4 at 127/127), ran pwsh tools/grinder/dump.ps1 func_80017848 with the CANONICAL cc1, extracted the func region from .cse/.loop/.cse2/.combine/.lreg and read the loop-2 preheader insn chain.
- result: D3's loop-2 copy is insn 338 (set (reg 122) (reg/v 81)) - ABSENT from .cse, first present in .loop, placed after the LICM'd ctx+0x10 load (insn 337) and after the base add (insn 162). It is loop.c's invariant temporary and it copies BASE, whose defining insn immediately precedes it; optimize_reg_copy_1's precondition (source dies at the copy, earlier uses to re-point) can never hold for it under any spelling of the body-limit use.
- verdict: KILLED

## [s22] The copy-before-base-add order target has is produced by local-alloc's optimize_reg_copy_1, not by cse or combine, and requires exactly (i) a copy insn textually before the base add and (ii) the copy's SOURCE dying at that copy.
- mechanism: optimize_reg_copy_1 (local-alloc.c:700, dispatched :1006) rewrites SRC to DEST over the range where SRC dies; nothing after local-alloc runs DCE, so the now-redundant copy stays.
- probe: Same dump set, loop 1 (which reproduces target's preheader byte-for-byte): compared insn 89's operand across passes.
- result: .cse insn 89 = (plus (reg 84) (reg 79)) - the ORIGINAL. .lreg insn 89 = (plus (reg 84) (reg 80)) - the COPY, with REG_DEAD reg79 carried on the copy insn 83. No intervening pass (loop, cse2, combine, flow) touches the operand. At cse the base add always reads the original, so the order is never a cse decision.
- verdict: CONFIRMED

## [s22] The V1 residual is a SITE-SWAP, not two independent defects: target re-reads ctx+0xC where we emit a copy (loop 1's exit) and emits a copy where we re-read (loop 2's preheader).
- mechanism: cse works per extended basic block; loop 2's guard label has two predecessors and loop 1's exit block follows the loop labels, so both sites start fresh EBBs with no mem->reg equivalence - a C-level read there is always a real load and a C-level copy is always a move.
- probe: objdump-normalised diff of V1 (score 3, 127/127) against asm/funcs/func_80017848.s, plus .lreg slot identification.
- result: Exactly three real instruction differences: loop-1 exit target `lw a0,12(s2)` vs ours `addu a0,a3,zero`; loop-2 preheader target `addu a3,a0,zero` vs ours `lw v0,12(s2)`; loop-2 base add target `addu a0,a1,a3` vs ours `addu a0,a1,v0`. V1's insn 162 is the loop-2 preheader load carrying REG_EQUIV, occupying exactly target's copy slot. Loop 1's preheader is byte-exact with target INCLUDING hard registers ($7/$4/$5/$6/$2/$3 = a3/a0/a1/a2/v0/v1), and target's two loop preheaders are byte-identical to each other apart from body offsets.
- verdict: CONFIRMED

## [s22] Performing the swap directly closes the gap: loop 1's exit tail becomes a fresh `p = *(u8 **)(ctx + 0xC);` re-read (giving target's lw) while loop 2's base addend reads the loop-1 copy destination `q` (keeping q's later-EBB use so loop 1's copy survives), with `q = p;` pre-initialised before loop 1's guard for definedness on the skip path.
- mechanism: E-s20-1/E-s21-4's materialisation predicate (a preheader copy exists iff the copy's DESTINATION has a later-EBB use) combined with E-s22-3's site-swap: giving q a use past the join should keep loop 1's copy while the tail re-read supplies target's lw.
- probe: Cell G1 built on V1, measured with sandbox --disable all, residual diffed.
- result: 13 at 126 insns. The pre-init `q = p;` relocates loop 1's copy (it becomes `addu a0,a2,zero` under a shifted register assignment), loop 2 still gets NO copy at all, and the added definition perturbs block ordering (a blez/j pair moves), losing one instruction net. Banked as rejected/s22_q_preinit_tail_reread_l2_addend_q_costs_13.c.
- verdict: KILLED

## s23 (2026-08-18, forensics)

### H-s23-1 (CONFIRMED). `use_crosses_set_p` is reachable from honest C and produces target's exact instruction stream.
Mechanism: combine's `can_combine_p` refuses to substitute a copy's source into a
later insn when that source is SET in between (`use_crosses_set_p`). The only
value target computes between the preheader copy and the base add is the links
pointer, so assigning it to the carried pointer local supplies the required set;
cse is likewise blocked from canonicalising the base add's operand back to the
original, because the original has been invalidated.
Probe: cell Q10 (both loops), plus Q1/Q2/Q3 (loop 2 only) and controls Q4/Q6, all
on the W1 chassis; residual read with objdump normalisation.
Result: Q10 = 14 at 127/127 with NO instruction-kind, count or ordering
difference anywhere in the function — both preheader copies present in target's
slot, base adds reading the copy, and loop 1's exit tail an honest
`lw a0,0xC(s2)` re-read for the first time in 23 sessions. The whole score is
register identity.
Verdict: CONFIRMED.

### H-s23-2 (KILLED). The same family can be tuned to 0 by allocation-level C changes.
Mechanism: if the residual is only register identity, association order, variable
splitting, creation order and guard shape should be able to move the allocation
onto target's.
Probe: 21 cells (Q1-Q15, R1-R8, S1-S5) sweeping association, split/fuse of `base`
and of the copy destination, guard two-step vs inline, creation and statement
order; plus `.greg`/`.lreg` dumps of Q10 and S1.
Result: floor of the family is 12 (Q14/R2/R3/R5), and the RTL says why: the
mechanism requires the carried pointer and the links pointer to be the SAME C
local, hence one pseudo and one hard register, while target keeps the pointer in
`$a0` (dead at the base add, register reused by `base`) and links in `$a2`. Our
`p` is live through the loop body, so it conflicts with `base` and `base` can
never inherit its register. No spelling can unfuse them, because the set of the
copy's source is the mechanism.
Verdict: KILLED — structurally, not merely by price.

### H-s23-3 (CONFIRMED). Global-alloc priority is computable from `.lreg` and steerable from C.
Mechanism: `global.c:allocno_compare` sorts by
`floor_log2(n_refs) * n_refs / live_length`; `.lreg`'s `Register N used R times
across L insns` gives both inputs.
Probe: compute the priorities for Q10's allocnos, predict that splitting `base`
into `base1`/`base2` (10/22 -> 5/11, priority 1.364 -> 0.909) drops it below `p`
(0.923) and moves `p` from `$a1` to `$a0`; build cell S1 and read `.greg`.
Result: `.greg` order changed exactly as predicted (`88 98 110 79 78 80 81 ...`,
p ahead of both bases) and dispositions show `78 in 4` = `$a0`. Score unchanged
at 14 because the win is paid back at `base` (H-s23-2).
Verdict: CONFIRMED — and it generalises to any register-identity residual in the
queue.

## [s23] combine's use_crosses_set_p refusal is reachable from honest C and yields target's preheader shape at BOTH loops, including loop 1's exit fresh read.
- mechanism: can_combine_p declines to substitute a copy's source into a later insn when that source is SET in between; the only value target computes between the preheader copy and the base add is the links pointer, so assigning it to the carried pointer local supplies the set. cse is blocked from canonicalising the base add's operand back to the original for the same reason, and the clobbered pointer forces an honest ctx+0xC re-read at the loop exit.
- probe: Cell Q10 (mechanism applied to both loops, built on the W1 two-step-guard chassis) plus Q1/Q2/Q3 (loop 2 only), controls Q4 (links reuse without the copy) and Q6 (copy without the set); objdump-normalised residual diff against asm/funcs/func_80017848.s.
- result: Q10 = 14 at 127/127 with NO instruction-kind, count or ordering difference anywhere in the function: both preheaders emit 'copy / links load / base add reading the copy' in target's slots, and loop 1's exit tail is 'lw a1,12(s2) / sll a0,s4,6' against target's 'lw a0,12(s2) / sll a1,s4,6'. Every one of the 14 points is register identity. Controls: Q4 = 14 (same register cascade, no copy), Q6 = 5 at 126 (copy deleted, as the s16 trichotomy predicts).
- verdict: CONFIRMED

## [s23] The use_crosses_set_p family can be tuned to distance 0 by C-level allocation changes, since its residual is purely register identity.
- mechanism: association order, variable splitting/fusing, creation order, statement order and guard shape all move GCC 2.7.2's allocno priority and hence hard-register assignment.
- probe: 21 cells (Q1-Q15, R1-R8, S1-S5) sweeping every one of those axes on the exact-instruction-stream chassis, plus .greg/.lreg dumps of Q10 and S1 read for conflicts and dispositions.
- result: Family floor is 12 (Q14/R2/R3/R5); nothing below. The RTL gives the reason: the mechanism REQUIRES a set of the copy's source, the only available value is links, so the carried pointer and links are one C local, one pseudo, one hard register. Target keeps the pointer in $a0 (dead at the base add, its register reused by base) and links in $a2 - impossible for one pseudo. With p carrying links it is live through the body, conflicts with base, and base can never inherit its register (S1 .greg: '78 in 4  80 in 5  81 in 5').
- verdict: KILLED

## [s23] GCC 2.7.2's global-alloc priority is computable from the .lreg dump header and is steerable from C, so hard-register identity is a predictable quantity rather than a black box.
- mechanism: global.c's allocno_compare sorts allocnos by floor_log2(n_refs) * n_refs / live_length (times size); .lreg prints 'Register N used R times across L insns' for every pseudo, giving both inputs.
- probe: Computed priorities for Q10's allocnos (base 10/22 = 1.364, q 4/6 = 1.333, p 12/39 = 0.923, sh 3/8 = 0.375), checked them against .greg's ';; 13 regs to allocate:' order, then PREDICTED that splitting base into base1/base2 (5/11 = 0.909, below p) would move p ahead of base and hand p target's $a0; built cell S1 and read .greg.
- result: The printed order matched the computed priorities exactly, and S1's order became '88 98 110 79 78 80 81 ...' with dispositions '78 in 4' - p in $a0 as predicted, first try. Score unchanged at 14 because the win is paid back at base (see the kill above).
- verdict: CONFIRMED

## s24 (2026-08-18, escalation/disposition)

### H-s24-A — KILLED
**Statement.** The links value can be given its own pseudo while still supplying
the SET of the copy's source that `use_crosses_set_p` needs, if the set's value is
consumed into a separate local inside the PREHEADER — so the carried pointer dies
before the loop body, `base` stops conflicting with it, and E-s23-2's register
conflict disappears while the copy still survives combine.
**Mechanism (predicted).** `p = *(u8 **)(ctx + 0x10); lnk = p;` gives `p` a live
range that ends inside the preheader; `base` may then reuse `p`'s hard register
exactly as target does, while `lnk` carries links through the body in its own
register.
**Probe.** Cells F2 (reusing loop 1's `lnk`) and F3 (a fresh `lnk2`), both on the
V1 chassis so loop 1's bytes stay exact and only loop 2 carries the construct.
`sandbox func_80017848 --disable all`.
**Result.** F2 = 8 at 126/127 insns; F3 = 9 at 126/127 insns. Both are one
instruction SHORT and the missing instruction is the preheader copy itself.
**Verdict. KILLED.** Copy propagation folds `lnk = p` back into a single pseudo,
so the required SET no longer separates the copy from the base add,
`use_crosses_set_p` never fires, and combine substitutes and deletes the copy.
The premise ("a second local can hold links without fusing with the pointer") is
false at RTL.

### H-s24-B — CONFIRMED (negative, and it closes s23's frontier #1)
**Statement.** The `use_crosses_set_p` family admits exactly two positions and
both are measured dead, so no third variant of it exists.
**Mechanism.** The refusal requires an RTL set of the copy's SOURCE between the
copy and the base add; the only value target computes there is the links pointer;
therefore the carried-pointer local must receive it. Either the body reads that
same local (one pseudo, live through the body, conflicts with `base` — E-s23-2,
12-16 over 23 cells) or the value is copied out so the pointer dies early
(copy-prop refolds, the set vanishes, the copy dies — H-s24-A, 8-9 at 126 insns).
**Probe.** The two branches are the union of s23's 21 tuning cells and this
session's F2/F3.
**Verdict. CONFIRMED.** Combined with s17's E-s17-1 (all seven `can_combine_p`
refusal paths enumerated) and E-s20-1 (the cse-side later-EBB-use predicate, whose
every consumer site for loop 2 is measured dead), there is no remaining producer
of a surviving loop-2 preheader copy reachable from pure C on this chassis.

### H-s24-C — CONFIRMED by construction (s23 frontier #3 retired)
**Statement.** The allocno-priority instrument (E-s23-3) cannot move V1's floor.
**Mechanism.** The instrument reorders HARD-REGISTER assignment. V1's residual is
two instruction-KIND differences (a load where target has a copy, and a copy where
target has a load) plus the base add's operand; allocation order cannot convert a
`lw` into an `addu`.
**Verdict. CONFIRMED** — the instrument is transferable to other queue items with
register-identity-only residuals, not to this one.

## [s24] The ledger's floor of 3 for func_80017848 is reproducible on the current chassis with memory/grind/func_80017848/candidate.c applied to src/ings.c (i.e. the disposition entry is not quoting a stale number).
- mechanism: engine sandbox with --disable all strips both asmfix rules (rules_dropped 2) and 49 cheat-asm instructions, so the printed score is the honest pure-C distance for whatever body is in src/ings.c at measurement time.
- probe: Measured the committed HEAD body, then spliced candidate.c's body (candidate.c:325-403) over src/ings.c:816-872 preserving LF, and measured again.
- result: HEAD body: score 16, target_insns 127 / build_insns 125, scorable true, rules_dropped 2, cheat_asm_stripped 49. candidate.c body: score 3, target_insns 127 / build_insns 127, same rules_dropped/cheat_asm_stripped. Floor 3 reproduced exactly; identical to sessions 9-23.
- verdict: CONFIRMED

## [s24] Endgame-lock gate (a) (canonical-asm via STRONG hand-coded signals) still fails for func_80017848 on the current tree.
- mechanism: tools/scan_hand_coded.py scores eight structural signals (S1 multu pacing, S2 empty-body branches, S3 spill/reg profile, S4 front-loaded loads, S5 sibling cluster, S6 BIOS jumptable, S7 unsaved callee-saves, S8 redundant mask-before-shift) over the target's 127 instructions; STRONG requires S1/S2/S6.
- probe: python3 tools/scan_hand_coded.py --single func_80017848
- result: HAND_CODED: tier=LOW score=0/8 (127 insns). All eight signals unset: 0 multu/mflo pairs, no empty-body branches, 127 insns / 7 spills / 12 distinct regs, max load burst 3 in any 8-insn window, no high-similarity sibling (jaccard < 0.5), no BIOS jumptable pattern, every callee-save use has an $sp save, no redundant mask-before-shift. Unchanged from session 17.
- verdict: KILLED

## [s24] The previous session-24 run was discarded for a mechanical encoding defect rather than on the merits of its findings, and that defect is in a file this session may repair.
- mechanism: tools/grinder/grindlib.py:388 opens docs/grind/decisions.md with encoding='utf-8' inside validate_outcome's owner-gated branch; any cp1252 byte in that file raises UnicodeDecodeError before the OWNER-ESCALATION entry can be confirmed, discarding the session.
- probe: Byte-scanned docs/grind/decisions.md for non-UTF-8 sequences, re-encoded each offending byte via cp1252 -> UTF-8, then re-read the file as UTF-8; separately confirmed the s24 ledger writes (evidence.md E-s24-1..4, hypotheses.md H-s24-A..C, 2 new rejected forms, the decisions.md entry) were intact on disk.
- result: 8 offending bytes found, all 0x97 (cp1252 em dash), first at byte offset 1237144 inside the 2026-08-18 rederive entry; all re-encoded to UTF-8 U+2014 and the 1,249,661-byte file now decodes cleanly. All s24 ledger artifacts present and unmodified; rejected bank stands at 172 files.
- verdict: CONFIRMED

## s25 (2026-08-25, escalation/disposition)

### H-s25-A - KILLED
**Statement.** The three-instruction residual is (or may be) an artifact of the
decompals/mips-gcc-2.7.2 port rather than a difference in C source structure - i.e.
the original PsyQ cc1psx, given candidate.c's body, would emit target's
`lw $a0,0xC($s2)` / `addu $a3,$a0,$zero` / `addu $a0,$a1,$a3` where our port emits
a copy / a load / an add on $v0.
**Mechanism (predicted).** cc1psx is GCC 2.7.2.SN.1, SN Systems' fork; the port is
kmc-tailored. A fork difference in combine's copy-substitution or in the first
scheduler pass would show up exactly as an instruction-KIND swap of this shape, and
would explain why 24 sessions of C-level search cannot move it.
**Probe.** Diagnostic-only calibration per .claude/rules/no-compiler-divergence.md
(never a build path; no toolchain/Makefile file touched). One preprocessed
`src/ings.c` (candidate body spliced in) compiled twice - `tools/gcc-2.7.2/cc1`
with canonical CC_FLAGS, and `tools/cc1psx_wrapper.sh` with the flag subset
cc1psx accepts - then func_80017848's pre-maspsx instruction streams diffed.
**Result.** 111 instructions each. Identical except for label spelling and ONE
adjacent-pair scheduling swap at the math_Distance3D preamble
(port `sll $16,$20,6 / lw $5,12($18)`; cc1psx `lw $5,12($18) / sll $16,$20,6`).
All three residual instructions are byte-identical between the compilers. At the
one divergent site, TARGET matches the PORT, so cc1psx would score >= 5.
**Verdict. KILLED** - and it is a positive result: the residual is a genuine
C-source-structure difference, the port is the correct calibration for this
executable, and the pure-C match therefore provably exists in the C, exactly where
25 sessions have been looking.

### H-s25-B - KILLED (executes the queue item's owner directive)
**Statement.** The ra_solver / sched_solver suite can type this residual as
REACHABLE or FORECLOSED and hand back a ranked C-lever vector, as the owner
directive on the queue item recommends for RA/scheduler-tiebreak residuals.
**Mechanism (predicted).** `inverse_compose.py classify` picks the model that owns
the first divergence; if it lands on global.c/local-alloc, the allocno-priority
instrument (E-s23-3) gives a directed perturbation instead of a spelling search.
**Probe.** `bash tools/ra_solver/mkasm_honest.sh ings` then
`python3 tools/ra_solver/inverse_compose.py classify ings func_80017848`.
**Result.** The tool selected its TEXT-stream path (func_80017848 is not
`replace_with_asmfile`-wired) and reported "FIRST DIVERGENCE: IDENTICAL - the
honest stream already equals target", while the sandbox reported 3 for the same
tree. The verdict is fiction: not being asmfile-wired, `ings.tgt.s` is built from
the same C body as `ings.hon.s`, so the classifier diffed our output against
itself.
**Verdict. KILLED** on two independent grounds. (1) The tool is inapplicable to
this function as wired, and - the transferable part - its `classify` verdict must
not be trusted for ANY function that is not replace_with_asmfile-wired. (2) The
model-side answer is unchanged from H-s24-C: the residual is two instruction KINDS
plus one operand, and no allocation order or schedule turns a `lw` into an `addu`,
so both solvers are structurally outside this residual regardless of wiring.

### H-s25-C - CONFIRMED (chassis correction)
**Statement.** The 2026-08-18 escalation entry's premise - "two asmfix.txt rules
hold the byte-match" - is no longer true, so the disposition had to be re-derived
on the current chassis rather than re-affirmed from the ledger.
**Probe.** `grep -n func_80017848 asmfix.txt` (empty); `src/ings.c:590` is
`INCLUDE_ASM("asm/funcs", func_80017848);`; sandbox with candidate.c spliced in.
**Result.** score 3, 127/127, scorable, **rules_dropped 0**, cheat_asm_stripped 4
(all four belong to other functions in ings.c). The floor is identical to the
pre-migration measurement, so every banked spelling conclusion transfers, but the
function now carries zero rules and zero cheat-asm of its own.
**Verdict. CONFIRMED.** The disposition is a pure "3 instructions short in honest
C" refusal, not a debt or an integration handoff.

## [s25] The three-instruction residual is an artifact of the decompals/mips-gcc-2.7.2 port rather than a difference in C source structure - i.e. the original PsyQ cc1psx, given candidate.c's body, would emit target's lw/addu/addu where our port emits a copy, a load and an add on $v0.
- mechanism: cc1psx is GCC 2.7.2.SN.1 (SN Systems fork); the port is kmc-tailored. A fork difference in combine's copy substitution or in the first scheduler pass would present exactly as an instruction-KIND swap of this shape and would explain why 24 sessions of C-level search cannot move it.
- probe: Diagnostic-only calibration per .claude/rules/no-compiler-divergence.md (never a build path; no toolchain or Makefile file touched). One preprocessed src/ings.c with candidate.c's body spliced in, compiled twice: tools/gcc-2.7.2/cc1 with canonical CC_FLAGS, and tools/cc1psx_wrapper.sh (cc1psx.exe under dosemu2) with the flag subset it accepts. func_80017848's pre-maspsx instruction streams diffed (tmp/grind/func_80017848/s25/cc1psx_vs_port.diff).
- result: 111 instructions from each compiler. Streams identical except for label spelling ($L128 vs .L129) and ONE adjacent-pair scheduling swap at the math_Distance3D preamble: port 'sll $16,$20,6 / lw $5,12($18)', cc1psx 'lw $5,12($18) / sll $16,$20,6'. All three residual instructions are byte-identical between the compilers. At the one divergent site TARGET (asm/funcs/func_80017848.s, 0x80017974: sll $s0,$s4,6 / lw $a1,0xC($s2)) agrees with the PORT, so cc1psx would score >= 5, not better.
- verdict: KILLED

## [s25] The ra_solver / sched_solver suite can type this residual as REACHABLE or FORECLOSED and return a ranked C-lever vector, as the owner directive on the queue item recommends for RA/scheduler-tiebreak residuals.
- mechanism: inverse_compose.py classify picks the model that owns the first divergence; if it lands on global.c / local-alloc, the s23 allocno-priority instrument gives a directed perturbation instead of a spelling search.
- probe: bash tools/ra_solver/mkasm_honest.sh ings, then python3 tools/ra_solver/inverse_compose.py classify ings func_80017848.
- result: The tool selected its TEXT-stream path ('func_80017848 is not replace_with_asmfile-wired') and reported 'FIRST DIVERGENCE: IDENTICAL - the honest stream already equals target', while the sandbox reported 3 on the same tree. The verdict is fiction: not being asmfile-wired, ings.tgt.s is built from the same spliced C body as ings.hon.s, so the classifier diffed our own output against itself.
- verdict: KILLED

## [s25] The 2026-08-18 escalation entry's premise - that two asmfix.txt rules hold this function's byte-match - still describes the tree, so the disposition can be re-affirmed from the ledger rather than re-derived.
- mechanism: The 2026-08-19 asm-until-matched migration retired rules for the queue's INCOMPLETE functions; migration_pin.json records rules_retired 2 for this one.
- probe: grep -n func_80017848 asmfix.txt; read src/ings.c:590; splice candidate.c:325-403 over the INCLUDE_ASM line and run sandbox func_80017848 --disable all.
- result: asmfix.txt has no entry for this function; src/ings.c:590 is INCLUDE_ASM("asm/funcs", func_80017848);. Sandbox with the candidate body: score 3, target_insns 127, build_insns 127, scorable true, rules_dropped 0, cheat_asm_stripped 4 (all four belong to other functions in ings.c). The premise is FALSE - the floor is unchanged but the function now carries zero rules and zero cheat-asm of its own.
- verdict: KILLED

## s26 (2026-08-30, structural)

### H-s26-A - CONFIRMED (structure), KILLED (score)
**Statement.** Target's loop-1 exit tail is literally `p = *(u8 **)(ctx + 0xC);
sh = slot_a << 6;` (its `lw $a0,0xC($s2)` / `sll $a1,$s4,6` sit on loop 1's TAKEN
edge, before the join label), and writing exactly that - with loop 2's guard and
base both reading the recomputed `sh` and `sh2` deleted - reproduces target's join
region instruction-for-instruction.
**Mechanism.** The join .L8001791C has two predecessors; on the skip path $a0/$a1
survive from loop 1's guard block, on the taken path loop 1 clobbers $a0 so both
must be re-materialised. That is a C-level re-assignment on the taken path only,
which is what a tail inside the `if (i < t) { ... }` block compiles to.
**Probe.** Cell A, plus its order-swapped control cell C.
**Result.** Cell A = **4** at 127 target / 126 build - the join region is byte-exact,
and the entire residual is the two missing preheader copies. Cell C (shift before
reload) = **6**, so the order is load-bearing.
**Verdict. CONFIRMED as the structural reading, KILLED as a score improvement.**
Chassis A is the sharpest DESCRIPTION of the wall to date (it isolates the residual
to "the two copies, nothing else") but is one point worse than the candidate, which
buys loop 1's copy with the `p = q` second-use lever.

### H-s26-B - KILLED
**Statement.** Loop 2's base addend can avoid the cse merge with the guard's address
add (s15(4), priced 8) by living in a DEDICATED carrier that is already set on both
predecessor edges of the join, so cse - whose extended basic block begins at the
join - has no equivalence between it and `p`.
**Mechanism (predicted).** cse cannot equate two pseudos whose defining insns lie
outside the current EBB, so `base = sh + r` would not be folded onto the guard's
`sh + p`, and `r` being already in a register would emit no load.
**Probe.** Cells E (`r = p;`) and F (`r = *(u8 **)(ctx + 0xC);`), each assigned at
BOTH of `p`'s assignment sites, on chassis A.
**Result.** Both **12** at 127/127. The carrier must be materialised separately on
each predecessor path, and those two instructions are exactly what target does not
spend. The predicted cse behaviour may well hold; it is unbuyable regardless.
**Verdict. KILLED** - the pre-join-carrier family is 9 points worse than chassis A
and 9 worse than the candidate.

### H-s26-C - KILLED
**Statement.** The out-of-block second use that loop 2's addend needs (to orphan its
copy from combine, the way loop 1's `p = q` tail does) can be placed inside loop 2's
own BODY - a different basic block from the preheader - instead of in the post-loop
tail, where every site is priced 19-22 by register pressure.
**Mechanism (predicted).** s16's trichotomy: a preheader copy survives combine iff
its destination's only use is out of block. The loop body is out of block and is
reached only when the loop runs, so it should cost nothing on the skip path.
**Probe.** Cell H on chassis A: loop 2's do-while condition rewritten from
`*(s32 *)(base + 0x20)` to `*(s32 *)(sh + (s32)q + 0x20)`, giving the named addend
`q` a body use.
**Result.** **6** at the correct 127/127. loop.c's move_movables hoists the DERIVED
invariant `sh + q` rather than the addend itself, so the copy that survives is
`move a1,a0` - a copy of the BASE landing one slot AFTER the base add - which is
cell C1's (s15) failure mode reproduced on a new chassis. The addend stays a `lw`.
**Verdict. KILLED.** An in-body second use cannot produce an addend copy; loop.c
always hoists the derived address, never its operand. Together with s10/s11's 19-22
pricing of every post-loop site and s26's 12 for a pre-join carrier, loop 2 has NO
free out-of-block use site anywhere.

## [s26] Target's loop-1 exit tail is literally `p = *(u8**)(ctx+0xC); sh = slot_a<<6;`, and writing exactly that (loop 2's guard and base both reading the recomputed sh, sh2 deleted) reproduces target's join region instruction-for-instruction.
- mechanism: The join .L8001791C has two predecessors; on the loop-1 skip path $a0/$a1 survive from loop 1's guard block, on the taken path loop 1 clobbers $a0 so both are re-materialised. That is a C-level re-assignment on the taken path only.
- probe: Cell A on the candidate chassis, plus the order-swapped control cell C.
- result: Cell A = 4 (127 target / 126 build); the join region is byte-exact including the taken-edge lw/sll pair, and the entire residual is the two missing preheader copies. Cell C (shift before reload) = 6.
- verdict: CONFIRMED as structure, KILLED as a score improvement (4 > the candidate's 3)

## [s26] Loop 2's base addend can dodge the cse merge with the guard's address add by living in a dedicated carrier already set on both predecessor edges of the join.
- mechanism: cse's extended basic block begins at the join, so it has no equivalence between the carrier and p and cannot fold base = sh + r onto the guard's sh + p; the carrier being in a register would emit no load.
- probe: Cells E (r = p) and F (r = fresh read), each assigned at both of p's assignment sites, on chassis A.
- result: Both 12 at 127/127 - the carrier must be materialised separately on each predecessor path, which is exactly the instruction per path target does not spend.
- verdict: KILLED

## [s26] The out-of-block second use loop 2's addend needs can be placed inside loop 2's own BODY instead of in the post-loop tail, where every site is priced 19-22.
- mechanism: s16's trichotomy - a preheader copy survives combine iff its destination's only use is out of block; the loop body is a different basic block and is reached only when the loop runs.
- probe: Cell H on chassis A - loop 2's do-while condition rewritten from *(s32*)(base + 0x20) to *(s32*)(sh + (s32)q + 0x20).
- result: 6 at 127/127. loop.c's move_movables hoists the DERIVED invariant sh+q, not the addend, so the surviving copy is `move a1,a0` - a copy of the BASE one slot after the base add (cell C1's failure mode on a new chassis); the addend stays a lw.
- verdict: KILLED

## [s26] Target's loop-1 exit tail is literally `p = *(u8 **)(ctx + 0xC); sh = slot_a << 6;` - its `lw $a0,0xC($s2)` / `sll $a1,$s4,6` sit on loop 1's TAKEN edge, before the join label - and writing exactly that, with loop 2's guard and base both reading the recomputed `sh` and `sh2` deleted, reproduces target's whole join region instruction-for-instruction.
- mechanism: The join .L8001791C has two predecessors: on the loop-1 skip path $a0/$a1 survive from loop 1's guard block, on the taken path loop 1 clobbers $a0 so both must be re-materialised. That is a C-level re-assignment on the taken path only, i.e. a tail inside the `if (i < t) { ... }` block.
- probe: Cell A on the candidate chassis (tmp/grind/func_80017848/s26/body_A.c), plus the order-swapped control cell C; both scored with `sandbox func_80017848 --disable all`, cell A disassembled and diffed against asm/funcs/func_80017848.s.
- result: Cell A = 4 at 127 target / 126 build. The join region is byte-exact, including the taken-edge lw/sll pair before the label - the first chassis in 26 sessions to reproduce it - and the ENTIRE residual on chassis A is the two missing preheader copies (loop 1 short one insn; loop 2 emitting `lw v0,12(s2)` in the copy's slot). Cell C (shift before reload) = 6, so the statement order is load-bearing. Cell D (named q2/lnk2 locals mirroring loop 1) = 4, so loop 2's preheader naming stays inert on the new chassis too.
- verdict: CONFIRMED

## [s26] Loop 2's base addend can dodge the cse merge with the guard's address add (s15(4), priced 8) by living in a DEDICATED carrier that is already set on both predecessor edges of the join, so cse - whose extended basic block begins at the join - has no equivalence between it and `p`.
- mechanism: cse cannot equate two pseudos whose defining insns lie outside the current extended basic block, so `base = sh + r` would not be folded onto the guard's `sh + p`, and `r` being already in a register would emit no load where target has its copy.
- probe: Cells E (`r = p;`) and F (`r = *(u8 **)(ctx + 0xC);`) on chassis A, with `r` assigned at BOTH of `p`'s assignment sites (the pre-loop-1 read and loop 1's exit tail) so it is live on both predecessor edges.
- result: Both cells = 12 at 127/127, i.e. 9 points worse than chassis A and 9 worse than the candidate. The carrier must be materialised separately on each predecessor path, and those two instructions are exactly the ones target does not spend. The predicted cse behaviour may well hold; the family is unbuyable regardless.
- verdict: KILLED

## [s26] The out-of-block second use that loop 2's addend needs - to orphan its copy from combine the way loop 1's `p = q` tail does - can be placed inside loop 2's own BODY, a different basic block from the preheader, instead of in the post-loop tail where every site is priced 19-22 by register pressure.
- mechanism: s16's survival trichotomy: a preheader reg-reg copy survives combine iff its destination's only use is out of block. Loop 2's body is out of block and is reached only when the loop runs, so it should cost nothing on the skip path.
- probe: Cell H on chassis A: loop 2's do-while condition rewritten from `*(s32 *)(base + 0x20)` to `*(s32 *)(sh + (s32)q + 0x20)`, giving the named addend `q` a body use; result disassembled (tmp/grind/func_80017848/s26/build_H.txt).
- result: 6 at the correct 127/127. loop.c's move_movables hoists the DERIVED loop-invariant address `sh + q`, not the addend itself, so the copy that survives is `move a1,a0` - a copy of the BASE landing one slot AFTER the base add, which is s15's cell C1 failure mode reproduced on a new chassis - while the addend stays a `lw`. An in-body second use cannot produce an addend copy.
- verdict: KILLED

## 2026-09-01 — operator reopen note (owner ruling 2026-09-01 (decisions.md FORECLOSED-BUCKET REVIEW entry))

Returned to active under Ruling A. Ground: guard-only duplication was never measured (only whole-loop duplication was priced at 35), and the load-bearing cse EBB-boundary attribution is behavioural inference, never dump-confirmed. The 2026-08-30 record's 'byte-matches only via a cheat' sentence is RETRACTED by the ruling (zero rules/cheat-asm since 2026-08-19). Named probes: (1) duplicate loop 2's 4-instruction GUARD (not body) into the loop-1 skip path, score on chassis A and the candidate chassis; (2) run dump.ps1 .cse/.combine on chassis A to convert the EBB account from inference to attribution.

## s27 (escalation, 2026-09-01)

**H-s27-1 — Loop 2's preheader copy becomes reachable if loop 2's guard block stops
being a join, because cse's extended basic block would then contain the load of
ctx+0xC.** (Inherited s26 frontier item 1; the owner's 2026-09-01 Ruling-A named probe.)
Probe: three guard-duplication spellings with a single shared loop body — G1 (guard only,
duplicated into the loop-1 taken path, `goto` into the shared preheader), G2 (guard +
preheader duplicated into the taken path, `goto` into the shared do-while body), G3 (same
as G2 but duplicated into the SKIP path). Result: **18 / 44 / 47**, build_insns
**134 / 133 / 136** against a 127-insn target. jump2 does not cross-jump the duplicated
tails back together (after cse the two arms are no longer identical, which is exactly what
`do_cross_jump` requires). **KILLED** — the mechanism is real (see H-s27-2) but no C
spelling of it is affordable; whole-loop duplication was already 35 (s12).

**H-s27-2 — the EBB-boundary account of the loop-1/loop-2 asymmetry is correct.**
Probe: `.cse` dump of chassis A via `tools/grinder/dump.ps1` (dumps/ings.cse:4270-4915).
Result: loop 1's preheader re-read is folded away by substitution (insn 89 reads reg 79,
the guard block's own load, directly); loop 2's is emitted as a real load (insn 162) because
`code_label 145` — the join loop 1's guard branches to — ends cse's block scan
(cse.c:8038). **CONFIRMED**, and it is now dump-evidence rather than three sessions of
inference.

**H-s27-3 — cse is the producer of TARGET's loop-2 preheader copy `addu $a3,$a0,$zero`.**
Probe: read target's own CFG in `asm/funcs/func_80017848.s` against the confirmed cse rule
from H-s27-2. Result: target has the identical join (`blez $v0, .L8001791C` at 0x800178C8)
with its loop-1 exit-tail reload of ctx+0xC at 0x80017914 sitting BEFORE the label, so cse's
window for target's own loop-2 preheader excluded that load too. **KILLED** — cse cannot be
the producer of the copy we are missing. Every future proposal must name a producer that
works ACROSS a join label and costs zero instructions (target 127, candidate already 127).

**H-s27-4 — putting the ctx+0xC load INSIDE loop 2's guard block (after the join) gives the
preheader an in-block equal to fold against, producing the copy without duplicating
anything.** Probe: cells J (guard reads ctx+0xC inline), J2 (J plus dropping the now-dead
loop-1 exit-tail re-read), L (J with the read named into a fresh local `r` whose second use
is the do-while condition), L2 (J with `r`'s second use post-loop, as math_Distance3D's first
argument), all on chassis A. Result: **14 / 14 / 13 / 21**, build_insns 125 / 125 / 126 / 125
— cse folds the in-block pair by SUBSTITUTION and the form comes out SHORT, and the
out-of-block second use that preserves a copy elsewhere in this function is priced exactly
where the banked table already put it. **KILLED.**

## [s27] Loop 2's preheader copy becomes reachable if loop 2's guard block stops being a join, because cse's extended basic block would then contain the load of ctx+0xC (s26 frontier item 1; the owner's 2026-09-01 Ruling-A named probe).
- mechanism: cse_end_of_basic_block (tools/gcc-2.7.2/cse.c:8038) ends the block scan at ANY code label, so the load that precedes the join is outside the window in which cse processes loop 2's preheader; removing the join would put them in one window and the redundant re-read would fold as it does in loop 1.
- probe: Three guard-duplication cells on the candidate chassis, all keeping ONE shared loop body: G1 = loop-2 guard duplicated into the loop-1 taken path with a goto into the shared preheader+body; G2 = guard AND preheader duplicated into the taken path with a goto into the shared do-while body; G3 = same as G2 but duplicated into the loop-1 SKIP path (the literal wording of the ruling).
- result: G1 = 18 (134 build insns), G2 = 44 (133), G3 = 47 (136), against a 127-insn target and a floor of 3. jump2 does not cross-jump the duplicated tails back together, because after cse the two arms are no longer identical (one arm's redundant read is folded, the other's is not) - exactly the identity do_cross_jump requires. Whole-loop duplication was already 35 (s12).
- verdict: KILLED

## [s27] The EBB-boundary account of the loop-1 / loop-2 asymmetry is correct and not merely a behavioural inference.
- mechanism: cse folds a redundant read only against an equal expression recorded earlier in the SAME extended basic block; the join label between loop 1's exit tail and loop 2's guard starts a new block.
- probe: pwsh tools/grinder/dump.ps1 -Func func_80017848 with body_A.c applied; read post-cse RTL at tmp/grind/func_80017848/dumps/ings.cse:4270-4915.
- result: Loop 1: insn 64 loads ctx+0xC after code_label 61, and the preheader (insn 89) reuses reg 79 directly - the re-read is substituted away. Loop 2: insn 141 loads ctx+0xC, then code_label 145 (the join), then the preheader at insn 162 is still a real (mem (plus (reg 72) (const_int 12))). Both fates visible side by side in one dump.
- verdict: CONFIRMED

## [s27] cse is the producer of TARGET's loop-2 preheader copy 'addu $a3,$a0,$zero' at 0x80017930.
- mechanism: If cse folded a redundant ctx+0xC re-read into an orphaned copy in target's loop-2 preheader, the load it folded against would have to sit inside the same cse block.
- probe: Read target's own CFG in asm/funcs/func_80017848.s against the dump-confirmed cse rule from the previous hypothesis.
- result: Target carries the identical join: the loop-1 guard's 'blez $v0, .L8001791C' (0x800178C8) branches to the label that loop 2's guard block starts at, and the loop-1 exit-tail reload 'lw $a0, 0xC($s2)' / 'sll $a1, $s4, 6' sits at 0x80017914/18, BEFORE that label. cse's window excluded that load in the original compilation too, so cse cannot have produced the copy. The EBB-removal frontier was aimed at a mechanism the original build did not use.
- verdict: KILLED

## [s27] Putting the ctx+0xC load INSIDE loop 2's guard block (after the join) gives the preheader an in-block equal to fold against, producing the copy with no duplication at all.
- mechanism: Same cse rule, satisfied by relocating the load instead of removing the label; a fresh named local carrying the read, with an out-of-block second use downstream of the base add, should then survive combine per the s16 trichotomy.
- probe: Cells J (guard reads *(u8**)(ctx+0xC) inline instead of p), J2 (J plus dropping the now-dead loop-1 exit-tail re-read), L (J with the read named into a fresh local r whose second use is the do-while condition), L2 (J with r's second use post-loop as math_Distance3D's first argument) - all on chassis A.
- result: 14 (125 insns) / 14 (125) / 13 (126) / 21 (125). cse folds the in-block pair by SUBSTITUTION, so the forms come out SHORT rather than gaining a copy, and the out-of-block second use prices exactly where the banked table already put it (post-loop 19-22, in-body 6, pre-join 12).
- verdict: KILLED

## s28 (forensics, 2026-09-03)

- **H-s28-1 (KILLED, class):** No pass in GCC 2.7.2 other than cse.c:7969
  (`cse_set_around_loop`), loop.c's movable/giv/biv emitters, reload1.c's reload
  emission, flow.c's auto-inc rewriter, jump.c's if-conversion paths, unroll.c and
  integrate.c can emit a `(set (reg) (reg))` that was not in the incoming RTL; the two
  producers the s27 frontier named as unread — `regmove.c` and `optimize_reg_copy_3` —
  do not exist in this compiler at all.
  - mechanism: a mechanical `emit_move_insn`/`gen_move_insn` call-site census over
    tools/gcc-2.7.2 (per-file counts in E-s28-1). combine.c, local-alloc.c, global.c,
    reload.c, reorg.c, sched.c, stupid.c and caller-save.c contain ZERO such call sites
    and so cannot create a copy under any input.
  - probe: the census plus `ls tools/gcc-2.7.2/r*.c` and the local-alloc.c prototype
    block (only `optimize_reg_copy_1`/`_2` are declared, at :251/:252).
  - result: frontier candidate list closed. `unroll.c`'s 9 sites are unreachable because
    `-funroll-loops` is absent from CC_FLAGS; `integrate.c`'s 2 need inlining;
    `flow.c`'s 1 is auto-inc, which MIPS lacks. Only reload1.c is left unmeasured and it
    requires a spill/inheritance situation at the copy site, which 0x800178D0 is not.
  - predicate_cite: tools/gcc-2.7.2/local-alloc.c:251
  - kill_scope: class
  - measured_on: source census of tools/gcc-2.7.2 at the frozen toolchain revision; no
    build involved, no FAKE constructs.

- **H-s28-2 (KILLED, instance):** Writing loop 1 and/or loop 2 as a source-level `while`
  loop with body-inline invariants, so that jump.c's `duplicate_loop_exit_test` fires
  and opens `cse_set_around_loop`'s `REG_LOOP_TEST_P` gate, does not put a reg-reg copy
  in either preheader and costs instructions against the 127-insn target.
  - mechanism: cse.c:7969 is cse's only copy emitter and plants its copy in the
    preheader via a backward scan that starts inside the preheader (so a join label does
    not foreclose it); its gate `REG_LOOP_TEST_P` (cse.c:7933) is set only at jump.c:2253
    inside `duplicate_loop_exit_test`, which jump.c:626 calls only when a
    `NOTE_INSN_LOOP_BEG` is immediately followed by a simplejump — the RTL a source-level
    `while`/`for` expands to, never the candidate's pre-rotated `if + do/while`.
  - probe: cells W1 (both loops `while`), W2 (loop 2 only), W3 (loop 1 only) on the
    candidate chassis, plus `pwsh tools/grinder/dump.ps1 -Func func_80017848` on W1.
  - result: W1 = 29 at 120 insns, W2 = 19 at 123, W3 = 21 at 123. The gate DID open —
    `NOTE_INSN_LOOP_VTOP` (notes 319 and 328) and the remapped guard pseudos 160-163 /
    164-168 are in `tmp/grind/func_80017848/dumps/ings.jump` — but `ings.loop` shows
    both preheaders holding only LICM hoists (loop 1: insns 335/336/337; loop 2: insns
    331/332/333/334) and no copy. LICM additionally hoists the loop BOUND load, which
    target does not do (it reloads at 0x80017900), which is where the missing
    instructions go.
  - kill_scope: instance
  - measured_on: candidate chassis (memory/grind/func_80017848/candidate.c over the
    src/ings.c:719 INCLUDE_ASM anchor), floor 3 at 127/127, zero FAKE constructs present.

- **H-s28-3 (KILLED, instance):** loop.c's fourteen move emitters are not the producer of
  target's preheader copy `addu $a3, $a0, $zero` (0x800178D0).
  - mechanism: loop.c:1702's `emit_move_insn (m->set_dest, m->set_src)` runs only when
    `m->move_insn` is set, and loop.c:657-670 sets it only for an invariant carrying a
    `REG_EQUIV` note or a CONSTANT `REG_EQUAL` note — the emitted source is that note's
    expression, so a pseudo-to-pseudo copy is not expressible there. The remaining seven
    sites (loop.c:3940, 3948, 3972, 4065, 5868, 5891, 6098) are giv/biv initialisers and
    require strength reduction to have run on the loop.
  - probe: end-to-end read of the fourteen call sites plus a structural read of the
    target listing for strength-reduction signatures.
  - result: target's loops recompute the address from scratch every iteration
    (`addu $v0, $a0, $v1` at 0x800178DC and in the bottom delay slot at 0x80017910) with
    the index incremented separately (`addiu $v1, $v1, 0x1` at 0x800178FC); no
    incremented address register exists in the listing, so no giv was created and the
    seven giv/biv emitters never ran.
  - kill_scope: instance
  - measured_on: target listing asm/funcs/func_80017848.s plus the frozen loop.c source;
    no FAKE constructs.

## [s28] No pass in GCC 2.7.2 other than cse.c:7969 (cse_set_around_loop), loop.c's movable/giv/biv emitters, reload1.c's reload emission, flow.c's auto-inc rewriter, jump.c's if-conversion paths, unroll.c and integrate.c can emit a (set (reg) (reg)) that was not in the incoming RTL, and the two producers the s27 frontier named as unread - regmove.c and optimize_reg_copy_3 - are absent from this compiler.
- mechanism: A mechanical emit_move_insn/gen_move_insn call-site census over tools/gcc-2.7.2. combine.c, local-alloc.c, global.c, reload.c, reorg.c, sched.c, stupid.c and caller-save.c contain zero such call sites, so they can rewrite or delete an existing copy but cannot create one under any source input. local-alloc.c declares only optimize_reg_copy_1 (:251/:700) and _2 (:252/:874); ls tools/gcc-2.7.2/r*.c contains no regmove.c.
- probe: Per-file grep -c of emit_move_insn|gen_move_insn across loop.c, combine.c, cse.c, flow.c, jump.c, local-alloc.c, global.c, reload.c, reload1.c, reorg.c, sched.c, stupid.c, unroll.c, caller-save.c, integrate.c; plus ls tools/gcc-2.7.2/r*.c and the local-alloc.c prototype block; plus Makefile:35 for the -funroll-loops flag.
- result: loop.c 14, reload1.c 13, unroll.c 9, jump.c 7, integrate.c 2, cse.c 1, flow.c 1, everything else 0. unroll.c's 9 are unreachable (-funroll-loops absent from CC_FLAGS, Makefile:35); integrate.c's 2 need inlining this function does not do; flow.c's 1 is auto-inc address rewriting and MIPS has no auto-inc. The frontier's named candidate list is closed: only reload1.c survives as an unmeasured producer, and it needs a spill/inheritance situation, which the copy site at 0x800178D0 (two values live in hard registers across it, no stack slot involved) is not.
- verdict: KILLED
- kill_scope: class
- measured_on: source census of tools/gcc-2.7.2 at the frozen toolchain revision; no build involved, no FAKE constructs present
- predicate_cite: tools/gcc-2.7.2/local-alloc.c:251

## [s28] Writing loop 1 and/or loop 2 as a source-level while loop with body-inline invariants, so that jump.c's duplicate_loop_exit_test fires and opens cse_set_around_loop's REG_LOOP_TEST_P gate, puts no reg-reg copy in either preheader and costs instructions against the 127-insn target.
- mechanism: cse.c:7969 is cse's only copy emitter; it plants the copy in the loop PREHEADER via a backward scan that starts inside the preheader, so unlike cse's EBB scan it is not foreclosed by the join label s27 proved target carries. Its gate REG_LOOP_TEST_P (cse.c:7933) is set only at jump.c:2253 inside duplicate_loop_exit_test, which jump.c:626 calls only when a NOTE_INSN_LOOP_BEG is immediately followed by a simplejump - the RTL a source-level while/for expands to, never the candidate's pre-rotated if + do/while.
- probe: Cells W1 (both loops as while), W2 (loop 2 only), W3 (loop 1 only) built on the candidate chassis with the links pointer, base address and bound read inline in the loop body so LICM feeds the post-guard preheader; each scored with sandbox --disable all; plus pwsh tools/grinder/dump.ps1 -Func func_80017848 on W1 and a read of the .jump / .cse / .loop dumps.
- result: W1 = 29 at 120 build insns, W2 = 19 at 123, W3 = 21 at 123. The gate DID open: NOTE_INSN_LOOP_VTOP (emitted only by duplicate_loop_exit_test) is present for both loops in ings.jump as notes 319 and 328, and the duplicated guards carry the pass's freshly mapped pseudos (loop 1: 160/162/163 at insns 312/314/315; loop 2: 164/165/167/168 at insns 320/321/323/324). But cse_set_around_loop emitted nothing: ings.loop shows loop 1's preheader as insns 335/336/337 and loop 2's as 331/332/333/334, all three/four of them LICM hoists with no reg-reg copy among them. LICM additionally hoists the loop BOUND load out, which target does not do (target reloads it at 0x80017900), and that is where the missing instructions go.
- verdict: KILLED
- kill_scope: instance
- measured_on: candidate chassis (memory/grind/func_80017848/candidate.c applied over the src/ings.c:719 INCLUDE_ASM anchor), floor 3 at 127/127; zero FAKE constructs present in any cell

## [s28] loop.c's fourteen move emitters are not the producer of target's preheader copy addu $a3, $a0, $zero at 0x800178D0.
- mechanism: loop.c:1702's emit_move_insn (m->set_dest, m->set_src) runs only when m->move_insn is set, and loop.c:657-670 sets that bit only for an invariant carrying a REG_EQUIV note or a CONSTANT REG_EQUAL note; the emitted source is then that note's expression, so a pseudo-to-pseudo copy is not expressible on that path. The remaining seven sites (loop.c:3940, 3948, 3972, 4065, 5868, 5891, 6098) are giv/biv initialisers and require strength reduction to have run on the loop.
- probe: End-to-end read of the fourteen loop.c call sites, then a structural read of asm/funcs/func_80017848.s for strength-reduction signatures.
- result: Target's loops are not strength-reduced: the address is recomputed from scratch every iteration as addu $v0, $a0, $v1 (0x800178DC in the preheader, 0x80017910 in the bottom delay slot) while the index is incremented separately by addiu $v1, $v1, 0x1 (0x800178FC). No incremented address register appears anywhere in the 127-insn listing, so no giv was created and the seven giv/biv emitters never ran; the movable path cannot express the copy.
- verdict: KILLED
- kill_scope: instance
- measured_on: target listing asm/funcs/func_80017848.s against the frozen tools/gcc-2.7.2/loop.c; no build, no FAKE constructs

## s29 (forensics, 2026-09-03)

## [s29] reload1.c's reload/inheritance emission is the producer of target's preheader copies (s28 frontier item 1).
- mechanism: reload1.c holds 13 emit_move_insn/gen_move_insn sites; if one of them can emit a REGISTER-to-REGISTER copy with no memory operand, reload could plant target's `addu $a3,$a0,$zero` without any source-level copy statement.
- probe: (a) structural read of all 13 sites - they are only `gen_reload` (reload1.c:6907/6935/6956/6957/6963) and `inc_for_reload` (reload1.c:7116/7137/7152/7154/7168); (b) a direct measurement on the candidate chassis: extract every `(insn ...)` UID from the func_80017848 region of the pre-reload `.lreg` dump (ings.lreg:7203-8162) and the post-reload `.greg` dump (ings.greg:6283-7030) and compare the sets.
- result: 88 insns in, 88 insns out, and `comm -13` over the sorted UID lists is EMPTY - reload added nothing at all to this function. Structurally `inc_for_reload` is auto-increment reload (MIPS has none) and `gen_reload`'s bare reg-reg path (reload1.c:6963) only runs for an operand `find_reloads` pushed a reload for, which an `addu` with two GENERAL_REGS inputs never is. With E-s28-1's census this closes the producer question: no pass in this compiler creates the copies, so they are present at expand and came from the C.
- verdict: KILLED
- kill_scope: instance
- measured_on: candidate chassis (memory/grind/func_80017848/candidate.c applied over the src/ings.c:719 INCLUDE_ASM anchor), floor 3 at 127/127, zero FAKE constructs present; dumps regenerated this session via `pwsh tools/grinder/dump.ps1 -Func func_80017848`

## [s29] Giving the preheader copy's destination a second SET (reg_n_sets == 2, one copy statement per loop into one shared local `q`) is what keeps combine from substituting it away.
- mechanism: the s16 reading was that combine refuses to substitute a pseudo it has seen set more than once, so writing `q = *(u8 **)(ctx + 0xC);` in both preheaders would preserve both copies at no instruction cost.
- probe: re-measured `rejected/s16_sym_shared_addend_local_reg_n_sets_2_costs_4.c` verbatim as cell A on the CURRENT chassis (mandated kill re-audit; FAKE-ablation is vacuous - no form of this function has ever carried a FAKE construct), then disassembled and diffed it (tmp/grind/func_80017848/s29/A_diff.txt).
- result: 4, at 127 target / 126 build insns. BOTH copies are absent: loop 1 collapses to `addu a0,a1,a0` (the missing 127th insn) and loop 2 emits `lw v0,12(s2)` / `addu a0,a1,v0`. Reading the gate directly confirms why: `can_combine_p` (combine.c:880-1030) and the `added_sets_2 = ! dead_or_set_p (i3, i2dest)` test (combine.c:1458) never consult `reg_n_sets` of i2dest at all.
- verdict: KILLED
- kill_scope: instance
- measured_on: chassis A (tmp/grind/func_80017848/s29/ings.orig.c = committed HEAD INCLUDE_ASM anchor + the cell body), no FAKE constructs

## [s29] Making the two preheaders source-symmetric by giving loop 2 its own named `lnk = *(u8 **)(ctx + 0x10);` between the copy and the base add - the shape target's `addu a3,a0,zero / lw a2,16(s2) / addu a0,a1,a3` literally shows - recovers loop 2's copy on the reg_n_sets-2 chassis.
- mechanism: target's two preheaders are byte-identical 4-insn sequences with identical register seats, which is the signature of one source construct written twice; the candidate's asymmetry (loop 1 has `lnk`, loop 2 reads ctx+0x10 inline in the body) is the obvious suspect.
- probe: cells B (cell A + named `lnk` in loop 2's preheader, body reads `lnk`), C (B + loop 1's guard written inline instead of two-step through `t`), D (B + loop 2's guard written two-step through `t` like loop 1), each applied over chassis A and scored with `sandbox --disable all`.
- result: B = 12 (126 build insns), C = 13 (126), D = 36 (126). Adding the named links local to loop 2 costs 8 points over cell A and does not restore either copy; the guard-shape variations move the score further away. Banked as rejected/s29_symmetric_lnk_local_in_loop2_preheader_costs_12.c, s29_symmetric_lnk_plus_inline_l1_guard_costs_13.c, s29_symmetric_lnk_plus_twostep_l2_guard_costs_36.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: chassis A (tmp/grind/func_80017848/s29/ings.orig.c + cell body), no FAKE constructs

## [s29] The 14-point residual of the s23 "exact instruction stream" form is an instruction-selection or ordering residual as well as a register one.
- mechanism: s23 recorded that the form emits target's entire instruction stream at 127/127 but scored 14; whether those 14 are register seats alone or also carry opcode/offset differences had never been separated, and the two need different tools.
- probe: re-measured `rejected/s23_crosses_set_p_reused_as_links_both_loops_exact_insn_stream_costs_14.c` as cell E on the current chassis; normalised both streams (tmp/grind/func_80017848/s29/E_T.txt, E_B.txt) and diffed them (E_diff.txt); then ran `python3 tools/ra_solver/inverse_compose.py classify ings func_80017848 --target-object build/src/ings.o --ours-object tmp/sandbox/func_80017848/ings.o`.
- result: 14 at 127/127, and the diff carries NO opcode, offset or operand-count difference anywhere - all 14 are the same instruction with different register seats, and they are one permutation repeated identically in both loops: pointer `$a0`->`$a1`, shift `$a1`->`$a0`, copy `$a3`->`$v0`, links `$a2`->`$a1`. classify prints `FIRST DIVERGENCE: RA / next tool: tools/ra_solver/inverse.py (global / local)` and lists exactly those four pairs. The func_80045294-s57 classify mis-typing hazard does not apply: its disambiguator (is the register already correct on the pseudo's OTHER references?) comes back NO - every reference of every pseudo carries the wrong seat consistently, which is a genuine allocation divergence.
- verdict: KILLED
- kill_scope: instance
- measured_on: chassis A (tmp/grind/func_80017848/s29/ings.orig.c + cell E body), floor 3 chassis, no FAKE constructs

## [s29] Target's preheader copies are bought through combine's `use_crosses_set_p` escape, and that escape is what costs the seats.
- mechanism: `can_combine_p` refuses the substitution at combine.c:914-917 when `! all_adjacent` and a register used in i2's SOURCE is set between i2 and i3. i2's source is the single pseudo `p`, so the intervening insn must assign `p` - i.e. the C must reuse the pointer variable to hold the links pointer. Cell E does exactly that and is the only banked form that reproduces both copies.
- probe: read of combine.c:880-1030 and combine.c:1458 to enumerate every escape (added_sets_2 / dead_or_set_p; use_crosses_set_p; the last_call_cuid call-crossing test), plus flow.c:2102's `BLOCK_NUM (y) == blocknum` LOG_LINK gate; cross-checked against cell E's measured stream and cell A's measured collapse.
- result: exactly three combine escapes exist and only two are reachable here (the call-crossing one needs a CALL in the preheader). The candidate spends escape 1 on loop 1 (`p = q` as a downstream second use, which materialises as `addu a0,a3,zero` where target has `lw a0,12(s2)` - the 3-point residual). Cell E spends escape 2 on both loops and gets the exact stream, but reusing `p` for links makes the links value live across the whole loop body, so `p` is seated at `$a1` while the short-lived shift takes `$a0`, the reverse of target which keeps links in its own pseudo at `$a2`. The escape and the seating are coupled through one source decision.
- verdict: CONFIRMED

## [s29] reload1.c's reload/inheritance emission is the producer of target's preheader copies at 0x800178D0 and 0x80017930 (the s28 frontier's last unmeasured producer).
- mechanism: reload1.c holds 13 emit_move_insn/gen_move_insn sites; if one can emit a REGISTER-to-REGISTER copy with no memory operand, reload could plant target's addu $a3,$a0,$zero with no source-level copy statement. The 13 sites are only gen_reload (reload1.c:6907/6935/6956/6957/6963) and inc_for_reload (reload1.c:7116/7137/7152/7154/7168); inc_for_reload is auto-increment reload, and gen_reload's bare reg-reg path at reload1.c:6963 runs only for an operand find_reloads pushed a reload for.
- probe: Structural read of all 13 sites, plus a direct measurement: extract every (insn ...) UID from the func_80017848 region of the pre-reload .lreg dump (ings.lreg:7203-8162) and the post-reload .greg dump (ings.greg:6283-7030) of the candidate chassis and compare the sets (dumps regenerated this session with pwsh tools/grinder/dump.ps1 -Func func_80017848).
- result: 88 insns in, 88 insns out, and comm -13 over the sorted UID lists is EMPTY - reload added nothing whatsoever to this function. An addu whose two inputs are already GENERAL_REGS pushes no reload, so gen_reload's reg-reg path is not reachable at that site, and MIPS has no auto-inc for inc_for_reload. Together with E-s28-1's emit_move_insn census this closes the producer question that has driven the last three sessions: no optimisation pass in this compiler creates the copies, so they are in the RTL at expand and came from the C.
- verdict: KILLED
- kill_scope: instance
- measured_on: candidate chassis (memory/grind/func_80017848/candidate.c applied over the src/ings.c:719 INCLUDE_ASM anchor), floor 3 at 127/127, rules_dropped 0, zero FAKE constructs present

## [s29] Giving the preheader copy's destination a second SET (reg_n_sets == 2, one copy statement per loop writing one shared local q) is what keeps combine from substituting the copy away.
- mechanism: The s16 reading was that combine refuses to substitute a pseudo it has seen set more than once, so a shared q written in both preheaders would preserve both copies at no instruction cost. This was the mandated kill re-audit target: the closest banked score-4 form.
- probe: Re-measured rejected/s16_sym_shared_addend_local_reg_n_sets_2_costs_4.c verbatim as cell A on the CURRENT chassis, then disassembled and diffed it against the target stream (tmp/grind/func_80017848/s29/A_diff.txt). FAKE-ablation is vacuous here - no form of this function has ever carried a FAKE construct, so tools/fake_ablate.py has nothing to remove.
- result: 4, at 127 target / 126 build insns - the kill holds on the current chassis. Both copies are absent from the build: loop 1 collapses to addu a0,a1,a0 (that collapse IS the missing 127th insn) and loop 2 emits lw v0,12(s2) / addu a0,a1,v0. Reading the gate confirms why: neither can_combine_p (combine.c:880-1030) nor the added_sets_2 = ! dead_or_set_p (i3, i2dest) test at combine.c:1458 consults reg_n_sets of i2dest at all.
- verdict: KILLED
- kill_scope: instance
- measured_on: chassis A (tmp/grind/func_80017848/s29/ings.orig.c = committed HEAD INCLUDE_ASM anchor, plus the cell body), floor-3 chassis, no FAKE constructs

## [s29] Making the two preheaders source-symmetric by giving loop 2 its own named lnk = *(u8 **)(ctx + 0x10); between the copy and the base add - the shape target's addu a3,a0,zero / lw a2,16(s2) / addu a0,a1,a3 literally shows - recovers loop 2's copy on the reg_n_sets-2 chassis.
- mechanism: Target's two preheaders are byte-identical 4-insn sequences with identical register seats, the signature of one source construct written twice; the standing forms are asymmetric (loop 1 carries a named lnk, loop 2 reads ctx+0x10 inline in the body), so the asymmetry was the obvious suspect.
- probe: Cells B (cell A plus a named lnk in loop 2's preheader, body reading lnk), C (B plus loop 1's guard written inline instead of two-step through t), D (B plus loop 2's guard written two-step through t like loop 1); each applied over chassis A and scored with sandbox --disable all.
- result: B = 12 (126 build insns), C = 13 (126), D = 36 (126). The named links local costs 8 points over cell A and restores neither copy; both guard-shape variations move further away. Banked as rejected/s29_symmetric_lnk_local_in_loop2_preheader_costs_12.c, rejected/s29_symmetric_lnk_plus_inline_l1_guard_costs_13.c, rejected/s29_symmetric_lnk_plus_twostep_l2_guard_costs_36.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: chassis A (tmp/grind/func_80017848/s29/ings.orig.c plus the cell body), floor-3 chassis, no FAKE constructs

## [s29] The 14-point residual of the s23 crosses-set form, which s23 recorded as emitting target's entire instruction stream, carries instruction-selection or ordering differences as well as register ones.
- mechanism: s23 recorded 127/127 with score 14 but never separated the two; opcode/offset residuals and register-seat residuals need different tools (source respelling vs the RA solver), so the classification decides where the next sessions spend their measurements.
- probe: Re-measured rejected/s23_crosses_set_p_reused_as_links_both_loops_exact_insn_stream_costs_14.c as cell E on the current chassis; normalised both instruction streams (tmp/grind/func_80017848/s29/E_T.txt, E_B.txt) and diffed them (E_diff.txt); then ran python3 tools/ra_solver/inverse_compose.py classify ings func_80017848 --target-object build/src/ings.o --ours-object tmp/sandbox/func_80017848/ings.o.
- result: 14 at 127/127, and the diff carries no opcode, no offset and no operand-count difference anywhere in the 127 instructions. All 14 are the same instruction with different register seats, and they are ONE permutation repeated identically in both loops: pointer $a0 -> $a1, shift $a1 -> $a0, preheader copy $a3 -> $v0, links $a2 -> $a1. classify prints FIRST DIVERGENCE: RA / next tool: tools/ra_solver/inverse.py (global / local) and lists exactly those four pairs. The func_80045294-s57 classify mis-typing hazard does not apply: its disambiguator asks whether the divergent register is already correct on the pseudo's OTHER references, and here every reference of every pseudo carries the wrong seat consistently, which is a genuine allocation divergence rather than a cse/canon_reg operand substitution.
- verdict: KILLED
- kill_scope: instance
- measured_on: chassis A (tmp/grind/func_80017848/s29/ings.orig.c plus the cell E body), floor-3 chassis, no FAKE constructs

## [s29] Target's two preheader copies are bought through combine's use_crosses_set_p escape, and that same escape is what displaces the register seats in cell E.
- mechanism: can_combine_p refuses the substitution at combine.c:914-917 when ! all_adjacent and a register used in i2's SOURCE is set between i2 and i3. i2's source is the single pseudo p, so the intervening insn must assign p - i.e. the C must reuse the pointer variable to hold the links pointer. Only two other escapes exist: added_sets_2 = ! dead_or_set_p (i3, i2dest) at combine.c:1458 (a live downstream use of the copy's dest, which forces a 2-set PARALLEL that fails recog on MIPS), and the CALL-crossing test at combine.c:928, which needs a call in the preheader and is unreachable here. flow.c:2102 additionally withholds the LOG_LINK entirely when the copy and its consumer are in different basic blocks, but every zero-cost spelling of that split is already priced (s26 pre-join carriers 12, s9/s21 q-preinit 11/12).
- probe: End-to-end read of combine.c:880-1030 and combine.c:1440-1470 to enumerate every escape, plus flow.c:2075-2110's LOG_LINK gate, cross-checked against cell E's measured exact stream and cell A's measured copy collapse.
- result: The candidate spends escape 1 on loop 1 (p = q in the exit tail as the downstream second use), which materialises as addu a0,a3,zero where target has lw a0,12(s2) - that IS the standing 3-point residual. Cell E spends escape 2 on both loops and reaches the exact stream, but reusing p for the links pointer makes the links value live across the whole loop body, so p is seated at $a1 while the short-lived shift takes $a0, the reverse of target which keeps links in its own pseudo at $a2. The escape and the seating are coupled through one source decision, which is the precise thing the next session has to break.
- verdict: CONFIRMED
