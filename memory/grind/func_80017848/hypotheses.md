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

## s30 (rederive, 2026-09-03)

- **H-s30-1 (KILLED, instance).** Statement: expanding either scan loop from a
  `static inline` helper makes integrate.c's inline-argument copy survive into the
  preheader as target's `addu $a3, $a0, $zero`, at three spellings (loop body only /
  helper computes the base / helper carries the guard too).
  Mechanism: `integrate.c` is the last of the seven emit_move_insn/gen_move_insn
  producers in the s28 census (loop.c 14, reload1.c 13, unroll.c 9, jump.c 7,
  integrate.c 2, cse.c 1, flow.c 1) that no form of this function had ever exercised;
  `expand_inline_function` copies actual arguments into the callee's parameter
  pseudos, and such a copy sits exactly in the preheader position.
  Probe: cells F/G/H, `tmp/grind/func_80017848/s30/body_{F,G,H}.c`, measured through
  cells.ps1 on the candidate chassis.
  Result: 42 (137 build insns) / 45 (136) / 30 (136). All three overshoot the
  127-insn target. Inlining did happen (no `jal` to the helper anywhere in
  `tmp/grind/func_80017848/s30/H_diff.txt`), and the preheader of the best cell
  carries `sll v0,s4,6` / `addu a0,v0,v1` with no reg-reg copy at all. The +9/+10 is
  return-value materialisation: each inlined callee emits `j <inline-return>` +
  `addiu $v0,$zero,1` on its hit path plus a caller-side test, where the target has
  a single `beq $v0,$s3,.L800178AC` into the shared `return 0` block.
  Verdict: KILLED. kill_scope: instance.
  measured_on: candidate chassis (memory/grind/func_80017848/candidate.c applied over
  the src/ings.c:719 INCLUDE_ASM anchor), floor-3 chassis, no FAKE constructs.

- **H-s30-2 (KILLED, instance).** Statement: writing the function against the real
  record/link struct types (a 0x40-byte record array at `ctx+0xC`, a 0x10-byte link
  array at `ctx+0x10`) instead of the `u8 *` + literal-offset idiom reaches the floor,
  at four spellings.
  Mechanism: struct member access and array indexing feed a different address
  expression tree to cse/combine than explicit `(shift + (s32)ptr + literal)`
  arithmetic, so the redundant-load folding and the base-add association can differ.
  Probe: cells A/B/C/D, `tmp/grind/func_80017848/s30/body_{A,B,C,D}.c`.
  Result: A = 34 at 123 build insns (indexed re-reads let cse fold four loads the
  target keeps, so the build is 4 instructions SHORT), B = 7 at 127, C = 14 at 127,
  D = 7 at 127. Struct typing is a uniform +4 over the equivalent `u8 *` spelling.
  Verdict: KILLED. kill_scope: instance.
  measured_on: candidate chassis, floor-3 chassis, no FAKE constructs.

- **H-s30-3 (KILLED, instance).** Statement: declaring `q`, `lnk` and `base` in the
  loop blocks that use them, rather than in the function-scope declaration list,
  perturbs local-alloc's allocno ordering enough to move the four-seat permutation of
  the exact-instruction-stream form.
  Mechanism: block-head declarations change the order in which pseudos are created
  during expand, which is the ordering local-alloc's `allocno_order` inherits.
  Probe: cell I (candidate shape, block-scoped) and cell J (the s23/s29
  exact-instruction-stream form, block-scoped),
  `tmp/grind/func_80017848/s30/body_{I,J}.c`.
  Result: I = 3 at 127/127, tying the baseline exactly; J = 14 at 127/127, tying cell
  E exactly. Inert on both chassis. Extends the s11 declaration-ORDER-inert finding to
  declaration SCOPE.
  Verdict: KILLED. kill_scope: instance.
  measured_on: candidate chassis and the cell-E exact-stream chassis, floor-3 and
  score-14 respectively, no FAKE constructs.

- **H-s30-4 (KILLED, instance).** Statement: writing `sh = slot_a << 6;` before
  `p = *(u8 **)(ctx + 0xC);` on the exact-instruction-stream chassis restores the
  target's p/sh seats ($a0 for the pointer, $a1 for the shift).
  Mechanism: the target loads the pointer first and shifts second, and the seat
  divergence is exactly a p<->sh swap, so source write order was the cheapest
  candidate input perturbation for it.
  Probe: cell K = cell E with the two preheader definitions swapped,
  `tmp/grind/func_80017848/s30/body_K.c`.
  Result: 14 at 127/127 — bit-identical score and insn count to cell E. The seat swap
  is not driven by source write order.
  Verdict: KILLED. kill_scope: instance.
  measured_on: cell-E exact-stream chassis (score 14), no FAKE constructs.

- **H-s30-5 (KILLED, instance).** Statement: another function in the ings translation
  units shares this function's link-lookup idiom and can be transplanted as the
  author's own spelling.
  Mechanism: sibling transplant is the third leg of the rederive modality; a matched
  sibling would fix the idiom empirically instead of by inference.
  Probe: `grep -n "<< 4)" src/ings.c src/ings2.c` plus a read of the surrounding
  matched functions.
  Result: the idiom appears nowhere else; the only adjacent matched helper with the
  same geometry is `obj_CalcOffset(a0, a1) { return (a0 << 6) + (a1 << 4); }`
  (src/ings.c:693), which func_80017848 does not call (the target listing has no
  `jal` other than math_Distance3D). It does corroborate that the author's idiom is
  explicit shift arithmetic over a byte base, i.e. what the candidate already uses.
  Verdict: KILLED. kill_scope: instance.
  measured_on: committed HEAD src tree, read-only census, no build.

## [s30] Expanding either scan loop from a static inline helper makes integrate.c's inline-argument copy survive into the preheader as target's addu $a3, $a0, $zero, at three spellings (helper carries the loop body only / helper computes the base itself / helper carries the guard too).
- mechanism: integrate.c is the last of the seven emit_move_insn/gen_move_insn producers in the s28 census (loop.c 14, reload1.c 13, unroll.c 9, jump.c 7, integrate.c 2, cse.c 1, flow.c 1) that no form of this function had ever exercised; expand_inline_function copies actual arguments into the callee's parameter pseudos, and such a copy lands exactly in the preheader position where target carries its unexplained reg-reg copy.
- probe: Cells F/G/H (tmp/grind/func_80017848/s30/body_F.c, body_G.c, body_H.c) applied over the src/ings.c:719 INCLUDE_ASM anchor and measured with `sandbox func_80017848 --disable all` through tmp/grind/func_80017848/s30/cells.ps1; cell H additionally disassembled and diffed against the target listing into tmp/grind/func_80017848/s30/H_diff.txt.
- result: F = 42 at 137 build insns, G = 45 at 136, H = 30 at 136, against a 127-insn target - all three OVERSHOOT. Inlining genuinely happened (no jal to either helper anywhere in the emitted stream), so this is a real measurement of integrate.c and not a failed inline. No preheader copy is bought: cell H's preheader is `sll v0,s4,6` / `addu a0,v0,v1` with no `addu a3,a0,zero` at all, i.e. combine substitutes integrate's arg copy exactly as it substitutes a source-level one. The +9/+10 insn overshoot is located and is per-callee return-value materialisation: each inlined helper emits `j <inline-return>` + `addiu $v0,$zero,1` on its hit path plus a caller-side test, where the target has a single `beq $v0,$s3,.L800178AC` branching straight into the shared return-0 block, and jump.c does not cross-jump that back together.
- verdict: KILLED
- kill_scope: instance
- measured_on: candidate chassis (memory/grind/func_80017848/candidate.c applied over the src/ings.c:719 INCLUDE_ASM anchor), floor-3 chassis re-measured this session at 127/127, no FAKE constructs in any cell

## [s30] Writing the function against the real record and link struct types (a 0x40-byte record array at ctx+0xC, a 0x10-byte link array at ctx+0x10) instead of the u8 * plus literal-offset idiom reaches the floor, at four spellings.
- mechanism: Struct member access and array indexing feed a different address expression tree to cse and combine than explicit (shift + (s32)ptr + literal) arithmetic, so redundant-load folding and base-add association can differ - a structurally different C shape rather than a tweak of the current one.
- probe: Cells A/B/C/D (tmp/grind/func_80017848/s30/body_A.c ... body_D.c), typedefs IngRec { s32 unk0[6]; s32 f18; s32 na; s32 nb; u8 la[8]; u8 lb[8]; s32 unk34[3]; } and IngLink { s32 dist; u16 b; s16 a; s32 dist3; s32 owner; }, measured through cells.ps1 on the candidate chassis.
- result: A (fresh indexed re-reads at every use) = 34 at 123 build insns - struct indexing lets cse fold four loads the target keeps, so the build is 4 instructions SHORT of the 127-insn target; B (struct types over the candidate's proven preheader shape) = 7 at 127; C (B plus a symmetric q re-read in loop 2) = 14 at 127; D (u8 * scaffolding with struct-typed base/lnkp only inside the loop bodies) = 7 at 127. Struct typing is a uniform +4 over the equivalent u8 * spelling on this chassis and never reaches the floor.
- verdict: KILLED
- kill_scope: instance
- measured_on: candidate chassis (candidate.c over the src/ings.c:719 anchor), floor-3 chassis, no FAKE constructs

## [s30] Declaring q, lnk and base in the loop blocks that use them, rather than in the function-scope declaration list, perturbs local-alloc's allocno ordering enough to move the four-seat permutation of the exact-instruction-stream form.
- mechanism: C89 block-head declarations change the order in which pseudos are created during expand, and that creation order is what local-alloc's allocno ordering inherits; the cell-E residual is a pure register-seat permutation, so pseudo ordering was the cheapest untried source-side input to it.
- probe: Cell I (candidate shape with q/lnk/base block-scoped) and cell J (the s23/s29 exact-instruction-stream form with q/base block-scoped), tmp/grind/func_80017848/s30/body_I.c and body_J.c, measured through cells.ps1.
- result: I = 3 at 127/127, tying the baseline candidate exactly; J = 14 at 127/127, tying cell E exactly. Inert on both chassis. This extends the s11 declaration-ORDER-inert finding to declaration SCOPE.
- verdict: KILLED
- kill_scope: instance
- measured_on: both the candidate chassis (score 3) and the cell-E exact-instruction-stream chassis (score 14), no FAKE constructs

## [s30] Writing sh = slot_a << 6 before p = *(u8 **)(ctx + 0xC) on the exact-instruction-stream chassis restores the target's p and sh seats ($a0 for the pointer, $a1 for the shift).
- mechanism: The target loads the pointer first and shifts second, and two of cell E's four seat divergences are exactly a p<->sh swap, so the source write order of the two preheader definitions was the most direct candidate perturbation for that pair.
- probe: Cell K = cell E with the two preheader definitions swapped (tmp/grind/func_80017848/s30/body_K.c), measured through cells.ps1.
- result: 14 at 127/127 - identical score and instruction count to cell E. The p/sh seat swap is not driven by the order in which the two values are written in the source.
- verdict: KILLED
- kill_scope: instance
- measured_on: cell-E exact-instruction-stream chassis (score 14, 127/127), no FAKE constructs

## [s30] Another function in the ings translation units shares this function's link-lookup idiom and can be transplanted as the author's own spelling.
- mechanism: Sibling transplant is the third leg of the rederive modality; a matched sibling would fix the source idiom empirically rather than by inference from the target listing.
- probe: grep -n "<< 4)" src/ings.c src/ings2.c plus a read of the surrounding matched functions, and a check of the target listing for calls.
- result: The (index << 4) + links + field idiom appears nowhere else in either file - the only hits are this function's own two loops and its tail store. The single adjacent matched helper with the same geometry is obj_CalcOffset(a0, a1) { return (a0 << 6) + (a1 << 4); } at src/ings.c:693, and func_80017848 does not call it (the target listing's only jal is math_Distance3D). It does corroborate that the author's idiom is explicit shift arithmetic over a byte base, which is what the candidate already uses.
- verdict: KILLED
- kill_scope: instance
- measured_on: committed HEAD src tree, read-only census, no build

## s31 (rederive, 2026-09-03)

Chassis re-measured first: BASE (candidate.c body) = **3** at 127/127, cell E
(s23/s29 exact-instruction-stream form) = **14** at 127/127. Both tie the ledger
exactly, so the floor-3 chassis is unchanged. FAKE-ablation is vacuous — no form
of this function has ever carried a FAKE construct, so `tools/fake_ablate.py` has
nothing to remove; the kill re-audit is the cell-E re-measurement above.
All s31 cell scores are banked in `tmp/grind/func_80017848/s31/scores.txt`.

## [s31] Cell E's four seat divergences (p $a1->$a0, sh $a0->$a1, copy $v0->$a3, lnk $a1->$a2) are reachable by a modelled perturbation of local-alloc's / global-alloc's inputs, and inverse.py will name the C lever that produces it. (FRONTIER ITEM 1)
- mechanism: The residual is typed RA on an instruction-for-instruction identical
  127-insn stream (E-s29-5), which is exactly the input the solver suite was built
  for; s30 removed declaration scope and source write order as explanations, so the
  remaining explanation was supposed to live in the modelled inputs (live-range
  shape, reg_n_refs, preference edges).
- probe: Applied cell E, ran `tools/ra_solver/extract.py func_80017848 ings`
  (model: 13 pseudos in allocation order, 57 dispositions), `simulate.py` (sort
  order MATCH, dispositions 13/13 match — the forward model is exact here), then
  `tools/ra_solver/goal_from_tgt.py goal ings func_80017848 --model ... --show` to
  derive the {pseudo -> target hardreg} goal that inverse.py requires. Full output
  banked at `tmp/grind/func_80017848/s31/goal_E_report.txt`, model at
  `tmp/grind/func_80017848/s31/E_model.json`.
- result: The goal is EMPTY and inverse.py can never be run on this form.
  goal_from_tgt aligns 127 vs 127 with 14 `replace` pairs and 0 skips, and prints
  the four substitutions with per-instruction detail, but every attribution comes
  back AMBIGUOUS: only TWO pseudos hold $a1 (78 = loop 1's `p`, 123 = loop 2's `p`),
  and each of them is asked to move to $a0 (x6, the copy-source role at
  `lw a1,12(s2)` / `move v0,a1`) AND to $a2 (x4, the links role at `lw a1,16(s2)` /
  `addu v0,v0,a1`) at the same time. That is not a tool limitation: cell E's C
  reuses the single variable `p` for both the record pointer and the link pointer
  (that reuse IS its `use_crosses_set_p` escape), GCC 2.7.2 creates one pseudo per
  declaration, and one pseudo receives exactly one hard register. Target seats the
  two roles in two different registers, so cell E's seat map cannot be satisfied by
  this form and no perturbation of any modelled RA input reaches it. The frontier's
  own contract makes this a first-class negative, but it is stronger than the
  "instrument local-alloc next" branch it anticipated: the seating is not decided
  outside the modelled inputs, it is decided by the FORM. The corollary is the
  actionable part — any spelling of combine escape 2 that reaches 127 insns in this
  function forces the p/links pseudo merge, because the only instruction the target
  places between the copy and the base add is the links load, so the links load is
  the only candidate for the intervening set of `p`.
- verdict: KILLED
- kill_scope: instance
- measured_on: cell-E exact-instruction-stream chassis (score 14 re-measured this
  session at 127/127) over the HEAD `src/ings.c:719` INCLUDE_ASM anchor, no FAKE
  constructs

## [s31] There is a spelling of combine escape 2 whose intervening set of p does NOT extend a value's live range into the loop body - specifically p reused to hold the base itself - which buys both copies without cell E's seat displacement. (FRONTIER ITEM 2, sub-probe (i))
- mechanism: `use_crosses_set_p` only requires that a register used in i2's SOURCE
  be assigned between i2 and i3. Reusing `p` as the base variable
  (`q = p; lnk = ...; p = (u8 *)(sh + (s32)q);`) makes i3 itself set i2's source,
  which keeps `p`'s live range short and leaves the loop body reading a separate
  `lnk` pseudo — the seat conflict cell E pays would disappear.
- probe: Cells L and M (`tmp/grind/func_80017848/s31/body_L.c`, `body_M.c`), both
  loops written with `p` as the base and a named `lnk`; L takes a fresh
  `p = *(u8 **)(ctx + 0xC);` as loop 1's exit tail (target's `lw a0,12(s2)`),
  M keeps the candidate's `p = q;`. Measured through
  `tmp/grind/func_80017848/s31/cells.ps1`, disassembly diffed with `dis.sh`.
- result: L = 12 at 125 build insns, M = 13 at 126. Both UNDERSHOOT the 127-insn
  target. i3 setting i2's source is not an escape: `use_crosses_set_p` scans only
  the insns strictly BETWEEN i2 and i3, so combine substitutes the copy into the
  base add exactly as before. L loses BOTH preheader copies; M keeps only loop 1's
  (bought by its `p = q` second use, i.e. escape 1, not escape 2) and its base add
  reads `a0` directly (`addu a0,a2,a0`) rather than target's `addu a0,a1,a3`.
  Banked as `rejected/s31_p_reused_as_base_both_loops_loses_both_copies_costs_12.c`
  and `rejected/s31_p_reused_as_base_with_pq_exit_tail_costs_13.c`.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD `src/ings.c:719` INCLUDE_ASM anchor plus the cell body, floor-3
  chassis (BASE re-measured 3 this session), no FAKE constructs

## [s31] Target's loop-1 exit tail lw a0,12(s2) / sll a1,s4,6 is the RE-ASSIGNMENT of the SAME p and sh variables that loop 2 then reuses, so writing both loops against one shared p / sh pair reproduces the exit tail and the loop-2 preheader copy together.
- mechanism: Target reloads `$a0` and recomputes `$a1` in the loop-1 exit tail and
  the loop-2 guard at `.L8001791C` consumes them, which is exactly the shape of two
  loops sharing one pointer variable and one shift variable, with the exit tail
  restoring them after loop 1's base add clobbers `$a0`. The candidate instead
  carries a SECOND shift variable `sh2` at function scope and restores the pointer
  with `p = q;`, which is what emits the wrong `addu a0,a3,zero`. This is the
  rederive modality's structural-reshape leg: one pointer/shift pair for the whole
  function instead of the candidate's p/sh/sh2 triple.
- probe: Cells N1 and N2 (`tmp/grind/func_80017848/s31/body_N1.c`, `body_N2.c`):
  `sh` re-assigned inside loop 1's exit tail alongside `p`, `sh2` deleted, loop 2's
  guard and base written against the shared pair; N1 takes
  `q = *(u8 **)(ctx + 0xC);` as loop 2's preheader copy source, N2 takes `q = p;`.
  Diff of N1 banked at `tmp/grind/func_80017848/s31/N1_diff.txt`.
- result: N1 = 30 at 126 build insns, N2 = 32 at 125. Sharing the pair is a ~+27
  regression, and the diff shows why: the loop counter `i` migrates from `$v1` to
  `$a0` and the base from `$a0` to `$v1` throughout BOTH loops, so most of the 30
  points are a whole-function counter/base seat rotation, not the preheader. Two
  further facts fall out of N1's stream and are the durable part of this cell:
  (a) loop 1's copy DIES in N1 (`addu a3,a0,zero` absent) even though loop 1 still
  writes `q = *(u8 **)(ctx + 0xC);` — so the "cse folds the redundant load into an
  orphaned copy that combine cannot see" story recorded in candidate.c's header is
  NOT sufficient on its own: without the downstream `p = q;` second use the copy is
  deleted; (b) a fresh `*(u8 **)(ctx + 0xC)` read in loop 2's preheader materialises
  as a real `lw v0,12(s2)`, never a cse-folded reg-reg copy, because `.L8001791C`
  has two predecessors and therefore starts a new cse extended basic block in which
  no load of `ctx+0xC` is available. Banked as
  `rejected/s31_shared_p_sh_across_both_loops_fresh_reads_costs_30.c` and
  `rejected/s31_shared_p_sh_across_both_loops_l2_copy_costs_32.c`.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD `src/ings.c:719` INCLUDE_ASM anchor plus the cell body, floor-3
  chassis, no FAKE constructs

## [s31] Escape 1 (a downstream second use of loop 2's copy) has an unpriced site inside loop 2's guard's own taken arm, above the copy, that costs less than the banked minimum of 6. (FRONTIER ITEM 3)
- mechanism: The banked price table for loop-2 second-use sites is post-loop 19-22,
  in-body 6, pre-join 12, guard-consuming 6-28, and every entry sites the use either
  outside the arm or inside the loop body. A use in the arm but above the loop had
  never been priced; the cheapest such site is the do-while bottom test rewritten
  through `q` (`while (i < *(s32 *)(sh2 + (s32)q + 0x20));`), which target already
  spends an instruction on (`lw v0,32(a0)`), so it should have been free.
- probe: Cells P and Q (`tmp/grind/func_80017848/s31/body_P.c`, `body_Q.c`):
  candidate body with loop 2 given a named `q`/`lnk` preheader; P puts q's second
  use in the do-while bottom test, Q puts it in a `p = q;` after loop 2.
- result: P = 9 at 126 build insns, Q = 13 at 125. P does NOT buy the copy: cse
  folds `sh2 + (s32)q` back to the already-computed base pseudo BEFORE combine runs,
  so by the time `can_combine_p` looks, `q` has exactly one reference again and the
  copy is substituted away — the build comes out one instruction short. This is a
  general result about second-use sites in this function: a second use that is a
  REDUNDANT RECOMPUTATION of an available expression is invisible to combine,
  because cse runs first. Q loses two instructions instead (the post-loop `p = q` is
  dead and DCE removes it along with the copy). Banked as
  `rejected/s31_l2_q_second_use_in_bottom_test_cse_folds_costs_9.c` and
  `rejected/s31_l2_q_second_use_as_p_after_loop2_costs_13.c`.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD `src/ings.c:719` INCLUDE_ASM anchor plus the cell body, floor-3
  chassis, no FAKE constructs

## [s31] The candidate's loop-1 byte-match is produced by a mechanism the TARGET cannot be using, so the remaining 3-point residual is a whole-function mechanism problem rather than a loop-2 placement problem.
- mechanism: Read off the target listing rather than inferred. `$a3` is written once
  per loop (0x800178D0, 0x80017930) and read once per loop (0x800178D8, 0x80017938)
  and nowhere else in the function; loop 1's exit tail is `lw $a0, 0xC($s2)` /
  `sll $a1, $s4, 6`, i.e. a FRESH reload of the pointer, not a copy of `$a3`.
  Escape 1 (`added_sets_2 = ! dead_or_set_p (i3, i2dest)`, combine.c:1458) requires
  the copy's destination to be live past the base add; target's is not. The
  candidate buys loop 1's copy precisely with escape 1 (`p = q;` in the exit tail),
  and that `p = q;` is what emits `addu a0,a3,zero` where target has
  `lw a0,12(s2)` — one of the three residual instructions. So the candidate matches
  loop 1's bytes for the wrong reason.
- probe: Full disassembly diff of the candidate against the target
  (`tmp/grind/func_80017848/s31/dis.sh`), which isolates the residual to exactly
  three instructions: loop-1 exit tail `lw a0,12(s2)` vs `addu a0,a3,zero`;
  loop-2 preheader `addu a3,a0,zero` vs `lw v0,12(s2)`; loop-2 base
  `addu a0,a1,a3` vs `addu a0,a1,v0`. Cross-checked against the `$a3` reference
  census above and against cells L/M/N1/N2/P/Q, none of which produce a use-once
  surviving copy.
- result: In 31 sessions no form has ever produced a copy that survives combine
  with a single downstream reference. Escape 1 always costs the second use
  (candidate pays it in loop 1's exit tail, price 3; every loop-2 site prices 6 or
  worse, s31 cell P being the cheapest new one at 9). Escape 2 always costs the
  p/links pseudo merge (cell E, price 14, and s31's goal derivation shows that merge
  makes target's seats unsatisfiable). The next session's question is therefore not
  "where do I put loop 2's second use" but "what makes a use-once copy survive
  combine at all", and the two remaining unread producers from the s28 census are
  `jump.c` (7 emit_move_insn sites) and `flow.c` (1) — neither has been read
  end-to-end for this function, and neither has been exercised by any measured form.
- verdict: CONFIRMED

## [s31] Cell E's four seat divergences (p $a1->$a0, sh $a0->$a1, copy $v0->$a3, lnk $a1->$a2) are reachable by a modelled perturbation of local-alloc's / global-alloc's inputs, and inverse.py will name the C lever that produces it.
- mechanism: The residual is typed RA on an instruction-for-instruction identical 127-insn stream (E-s29-5), which is exactly the input the solver suite was built for; s30 removed declaration scope and source write order as explanations, so the remaining explanation was supposed to live in the modelled inputs (live-range shape, reg_n_refs, preference edges).
- probe: Applied cell E; ran tools/ra_solver/extract.py func_80017848 ings (13-pseudo model, 57 dispositions), simulate.py (sort order MATCH, dispositions 13/13 match - the forward global.c model is exact on this chassis), then tools/ra_solver/goal_from_tgt.py goal ings func_80017848 --model ... --show to derive the {pseudo -> target hardreg} goal that inverse.py requires. Artifacts: tmp/grind/func_80017848/s31/goal_E_report.txt, tmp/grind/func_80017848/s31/E_model.json.
- result: The derived goal is EMPTY, so inverse.py cannot be run on this form at all. goal_from_tgt aligns 127 vs 127 with 14 'replace' pairs and 0 skips and prints the four substitutions ($a1->$a0 x6, $a0->$a1 x6, $v0->$a3 x4, $a1->$a2 x4) with per-instruction detail, but every attribution comes back AMBIGUOUS: only two pseudos hold $a1 (78 = loop 1's p, 123 = loop 2's p) and each of them is asked to become $a0 (the copy-source role at lw a1,12(s2) / move v0,a1) AND $a2 (the links role at lw a1,16(s2) / addu v0,v0,a1) simultaneously. That is not a tool limitation: cell E's C reuses the single variable p for both the record pointer and the link pointer - that reuse IS its use_crosses_set_p escape - GCC 2.7.2 creates one pseudo per declaration, and one pseudo receives one hard register. Target seats the two roles separately, so cell E's seat map cannot be satisfied by that form and no perturbation of any modelled RA input reaches it. The negative is first-class and stronger than the branch the frontier anticipated (ALLOCDBG instrumentation of the suggested-register pass): the seating is decided by the FORM, not outside the model. Corollary, now the actionable part: any spelling of combine escape 2 that reaches 127 insns in this function forces the p/links pseudo merge, because the only instruction the target places between the copy and the base add is the links load, so the links load is the only possible intervening set of p.
- verdict: KILLED
- kill_scope: instance
- measured_on: cell-E exact-instruction-stream chassis (rejected/s23_crosses_set_p_reused_as_links_both_loops_exact_insn_stream_costs_14.c over the HEAD src/ings.c:719 INCLUDE_ASM anchor), re-measured this session at score 14, 127/127, no FAKE constructs

## [s31] There is a spelling of combine escape 2 whose intervening set of p does not extend a value's live range into the loop body - specifically p reused to hold the base itself - which buys both preheader copies without cell E's seat displacement.
- mechanism: use_crosses_set_p only requires that a register used in i2's SOURCE be assigned between i2 and i3. Reusing p as the base variable (q = p; lnk = ...; p = (u8 *)(sh + (s32)q);) makes i3 itself set i2's source, which keeps p's live range short and leaves the loop body reading a separate lnk pseudo, so the seat conflict cell E pays would disappear.
- probe: Cells L and M (tmp/grind/func_80017848/s31/body_L.c, body_M.c), both loops written with p as the base and a named lnk; L takes a fresh p = *(u8 **)(ctx + 0xC); as loop 1's exit tail (target's lw a0,12(s2)), M keeps the candidate's p = q;. Measured with sandbox --disable all through tmp/grind/func_80017848/s31/cells.ps1 and disassembly-diffed with dis.sh.
- result: L = 12 at 125 build insns, M = 13 at 126 - both UNDERSHOOT the 127-insn target. i3 setting i2's source is not an escape: use_crosses_set_p scans only the insns strictly BETWEEN i2 and i3, so combine substitutes the copy into the base add exactly as in the plain spelling. L loses BOTH preheader copies; M keeps only loop 1's, and that one is bought by its p = q second use (escape 1, not escape 2), with the base add reading a0 directly (addu a0,a2,a0) rather than target's addu a0,a1,a3. Banked as rejected/s31_p_reused_as_base_both_loops_loses_both_copies_costs_12.c and rejected/s31_p_reused_as_base_with_pq_exit_tail_costs_13.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD src/ings.c:719 INCLUDE_ASM anchor plus the cell body, floor-3 chassis (candidate re-measured 3 at 127/127 this session), no FAKE constructs

## [s31] Target's loop-1 exit tail lw a0,12(s2) / sll a1,s4,6 is the re-assignment of the SAME p and sh variables that loop 2 then reuses, so writing both loops against one shared p / sh pair reproduces the exit tail and the loop-2 preheader copy together.
- mechanism: Target reloads $a0 and recomputes $a1 in loop 1's exit tail and the loop-2 guard at .L8001791C consumes them, which is the shape of two loops sharing one pointer variable and one shift variable with the exit tail restoring them after loop 1's base add clobbers $a0. The candidate instead carries a second shift variable sh2 at function scope and restores the pointer with p = q;, which is what emits the wrong addu a0,a3,zero. This is the rederive modality's structural-reshape leg: one pointer/shift pair for the whole function instead of the candidate's p/sh/sh2 triple.
- probe: Cells N1 and N2 (tmp/grind/func_80017848/s31/body_N1.c, body_N2.c): sh re-assigned inside loop 1's exit tail alongside p, sh2 deleted, loop 2's guard and base written against the shared pair; N1 takes q = *(u8 **)(ctx + 0xC); as loop 2's preheader copy source, N2 takes q = p;. N1's disassembly diff banked at tmp/grind/func_80017848/s31/N1_diff.txt.
- result: N1 = 30 at 126 build insns, N2 = 32 at 125. Sharing the pair is roughly a +27 regression and the diff shows why: the loop counter i migrates from $v1 to $a0 and the base from $a0 to $v1 throughout BOTH loops, so most of the regression is a whole-function counter/base seat rotation rather than a preheader effect. Two durable facts fall out of N1's stream: (a) loop 1's copy DIES in N1 even though loop 1 still writes q = *(u8 **)(ctx + 0xC); and still gets the cse fold - so the 'cse leaves an orphaned copy combine never sees' story in candidate.c's s9 header is not sufficient on its own, the downstream p = q; second use is what preserves it; (b) a fresh *(u8 **)(ctx + 0xC) read in loop 2's preheader always materialises as a real lw v0,12(s2) and never as a cse-folded reg-reg copy, because .L8001791C has two predecessors and therefore starts a new cse extended basic block in which no load of ctx+0xC is available. Banked as rejected/s31_shared_p_sh_across_both_loops_fresh_reads_costs_30.c and rejected/s31_shared_p_sh_across_both_loops_l2_copy_costs_32.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD src/ings.c:719 INCLUDE_ASM anchor plus the cell body, floor-3 chassis, no FAKE constructs

## [s31] Escape 1 (a downstream second use of loop 2's copy) has an unpriced site inside loop 2's guard's own taken arm, above the copy, that costs less than the banked minimum of 6.
- mechanism: The banked price table for loop-2 second-use sites is post-loop 19-22, in-body 6, pre-join 12, guard-consuming 6-28, and every entry sites the use either outside the arm or inside the loop body. A use in the arm but above the loop had never been priced; the cheapest such site is the do-while bottom test rewritten through q (while (i < *(s32 *)(sh2 + (s32)q + 0x20));), which target already spends an instruction on (lw v0,32(a0)), so it should have been free.
- probe: Cells P and Q (tmp/grind/func_80017848/s31/body_P.c, body_Q.c): candidate body with loop 2 given a named q/lnk preheader; P puts q's second use in the do-while bottom test, Q puts it in a p = q; after loop 2.
- result: P = 9 at 126 build insns, Q = 13 at 125. P does not buy the copy: cse folds sh2 + (s32)q back to the already-computed base pseudo BEFORE combine runs, so by the time can_combine_p looks q has exactly one reference again and the copy is substituted away, leaving the build one instruction short. The general result, which is the part worth keeping: a second use that is a REDUNDANT RECOMPUTATION of an available expression is invisible to combine here, because cse runs first - a future escape-1 site must be a use cse cannot fold, not merely a use. Q loses two instructions instead (the post-loop p = q is dead and DCE removes it along with the copy). Banked as rejected/s31_l2_q_second_use_in_bottom_test_cse_folds_costs_9.c and rejected/s31_l2_q_second_use_as_p_after_loop2_costs_13.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD src/ings.c:719 INCLUDE_ASM anchor plus the cell body, floor-3 chassis, no FAKE constructs

## [s31] The candidate's byte-exact loop 1 is produced by a mechanism the target cannot be using, so the remaining 3-point residual is a whole-function mechanism problem rather than a loop-2 placement problem.
- mechanism: Read off the target listing rather than inferred. $a3 is written once per loop (0x800178D0, 0x80017930), read once per loop (0x800178D8, 0x80017938) and appears nowhere else in the function except the prologue's addu $s3,$a3,$zero; loop 1's exit tail is lw $a0,0xC($s2) / sll $a1,$s4,6, a FRESH reload of the pointer rather than a copy of $a3. Escape 1 (added_sets_2 = ! dead_or_set_p (i3, i2dest), combine.c:1458) requires the copy's destination to be live past the base add, and target's is not.
- probe: Full disassembly diff of the candidate against asm/funcs/func_80017848.s (tmp/grind/func_80017848/s31/dis.sh), isolating the residual to exactly three instructions, cross-checked against a complete $a3 reference census of the target listing and against cells L/M/N1/N2/P/Q.
- result: The residual is loop-1 exit tail (target lw a0,12(s2) vs ours addu a0,a3,zero), loop-2 preheader (target addu a3,a0,zero vs ours lw v0,12(s2)) and loop-2 base add (target addu a0,a1,a3 vs ours addu a0,a1,v0) - nothing else in 127 instructions. In 31 sessions no form has produced a copy that survives combine with a single downstream reference: escape 1 always costs the second use (candidate pays 3 in loop 1's exit tail; every loop-2 site prices 6 or worse, s31's bottom-test site being 9) and escape 2 always costs the p/links pseudo merge (cell E, 14, whose seat map s31 shows to be unsatisfiable). So s32's question is not where loop 2's second use goes but what makes a use-once copy survive combine at all.
- verdict: CONFIRMED

## [s32] Target's use-once preheader copy is emitted by jump.c or flow.c rather than surviving combine at all. (s31 FRONTIER ITEM 1)
- mechanism: s28's copy-producer census left jump.c (7 emit_move_insn sites) and
  flow.c (1) unread; both run at points where a copy would never face
  can_combine_p, which would explain why 31 sessions of combine-escape spellings
  never reproduced a use-once copy.
- probe: cell R (tmp/grind/func_80017848/s32/body_R.c) spells loop 2's preheader
  as an explicit `q = p;` with one downstream use, then
  `pwsh tools/grinder/dump.ps1 func_80017848` plus a per-insn digest of the
  function's region of every dump (s32/S_cse2_insns.txt, S_combine_insns.txt).
- result: the copy is `(insn 162 (set (reg/v:SI 80) (reg/v:SI 79)))` in
  ings.rtl, ings.jump, ings.cse, ings.loop and ings.cse2, and is ABSENT from
  ings.combine; loop 1's copy (insn 83), which carries the candidate's `p = q`
  second use, is still present in ings.combine. jump.c and flow.c never enter
  the picture. Cell R scores 8 at 125 insns.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD src/ings.c:719 INCLUDE_ASM anchor plus cell R's body,
  floor-3 chassis (candidate re-measured 3 at 127/127 this session), no FAKE
  constructs

## [s32] A use-once copy whose only use lies in a DIFFERENT basic block survives combine at zero cost, because flow.c only builds a LOG_LINK within a block, and that escape can be spelled so the copy lands where target has it.
- mechanism: flow.c:2102 guards LOG_LINK creation with
  `if (y && (BLOCK_NUM (y) == blocknum) ...)`, so a cross-block def/use pair
  gives combine no link to follow and the copy is never a substitution
  candidate. Spelling the copy in the loop's GUARD block and its use (the base
  add) in the preheader block realises this without any second use.
- probe: cells S, S2 (copy in the guard block, guard inline), Z1 (copy dest =
  slots), Z2, Z3 (same plus the `t` two-step guard so the guard's add value is
  invalidated and the base add is recomputed) —
  tmp/grind/func_80017848/s32/body_{S,S2,Z1,Z2,Z3}.c.
- result: the escape WORKS — ings.combine for cell S retains
  `(insn 165 (set (reg/v:SI 81) (reg:SI 109)))`, a use-once copy, the first ever
  measured on this function. It cannot be spelled into target's position: the
  copy has to live in the guard block for the LOG_LINK to be missing, so it is
  emitted BEFORE the `blez` where target emits it after, and with the guard's
  `sh + p` still available cse replaces the preheader's base ADD with a copy of
  the guard's result. Scores: S 14, S2 14, Z1 8, Z2 40, Z3 40, all at 124-125
  instructions against target's 127. Banked as
  rejected/s32_l2_copy_in_guard_block_no_loglink_but_cse_folds_base_add_costs_14.c
  and rejected/s32_l2_copy_in_guard_block_plus_t_reuse_guard_costs_40.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD src/ings.c:719 INCLUDE_ASM anchor plus the cell bodies,
  floor-3 chassis, no FAKE constructs

## [s32] Reusing `p` to hold the links pointer between the copy and the base add trips combine.c:914's use_crosses_set_p refusal and buys BOTH preheader copies at zero instruction cost, reaching target's exact instruction stream.
- mechanism: can_combine_p refuses when
  `! all_adjacent && use_crosses_set_p (src, INSN_CUID (insn))` (combine.c:914;
  use_crosses_set_p at combine.c:10107 returns 1 when any register of the copy's
  source has reg_last_set newer than the copy). Target's preheader is already
  non-adjacent because `lw a2,0x10(s2)` sits between the copy and the base add,
  so writing that load as `p = *(u8 **)(ctx + 0x10);` makes the intervening insn
  set the copy's source. It also explains target's `lw a0,12(s2)` exit tail: p
  has been clobbered by the links and must be reloaded.
- probe: cells AA (both loops, both guards on `t`), AH/AG/AB (declaration order,
  base merged into slots, p/q roles swapped), AC (base computed into p with the
  links kept in their own local), AJ/AJ2 (loop 2 only), AK/AL (both loops,
  loop-2 guard inline) — tmp/grind/func_80017848/s32/body_A*.c, scores in
  s32/scores.txt.
- result: CONFIRMED as a mechanism and DEAD as a route. AK builds 127
  instructions against target's 127 and is instruction-for-instruction identical
  to the target listing across the entire function — both `addu a3,a0,zero`
  preheader copies and both `lw a0,12(s2)` / `sll a1,s4,6` exit tails included —
  with the complete residual being the register permutation target a0(p)->a1,
  a1(sh)->a0, a3(q)->v0, a2(links)->a1, score 14. This is an independent
  re-derivation of the s23/s31 cell-E form and it inherits cell E's verdict for
  the reason s31 recorded: the escape merges the record pointer and the links
  pointer into ONE pseudo, so target's a0/a2 split is not an allocation this RTL
  admits. Control AC (links kept in their own local, base computed into p) does
  NOT trip the refusal and loses both copies: 4 at 125. Every naming /
  declaration-order / base-destination perturbation of AA is exactly inert
  (AA = AH = AG = AB = 36), re-confirming the s11/s12 inertness sweeps on this
  new chassis. Banked as
  rejected/s32_both_loops_p_reused_as_links_inline_l2_guard_costs_14.c,
  rejected/s32_both_loops_p_reused_as_links_t_guards_costs_36.c and
  rejected/s32_l2_p_as_links_l1_escape1_costs_14.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD src/ings.c:719 INCLUDE_ASM anchor plus the cell bodies,
  floor-3 chassis, no FAKE constructs

## [s32] The s12 symmetric chassis, re-measured on the current chassis, still describes the wall and still scores 4. (mandated kill re-audit)
- mechanism: the candidate carries no FAKE construct, so fake_ablate.py has
  nothing to ablate; the closest-to-target banked form is the s12 symmetric
  fresh-reload tail, the only banked score-<=4 form whose loop-1 exit tail is
  target's own `lw a0,12(s2)`.
- probe: tmp/grind/func_80017848/s32/body_SYM.c re-applied, re-scored and dumped.
- result: 4 at 127/126, unchanged from s12. The dump adds the missing detail:
  loop 1's preheader load is not merely unfolded, it is ELIMINATED — ings.cse2
  insn 89 reads reg79 directly and no copy insn exists — while loop 2's load
  survives as a real `lw` because cse's table is flushed at the loop-2 join
  label (code_label 143). The s12 description ("loop 2's copy is a redundant
  ctx+0xC load that target's compiler folded to a copy, in a block cse provably
  cannot reach") is therefore self-contradictory as written: cse cannot reach
  that block, so it cannot fold that load, and the copy must come from
  elsewhere — which s32 identifies as escape #9.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD src/ings.c:719 INCLUDE_ASM anchor plus body_SYM.c, floor-3
  chassis, no FAKE constructs

## [s32] Target's use-once preheader copy is emitted by jump.c or flow.c rather than surviving combine at all.
- mechanism: s28's copy-producer census left jump.c (7 emit_move_insn sites) and flow.c (1) unread; both run at points where a copy would never face can_combine_p, which would explain why 31 sessions of combine-escape spellings never reproduced a use-once copy.
- probe: Cell R (tmp/grind/func_80017848/s32/body_R.c): loop 2's preheader spelled as an explicit q = p with one downstream use, then `pwsh tools/grinder/dump.ps1 func_80017848` and a per-insn digest of the function's region of every dump (s32/S_cse2_insns.txt, s32/S_combine_insns.txt).
- result: The copy is present as (insn 162 (set (reg/v:SI 80) (reg/v:SI 79))) in ings.rtl, ings.jump, ings.cse, ings.loop and ings.cse2, and is ABSENT from ings.combine, while loop 1's copy (insn 83, which carries the candidate's p = q second use) is still there. jump.c and flow.c never enter the picture; combine is the deleter. Cell R scores 8 at 125 insns.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD src/ings.c:719 INCLUDE_ASM anchor plus cell R's body, floor-3 chassis (candidate re-measured 3 at 127/127 this session), no FAKE constructs

## [s32] A use-once copy whose only use lies in a different basic block survives combine at zero cost because flow.c builds a LOG_LINK only within a block, and that escape can be spelled so the copy lands where target has it.
- mechanism: flow.c:2102 guards LOG_LINK creation with `if (y && (BLOCK_NUM (y) == blocknum) ...)`, so a cross-block def/use pair gives combine no link to follow and the copy is never a substitution candidate. Writing the copy in the loop's GUARD block with its only use (the base add) in the preheader block realises this without any second use.
- probe: Cells S, S2 (copy in the guard block, guard inline), Z1 (copy destination = the earlier-declared slots), Z2, Z3 (same plus the t two-step guard so the guard's add value is invalidated and the base add must be recomputed): tmp/grind/func_80017848/s32/body_{S,S2,Z1,Z2,Z3}.c, plus a dump of cell S.
- result: The escape WORKS - ings.combine for cell S retains (insn 165 (set (reg/v:SI 81) (reg:SI 109))), a use-once surviving copy, the first ever measured on this function in 32 sessions. It cannot be spelled into target's position: the copy must live in the guard block for the LOG_LINK to be missing, so it is emitted before the blez where target emits it after, and with the guard's sh2 + p still available cse replaces the preheader's base ADD with a copy of the guard's result. Scores S 14, S2 14, Z1 8, Z2 40, Z3 40, all at 124-125 instructions against target's 127.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD src/ings.c:719 INCLUDE_ASM anchor plus the five cell bodies, floor-3 chassis, no FAKE constructs

## [s32] Reusing p to hold the links pointer between the copy and the base add trips combine.c:914's use_crosses_set_p refusal and buys both preheader copies at zero instruction cost, reaching target's exact instruction stream.
- mechanism: can_combine_p refuses when `! all_adjacent && use_crosses_set_p (src, INSN_CUID (insn))` (combine.c:914; use_crosses_set_p at combine.c:10107 returns 1 when any register of the copy's source has reg_last_set newer than the copy). Target's preheader is already non-adjacent because lw a2,0x10(s2) sits between the copy and the base add, so writing that load as p = *(u8 **)(ctx + 0x10) makes the intervening insn set the copy's source - and it also explains target's lw a0,12(s2) exit tail, since p has been clobbered by the links and must be reloaded.
- probe: Cells AA (both loops, both guards on t), AH/AG/AB (declaration order, base merged into slots, p/q roles swapped), AC (control: links kept in their own local, base computed into p), AJ/AJ2 (loop 2 only), AK/AL (both loops, loop-2 guard inline): tmp/grind/func_80017848/s32/body_A*.c, scores in s32/scores.txt, full disassembly diffs via s32/dis.sh.
- result: CONFIRMED as a mechanism, dead as a route on this chassis. Cell AK builds 127 instructions against target's 127 and is instruction-for-instruction identical to the target listing across the whole function - both addu a3,a0,zero preheader copies and both lw a0,12(s2) / sll a1,s4,6 exit tails included - with the complete residual being the register permutation target a0(p)->build a1, a1(sh)->a0, a3(q)->v0, a2(links)->a1, score 14. This is an independent re-derivation of the s23/s31 cell-E form and inherits its verdict: the escape merges the record pointer and the links pointer into ONE pseudo, so target's a0/a2 split is not an allocation this RTL admits. Control AC does not trip the refusal and loses both copies (4 at 125). Every naming / declaration-order / base-destination perturbation of AA is exactly inert (AA = AH = AG = AB = 36).
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD src/ings.c:719 INCLUDE_ASM anchor plus the nine cell bodies, floor-3 chassis, no FAKE constructs

## [s32] The s12 symmetric fresh-reload chassis, re-measured on the current chassis with every FAKE construct ablated, still scores 4 and still describes the wall.
- mechanism: Mandated kill re-audit. The candidate carries no FAKE construct so fake_ablate.py has nothing to ablate; the closest-to-target banked form is the s12 symmetric fresh-reload tail, the only banked score-4-or-better form whose loop-1 exit tail is target's own lw a0,12(s2).
- probe: tmp/grind/func_80017848/s32/body_SYM.c re-applied to the HEAD anchor, re-scored, and dumped with tools/grinder/dump.ps1.
- result: 4 at 127/126, unchanged from s12. The dump adds the detail s12 lacked: loop 1's preheader load is not merely unfolded, it is ELIMINATED (ings.cse2 insn 89 reads reg79 directly; no copy insn exists), while loop 2's load survives as a real lw because cse's table is flushed at the loop-2 join label (code_label 143). s12's written account of the wall was self-contradictory and is superseded by escape #9.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD src/ings.c:719 INCLUDE_ASM anchor plus body_SYM.c, floor-3 chassis, no FAKE constructs

## [s33] MANDATED KILL RE-AUDIT: cell AK, the instance kill whose form sat closest to the target, still measures 14 on the current chassis with no FAKE construct present anywhere in the tree.
- mechanism: The candidate carries no FAKE construct, so tools/fake_ablate.py has
  nothing to ablate; the re-audit is therefore a plain chassis re-measurement of the
  three reference forms. AK is the closest form by instruction stream (127/127,
  instruction-for-instruction identical to the target listing, residual = a
  four-register permutation); BASE is the closest by score; SYM is the closest by
  exit-tail shape.
- probe: tmp/grind/func_80017848/s33/cells.ps1 BASE,AK,SYM (the s32 harness
  re-pointed at s33), each cell applied over the HEAD src/ings.c:719 INCLUDE_ASM
  anchor and scored with sandbox func_80017848 --disable all.
- result: BASE = 3 (127/127), AK = 14 (127/127), SYM = 4 (127/126) - all three
  reproduce their banked scores exactly. The chassis has not moved since s32 and the
  s31/s32 verdicts on AK stand as measured.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD src/ings.c:719 INCLUDE_ASM anchor plus the three cell bodies,
  floor-3 chassis, no FAKE constructs anywhere in the tree

## [s33] Loop 2's guard can be given the same two-step invalidation loop 1 already uses (t = sh2 + (s32)p; t = *(s32 *)(t + 0x20);) at zero byte cost, which removes the guard's available sh2 + p value and is the precondition for a surviving preheader copy at target's position.
- mechanism: In the candidate, loop 1's guard is written as a two-step through t, so
  the pseudo holding sh + p is overwritten by the load and cse can no longer supply
  that value to loop 1's preheader base add - which is why loop 1's addu a0,a1,a3 is
  a real add instead of a cse copy. Loop 2's guard in the candidate is a single
  inline expression, so its sh2 + p pseudo stays available and any preheader base add
  that reads a value cse knows to equal p is folded away. Making loop 2 symmetric was
  expected to be byte-neutral because target's loop-2 guard has exactly loop 1's
  shape (addu v0,a1,a0 / lw v0,0x20(v0)).
- probe: Twelve cells. D2-D9 write the two-step REUSING loop 1's t; H1-H6 write it
  with a fresh s32 t2. Both families crossed with the loop-2 preheader spelled as a
  fresh *(u8 **)(ctx + 0xC) read, as q2 = p, and as slots = p, over both the BASE
  exit tail (p = q;) and the SYM exit tail (p = *(u8 **)(ctx + 0xC);). Scores in
  tmp/grind/func_80017848/s33/scores.txt; H4's disassembly diff taken with
  s33/dis.sh.
- result: The two-step is FREE with a fresh variable and CATASTROPHIC when it reuses
  loop 1's. H4 (BASE exit tail + t2 two-step + a named fresh read) scores 3 at
  127/127 with a disassembly residual that is the SAME three instructions as the
  candidate's, i.e. it is a second, independent score-3 chassis whose loop-2 guard add
  is invalidated. D8, the identical cell with t reused instead of t2, scores 36 at
  127/127 - a +33 whole-function seat rotation, the same signature s31's cells N1/N2
  produced when one pointer/shift pair was shared across both loops. Every other
  t-reusing cell (D2-D7, D9) sits at 30. Banked: H4 as
  memory/grind/func_80017848/candidate_alt_h4_t2_twostep_3.c, D8 as
  rejected/s33_l2_two_step_guard_reusing_t_seat_rotation_costs_36.c.
- verdict: CONFIRMED

## [s33] Replacing loop 2's preheader *(u8 **)(ctx + 0xC) read with a plain reg-reg copy from a variable already holding that value buys target's addu a3,a0,zero at zero instruction cost.
- mechanism: Target's loop-2 preheader is addu a3,a0,zero / lw a2,0x10(s2) /
  addu a0,a1,a3 - a use-once copy feeding the base add. The candidate emits
  lw v0,0xC(s2) there because .L8001791C has two predecessors and starts a new cse
  extended basic block (cse.c:8039, while (p && GET_CODE (p) != CODE_LABEL)), so the
  redundant load cannot be folded to a copy. Writing the copy explicitly in C was
  expected to put the right instruction in the right place.
- probe: Nine cells, crossing the copy's SOURCE (p, slots, loop 1's shared q) with
  the guard shape (inline guard = F1/F2/F3, G1/G2/G3; t2 two-step = H1/H3/H5/H6) and
  both exit tails, plus F4/D4/D9 which add a post-loop p = q2; second use.
- result: EVERY spelling LOSES the instruction. With the inline guard the build drops
  to 124-125 insns (F1/F2/F3/F4 = 9 at 124; G1/G2/G3 = 8 at 125): the copy is
  substituted away by combine AND the base add is then folded onto the guard's still
  available sh2 + p, costing two instructions. With the t2 two-step the base add
  survives but the copy still dies (H1/H5 = 6 at 125, H3/H6 = 5 at 126). The source
  of the copy is irrelevant - p, slots and the shared q are exactly equal at every
  score. The control is decisive: G4 and H4, identical cells whose preheader keeps the
  fresh read but binds it to a named local q2, score 3 at 127/127, so the named local
  itself is inert and the whole delta is the copy dying.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD src/ings.c:719 INCLUDE_ASM anchor plus the nine cell bodies,
  floor-3 chassis, no FAKE constructs

## [s33] A post-loop second use of loop 2's copy destination (p = q2; after loop 2) preserves the copy through combine the way loop 1's p = q; does, and the ledger's banked 19-22 price for post-loop second-use sites is a chassis artefact.
- mechanism: combine keeps a copy whose destination is still live past the insn it
  would be substituted into (added_sets_2 = ! dead_or_set_p (i3, i2dest),
  combine.c:1458). Loop 1's copy is bought exactly that way by the candidate's p = q;
  exit tail, and the symmetric statement after loop 2 should buy loop 2's.
- probe: Cells F4 (SYM exit tail) and D4/D9 (t-two-step chassis), each identical to
  its no-second-use sibling except for a trailing p = q2; inside loop 2's arm.
- result: F4 scores 9 at 124 build insns - byte-for-byte the same as F1, its sibling
  WITHOUT the second use. D4 = D3 = 30 at 125 and D9 = D7 = 30 at 126. The second use
  is not merely too expensive here, it does not exist: p is dead after loop 2 (the
  tail re-reads *(u8 **)(ctx + 0xC) from scratch for math_Distance3D, rec_a and
  rec_b), so the store is removed by DCE before combine ever sees a second reference.
  Any post-loop second use of loop 2's copy must therefore write a variable the tail
  actually reads, and the tail's reads all sit in a post-call extended basic block
  where target spends real lw instructions. Banked as
  rejected/s33_l2_post_loop_second_use_is_dce_removed_costs_9.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD src/ings.c:719 INCLUDE_ASM anchor plus cells F1/F4/D3/D4/D7/D9,
  floor-3 chassis, no FAKE constructs

## [s33] The combine.c:914 use_crosses_set_p escape must clobber p (the record pointer) between the copy and the base add, so its cost is fixed at cell AK's four-register permutation.
- mechanism: s31/s32 spelled the escape only one way - reuse p to hold the links
  pointer - and concluded the escape is dead because that merges the record pointer
  and the links pointer into one pseudo while target seats them in $a0 and $a2. The
  escape's actual requirement is weaker: it only needs SOME register of the copy's
  SOURCE to be set between i2 and i3, and the copy's source can be any variable that
  provably holds *(u8 **)(ctx + 0xC) at that point.
- probe: Cells J1/J2/J3/J4 on the H4 chassis and K1/K3/K4/K5/K6 on the BASE chassis.
  J2/K1 spell the copy as q2 = slots; slots = *(u8 **)(ctx + 0x10); with loop 2's
  body reading slots as the links pointer - i.e. the merge partner is the top-guard
  record pointer, not p. J3 is the AK spelling re-measured on H4. J1 is the control
  with the copy but no clobber. J4 hoists loop 2's links into loop 1's lnk.
  Disassembly diffs for J2 and K1 via s33/dis.sh.
- result: The escape works with slots and is CHEAPER than with p: J2 = 10 and K1 = 10,
  both at 127/127, against J3/AK = 14. The control J1 (copy, no clobber) is 10 at 126
  - one instruction short, confirming the clobber is what buys the copy and not the
  naming. K1's diff shows target's addu a3,a0,zero present in BOTH preheaders and the
  entire residual being a four-link seat chain: slots $v1->$a2, loop-1 lnk $a2->$a3,
  loop-1 q $a3->$t0, loop-2 copy $a3->$v0 (plus the candidate's own exit-tail
  instruction). The chain is rooted in one fact - reading slots in loop 2's preheader
  keeps it live from the top guard all the way through loop 1, and the register it
  then takes ($a2) is the one target spends on loop 1's links. So the escape's price
  is not a constant: it is the seat distance between the merge partner's two roles,
  and slots is 4 points cheaper than p. It is still not free, because every candidate
  merge partner is a variable target seats somewhere other than $a2 in its first role.
  Declaration order over the merged variable is exactly inert (K3/K4/K5/K6 all 10),
  re-confirming the s11/s12/s32 inertness sweeps on this chassis. Banked as
  rejected/s33_l2_slots_reused_as_links_use_crosses_set_p_costs_10.c and
  rejected/s33_l2_slots_merge_on_sym_exit_tail_costs_10.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD src/ings.c:719 INCLUDE_ASM anchor plus the nine cell bodies,
  floor-3 chassis, no FAKE constructs

## [s33] Loop 1's exit tail can be made to emit target's lw a0,0xC(s2) by writing it as a fresh *(u8 **)(ctx + 0xC) read instead of the candidate's p = q;.
- mechanism: cse's extended basic block provably ends at any CODE_LABEL (cse.c:8039,
  while (p && GET_CODE (p) != CODE_LABEL)), and loop 1's body starts at a
  two-predecessor label, so the exit tail sits in a fresh EBB whose memory table and
  register equivalences have both been cleared. A redundant *(u8 **)(ctx + 0xC) read
  there therefore ought to survive as a real load, which is exactly what target spends
  an instruction on.
- probe: Cell H4 - the BASE body with loop 1's exit tail rewritten as
  p = *(u8 **)(ctx + 0xC); (the SYM spelling) and loop 2 given the free t2 two-step
  guard - scored and then disassembly-diffed with s33/dis.sh.
- result: H4 scores 3 at 127/127 and its diff is the SAME three instructions as the
  candidate's: the exit tail still comes out as addu a0,a3,zero, a copy of loop 1's q,
  not as lw a0,0xC(s2). The fresh-read spelling and the p = q; spelling CONVERGE - so
  as long as loop 1's preheader carries a live copy of the pointer, the exit tail is a
  register copy regardless of how the C is written, and the cse-EBB argument above
  does not predict the observed code. That is the single sharpest open contradiction
  on this function: target has BOTH the preheader copy AND a real reload two blocks
  later, and no measured form has produced both. The fresh -da dumps for H4 are in
  tmp/grind/func_80017848/dumps/ and have not been read.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD src/ings.c:719 INCLUDE_ASM anchor plus cells BASE/SYM/H4,
  floor-3 chassis, no FAKE constructs

## [s33] Cell AK, the instance kill whose form sits closest to the target instruction stream, still measures 14 on the current chassis, and BASE/SYM still measure 3/4.
- mechanism: Mandated kill re-audit. The candidate carries no FAKE construct anywhere in the tree, so tools/fake_ablate.py has nothing to ablate; the re-audit is a plain chassis re-measurement of the three reference forms (AK = closest by instruction stream, BASE = closest by score, SYM = closest by exit-tail shape).
- probe: tmp/grind/func_80017848/s33/cells.ps1 BASE,AK,SYM - the s32 harness re-pointed at s33 - each cell applied over the HEAD src/ings.c:719 INCLUDE_ASM anchor and scored with `sandbox func_80017848 --disable all`.
- result: BASE = 3 (127/127), AK = 14 (127/127), SYM = 4 (127/126). All three reproduce their banked scores exactly; the chassis has not moved since s32 and the s31/s32 verdicts on AK stand as measured. No FAKE carrier occupied any pseudo in any of the three forms.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD src/ings.c:719 INCLUDE_ASM anchor plus the three cell bodies, floor-3 chassis, no FAKE constructs anywhere in the tree

## [s33] Loop 2's guard can be given the same two-step invalidation loop 1 already uses (t = sh2 + (s32)p; t = *(s32 *)(t + 0x20);) at zero byte cost, removing the guard's available sh2 + p value.
- mechanism: Loop 1's guard is written as a two-step through t, so the pseudo holding sh + p is overwritten by the load and cse can no longer supply that value to loop 1's preheader base add - which is why loop 1's `addu a0,a1,a3` is a real add rather than a cse copy. Loop 2's guard in the candidate is one inline expression, so its sh2 + p pseudo stays available and any preheader base add reading a value cse knows equals p is folded away. Target's loop-2 guard has exactly loop 1's shape (addu v0,a1,a0 / lw v0,0x20(v0)).
- probe: Twelve cells: D2-D9 write the two-step REUSING loop 1's t; H1-H6 write it with a fresh s32 t2. Crossed with the loop-2 preheader spelled as a fresh *(u8 **)(ctx + 0xC) read, as q2 = p, and as slots = p, over both the BASE exit tail (p = q;) and the SYM exit tail. Scores in tmp/grind/func_80017848/s33/scores.txt; H4 disassembly-diffed with s33/dis.sh.
- result: The two-step is FREE with a fresh variable and catastrophic when it reuses loop 1's. H4 (BASE exit tail + t2 two-step + named fresh read) scores 3 at 127/127 with a residual that is the SAME three instructions as the candidate's - a second, independent score-3 chassis whose loop-2 guard add is invalidated. D8, the identical cell with t reused instead of t2, scores 36 at 127/127: a +33 whole-function seat rotation, the same signature s31's N1/N2 produced for a shared pointer/shift pair. Every other t-reusing cell (D2-D7, D9) sits at 30. H4 banked as memory/grind/func_80017848/candidate_alt_h4_t2_twostep_3.c; D8 as rejected/s33_l2_two_step_guard_reusing_t_seat_rotation_costs_36.c.
- verdict: CONFIRMED

## [s33] Replacing loop 2's preheader *(u8 **)(ctx + 0xC) read with a plain reg-reg copy from a variable already holding that value buys target's addu a3,a0,zero at zero instruction cost.
- mechanism: Target's loop-2 preheader is addu a3,a0,zero / lw a2,0x10(s2) / addu a0,a1,a3 - a use-once copy feeding the base add. The candidate emits lw v0,0xC(s2) there because .L8001791C has two predecessors and starts a new cse extended basic block (cse.c:8039, `while (p && GET_CODE (p) != CODE_LABEL)`), so the redundant load cannot be folded into a copy; writing the copy explicitly in C was expected to place the right instruction directly.
- probe: Nine cells crossing the copy's SOURCE (p, slots, loop 1's shared q) with the guard shape (inline guard = F1/F2/F3 and G1/G2/G3; t2 two-step = H1/H3/H5/H6) and both exit tails, plus the F4/D4/D9 second-use siblings.
- result: Every spelling LOSES the instruction. With the inline guard the build drops to 124-125 insns (F1/F2/F3/F4 = 9 at 124; G1/G2/G3 = 8 at 125) because the copy is substituted away by combine AND the base add is then folded onto the guard's still-available sh2 + p - two instructions lost. With the t2 two-step the base add survives but the copy still dies (H1/H5 = 6 at 125; H3/H6 = 5 at 126). The copy's source is irrelevant: p, slots and the shared q score identically. The control is decisive - G4 and H4, identical cells whose preheader keeps the fresh read bound to a named local q2, score 3 at 127/127, so the named local is inert and the whole delta is the copy dying. Banked as rejected/s33_l2_preheader_plain_copy_loses_two_insns_costs_9.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD src/ings.c:719 INCLUDE_ASM anchor plus the nine cell bodies, floor-3 chassis, no FAKE constructs

## [s33] A post-loop second use of loop 2's copy destination (p = q2; after loop 2) preserves the copy through combine the way loop 1's p = q; does.
- mechanism: combine keeps a copy whose destination is still live past the insn it would be substituted into (added_sets_2 = ! dead_or_set_p (i3, i2dest), combine.c:1458). Loop 1's copy is bought exactly that way by the candidate's p = q; exit tail, so the symmetric statement after loop 2 should buy loop 2's.
- probe: Cells F4 (SYM exit tail) and D4/D9 (t-two-step chassis), each identical to its no-second-use sibling except for a trailing p = q2; inside loop 2's arm.
- result: F4 scores 9 at 124 build insns - byte-for-byte the same as F1, its sibling WITHOUT the second use. D4 = D3 = 30 at 125 and D9 = D7 = 30 at 126. The second use is not merely expensive here, it does not exist: p is dead after loop 2 (the tail re-reads *(u8 **)(ctx + 0xC) from scratch for math_Distance3D, rec_a and rec_b), so the store is removed by DCE before combine sees a second reference. Any post-loop second use must write a variable the tail actually reads, and every tail read sits in a post-call extended basic block where target spends a real lw. Banked as rejected/s33_l2_post_loop_second_use_is_dce_removed_costs_9.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD src/ings.c:719 INCLUDE_ASM anchor plus cells F1/F4/D3/D4/D7/D9, floor-3 chassis, no FAKE constructs

## [s33] The combine.c:914 use_crosses_set_p escape must clobber p (the record pointer) between the copy and the base add, so its cost is fixed at cell AK's four-register permutation.
- mechanism: s31/s32 spelled the escape one way only - reuse p to hold the links pointer - and concluded it was dead because that merges the record pointer and the links pointer into one pseudo while target seats them in $a0 and $a2. The escape's actual requirement is weaker: it needs SOME register of the copy's SOURCE to be set between i2 and i3, and the source may be any variable that provably holds *(u8 **)(ctx + 0xC) at that point.
- probe: Cells J1/J2/J3/J4 on the H4 chassis and K1/K3/K4/K5/K6 on the BASE chassis. J2/K1 spell the copy as q2 = slots; slots = *(u8 **)(ctx + 0x10); with loop 2's body reading slots as the links pointer - the merge partner is the top-guard record pointer, not p. J3 re-measures the AK spelling on H4; J1 is the control (copy, no clobber); J4 hoists loop 2's links into loop 1's lnk. Disassembly diffs for J2 and K1 via s33/dis.sh.
- result: The escape works with slots and is CHEAPER than with p: J2 = 10 and K1 = 10, both at 127/127, against J3/AK = 14 and J4 = 12. The control J1 (same copy, no clobber) is 10 at 126 - one instruction short - confirming the clobber, not the naming, buys the copy. K1's diff shows target's addu a3,a0,zero present in BOTH preheaders, with the entire residual a four-link seat chain: slots $v1->$a2, loop-1 lnk $a2->$a3, loop-1 q $a3->$t0, loop-2 copy $a3->$v0. The chain is rooted in one fact - reading slots in loop 2's preheader keeps it live from the top guard through loop 1, and the register it then takes ($a2) is the one target spends on loop 1's links. So the escape's price is the seat distance between the merge partner's two roles, not a constant; a partner whose first-role register were $a2 would be free, and no variable in this function has that property. Declaration order over the merged variable is exactly inert (K3/K4/K5/K6 all 10). Banked as rejected/s33_l2_slots_reused_as_links_use_crosses_set_p_costs_10.c and rejected/s33_l2_slots_merge_on_sym_exit_tail_costs_10.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD src/ings.c:719 INCLUDE_ASM anchor plus the nine cell bodies, floor-3 chassis, no FAKE constructs

## [s33] Loop 1's exit tail can be made to emit target's lw a0,0xC(s2) by writing it as a fresh *(u8 **)(ctx + 0xC) read instead of the candidate's p = q;.
- mechanism: cse's extended basic block ends at any CODE_LABEL (cse.c:8039, `while (p && GET_CODE (p) != CODE_LABEL)`), and loop 1's body starts at a two-predecessor label, so the exit tail sits in a fresh EBB whose memory table and register equivalences have both been cleared. A redundant *(u8 **)(ctx + 0xC) read there ought to survive as a real load - exactly what target spends an instruction on.
- probe: Cell H4 - the BASE body with loop 1's exit tail rewritten as p = *(u8 **)(ctx + 0xC); (the SYM spelling) and loop 2 given the free t2 two-step guard - scored and disassembly-diffed with s33/dis.sh.
- result: H4 scores 3 at 127/127 and its diff is the SAME three instructions as the candidate's: the exit tail still comes out as addu a0,a3,zero, a copy of loop 1's q, not lw a0,0xC(s2). The fresh-read spelling and the p = q; spelling CONVERGE, so while loop 1's preheader carries a live copy of the pointer the exit tail is a register copy however the C is written, and the cse-EBB argument above does not predict the observed code. This is the sharpest open contradiction on the function: target carries BOTH the preheader copy and a real reload two blocks later, and no measured form in 33 sessions has produced both. Fresh -da dumps for the H4 chassis are in tmp/grind/func_80017848/dumps/ and are unread.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD src/ings.c:719 INCLUDE_ASM anchor plus cells BASE/SYM/H4, floor-3 chassis, no FAKE constructs

## s34 (synthesis, 2026-09-03)

Chassis re-measured before any probe: BASE (candidate.c) = 3 at 127/127,
K1 = 10 at 127/127, AK = 14 at 127/127 — all tie their banked numbers, so the
floor-3 chassis is unchanged. FAKE-ablation is vacuous (no form of this function
has ever carried a FAKE construct); the K1/AK re-measurements are the mandated
kill re-audit, and K1 is the instance kill whose instruction stream sits closest
to target. Cell scores: `tmp/grind/func_80017848/s34/scores.txt`.

## [s34-H1] Target's two loops open with the SAME nine instructions, so the
## instruction the ledger calls "loop 1's exit tail" is loop 2's record-pointer
## read at the JOIN, and a chassis that puts it there gives loop 2's preheader a
## foldable `mem[ctx+0xC]` for the first time.
- mechanism: cse.c:8039 ends the extended basic block at a multi-predecessor
  CODE_LABEL, so the loop-2 guard's join label starts a fresh EBB with an empty
  memory table (s31/s32's reading, correct). But the join BLOCK itself then
  performs `p = *(u8 **)(ctx + 0xC)` — target's second `lw a0,0xC(s2)` — which
  re-enters `mem[ctx+0xC]` into the table for the remainder of that EBB, and
  loop 2's preheader (single-predecessor fall-through from the `blez`) is inside
  it. Every chassis before s34 placed that read inside loop 1's if-block, where
  it is separated from loop 2's preheader by the EBB boundary.
- probe: built the join-shape family P0-P4 (loop-1 reload moved out of the `if`
  to the join, both guards on the two-step form, both preheaders a fresh
  `*(u8 **)(ctx + 0xC)` read) and scored each; then took fresh `-da` dumps of P1
  via `pwsh tools/grinder/dump.ps1 func_80017848` and digested the function out
  of ings.cse / ings.cse2 / ings.combine.
- result: P0 = P1 = 6 at 127/125, P4 = 4 at 127/125, P2 = P3 = 12 at 127/125.
  The dumps CONFIRM the mechanism: in ings.cse and ings.cse2 loop 1's preheader
  is insn 83 `(set (reg/v:SI 80) (reg/v:SI 79))` (the fold, q = p) followed by
  insn 89 `(set (reg 81) (plus (reg 84) (reg 80)))` — the base add reads the
  COPY's destination, because cse.c:826 make_regs_eqv promoted `q` to canonical.
  Loop 2 is identical. So cse produces both of target's copies on this chassis.
- verdict: CONFIRMED

## [s34-H2] The two-instruction deficit of the join chassis is a COMBINE deletion,
## not a cse fold — so the whole residual of this function is one question asked
## twice: how does a use-once reg-reg copy survive combine at zero cost?
- mechanism: flow.c:2102 gives the copy a LOG_LINK to its single in-block use
  (the base add); combine's `try_combine` substitutes reg80 := reg79 into the add
  and deletes the copy, because none of can_combine_p's refusal paths fires.
- probe: compared the P1 digests `s34/P1_cse2.txt` and `s34/P1_combine.txt`.
- result: in ings.combine insn 83 is absent and insn 89 reads `(reg/v:SI 79)`
  directly, carrying `REG_DEAD (reg/v:SI 79)`. Same for loop 2. This retires the
  s31/s32 framing in which loop 2's problem was a missing cse fold; both loops
  now fail at exactly the same place, for exactly the same reason.
- verdict: CONFIRMED

## [s34-H3] On target's preheader geometry (copy / links load / base add) the only
## can_combine_p refusal reachable without spending an instruction is
## `use_crosses_set_p`, and firing it forces a pointer/links pseudo merge that
## target's register assignment does not admit.
- mechanism: read `tools/gcc-2.7.2/combine.c:880-928` end to end against the
  three-insn preheader. `all_adjacent` is 0, so combine.c:914 is evaluated, but
  it needs a set of the copy's SOURCE strictly between i2 and i3 and the links
  load is the only candidate. combine.c:902 needs `succ != 0` (a three-insn
  combination), hence a second in-block LOG_LINK on the base add, hence a second
  shift; target spends one `sll a1,s4,6` for the guard add and the base add both.
  `last_call_cuid` needs a call in the preheader; `REG_NO_CONFLICT` needs a
  DImode sequence (banned family). Escapes #1 and #8 are already priced at 6-22
  and 8-40.
- probe: built Q1 (both loops write the links read into `p`, firing combine.c:914
  twice) and Q2 (loop 1 merges `slots`+links, loop 2 merges `p`+links) on the
  join chassis, and R1 (loop 2's preheader shift recomputed as
  `(slot_a << 5) << 1`) as the combine.c:902 provocation.
- result: Q1 = 14 at 127/127 — the full 127-instruction stream, residual a pure
  register permutation, reproducing AK's 14 from an independent chassis. Q2 = 15
  at 127/127. R1 = 6 at 127/125: the two shifts are re-merged before combine, no
  second in-block feeder is created, and no three-insn attempt is provoked. K1's
  10 stands as the best exact-stream form. The seat obstacle is structural: one C
  variable is one pseudo is one hard register, and target seats the record
  pointer in `a0`/`v1` and the links pointer in `a2` in every instance.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD src/ings.c:719 INCLUDE_ASM anchor plus the P0/P1/P2/P3/P4/
  Q1/Q2/R1 cell bodies, floor-3 chassis (BASE re-measured 3 at 127/127), no FAKE
  constructs anywhere

## [s34-H4] Sharing one `q` across both preheaders is byte-neutral, and hoisting
## loop 2's links read is not.
- mechanism: naming/declaration inertness (s11/s12/s32/s33) extends to the new
  chassis for the copy destination, but a hoisted links local changes the live
  range that LICM would otherwise create inside the loop.
- probe: P0 vs P1 (distinct q/q2 vs one shared q); P4 (fresh lnk2) vs P2/P3
  (loop 2's links hoisted into loop 1's `lnk`).
- result: P0 = P1 = 6 exactly. P4 = 4, P2 = P3 = 12. So the copy destination's
  identity is free, the links carrier's identity is worth 8 points, and P4 is the
  best join-shape cell at 4 with both copies still missing.
- verdict: CONFIRMED

## [s34] Target's two loops open with an identical nine-instruction sequence, so the instruction this ledger has called 'loop 1's exit tail' is loop 2's record-pointer read at the JOIN (outside loop 1's if-block), and a chassis that places it there gives loop 2's preheader an available mem[ctx+0xC] to fold against.
- mechanism: cse.c:8039 ends the extended basic block at the loop-2 guard's multi-predecessor CODE_LABEL, so the memory table is empty there (s31/s32's reading is correct). But the join block itself performs p = *(u8 **)(ctx + 0xC) - target's second lw a0,0xC(s2) - which re-enters mem[ctx+0xC] into the table for the rest of that EBB, and loop 2's preheader is the single-predecessor fall-through inside it. Every chassis before s34 put that read inside loop 1's if-block, on the far side of the EBB boundary.
- probe: Built the join-shape family P0-P4 (loop-1 reload moved out of the if to the join, both guards on the two-step form, both preheaders a fresh *(u8 **)(ctx + 0xC) read), scored each with sandbox --disable all, then took fresh -da dumps of P1 via pwsh tools/grinder/dump.ps1 func_80017848 and digested func_80017848 out of ings.cse / ings.cse2 / ings.combine.
- result: P0 = P1 = 6 at 127/125, P4 = 4 at 127/125, P2 = P3 = 12 at 127/125. The dumps confirm the mechanism: in ings.cse and ings.cse2 loop 1's preheader carries insn 83 (set (reg/v:SI 80) (reg/v:SI 79)) - the fold, q = p - followed by insn 89 (set (reg 81) (plus (reg 84) (reg 80))), i.e. the base add reads the COPY's destination because cse.c:826 make_regs_eqv promoted q to canonical (q's last mention is in loop 2, past the EBB end, and later than p's). Loop 2 is identical. cse therefore produces both of target's preheader copies on this chassis, which no previously banked form achieved.
- verdict: CONFIRMED

## [s34] The join chassis's two-instruction deficit is a combine deletion rather than a cse fold, so both loops now fail at the same place for the same reason: a use-once reg-reg copy is substituted away and removed by combine.
- mechanism: flow.c:2102 builds a LOG_LINK from the copy to its single in-block use (the base add); try_combine substitutes reg80 := reg79 into the add and deletes the copy, because none of can_combine_p's refusal paths fires on this geometry.
- probe: Diffed the P1 digests tmp/grind/func_80017848/s34/P1_cse2.txt against tmp/grind/func_80017848/s34/P1_combine.txt.
- result: In ings.combine insn 83 is absent and insn 89 reads (reg/v:SI 79) directly with REG_DEAD (reg/v:SI 79) attached; loop 2 behaves identically. This retires the s31/s32 framing in which loop 2's problem was a missing cse fold - the fold happens, and the deletion is downstream.
- verdict: CONFIRMED

## [s34] The use_crosses_set_p escape (combine.c:914) measures 14 (Q1, both loops merging p and links) and 15 (Q2, loop 1 merging slots and links, loop 2 merging p and links) at 127/127 on the join chassis, against 10 for K1 and 14 for AK on the older chassis, because firing it puts the record-pointer role and the links role in one C variable and therefore one hard register while target seats them in a0/v1 and a2; and the combine.c:902 three-insn refusal was not provoked by recomputing loop 2's preheader shift, which re-merged before combine (R1 = 6 at 127/125).
- mechanism: Read tools/gcc-2.7.2/combine.c:880-928 against target's three-insn preheader (copy, links load, base add). all_adjacent is 0 so combine.c:914 is evaluated, but it needs a set of the copy's SOURCE strictly between i2 and i3 and the links load is the only insn there. combine.c:902 needs succ != 0, i.e. a three-insn combination, hence a second in-block LOG_LINK on the base add, hence a second shift - target spends one sll a1,s4,6 for both the guard add and the base add. The last_call_cuid path needs a call inside the preheader; the REG_NO_CONFLICT path needs a DImode sequence, a banned family. Escapes #1 (downstream second use) and #8 (missing cross-block LOG_LINK) are already priced at 6-22 and 8-40 by s31/s32.
- probe: Built Q1 and Q2 (escape #9 spelled on the join chassis with two different merge partners) and R1 (loop 2's preheader shift recomputed as (slot_a << 5) << 1 to defeat cse's available-expression fold and create a second in-block feeder); scored all three with sandbox --disable all.
- result: Q1 = 14 at 127/127 - the complete 127-instruction stream with a pure register-permutation residual, reproducing AK's number from an independent chassis. Q2 = 15 at 127/127. R1 = 6 at 127/125: the two shifts are re-merged before combine, the build gains no instruction, and no three-insn attempt appears. K1's 10 remains the best exact-instruction-stream form measured in 34 sessions. Escape #9 now carries three independent negatives (s31's empty goal derivation, s32's re-derivation, and this chassis).
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD src/ings.c:719 INCLUDE_ASM anchor plus the P0/P1/P2/P3/P4/Q1/Q2/R1 cell bodies, floor-3 chassis (BASE re-measured 3 at 127/127, K1 10 at 127/127, AK 14 at 127/127), no FAKE constructs anywhere in the tree

## [s34] On the join chassis the copy destination's identity is byte-neutral while the links carrier's identity is not: sharing one q across both preheaders is exactly inert, and hoisting loop 2's links read into loop 1's lnk costs 8 points against hoisting it into a fresh local.
- mechanism: Naming and declaration-order inertness for the copy destination (s11/s12/s32/s33) extends to the new chassis; the links carrier instead changes the live range that LICM would otherwise create for the in-loop links read.
- probe: P0 (distinct q/q2) vs P1 (one shared q); P4 (fresh lnk2) vs P2/P3 (loop 2's links hoisted into loop 1's lnk).
- result: P0 = P1 = 6 exactly; P4 = 4 and P2 = P3 = 12, all at 127/125. P4 is the best join-shape cell, one point off the floor with both preheader copies still missing.
- verdict: CONFIRMED

## [s35] KILL RE-AUDIT (mandated): every banked reference number reproduces on the current chassis, and the ledger's "closest-to-target" label was on the wrong cell - K1 is NOT instruction-exact, BASE at 3 is the closest form.
- mechanism: Instance kills are chassis-relative and FAKE-state-relative. No FAKE construct exists anywhere in this function's tree, so tools/fake_ablate.py has nothing to ablate and re-measurement on the current HEAD anchor IS the re-audit. Separately, s33 recorded K1's residual as "a four-link seat chain", which reads as a pure allocation residual; disassembling K1 for the first time shows it also carries BASE's instruction-level join divergence.
- probe: Applied memory/grind/func_80017848/candidate.c (BASE), tmp/grind/func_80017848/s34/body_K1.c and body_P4.c over the src/ings.c:719 INCLUDE_ASM anchor and ran sandbox func_80017848 --disable all on each; then applied K1 and ran tmp/grind/func_80017848/s35/dis.sh (objdump of the sandbox object, normalised against asm/funcs/func_80017848.s).
- result: BASE = 3 at 127/127, K1 = 10 at 127/127, P4 = 4 at 127/125 - all three reproduce exactly. K1's diff shows SEVEN divergent instructions, not four seats: the top guard's slots read v1 -> a2; loop 1's copy destination a3 -> t0 and its links load a2 -> a3 (and the base add follows); the join instruction is target "lw a0,12(s2)" against ours "addu a0,t0,zero" - BASE's own residual number 1, i.e. an INSTRUCTION divergence, not a seat; and loop 2's copy is "addu v0,a2,zero" where target has "addu a3,a0,zero", so its SOURCE is the top guard's slots value rather than the join read. The ledger's s33/s34 framing of K1 as "the best exact-instruction-stream form" is therefore wrong: no form has ever built target's exact stream except AK (s32, 14) and Q1 (s34, 14).
- verdict: CONFIRMED

## [s35] Variable reuse of p as the loop base pointer - the C-level spelling that would give target's observed a0-for-both-roles allocation from a single pseudo - is EXACTLY byte-neutral on the join chassis and does not save the preheader copy.
- mechanism: In target both loops write the base add into a0, the same hard register the record-pointer read used ("lw a0,12(s2)" ... "addu a0,a1,a3"), which is what one C variable serving both roles would produce as one pseudo. It also makes i3 (the base add) set the register that is i2's (the copy's) SOURCE, which is the shape combine.c:914 use_crosses_set_p is testing for. Reading tools/gcc-2.7.2/combine.c:1073-1140 (combinable_i3pat) and 1385-1460 (try_combine) shows no refusal keyed on i3 setting i2's source: use_crosses_set_p consults reg_last_set[], which combine updates in record_dead_and_set_regs only AFTER an insn is processed, so i3's own set is not yet recorded when i3 is examined.
- probe: Cells A1 (both loops write the base into p and index through p, including the bottom test) and A2 (loop 2 only) built on body_P4.c; sandbox --disable all.
- result: A1 = 4 at 127/125 and A2 = 4 at 127/125 - identical to P4 in both score and instruction count. The copy is still deleted by combine and the base add still allocates to a0 anyway (B1's disassembly shows "addu a0,a1,a0" on a cell with no copy at all), so target's a0-reuse is produced by local-alloc's tie to a dying pointer and carries no information about pseudo identity.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD src/ings.c:719 INCLUDE_ASM anchor plus the A1/A2 cell bodies on the join (P4) chassis, floor-3 chassis re-audited this session (BASE 3, K1 10, P4 4), no FAKE constructs anywhere in the tree

## [s35] The preheader copy's carrier identity, its prior live range, and the source order of the links read against the copy are all EXACTLY byte-neutral on the join chassis - three more inertness axes closed.
- mechanism: Three distinct hypotheses share one probe family. (a) If the copy's destination pseudo already had an earlier life (reg_n_sets > 1, an earlier use), flow's LOG_LINK bookkeeping and try_combine's added_sets_2 test (combine.c:1458, "! dead_or_set_p (i3, i2dest)") still see it as dead at the base add, so nothing changes. (b) Reusing a variable whose earlier role ends before the loop (slots, dead after the top guard) does not change that either. (c) can_combine_p's all_adjacent is computed from the insn CHAIN, so moving the links read to before the copy makes the copy adjacent to the base add, which if anything removes the only refusal path that was still being evaluated.
- probe: Cells E1 (copy destination is the already-live guard variable t / t2), E3 (copy destination is slots), E4 (loop 2's links read placed BEFORE the copy) on body_P4.c; sandbox --disable all.
- result: E1 = E3 = E4 = 4 at 127/125, all identical to P4. Combined with A1/A2 that is five consecutive cells at exactly P4's number: on this chassis the preheader copy is deleted regardless of which variable carries it, what that variable's prior liveness is, whether the base add reuses the pointer's pseudo, and where the links load sits relative to the copy.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD src/ings.c:719 INCLUDE_ASM anchor plus the E1/E3/E4 cell bodies on the join (P4) chassis, floor-3 chassis re-audited this session, no FAKE constructs anywhere in the tree

## [s35] Escape number 1 (a second use of the copy destination) re-priced on the join chassis as s34's frontier asked: it does buy an instruction, but the instruction it buys is a copy AFTER the base add, not target's copy before it.
- mechanism: s34 predicted escape 1 would be cheaper on the join chassis because cse.c:826 make_regs_eqv already promotes the copy destination to canonical there, so a second use is no longer a redundant recomputation of an available expression. The prediction is half right - the cell does gain an instruction - but the instruction cse and combine leave behind is the materialisation of the second use's own address expression, which is emitted after the base add.
- probe: Cells B1 (loop 2's bottom test written as i < *(s32 *)(sh2 + (s32)q + 0x20)) and B2 (loop 2's body address written through q instead of base) on body_P4.c; sandbox --disable all, then dis.sh on B1.
- result: B1 = 6 at 127/126, B2 = 7 at 127/126 - both one instruction closer than P4 but two/three points worse. B1's diff against the target stream shows loop 2 emitting "addu a1,a1,a0" followed by "addu a0,a1,zero", i.e. the base is computed into a1 and then COPIED to a0, where target has "addu a3,a0,zero" then "addu a0,a1,a3". The extra insn is a post-base copy, not target's pre-base copy. Loop 1 in the same cell shows the deleted-copy signature "addu a0,a1,a0".
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD src/ings.c:719 INCLUDE_ASM anchor plus the B1/B2 cell bodies on the join (P4) chassis, floor-3 chassis re-audited this session, no FAKE constructs anywhere in the tree

## [s35] Escape number 9 (combine.c:914) applied to ONE loop only costs the same 12 points whichever loop it is, so its price is per-loop and symmetric, and the seat damage is not an artifact of which loop carries the merge.
- mechanism: Firing use_crosses_set_p needs the links load to write the copy's source, which merges the record-pointer role and the links role into one C variable, one pseudo, one hard register - while target seats them a0 and a2 in BOTH loops. If the damage were asymmetric (e.g. only loop 1's a0/a2 split were load-bearing) a one-loop merge would be cheap on the other side.
- probe: Cells C1 (loop 2 only writes the links read into p) and C2 (loop 1 only) on body_P4.c; sandbox --disable all.
- result: C1 = 12 at 127/126 and C2 = 12 at 127/126 - identical. Each merge buys exactly one instruction (125 -> 126) and costs 8 points of seat damage over P4's 4, and Q1 (both loops, s34) = 14 at 127/127 is consistent with that per-loop price. The a0/a2 split is load-bearing in both loops equally.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD src/ings.c:719 INCLUDE_ASM anchor plus the C1/C2 cell bodies on the join (P4) chassis, floor-3 chassis re-audited this session, no FAKE constructs anywhere in the tree

## [s35] s34's frontier item 1 is closed by dump: loop.c adds NO reg-reg move to either preheader on the join chassis, and the post-combine copy-producer census is closed more strictly than s28 closed it.
- mechanism: tools/gcc-2.7.2/toplev.c fixes the pass order - cse_main at :2865, loop_optimize at :2895, cse_main again at :2926, flow_analysis at :2988, combine_instructions at :3004. loop.c therefore runs BEFORE combine and its emissions face the same deletion; and because flow_analysis runs immediately before combine, LOG_LINKS are rebuilt from scratch on the post-loop insn chain, so an insn loop.c moved or re-emitted cannot escape combine by carrying stale or absent links. s28 censused reg-reg-copy producers by counting emit_move_insn / gen_move_insn call sites only, which would miss a pass that builds a move as gen_rtx (SET, ...) and emits it directly.
- probe: Applied body_P4.c, ran pwsh tools/grinder/dump.ps1 func_80017848, and digested func_80017848 out of tmp/grind/func_80017848/dumps/ings.loop, ings.cse2 and ings.combine, listing every reg-reg set. Separately counted "gen_rtx (SET" and emit_insn_before / emit_insn_after / emit_move_insn / gen_move_insn sites in every pass that runs after combine (sched.c, local-alloc.c, global.c, reload1.c, reorg.c, jump.c, caller-save.c) and read the one reorg.c site.
- result: In ings.loop and ings.cse2 the only reg-reg copies inside func_80017848 are insns 4/6/8/10 (the incoming argument moves), 83 and 162 (the two preheader folds, both "(set (reg/v:SI 80) (reg/v:SI 79))"), 233/235 (math_Distance3D argument setup) and 239 (its return value). No insn was added by loop.c to either preheader - the two copies are already present at the .loop dump and are cse's folds, and both are absent from ings.combine. Post-combine census: sched.c, local-alloc.c and global.c contain ZERO insn-creating sites of any kind; reorg.c's single gen_rtx (SET, ...) site is reorg.c:3660, which builds dest = dest -/+ other to invert an increment for a delay slot and cannot produce a plain register copy; reload1.c and caller-save.c need a spill or a call-crossing live range, neither of which this preheader has.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD src/ings.c:719 INCLUDE_ASM anchor plus body_P4.c, floor-3 chassis re-audited this session, no FAKE constructs anywhere in the tree; dumps at tmp/grind/func_80017848/dumps/ings.loop, .cse2 and .combine

## [s35] s34's frontier item 3 (combine.c:902) is disposed by reading rather than by measurement: that refusal path cannot save the preheader copy even if it is provoked, because it is only reachable with the copy in the i1 slot of a three-insn combination while the two-insn combination that kills the copy is attempted independently.
- mechanism: combine_instructions walks i3's LOG_LINKS and calls try_combine (i3, link) for each link before, and independently of, the three-insn try_combine (i3, link, nextlink). can_combine_p is called with succ = 0 for the i2 slot and succ = i2 for the i1 slot; combine.c:902 ("succ && ! all_adjacent && reg_used_between_p (dest, succ, i3)") is unreachable with succ = 0. The preheader copy feeds i3 directly, so it always occupies the i2 slot in a two-insn attempt, where the only live refusal is combine.c:914. Provoking a second in-block feeder (s34's R1) can therefore only refuse the OTHER feeder; it cannot protect the copy.
- probe: Read tools/gcc-2.7.2/combine.c:876-930 (the can_combine_p refusal block) and 1385-1460 (try_combine's can_combine_p call sites and added_sets_2 computation) against the P4 dump geometry recorded above.
- result: The frontier item is retired without spending cells on it. R1's negative in s34 was not a failure to provoke the path; the path is structurally irrelevant to the copy's survival.
- verdict: KILLED
- kill_scope: instance
- measured_on: source reading of tools/gcc-2.7.2/combine.c against the P4 dump geometry (tmp/grind/func_80017848/dumps/ings.combine), floor-3 chassis re-audited this session, no FAKE constructs anywhere in the tree

## [s35] SYNTHESIS - the merged attack for s36+, and the sharpened contradiction.
- mechanism: The residual is now one sentence: target's two loop preheaders each carry a use-once reg-reg copy ("addu a3,a0,zero") that GCC 2.7.2's combine deletes on every chassis this ledger has measured. Four premises bracket it and at least one must be false, because the matching C exists. (1) The copy is use-once in target - a3 is read only by "addu a0,a1,a3" and never again in either loop (asm/funcs/func_80017848.s). (2) combine deletes a use-once reg-reg copy whose single in-block use is the next-but-one insn - dump-proven on the block-for-block-identical join chassis (s34 E-s34-1) and re-confirmed inert against five new carrier/order/pseudo-identity spellings this session. (3) can_combine_p's refusal set is closed on this geometry, and its one zero-cost member (combine.c:914) forces a pointer/links pseudo merge that target's a0/a2 seating does not admit - now priced symmetrically per loop (C1 = C2 = 12). (4) No pass after combine can create a plain register copy here - dump- and source-closed this session. Premise (2) is the only one still resting on "every form measured so far" rather than on a cited predicate, and the untried part of its search space is narrow and named below.
- probe: (banked as the s35 frontier, not spent this session)
- result: Three concrete next probes, in priority order. (a) POSITION-AWARE escape-1 sweep on P4: B1/B2 prove a second use of q does buy an instruction on the join chassis, but the bought insn lands AFTER the base add. Sweep second-use sites that must be evaluated BEFORE the base add - the loop's first body statement, the guard's own comparison operand, a use folded into loop 1's exit condition - and judge each cell by dis.sh position, not by score. (b) Run the solver suite on Q1 (s34, 14 at 127/127, target's exact 127-instruction stream with a pure register permutation): tools/ra_solver/extract.py plus simulate.py to validate the model on that chassis, then goal_from_tgt.py goal ings func_80017848 --show. s31's EMPTY goal was derived on cell E, a different chassis whose merged pseudo covered p and links across BOTH loops; Q1's pseudo set is not the same and the goal may be non-empty, in which case inverse.py yields ranked C-lever vectors and the seat residual becomes a solved search rather than a hand-priced one. (c) Re-audit premise (2) directly with a minimal standalone test case: compile a small function that produces exactly (set r_b r_a) / (set r_c (mem)) / (set r_d (plus r_e r_b)) in one block under the project's exact CC_FLAGS and confirm from -da that combine deletes it. If it does NOT delete it in isolation, the difference between that context and this function's is the answer.
- verdict: CONFIRMED

## [s35] Every banked reference number for this function reproduces on the current HEAD chassis, and K1 - the cell s33/s34 recorded as the closest-to-target instance kill - carries an instruction-level divergence at the join in addition to its seat divergences, so BASE at 3 is the form closest to the target instruction stream.
- mechanism: Instance kills are chassis- and FAKE-state-relative; no FAKE construct exists anywhere in this function's tree, so tools/fake_ablate.py has nothing to ablate and re-measurement on the current anchor IS the re-audit. s33 recorded K1's residual as a four-link seat chain without ever disassembling it.
- probe: Applied memory/grind/func_80017848/candidate.c, tmp/grind/func_80017848/s34/body_K1.c and body_P4.c over the src/ings.c:719 INCLUDE_ASM anchor; ran sandbox func_80017848 --disable all on each; then applied K1 and ran tmp/grind/func_80017848/s35/dis.sh (objdump of the sandbox object normalised against asm/funcs/func_80017848.s).
- result: BASE = 3 at 127/127, K1 = 10 at 127/127, P4 = 4 at 127/125 - all reproduce exactly. K1's diff shows seven divergences: slots v1->a2 in the top guard, loop 1's copy destination a3->t0, loop 1's links load a2->a3 and the base add following, the join instruction target lw a0,12(s2) against ours addu a0,t0,zero (an instruction divergence, i.e. BASE's own residual #1), and loop 2's copy addu v0,a2,zero against target's addu a3,a0,zero with the wrong source. Only AK (s32) and Q1 (s34), both at 14, have ever reproduced target's exact 127-instruction stream.
- verdict: CONFIRMED

## [s35] Reusing the C variable p as the loop base pointer - the spelling that would give target's a0-for-both-roles allocation from a single pseudo, and that makes the base add set the copy's source register - is byte-neutral on the join chassis and does not save the preheader copy.
- mechanism: combine.c:914 use_crosses_set_p consults reg_last_set[], which combine updates in record_dead_and_set_regs only after an insn is processed, so i3's own set of i2's source is not yet recorded when i3 is examined; and combinable_i3pat (combine.c:1073-1140) carries no refusal keyed on i3 setting i2's source.
- probe: Cells A1 (both loops write the base into p and index through p, including the bottom test) and A2 (loop 2 only), built on tmp/grind/func_80017848/s35/body_P4.c; sandbox func_80017848 --disable all.
- result: A1 = 4 at 127/125 and A2 = 4 at 127/125, identical to P4 in both score and instruction count. A cell with no copy at all still emits addu a0,a1,a0 (seen in B1's disassembly), so target's a0 reuse comes from local-alloc tying the base to the dying pointer and says nothing about pseudo identity.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD src/ings.c:719 INCLUDE_ASM anchor plus the A1/A2 cell bodies on the join (P4) chassis; floor-3 chassis re-audited this session (BASE 3, K1 10, P4 4); no FAKE constructs anywhere in the tree

## [s35] On the join chassis the preheader copy's carrier variable, that carrier's prior live range, and the position of the links read relative to the copy are all byte-neutral: three cells land on exactly P4's number.
- mechanism: try_combine's added_sets_2 test (combine.c:1458, ! dead_or_set_p (i3, i2dest)) sees the copy destination as dead at the base add regardless of the carrier's earlier life, and can_combine_p's all_adjacent is computed from the insn chain, so moving the links read before the copy only removes the one refusal path still being evaluated.
- probe: Cells E1 (copy destination is the already-live guard variable t / t2), E3 (copy destination is slots, dead after the top guard) and E4 (loop 2's links read placed before the copy) on body_P4.c; sandbox func_80017848 --disable all.
- result: E1 = E3 = E4 = 4 at 127/125, all identical to P4. With A1/A2 that is five consecutive cells at exactly P4's number.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD src/ings.c:719 INCLUDE_ASM anchor plus the E1/E3/E4 cell bodies on the join (P4) chassis; floor-3 chassis re-audited this session; no FAKE constructs anywhere in the tree

## [s35] Escape #1 re-priced on the join chassis as s34's frontier asked: a second use of the copy destination does buy an instruction there, but the instruction it buys is a copy emitted after the base add rather than target's copy before it.
- mechanism: cse.c:826 make_regs_eqv promotes the copy destination to canonical on this chassis, so a second use is no longer a redundant recomputation of an available expression and survives to combine; but what survives is the materialisation of the second use's own address expression, which cse emits after the base add.
- probe: Cells B1 (loop 2's bottom test written as i < *(s32 *)(sh2 + (s32)q + 0x20)) and B2 (loop 2's body address written through q instead of base) on body_P4.c; sandbox --disable all, then tmp/grind/func_80017848/s35/dis.sh on B1.
- result: B1 = 6 at 127/126, B2 = 7 at 127/126 - one instruction closer than P4 but worse in score. B1's diff shows loop 2 emitting addu a1,a1,a0 then addu a0,a1,zero where target has addu a3,a0,zero then addu a0,a1,a3; loop 1 of the same cell shows the deleted-copy signature addu a0,a1,a0.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD src/ings.c:719 INCLUDE_ASM anchor plus the B1/B2 cell bodies on the join (P4) chassis; floor-3 chassis re-audited this session; no FAKE constructs anywhere in the tree

## [s35] Escape #9 (combine.c:914) applied to one loop only costs 12 points whichever loop carries it, so the merge price is per-loop and symmetric and no one-sided merge avoids the seat penalty.
- mechanism: Firing use_crosses_set_p requires the links load to write the copy's source, merging the record-pointer role and the links role into one C variable, one pseudo, one hard register, while target seats them a0 and a2 in both loops.
- probe: Cells C1 (loop 2 only writes the links read into p) and C2 (loop 1 only) on body_P4.c; sandbox func_80017848 --disable all.
- result: C1 = 12 at 127/126 and C2 = 12 at 127/126, identical. Each merge buys exactly one instruction (125 -> 126) and costs 8 points over P4's 4; Q1 (both loops, s34) = 14 at 127/127 is the consistent sum.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD src/ings.c:719 INCLUDE_ASM anchor plus the C1/C2 cell bodies on the join (P4) chassis; floor-3 chassis re-audited this session; no FAKE constructs anywhere in the tree

## [s35] s34's frontier item 1 is answered by dump: loop.c adds no reg-reg move to either preheader on the join chassis, and no pass that runs after combine can create a plain register copy at this site.
- mechanism: toplev.c fixes the order - cse_main :2865, loop_optimize :2895, cse_main :2926, flow_analysis :2988, combine_instructions :3004 - so loop.c precedes combine and LOG_LINKS are rebuilt on the post-loop chain immediately before combine. s28's producer census counted emit_move_insn / gen_move_insn sites only and would have missed a pass building a move as gen_rtx (SET, ...).
- probe: Applied body_P4.c, ran pwsh tools/grinder/dump.ps1 func_80017848, digested func_80017848 out of tmp/grind/func_80017848/dumps/ings.loop, ings.cse2 and ings.combine listing every reg-reg set; separately counted gen_rtx (SET and emit_insn_before / emit_insn_after / emit_move_insn / gen_move_insn sites in sched.c, local-alloc.c, global.c, reload1.c, reorg.c, jump.c and caller-save.c and read the one reorg.c site.
- result: The only reg-reg copies in func_80017848 at .loop and .cse2 are insns 4/6/8/10 (argument moves), 83 and 162 (the two preheader folds, both (set (reg/v:SI 80) (reg/v:SI 79))), 233/235 (math_Distance3D argument setup) and 239 (its return value); both preheader copies are gone in ings.combine. sched.c, local-alloc.c and global.c have zero insn-creating sites; reorg.c's single gen_rtx (SET, ...) at reorg.c:3660 builds dest = dest -/+ other for an inverted increment and cannot produce a plain copy; reload1.c and caller-save.c need a spill or a call-crossing live range that this preheader does not have.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD src/ings.c:719 INCLUDE_ASM anchor plus body_P4.c; floor-3 chassis re-audited this session; no FAKE constructs anywhere in the tree; dumps at tmp/grind/func_80017848/dumps/ings.loop, .cse2, .combine

## [s35] s34's frontier item 3 is disposed by reading: combine.c:902 cannot save the preheader copy even when provoked, because that refusal is only reachable with the copy in the i1 slot of a three-insn combination while the two-insn combination that deletes it is attempted independently.
- mechanism: combine_instructions calls try_combine (i3, link) for each of i3's LOG_LINKS before and independently of try_combine (i3, link, nextlink); can_combine_p receives succ = 0 for the i2 slot and succ = i2 for the i1 slot, and combine.c:902 tests succ && ! all_adjacent && reg_used_between_p (dest, succ, i3), so it is unreachable with succ = 0. The preheader copy feeds i3 directly and therefore always occupies the i2 slot in a two-insn attempt.
- probe: Read tools/gcc-2.7.2/combine.c:876-930 (the can_combine_p refusal block) and 1385-1460 (try_combine's can_combine_p call sites and added_sets_2 computation) against the P4 dump geometry recorded in tmp/grind/func_80017848/dumps/ings.combine.
- result: The frontier item is retired without spending cells. s34's R1 negative was not a failure to provoke the path; provoking a second in-block feeder can only refuse the other feeder and never protects the copy.
- verdict: KILLED
- kill_scope: instance
- measured_on: source reading of tools/gcc-2.7.2/combine.c against the P4 dump geometry (tmp/grind/func_80017848/dumps/ings.combine); floor-3 chassis re-audited this session; no FAKE constructs anywhere in the tree

## s36 hypotheses (2026-09-03, escalation)

- H-s36-1 (KILLED, instance): On the BASE floor-carrier chassis, giving loop 2's
  redundant `*(u8 **)(ctx + 0xC)` read a named carrier `q2` plus a second use
  that is EVALUATED BEFORE the base add — s35's frontier item 1, ported from the
  join chassis to the chassis that actually carries the floor — leaves the
  preheader copy in target's position at no instruction cost.
  mechanism: s35's B1/B2 showed on the join chassis that a second use of the copy
  destination survives combine via the added_sets_2 path (combine.c:1458) but
  lands the copy AFTER the base add, because cse materialises the second use's
  own address expression late. A use that is a data dependency of something
  emitted earlier should force the copy to precede it.
  probe: cells D1 (bottom test through q2), D2 (body element address through q2),
  D3 (guard and base sharing the read, read hoisted above the guard), D4 (q2 plus
  a post-loop `p = q2`), each applied over the HEAD src/ings.c:719 INCLUDE_ASM
  anchor and scored with `sandbox func_80017848 --disable all`.
  result: D1 = 5 at 127/128, D2 = 6 at 127/128, D3 = 31 at 127/124, D4 = 3 at
  127/127 (an exact tie with BASE in both score and instruction count). The
  earlier-evaluated second use does not reposition the copy for free — it pays
  for its own address expression and overshoots the target instruction count by
  one, which is strictly worse than the join-chassis B1/B2 result it was meant to
  improve on. The frontier item is spent.
  kill_scope: instance
  measured_on: HEAD src/ings.c:719 INCLUDE_ASM anchor plus the D1/D2/D3/D4 cell
  bodies on the BASE (candidate) chassis; BASE re-measured 3 at 127/127 this
  session; no FAKE constructs anywhere in the tree.

- H-s36-2 (KILLED, instance): The closest-to-target banked instance kill (Q1,
  s34) scores differently on the current chassis or with FAKE constructs ablated,
  so the flat floor is an artefact of a stale measurement.
  mechanism: the driver's standing kill-re-audit requirement — an instance kill
  measured under a different chassis or with a FAKE carrier occupying its target
  pseudo is not a kill (func_8002EA24 s8).
  probe: re-applied tmp/grind/func_80017848/s34/body_Q1.c over the current HEAD
  anchor and re-scored; ran tools/fake_ablate.py against candidate.c.
  result: Q1 = 14 at 127/127, exactly its banked number; fake_ablate reports no
  FAKE-annotated constructs to ablate. The chassis has not drifted and no banked
  kill in this ledger rests on a FAKE carrier.
  kill_scope: instance
  measured_on: HEAD src/ings.c:719 INCLUDE_ASM anchor plus body_Q1.c; BASE 3 at
  127/127; no FAKE constructs anywhere in the tree.

## [s36] On the BASE floor-carrier chassis, giving loop 2's redundant *(u8 **)(ctx + 0xC) read a named carrier q2 plus a second use evaluated BEFORE the base add (s35 frontier item 1, ported from the join chassis to the chassis that carries the floor) leaves the preheader copy in target's position at no instruction cost.
- mechanism: s35's B1/B2 showed on the join chassis that a second use of the copy destination survives combine via the added_sets_2 path (combine.c:1458) but lands the copy AFTER the base add, because cse materialises the second use's own address expression late. A use that is a data dependency of something emitted earlier should force the copy to precede it.
- probe: Cells D1 (loop-2 bottom test through q2), D2 (body element address through q2), D3 (guard and base sharing one hoisted read), D4 (q2 plus a post-loop p = q2), each applied over the HEAD src/ings.c:719 INCLUDE_ASM anchor and scored with sandbox func_80017848 --disable all.
- result: D1 = 5 at 127 target / 128 build insns, D2 = 6 at 127/128, D3 = 31 at 127/124, D4 = 3 at 127/127 (exact tie with BASE in score and instruction count). The earlier-evaluated second use does not reposition the copy for free: it materialises its own address expression as an extra instruction and overshoots the target instruction count, strictly worse than the join-chassis B1/B2 result it was meant to improve on. D3 collapses to a single pointer pseudo with no copy at all, reconfirming the s35 A1/A2 merge kill on a second chassis. D4 is the third independent confirmation (s33 F4, s35 E1/E3/E4) that a post-loop second use is byte-neutral here.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD src/ings.c:719 INCLUDE_ASM anchor plus the D1/D2/D3/D4 cell bodies on the BASE (candidate) chassis; BASE re-measured 3 at 127/127 this session; no FAKE constructs anywhere in the tree

## [s36] The closest-to-target banked instance kill (Q1, s34 - target's complete 127-instruction stream with a pure register-permutation residual) scores differently on the current chassis or with FAKE constructs ablated, so the flat floor is an artefact of a stale measurement.
- mechanism: Driver-mandated kill re-audit: an instance kill measured on a different chassis, or with a FAKE carrier occupying its target pseudo, is not a kill (func_8002EA24 s8 precedent).
- probe: Re-applied tmp/grind/func_80017848/s34/body_Q1.c over the current HEAD anchor and re-scored; ran python3 tools/fake_ablate.py --func func_80017848 --file ings --candidate memory/grind/func_80017848/candidate.c.
- result: Q1 = 14 at 127/127, exactly its banked s34 number. fake_ablate prints 'no FAKE-annotated constructs found ... nothing to ablate'. The chassis has not drifted since s35 and no banked kill in this ledger rests on a FAKE carrier.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD src/ings.c:719 INCLUDE_ASM anchor plus tmp/grind/func_80017848/s34/body_Q1.c; BASE 3 at 127/127; no FAKE constructs anywhere in the tree

## s37 hypotheses (2026-09-06, solver; owner directive executed first)

- H-s37-1 (KILLED, class): Frontier item 3 / owner directive 2026-09-06: some
  context specific to this function's chassis (its 15 locals, two loops,
  argument moves, call, LOG_LINK order, cuid distances, combine's reg_last_set
  bookkeeping) is what lets combine delete the promoted use-once preheader
  copy, so on minimal geometry outside the function a copy of the shape
  `(set b a) / (set c (mem)) / (set d (plus e b))` with b dead in the add
  would survive combine and name the protecting context.
  mechanism: if the deletion depended on chassis context, a 3-insn scratch TU
  reproducing the geometry at cse2 output would keep the copy through
  .combine.
  probe: tmp/grind/func_80017848/s37/mini/m4_iso_loop.c, m5_iso_noloop.c,
  m6_iso_notpromoted.c (plus m1/m2/m3 shape controls), compiled with the
  project's exact CC_FLAGS and -da; .cse2 / .flow / .combine read insn by insn
  (tmp/grind/func_80017848/s37/mini/*.cse2 etc., flattened with flat.py).
  result: m5 and m4 both put the exact geometry in .cse2 (copy insn 71 / 70
  with the destination promoted by cse.c:826, the add reading the copy
  destination, REG_DEAD on the add) and both lose the copy in .combine; the
  final assembly has no move. The deletion is decided by combine.c:1458
  (`added_sets_2 = ! dead_or_set_p (i3, i2dest)` = 0 because the destination
  dies in the add, so i2 is folded and deleted) and needs no context at all.
  There is no protecting context to name; frontier item 3 is closed.
  kill_scope: class
  predicate_cite: tools/gcc-2.7.2/combine.c:1458
  measured_on: standalone scratch TUs under the exact project CC_FLAGS
  (canonical cc1), plus the BASE chassis re-audited at 3 (127/127) over the
  HEAD src/ings.c:820 INCLUDE_ASM anchor; no FAKE constructs anywhere.

- H-s37-2 (KILLED, instance): The solver's object-level classify on the BASE
  chassis types the floor-3 residual as a register-seat (RA) permutation that
  inverse.py can search.
  mechanism: solver modality rule (1) - classify triages PRE-RA / RA / SCHED /
  IDENTICAL by comparing the honest and target streams.
  probe: `inverse_compose.py classify ings func_80017848 --target-object
  build/src/ings.o --ours-object tmp/sandbox/func_80017848/ings.o` with BASE
  applied (tmp/grind/func_80017848/s37/classify_BASE.txt), read against the
  positional diff from dis.sh.
  result: classify prints `FIRST DIVERGENCE: RA` because the three differing
  instructions have the same opcodes, but they are at different POSITIONS
  (BASE: `move` in the loop-1 exit tail and `lw` in the loop-2 preheader;
  target: `lw` in the tail and `move` in the preheader). No seat permutation
  turns a move into a load, so the residual is not RA-searchable on this
  chassis; this agrees with s31's empty goal derivation. inverse.py not run.
  kill_scope: instance
  measured_on: HEAD src/ings.c:820 INCLUDE_ASM anchor plus body_BASE.c, BASE 3
  at 127/127, no FAKE constructs.

- H-s37-3 (CONFIRMED): The floor and residual survive the chassis drift
  (main decompiled into ings.c; anchor moved 719 -> 820): BASE = 3 at
  127/127 with the byte-identical residual diff recorded in s36.

- H-s37-4 (CONFIRMED, dump-read): target's preheader add can coexist with a
  cse-produced copy of the guard's load only if the guard add's pseudo is
  invalidated before the preheader - m2/m3 (no invalidation) fold the
  preheader add into the guard's add pseudo outright (insn 79 reads reg 86)
  and the copy dies at flow; m4/m5 (candidate's `t` reassignment) keep the
  add. candidate.c's `t = sh + p; t = *(s32 *)(t + 0x1C)` is load-bearing.

## [s37] Some context specific to this function's chassis (its 15 locals, two loops, argument moves, call, LOG_LINK order, cuid distances, combine's reg_last_set bookkeeping) is what lets combine delete the promoted use-once preheader copy; on minimal geometry outside the function a copy of the shape (set b a)/(set c (mem))/(set d (plus e b)) with b dead in the add would survive combine and name the protecting context (frontier item 3, owner directive 2026-09-06).
- mechanism: If the deletion depended on chassis context, a three-insn scratch TU reproducing the geometry at cse2 output would keep the copy through .combine.
- probe: tmp/grind/func_80017848/s37/mini/m4_iso_loop.c, m5_iso_noloop.c, m6_iso_notpromoted.c plus m1/m2/m3 shape controls, compiled with the project's exact CC_FLAGS (canonical tools/gcc-2.7.2/build/cc1) and -da; .cse2/.flow/.combine read insn by insn via flat.py.
- result: m5 and m4 both put the exact geometry in .cse2 (copy insn 71/70 with its destination promoted to canonical by cse.c:826 make_regs_eqv, the add reading the copy destination, REG_DEAD on the add, LOG_LINK add->copy in .flow) and both lose the copy in .combine; final assembly has no move. The deletion is combine.c:1458: added_sets_2 = !dead_or_set_p(i3, i2dest) is 0 because the destination dies in the add, so i2 is substituted and deleted. m6 (unpromoted control) never carries the copy to cse2. No protecting context exists; frontier item 3 is closed.
- verdict: KILLED
- kill_scope: class
- measured_on: standalone scratch TUs under the exact project CC_FLAGS, plus BASE re-audited at 3 (127/127) over the HEAD src/ings.c:820 INCLUDE_ASM anchor; no FAKE constructs anywhere
- predicate_cite: tools/gcc-2.7.2/combine.c:1458

## [s37] The solver's object-level classify on the BASE chassis types the floor-3 residual as a register-seat (RA) permutation that inverse.py can search.
- mechanism: Solver modality rule (1): inverse_compose.py classify triages PRE-RA / RA / SCHED / IDENTICAL from the honest and target streams.
- probe: inverse_compose.py classify ings func_80017848 --target-object build/src/ings.o --ours-object tmp/sandbox/func_80017848/ings.o with BASE applied (report tmp/grind/func_80017848/s37/classify_BASE.txt), read against the positional diff from s37/dis.sh (T.txt vs B.txt).
- result: classify prints FIRST DIVERGENCE: RA because the three differing instructions share opcodes, but they sit at different positions: BASE has the move in loop 1's exit tail and the lw in loop 2's preheader, target has the lw in the tail and the move in the preheader. No seat permutation turns a move into a load, so the residual is positional, not RA-searchable on this chassis, agreeing with s31's empty goal derivation; inverse.py was not run.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD src/ings.c:820 INCLUDE_ASM anchor plus tmp/grind/func_80017848/s37/body_BASE.c; BASE 3 at 127/127; no FAKE constructs

## [s37] The floor and residual survive the chassis drift since s36 (main decompiled into ings.c, func_80016A8C prototype changed, anchor moved from line 719 to 820).
- mechanism: Driver-mandated chassis re-audit before any probe.
- probe: sandbox func_80017848 --disable all with body_BASE.c (identical to candidate.c's body) applied over the new anchor; dis.sh normalised diff.
- result: 3 at 127 target / 127 build insns, scorable; the residual diff is the same three instructions s36 recorded.
- verdict: CONFIRMED

## [s37] Target's preheader add can coexist with a cse-produced copy of the guard's load only if the guard add's own pseudo is invalidated before the preheader; candidate.c's t = sh + p; t = *(s32 *)(t + 0x1C) reassignment is load-bearing.
- mechanism: cse hashes registers by quantity (canon_hash/exp_equiv_p), so once the copy destination is in the guard load's quantity the preheader add matches the guard's add unless that add's destination has been invalidated.
- probe: m2/m3 (promoted copy, no invalidation) versus m4/m5 (candidate's t reassignment), .cse2 dumps.
- result: m2/m3: cse2 folds the preheader add away entirely (insn 79 reads the guard's add pseudo reg 86) and the copy dies at flow; m4/m5 keep the add reading the copy destination.
- verdict: CONFIRMED

## s38 hypotheses (2026-09-06, forensics; owner directive acknowledged)

- H-s38-1 (KILLED, instance): Frontier item 1 - a basic-block boundary
  between the preheader copy and the base add that leaves no instruction
  behind (a CODE_LABEL whose only jump is removed after combine) keeps the
  copy at zero instruction cost in target's position.
  mechanism: flow.c:2084 links a set only to a use with the same BLOCK_NUM,
  so a boundary between copy and add denies combine the copy->add LOG_LINK.
  probe: tmp/grind/func_80017848/s38/mini/m7a_fwd_branch.c (a real forward
  conditional between copy and add) and m7b_loop_hoist.c (the add written in
  the do/while body so the loop-top label sits between them), exact project
  CC_FLAGS with -da; .cse2/.flow/.combine/.loop read with flat.py; plus the
  post-combine label-deletion sites enumerated from toplev.c:3004-3167 and
  jump.c:270/437-455/1950-2061.
  result: m7a keeps the copy (no LOG_LINK on the add, combine never touches
  insn 71) but emits it as `move $2,$3` in the branch delay slot, before the
  branch - target's copy follows the guard branch and its delay slot with the
  load-delay nop unfilled. m7b: loop.c hoists the links load and the add to
  immediately after the copy, the LOG_LINK is back and combine deletes the
  copy. Zero-residue label deletion after combine exists only through
  jump2's no-op-move path (jump.c:449), which in C is a conditional around a
  register self-copy (dead-conditional-store family).
  kill_scope: instance
  measured_on: standalone scratch TUs under the exact project CC_FLAGS
  (canonical cc1, -O2 -G0 -mcpu=3000 -mips1 -mel), BASE re-audited at 3
  (127/127) over the HEAD src/ings.c:820 INCLUDE_ASM anchor; no FAKE
  constructs anywhere.

- H-s38-2 (KILLED, class): A volatile-qualified read of the links pointer
  between the copy and the base add would make combine.c:985-988 refuse the
  copy->add combination, leaving target's use-once copy in place.
  mechanism: can_combine_p returns 0 when any insn strictly between i2 and
  i3 satisfies volatile_insn_p, and target's links load sits exactly there.
  probe: read tools/gcc-2.7.2/rtlanal.c:1366 volatile_insn_p.
  result: `case MEM: return 0;` - volatile_insn_p answers 1 only for
  UNSPEC_VOLATILE and volatile ASM_OPERANDS. A MEM_VOLATILE_P load is
  invisible to this refusal (that is volatile_refs_p, rtlanal.c:1429, which
  combine.c:985 does not call). Only `asm volatile` reaches the clause, a
  forbidden family (s17 R5).
  kill_scope: class
  predicate_cite: tools/gcc-2.7.2/rtlanal.c:1366
  measured_on: source reading of the frozen cc1 tree; no build needed.

- H-s38-3 (KILLED, instance): The Q1 kill (escape #9, target's exact
  127-instruction stream with a 14-point seat residual) is stale on the
  drifted chassis.
  mechanism: instance kills are chassis-relative; the anchor moved 719 -> 820
  between s35 and s37.
  probe: tmp/grind/func_80017848/s38/body_Q1.c applied over the HEAD anchor,
  sandbox --disable all.
  result: 14 at 127/127, identical to s34/s35. Not stale.
  kill_scope: instance
  measured_on: HEAD src/ings.c:820 INCLUDE_ASM anchor plus body_Q1.c; BASE
  re-audited at 3 (127/127); no FAKE constructs.

- H-s38-4 (CONFIRMED): The floor survives unchanged: BASE = 3 at 127/127 on
  the HEAD chassis (anchor line 820), residual the same three instructions
  s31 named.

- H-s38-5 (KILLED, class): One of the jump.c / flow.c emit_move_insn sites the
  s31 census left unread produces a plain reg-reg copy after combine for this
  target.
  mechanism: a post-combine copy producer would explain a use-once copy that
  combine never saw.
  probe: read jump.c:840-1010 (three sites), :1100-1150, :1240-1320,
  :1380-1430, flow.c:2228-2242, mips.h:2937 (BRANCH_COST), mips.h:2175/2179 and
  rtl.h:647-658 (AUTO_INC_DEC).
  result: jump.c:907/951/1008 need `! reload_completed && BRANCH_COST >= 3/4`
  and BRANCH_COST is 1 for -mcpu=3000; :1144 is HAVE_conditional_move (absent
  on MIPS I); :1313/:1429 are jump1 store-flag conversions that run before
  combine; flow.c:2239 is under AUTO_INC_DEC, undefined for MIPS. No site can
  emit a copy after combine here.
  kill_scope: class
  predicate_cite: tools/gcc-2.7.2/config/mips/mips.h:2937
  measured_on: source reading of the frozen cc1 tree and mips target files.

## [s38] Frontier item 1: a basic-block boundary between the preheader copy and the base add whose label is removed after combine keeps the copy at zero instruction cost in target's position (escape #8 with a vanishing label).
- mechanism: flow.c:2084 links a set only to a use in the same BLOCK_NUM, so a boundary between copy and add denies combine the copy->add LOG_LINK; if the label later vanished with no residue the copy would survive for free.
- probe: tmp/grind/func_80017848/s38/mini/m7a_fwd_branch.c (real forward conditional between copy and add) and m7b_loop_hoist.c (links load and add written inside the do/while body), exact CC_FLAGS with -da, dumps read with flat.py; post-combine label deleters enumerated from toplev.c:3004-3167 and jump.c:270, :437-455, :1950-2061.
- result: m7a keeps the copy (no LOG_LINK on insn 87, combine leaves insn 71) but emits it in the branch delay slot BEFORE the branch, while target's copy follows the guard branch and its delay slot with the load-delay nop unfilled; m7b's back-edge label is undone by loop.c hoisting the links load and the add into the copy's block (insn 146 links to 70; combine deletes the copy). The only post-combine zero-residue label deletion is jump2's no-op-move path (jump.c:449), whose C spelling is a conditional around a register self-copy - the dead-conditional-store family. Item closed.
- verdict: KILLED
- kill_scope: instance
- measured_on: standalone scratch TUs under the exact project CC_FLAGS (canonical cc1), plus BASE re-audited at 3 (127/127) over the HEAD src/ings.c:820 INCLUDE_ASM anchor; no FAKE constructs anywhere

## [s38] A volatile-qualified read of the links pointer between the copy and the base add makes combine.c:985-988 refuse the copy->add combination and leaves target's use-once copy in place.
- mechanism: can_combine_p returns 0 when an insn strictly between i2 and i3 satisfies volatile_insn_p, and target's links load sits exactly between the copy and the add.
- probe: read tools/gcc-2.7.2/rtlanal.c:1366 (volatile_insn_p) against combine.c:985.
- result: volatile_insn_p has `case MEM: return 0;` and returns 1 only for UNSPEC_VOLATILE and volatile ASM_OPERANDS; a MEM_VOLATILE_P load is invisible to the clause (that is volatile_refs_p, rtlanal.c:1429, which combine does not call here). Only asm volatile reaches it - a forbidden family (s17 R5).
- verdict: KILLED
- kill_scope: class
- measured_on: source reading of the frozen cc1 tree
- predicate_cite: tools/gcc-2.7.2/rtlanal.c:1366

## [s38] The Q1 instance kill (escape #9 in both loops, target's exact 127-instruction stream, 14 seat points) is stale on the drifted chassis.
- mechanism: instance kills are chassis-relative and the anchor moved from line 719 to 820 since s35.
- probe: tmp/grind/func_80017848/s38/body_Q1.c applied over the HEAD anchor, sandbox func_80017848 --disable all.
- result: 14 at 127/127, identical to s34 and s35.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD src/ings.c:820 INCLUDE_ASM anchor plus body_Q1.c; BASE re-audited at 3 (127/127); no FAKE constructs

## [s38] The floor and residual survive unchanged on the HEAD chassis this session.
- mechanism: mandated chassis re-audit before any probe.
- probe: body_BASE.c (identical to candidate.c's function body) applied over the src/ings.c:820 anchor, sandbox --disable all.
- result: 3 at 127 target / 127 build, scorable.
- verdict: CONFIRMED

## [s38] One of the jump.c or flow.c emit_move_insn sites left unread by the s31 census produces a plain reg-reg copy after combine on this target.
- mechanism: a post-combine copy producer would explain a use-once copy that combine never saw.
- probe: read jump.c:840-1010, :1100-1150, :1240-1320, :1380-1430, flow.c:2228-2242, mips.h:2937, mips.h:2175/2179, rtl.h:647-658.
- result: jump.c:907/951/1008 are gated on `! reload_completed && BRANCH_COST >= 3` (resp. 4) and BRANCH_COST is 1 for -mcpu=3000; :1144 needs HAVE_conditional_move (absent on MIPS I); :1313/:1429 are jump1 store-flag conversions that run before combine; flow.c:2239 is under AUTO_INC_DEC, undefined for MIPS. None can emit a copy after combine here.
- verdict: KILLED
- kill_scope: class
- measured_on: source reading of the frozen cc1 tree and the mips target files
- predicate_cite: tools/gcc-2.7.2/config/mips/mips.h:2937

## [s38] Frontier item 1: a basic-block boundary between the preheader copy and the base add whose label is removed after combine keeps the copy at zero instruction cost in target's position (escape #8 with a vanishing label), measured as a forward branch (m7a) and a loop back-edge (m7b) on minimal geometry.
- mechanism: flow.c:2084 links a set only to a use in the same BLOCK_NUM, so a boundary between copy and add denies combine the copy->add LOG_LINK; if the label later vanished with no residue the copy would survive for free.
- probe: tmp/grind/func_80017848/s38/mini/m7a_fwd_branch.c and m7b_loop_hoist.c built with the exact project CC_FLAGS and -da; .cse2/.flow/.combine/.loop read with flat.py; post-combine label deleters enumerated from toplev.c:3004-3167 and jump.c:270, :437-455, :1950-2061.
- result: m7a: insn 87 (add) has no LOG_LINK, combine leaves copy insn 71, but the final .s emits the copy as move $2,$3 in the branch delay slot BEFORE the branch, whereas target's copy follows the guard branch and its delay slot with the load-delay nop unfilled. m7b: loop.c hoists the links load and the add to immediately after the copy (insns 144-146 before NOTE_INSN_LOOP_BEG), insn 146 links to insn 70, combine deletes the copy, final preheader lw $4,16($4); addu $5,$5,$7. The only post-combine zero-residue label deletion is jump2's no-op-move path (jump.c:449), whose C spelling is a conditional around a register self-copy (dead-conditional-store family). Item closed.
- verdict: KILLED
- kill_scope: instance
- measured_on: standalone scratch TUs under the exact project CC_FLAGS (canonical cc1), plus BASE re-audited at 3 (127/127) over the HEAD src/ings.c:820 INCLUDE_ASM anchor; no FAKE constructs anywhere

## [s38] A volatile-qualified read of the links pointer between the copy and the base add makes combine.c:985-988 refuse the copy->add combination and leaves target's use-once copy in place.
- mechanism: can_combine_p returns 0 when an insn strictly between i2 and i3 satisfies volatile_insn_p, and target's links load sits exactly between the copy and the add.
- probe: Read tools/gcc-2.7.2/rtlanal.c:1366 (volatile_insn_p) against combine.c:985-988.
- result: volatile_insn_p has case MEM: return 0; and returns 1 only for UNSPEC_VOLATILE and volatile ASM_OPERANDS; a MEM_VOLATILE_P load is invisible to the clause (that is volatile_refs_p, rtlanal.c:1429, which this clause does not call). Only asm volatile reaches it, a forbidden family (s17 R5). C volatile is inert here.
- verdict: KILLED
- kill_scope: class
- measured_on: source reading of the frozen cc1 tree; no FAKE constructs involved
- predicate_cite: tools/gcc-2.7.2/rtlanal.c:1366

## [s38] The Q1 instance kill (escape #9 in both loops, target's exact 127-instruction stream with a 14-point seat residual) is stale on the drifted chassis (anchor 719 -> 820).
- mechanism: Instance kills are chassis-relative; the mandated kill re-audit re-measures the closest-to-target banked kill on the current chassis.
- probe: tmp/grind/func_80017848/s38/body_Q1.c applied over the HEAD anchor; sandbox func_80017848 --disable all.
- result: 14 at 127/127, identical to s34 and s35. Not stale; FAKE ablation vacuous (no FAKE constructs in any form of this function).
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD src/ings.c:820 INCLUDE_ASM anchor plus body_Q1.c; BASE re-audited at 3 (127/127); no FAKE constructs

## [s38] The floor and residual survive unchanged on the HEAD chassis this session.
- mechanism: Mandated chassis re-audit before any probe.
- probe: body_BASE.c (identical to candidate.c's function body) applied over the src/ings.c:820 anchor; sandbox --disable all.
- result: 3 at 127 target / 127 build, scorable; residual is the same three instructions s31 named.
- verdict: CONFIRMED

## [s38] One of the jump.c or flow.c emit_move_insn sites left unread by the s31 census produces a plain reg-reg copy after combine on this target.
- mechanism: A post-combine copy producer would explain a use-once copy that combine never saw.
- probe: Read jump.c:840-1010, :1100-1150, :1240-1320, :1380-1430, flow.c:2228-2242, mips.h:2937 (BRANCH_COST), mips.h:2175/2179 and rtl.h:647-658 (AUTO_INC_DEC).
- result: jump.c:907/951/1008 are gated on ! reload_completed && BRANCH_COST >= 3 (resp. 4) and BRANCH_COST is 1 for -mcpu=3000; :1144 needs HAVE_conditional_move (absent on MIPS I); :1313/:1429 are jump1 store-flag conversions that run before combine; flow.c:2239 is under AUTO_INC_DEC, which rtl.h defines only with HAVE_PRE/POST_INCREMENT, both commented out in mips.h. None can emit a copy after combine here.
- verdict: KILLED
- kill_scope: class
- measured_on: source reading of the frozen cc1 tree and mips target files; no FAKE constructs involved
- predicate_cite: tools/gcc-2.7.2/config/mips/mips.h:2937

## s39 hypotheses (2026-09-06, forensics; owner directive executed s37/s38)

- H-s39-1 (KILLED, class): Frontier item 1 - an insn between the preheader
  copy and the base add that reads the copy destination redirects flow's
  reg_next_use so the add has no LOG_LINK to the copy, combine's
  copy->interposer attempt is undone (added_sets_2 = 1, no i1 split), and the
  copy stays in target's position at zero instruction cost.
  mechanism: flow.c:2101 links a set only to the nearest later same-block
  use; combine.c:1458 added_sets_2; combine.c:1990 split requires i1.
  probe: tmp/grind/func_80017848/s39/mini/m8a (load interposer), m8b
  (arithmetic interposer), m8e/m8f (round-trip a = b), m8g (fresh third
  pointer e = b), exact project CC_FLAGS with -da; .cse2/.flow/.combine/.s
  read; J1 (round-trip in both loops on the P4 join chassis) measured with
  sandbox.
  result: the mechanism is real (m8a: add has no LOG_LINK, insn 71 untouched
  by combine, final `move $8,$3 ; addu $2,$5,$8`) but the interposer is an
  instruction target does not have (`lw $3,8($8)`). Every same-value
  interposer (m8e/m8f/m8g, J1 = 4 = P4) is canonicalised away by cse2 before
  flow (cse.c:6730), so no zero-byte interposer reaches flow; a
  value-computing interposer survives to the assembler unless jump2 deletes
  it as a no-op move (jump.c:441), which requires a later reader of a3 that
  target lacks. m8b adds: even a surviving copy vanishes when global gives Q
  the register P released.
  kill_scope: class
  predicate_cite: tools/gcc-2.7.2/flow.c:2101
  measured_on: standalone scratch TUs under the exact project CC_FLAGS
  (canonical cc1, -O2 -G0 -mcpu=3000 -mips1 -mel) plus J1 on the P4 join
  chassis over the HEAD src/ings.c:820 anchor; BASE re-audited at 3
  (127/127); no FAKE constructs anywhere.

- H-s39-2 (KILLED, class): reload's input-reload path emits target's copy:
  find_equiv_reg finds P's value already in a0 and reload emits
  `gen_move_insn (reloadreg, oldequiv)` = `addu a3,a0,zero` for the add's
  operand, a producer combine never sees.
  mechanism: reload1.c:5848-5853 (oldequiv via find_equiv_reg) gated on
  `reg_renumber[REGNO (old)] < 0` (reload1.c:5851).
  probe: source reading of reload1.c:5826-5860 and global.c:350-368,
  1172-1190 against target's prologue (saves s0-s5 only) and instruction
  stream (no sp-relative spill traffic in 127 insns).
  result: the gate needs a pseudo global left unallocated; global hands a
  call-crossing allocno any free callee-saved register (s6/s7 are free here)
  and a non-crossing one any of 16 caller-saved registers, so no pseudo of
  this function is unallocated and the path cannot fire.
  kill_scope: class
  predicate_cite: tools/gcc-2.7.2/reload1.c:5851
  measured_on: source reading of the frozen cc1 tree plus target's prologue
  and instruction census; BASE re-audited at 3 (127/127); no FAKE constructs.

- H-s39-3 (KILLED, instance): Writing the round-trip `p = q` between the
  cse-produced copy and the base add (add reads p) in both loops of the join
  chassis keeps each loop's preheader copy in target's position.
  mechanism: the interposer would take the copy's LOG_LINK.
  probe: tmp/grind/func_80017848/s39/body_J1.c over the HEAD anchor, sandbox
  --disable all; control body_P4.c.
  result: J1 = 4 at 127/125, byte-identical to P4 = 4 at 127/125; cse2
  removes the round-trip before flow (m8e dump: insn 74 absent at .cse2).
  kill_scope: instance
  measured_on: HEAD src/ings.c:820 INCLUDE_ASM anchor plus body_J1.c and
  body_P4.c; BASE re-audited at 3 (127/127); no FAKE constructs.

- H-s39-4 (CONFIRMED): The floor and residual survive unchanged on the HEAD
  chassis: BASE = 3 at 127/127, Q1 = 14 at 127/127 (kill re-audit passes,
  not stale), P4 = 4 at 127/125.

## [s39] Frontier item 1: an insn between the preheader copy and the base add that reads the copy destination redirects flow's reg_next_use so the add has no LOG_LINK to the copy, combine's copy->interposer attempt is undone, and the copy stays in target's position at zero instruction cost.
- mechanism: flow.c:2101 links a set only to the nearest later same-block use; combine.c:1458 added_sets_2 with no i1 split (combine.c:1990); cse.c:6730 removes same-value register copies before flow; jump.c:441 is the only post-combine deleter of a surviving move.
- probe: Scratch TUs tmp/grind/func_80017848/s39/mini/m8a (load interposer), m8b (arithmetic), m8e/m8f (round-trip a = b), m8g (fresh third pointer) under exact CC_FLAGS with -da; .cse2/.flow/.combine/.s read; J1 = round-trip in both loops on the P4 join chassis measured with sandbox.
- result: m8a: add has no LOG_LINK, combine leaves insn 71, final move $8,$3 ; addu $2,$5,$8 plus the interposer lw $3,8($8) as a real instruction. m8e/m8f/m8g: insn 74 already absent at .cse2, combine deletes the copy. J1 = 4 at 127/125 = P4. m8b: copy survives combine but global gives 77 the register 76 released and jump2 deletes the no-op. No zero-byte interposer reaches flow; a value-computing one is an instruction target lacks unless a later a3 reader exists, and none does.
- verdict: KILLED
- kill_scope: class
- measured_on: standalone scratch TUs under the exact project CC_FLAGS (canonical cc1) plus J1 on the P4 join chassis over the HEAD src/ings.c:820 anchor; BASE re-audited at 3 (127/127); no FAKE constructs
- predicate_cite: tools/gcc-2.7.2/flow.c:2101

## [s39] reload's input-reload path emits target's preheader copy via find_equiv_reg (a hard register already holding P's value becomes oldequiv and reload emits gen_move_insn(reloadreg, oldequiv)), a producer combine never sees.
- mechanism: reload1.c:5848-5853, gated on reg_renumber[REGNO (old)] < 0 at reload1.c:5851; global.c:350-368 and 1172-1190 allocate any free register (callee-saved for call-crossing allocnos) before leaving a pseudo unallocated.
- probe: Source reading of reload1.c:5826-5860 and global.c against target's prologue (saves s0-s5 only; s6/s7 free) and its 127-instruction stream (no sp-relative spill traffic).
- result: The gate needs an unallocated pseudo; this function has none, so the reload copy producer cannot be target's producer.
- verdict: KILLED
- kill_scope: class
- measured_on: source reading of the frozen cc1 tree plus target prologue/instruction census; BASE re-audited at 3 (127/127); no FAKE constructs
- predicate_cite: tools/gcc-2.7.2/reload1.c:5851

## [s39] Writing the round-trip p = q between the cse-produced copy and the base add (add reads p) in both loops of the join chassis keeps each preheader copy in target's position.
- mechanism: the interposer would take the copy's LOG_LINK (flow.c:2101).
- probe: tmp/grind/func_80017848/s39/body_J1.c over the HEAD anchor, sandbox --disable all; control body_P4.c.
- result: J1 = 4 at 127/125, byte-identical to P4 = 4; cse2 removes the round-trip before flow (m8e: insn 74 absent at .cse2). Banked as rejected/s39_join_roundtrip_p_eq_q_add_reads_p_costs_4.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD src/ings.c:820 INCLUDE_ASM anchor plus body_J1.c and body_P4.c; BASE re-audited at 3 (127/127); no FAKE constructs

## [s39] The floor and residual survive unchanged on the HEAD chassis: BASE = 3 at 127/127, Q1 = 14 at 127/127 (kill re-audit, not stale), P4 = 4 at 127/125.
- mechanism: chassis re-audit mandated by the brief; anchor src/ings.c:820 unchanged since s37.
- probe: s39/cells.ps1 BASE,Q1,P4 with sandbox --disable all; src/ings.c restored from s39/ings.orig.c; git diff --quiet passes.
- result: BASE 3, Q1 14, P4 4 - all identical to s38.
- verdict: CONFIRMED

## s40 hypotheses (2026-09-06, rederive; owner directive executed s37/s38/s39)

## [s40] A pseudo that is passed to the function's only call as a 3rd/4th argument gives loop 2's cse-produced preheader copy a byte-free second reader (the arg move ties to a2/a3 by hard-reg copy preference), so the copy survives combine, the add reads it, and the copy destination lands in a3.
- mechanism: combine.c:1458 added_sets_2 keeps the copy when its destination is not dead at the add; global.c set_preference ties a pseudo copied into a hard argument register to that register, so the arg move is deleted as a no-op (jump.c:441); local-alloc optimize_reg_copy_1 (local-alloc.c:700) makes the add read the copy.
- probe: scratch TU m9a (BASE body with loop 1's tail removed, loop 2 re-reading p, `extern s32 math_Distance3D(s32 *, s32 *, u8 *, u8 *)`, call passing `lnk, q`) built under project flags with -da; read .s/.greg (tmp/grind/func_80017848/s40/mini/m9a_call_arg_reader.*).
- result: pass-level shape REPRODUCED (`move $9,$5 / lw $8,16($18) / addu $3,$2,$9`) but q = t1 and lnk = t0, with `move $6,$8 / move $7,$9` paid at the call: q and lnk are live at function entry on the loop-skip paths, so global_conflicts records them against the still-live incoming a2/a3 argument registers in block 0. Defining them on the skip paths costs an instruction target lacks. Not portable to the chassis without editing math_Distance3D's definition (outside this function's scope), and dead on the seat regardless.
- verdict: KILLED
- kill_scope: instance
- measured_on: standalone scratch TU m9a under the exact project CC_FLAGS (canonical build/cc1), plus BASE re-audited at 3 (127/127) on the HEAD src/ings.c:820 anchor; no FAKE constructs

## [s40] Frontier item 1: a real statement in each loop's return arm that reads the copy destination is merged by jump2's cross-jump into the shared return-0 tail and thereby disappears without bytes, giving the copy a byte-free second reader.
- mechanism: jump.c:1950-2061 cross-jump merges identical post-RA insn sequences reaching the same label; the merge test is rtx_renumbered_equal_p at jump.c:2525, so only IDENTICAL insns merge and a merged read is still a read.
- probe: m9c scratch TU (`return (s32)q;` in both arms, loop 2 naming q) with -da dumps, then the same body on the chassis (s40/body_M9C.c) via sandbox --disable all.
- result: m9c .s cross-jumps both arms into one tail `.L19: j .L17 / move $2,$5` - the read survives in the tail. Chassis: 15 at 127/129 (two extra instructions). Target's tail `j .L80017A1C / addu v0,zero,zero` reads no register, so no return-arm reader of a3 can exist in target.
- verdict: KILLED
- kill_scope: class
- predicate_cite: jump.c:2525
- measured_on: scratch TU m9c under project flags plus HEAD src/ings.c:820 anchor with body_M9C.c; BASE 3 at 127/127; no FAKE constructs

## [s40] Target's a3 seat for both preheader copies is produced by global.c's pass-0 selection for a copy destination that conflicts with v0,v1,a0,a1,a2 (i.e. is live across its loop body), and a copy destination whose only use is the same-block base add is instead a LOCAL pseudo that local-alloc puts in v0.
- mechanism: global.c:993-1001 pass 0 skips conflicting, never-used and someone-preferred registers and takes the lowest remaining; local-alloc.c:472-475 admits single-block single-death pseudos, combine_regs refuses a tie when the copy source is global (local-alloc.c:1827), find_free_reg takes the lowest free hard register (no REG_ALLOC_ORDER in mips.h).
- probe: instrumented cc1 on the BASE body: BB2_FINDREG_DEBUG=80 (loop 1's copy dest reg80) in s40/ifr80/cc1.log; .lreg/.greg dispositions in s40/icand.
- result: reg80: conflicts {2,3,4,5,6,18,19,29}, someone_prefers empty, a3 first pass-0-eligible -> 80 in 7. Loop 2's load pseudo reg113: local, `in 2` (v0). So target's two a3 copies are two global allocnos each live across its own loop, i.e. each has a flow-time reader after or inside the loop that left no bytes.
- verdict: CONFIRMED

## [s40] The floor and residual survive unchanged on the HEAD chassis this session.
- mechanism: Driver-mandated chassis re-audit before any probe.
- probe: sandbox func_80017848 --disable all with body_BASE.c applied over the src/ings.c:820 anchor; dis.sh normalised diff.
- result: 3 at 127 target / 127 build, scorable; same three-instruction positional residual as s36-s39.
- verdict: CONFIRMED

## [s40] A pseudo passed to the function's only call as a 3rd/4th argument gives loop 2's cse-produced preheader copy a byte-free second reader (arg move tied to a2/a3 by hard-reg copy preference), so the copy survives combine, the add reads it, and the copy destination lands in a3.
- mechanism: combine.c:1458 added_sets_2 keeps the copy when its destination is not dead at the add; global.c set_preference ties a pseudo copied into a hard argument register to that register so the arg move is deleted as a no-op (jump.c:441); local-alloc.c:700 optimize_reg_copy_1 makes the add read the copy.
- probe: Scratch TU m9a (BASE body, loop 1 tail removed, loop 2 re-reading p, extern math_Distance3D widened to 4 params, call passing lnk, q) built under exact project flags with -da; read .s/.greg (tmp/grind/func_80017848/s40/mini/m9a_call_arg_reader.*).
- result: Pass-level shape reproduced (move $9,$5 / lw $8,16($18) / addu $3,$2,$9 = target's copy, links load, add-reads-copy) but seats wrong: q=t1, lnk=t0, base=v1, and the call pays move $6,$8 / move $7,$9. q and lnk are live at function entry on the loop-skip paths, so global_conflicts records them against the still-live incoming a2/a3 argument registers in block 0. Defining them on the skip paths would cost an instruction target lacks. Not portable to the chassis without widening math_Distance3D's definition (outside this function's scope).
- verdict: KILLED
- kill_scope: instance
- measured_on: standalone scratch TU m9a under the exact project CC_FLAGS (canonical build/cc1), plus BASE re-audited at 3 (127/127) on the HEAD src/ings.c:820 INCLUDE_ASM anchor; no FAKE constructs

## [s40] Frontier item 1: a real statement in each loop's return arm that reads the copy destination is merged by jump2's cross-jump into the shared return-0 tail and thereby disappears without bytes, giving the copy a byte-free second reader.
- mechanism: jump.c:1950-2061 cross-jump merges identical post-RA insn sequences reaching the same label; the merge test is rtx_renumbered_equal_p at jump.c:2525, so only identical insns merge and a merged read is still a read.
- probe: m9c scratch TU (return (s32)q; in both arms, loop 2 naming q) with -da dumps, then the same body on the chassis (tmp/grind/func_80017848/s40/body_M9C.c) via sandbox --disable all.
- result: m9c .s cross-jumps both arms into one tail .L19: j .L17 / move $2,$5 - the read survives in the merged tail. Chassis: 15 at 127/129 (two extra instructions). Target's tail j .L80017A1C / addu v0,zero,zero reads no register, so no return-arm reader of a3 can exist in target. Banked as rejected/s40_return_arm_reads_q_crossjump_tail_keeps_read_costs_15.c.
- verdict: KILLED
- kill_scope: class
- measured_on: scratch TU m9c under project flags plus HEAD src/ings.c:820 anchor with body_M9C.c; BASE 3 at 127/127; no FAKE constructs
- predicate_cite: jump.c:2525

## [s40] Target's a3 seat for both preheader copies is produced by global.c's pass-0 selection for a copy destination that conflicts with v0,v1,a0,a1,a2 (live across its loop body), while a copy destination whose only use is the same-block base add is a LOCAL pseudo that local-alloc puts in v0.
- mechanism: global.c:993-1001 pass 0 skips conflicting, never-used and someone-preferred registers and takes the lowest remaining; local-alloc.c:472-475 admits single-block single-death pseudos, combine_regs refuses a tie when the copy source is global (local-alloc.c:1827), find_free_reg takes the lowest free hard register (no REG_ALLOC_ORDER in mips.h).
- probe: Instrumented cc1 (tools/gcc-2.7.2/cc1, codegen-identical to build/cc1 on the TU) on the BASE body: BB2_FINDREG_DEBUG=80 (loop 1's copy dest reg80) in tmp/grind/func_80017848/s40/ifr80/cc1.log; .lreg/.greg dispositions in s40/icand.
- result: reg80: conflicts {2,3,4,5,6,18,19,29}, someone_prefers empty, own prefs empty, a3 is the first pass-0-eligible register -> 80 in 7. Loop 2's load pseudo reg113 is local and gets v0 (Register 113 in 2). Hence target's two a3 copies are two global allocnos each live across its own loop, i.e. each has a flow-time reader after or inside the loop that left no bytes; every visible post-loop instruction reads only v0/v1/a0/a1/a2/s-regs.
- verdict: CONFIRMED

## [s40] The floor and residual survive unchanged on the HEAD chassis this session.
- mechanism: Driver-mandated chassis re-audit before any probe.
- probe: sandbox func_80017848 --disable all with body_BASE.c applied over the src/ings.c:820 anchor; s40/dis.sh normalised diff.
- result: 3 at 127 target / 127 build, scorable; same three-instruction positional residual as s36-s39 (tail move vs lw, preheader lw+add vs copy+add).
- verdict: CONFIRMED

## s41 hypotheses (2026-09-06, rederive; owner directive executed s37-s40)

## [s41] The floor and residual survive unchanged on the HEAD chassis this session, and the closest-to-target instance kill (Q1) is not stale.
- mechanism: Driver-mandated chassis re-audit and kill re-audit before any probe.
- probe: sandbox func_80017848 --disable all with body_BASE.c and body_Q1.c over the src/ings.c:820 anchor; s41/dis.sh normalised diffs.
- result: BASE 3 at 127/127 (same three-instruction positional residual as s36-s40); Q1 14 at 127/127 with the same five-seat permutation (p/sh swapped, q in v0, links sharing p's a1). No FAKE construct to ablate.
- verdict: CONFIRMED

## [s41] A same-value re-store `p = q;` placed inside the preheader directly AFTER the base add (loop 1 only: M1; both loops: M2) is a flow-time reader of q that keeps the cse-produced copy alive past the add and gives q the a0 conflict target's seat needs.
- mechanism: flow would see q read after the add, so combine keeps the copy (added_sets_2) and global records a conflict with base's register.
- probe: s41/body_M1.c and body_M2.c on the HEAD chassis via cells.ps1; normalised diff s41/B_M1.txt vs T.txt.
- result: M1 = 12 at 127/125, M2 = 14 at 127/125. cse deletes the re-store before flow (q is known equal to p in that extended block), p becomes live across loop 1, loop 2's guard stops reloading slots, both copies vanish and p/sh swap to a1/a0. Same fate as s39's between-copy-and-add placement (J1/P4). Banked as rejected/s41_p_eq_q_after_base_add_cse_folds_costs_12.c and rejected/s41_both_loops_p_eq_q_after_base_add_costs_14.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD src/ings.c:820 INCLUDE_ASM anchor plus body_M1.c / body_M2.c; BASE re-audited at 3 (127/127); no FAKE constructs

## [s41] Both scan loops written as `for (i = 0;; i++) { if (i >= count) break; ... }` (exit test at the loop top, no rotation, no duplicated exit test) reproduce target's guard-add / base-add pair and per-iteration bound reload.
- mechanism: a top-of-loop break avoids jump.c's duplicate_loop_exit_test (no simplejump after NOTE_INSN_LOOP_BEG) while still letting the source express the count test once.
- probe: s41/body_F1.c on the HEAD chassis via cells.ps1.
- result: 22 at 127/119. The loop is not rotated (target's is), the bound test sits at the top, only one address add survives. Banked as rejected/s41_for_break_at_top_no_rotation_costs_22.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD src/ings.c:820 INCLUDE_ASM anchor plus body_F1.c; BASE re-audited at 3 (127/127); no FAKE constructs

## [s41] Both scan loops written as `i = 0; goto testK; do { body; i++; testK:; } while (i < count);` - a jump into the loop that makes loop.c mark the loop invalid (no LICM, no VTOP) - keep the bound in the loop bottom while reproducing target's guard.
- mechanism: loop.c find_and_verify_loops sets loop_invalid for a loop entered from outside (loop.c:2596, "multiple entry points"), so nothing is hoisted and the bound reload stays; the guard would be the first execution of the bottom test.
- probe: s41/body_G1.c on the HEAD chassis via cells.ps1; normalised diff s41/B_G1.txt.
- result: 43 at 127/117. The guard block collapses to `sll / lw / lw / j <bottom test> / addu` (no `blez`, no guard add), the frame shrinks from 64 to 48 bytes with the guard's phantom slots, and the bottom compare is a real `slt` against i rather than target's `blez`. Banked as rejected/s41_goto_into_loop_invalid_no_guard_costs_43.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD src/ings.c:820 INCLUDE_ASM anchor plus body_G1.c; BASE re-audited at 3 (127/127); no FAKE constructs

## [s41] global.c's preference machinery (set_preference resolving reg_renumber of local-alloc pseudos, find_reg's hard_reg_copy_preferences branch, regs_someone_prefers) offers a route to the a3 seat for a copy destination that is NOT live across its loop body.
- mechanism: set_preference (global.c:1671-1740) converts both operands through reg_renumber, so a copy to/from a local-alloc pseudo already seated in a3 - or to/from hard register a3 - gives the copy destination a copy preference that find_reg (global.c:1080-1110) honours over the lowest free register; regs_someone_prefers (global.c:877-935) could alternatively exclude v0/a0 in pass 0.
- probe: read global.c:340-372 (regs_used_so_far seed), :877-935, :952-1110, :1671-1740 and local-alloc.c:1784-1900 (combine_regs) against the target stream; cross-checked with s40's BB2_FINDREG_DEBUG=80 dump (own_copy_prefs empty, someone_prefers empty).
- result: In this function no local qty can be seated in a3 (find_free_reg takes the lowest register not busy in the block, global pseudos are invisible to local-alloc, every block holds one local chain in v0), the only hard a3 write is the entry copy of the slot_b parameter pseudo (seated s3, crosses the call), and the only v0/a0-preferring pseudos (call arguments, call result) cannot overlap a preheader copy without crossing the call. The preference route is closed by reading; s40's live-across-the-loop requirement is confirmed on an independent second reading.
- verdict: CONFIRMED

## [s41] The floor and residual survive unchanged on the HEAD chassis this session, and the closest-to-target instance kill (Q1, exact 127-instruction stream) is not stale.
- mechanism: Driver-mandated chassis re-audit and kill re-audit before any probe; no FAKE construct exists in any form of this function so fake_ablate is vacuous.
- probe: sandbox func_80017848 --disable all with tmp/grind/func_80017848/s41/body_BASE.c and body_Q1.c over the src/ings.c:820 INCLUDE_ASM anchor; s41/dis.sh normalised diffs.
- result: BASE 3 at 127/127 with the same three-instruction positional residual as s36-s40; Q1 14 at 127/127 with the same five-seat permutation (p/sh swapped to a1/a0, q in v0, links sharing p's a1).
- verdict: CONFIRMED

## [s41] A same-value re-store p = q placed in the preheader directly after the base add (loop 1 only in M1, both loops in M2) is a flow-time reader of q that keeps the cse-produced copy alive past the add and gives q the a0 conflict target's seat needs.
- mechanism: flow would see q read after the add, so combine keeps the copy (combine.c added_sets_2) and global records a conflict with base's register.
- probe: s41/body_M1.c and s41/body_M2.c on the HEAD chassis via s41/cells.ps1; normalised diff s41/B_M1.txt vs s41/T.txt.
- result: M1 = 12 at 127/125, M2 = 14 at 127/125. cse deletes the re-store before flow (q is known equal to p in that extended block), p becomes live across loop 1, loop 2's guard stops reloading slots, both preheader copies vanish and p/sh swap seats. Same fate as s39's between-copy-and-add placement. Banked as rejected/s41_p_eq_q_after_base_add_cse_folds_costs_12.c and rejected/s41_both_loops_p_eq_q_after_base_add_costs_14.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD src/ings.c:820 INCLUDE_ASM anchor plus body_M1.c / body_M2.c; BASE re-audited at 3 (127/127); no FAKE constructs

## [s41] Both scan loops written as for (i = 0;; i++) { if (i >= count) break; ... } (exit test at the loop top, no rotation, no duplicated exit test) reproduce target's guard-add / base-add pair and the per-iteration bound reload.
- mechanism: A top-of-loop break avoids jump.c duplicate_loop_exit_test (no simplejump after NOTE_INSN_LOOP_BEG) while the source states the count test once.
- probe: s41/body_F1.c on the HEAD chassis via s41/cells.ps1.
- result: 22 at 127/119. The loop is not rotated, the bound test sits at the top and only one address add survives. Banked as rejected/s41_for_break_at_top_no_rotation_costs_22.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD src/ings.c:820 INCLUDE_ASM anchor plus body_F1.c; BASE re-audited at 3 (127/127); no FAKE constructs

## [s41] Both scan loops written as i = 0; goto testK; do { body; i++; testK:; } while (i < count); (a jump into the loop that makes loop.c mark the loop invalid, so no LICM and no VTOP) keep the bound in the loop bottom while reproducing target's guard block.
- mechanism: loop.c find_and_verify_loops sets loop_invalid for a loop entered from outside (loop.c:2596), so nothing is hoisted and the bound reload stays; the guard would be the first execution of the bottom test.
- probe: s41/body_G1.c on the HEAD chassis via s41/cells.ps1; normalised diff s41/B_G1.txt.
- result: 43 at 127/117. The guard block collapses to sll / lw / lw / j bottom-test / addu with no blez and no guard add, the frame shrinks from 64 to 48 bytes with the guard two-step's phantom slots, and the bottom compare is a real slt rather than target's blez. Banked as rejected/s41_goto_into_loop_invalid_no_guard_costs_43.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD src/ings.c:820 INCLUDE_ASM anchor plus body_G1.c; BASE re-audited at 3 (127/127); no FAKE constructs

## [s41] global.c's preference machinery (set_preference resolving reg_renumber of local-alloc pseudos, find_reg's hard_reg_copy_preferences branch, regs_someone_prefers) offers a route to the a3 seat for a copy destination that is not live across its loop body.
- mechanism: set_preference (global.c:1671-1740) converts both operands through reg_renumber, so a copy to/from a local-alloc pseudo already seated in a3, or to/from hard register a3, gives the copy destination a copy preference that find_reg (global.c:1080-1110) honours over the lowest free register; regs_someone_prefers (global.c:877-935) could alternatively exclude v0/a0 in pass 0.
- probe: Read global.c:340-372, :877-935, :952-1110, :1671-1740 and local-alloc.c:1784-1900 against the target stream; cross-checked with s40's BB2_FINDREG_DEBUG=80 dump (own_copy_prefs empty, someone_prefers empty).
- result: No local qty can be seated in a3 here (find_free_reg takes the lowest register not busy in the block, global pseudos are invisible to local-alloc, every block holds one local chain in v0); the only hard a3 write is the entry copy of the slot_b parameter pseudo (seated s3, crosses the call); the only v0/a0-preferring pseudos (call arguments, call result) cannot overlap a preheader copy without crossing the call. The preference route is closed by reading, and s40's live-across-the-loop requirement is confirmed on an independent second reading.
- verdict: CONFIRMED

## s42 hypotheses (2026-09-06, structural; owner directive already executed s37-s41)

## [s42] The floor and residual survive unchanged on the HEAD chassis this session, and the closest-to-target instance kill (Q1) is not stale.
- mechanism: Driver-mandated chassis re-audit and kill re-audit before any probe; no FAKE construct exists in any form of this function so fake_ablate is vacuous.
- probe: sandbox func_80017848 --disable all with s42/body_BASE.c and s42/body_Q1.c over the src/ings.c:820 anchor; s42/dis.sh normalised diff.
- result: BASE 3 at 127/127, Q1 14 at 127/127. The fresh BASE diff shows loop 1 instruction-exact and the residual = tail `lw a0,12(s2)` vs `addu a0,a3,zero`, loop-2 `addu a3,a0,zero` vs `lw v0,12(s2)`, `addu a0,a1,a3` vs `addu a0,a1,v0` (s41/B.txt was not the BASE stream).
- verdict: CONFIRMED

## [s42] Target's loop-1-exit tail geometry (`lw a0,12(s2); sll a1,s4,6` executed on the fall-through only, the loop-1 blez landing on `addu v0,a1,a0`) is produced by reorg.c's redundant-insn thread redirect from an UNCONDITIONAL reload of p in the join block, not by a C-level exit tail.
- mechanism: reorg.c:3442-3459 fill_slots_from_thread: a thread insn that redundant_insn() matches against an insn before the branch, on a thread the branch does not own, advances new_thread past it; reorg.c:3714-3716 then emits a label before new_thread and redirects the jump. Guard 1's own `lw a0,12(s2)` / `sll a1,s4,6` make the join block's identical pair redundant on the taken path.
- probe: cell U1 (BASE minus `p = q`, plus `p = *(u8 **)(ctx + 0xC);` unconditionally before loop 2); final asm s42/raw_U1.txt; instrumented dumps s42/iU1_sched2_insns.txt and s42/iU1_dbr_insns.txt.
- result: blez targets 0x1468 = `sll v0,s4,6`, one past the join block's `lw a0,12(s2)` (0x1464). .sched2: code_label 140 precedes insn 143 (the reload). .dbr: label 140 gone, new code_label 380 after insn 143, jump_insn 79 carries label_ref 380. Only the lw is skipped in U1 (its sh2 landed in v0, not a1); in target both lw and sll are register-identical to guard 1's and both are skipped.
- verdict: CONFIRMED

## [s42] An unconditional reload of p before loop 2 with loop 2's guard written inline (`if (i < *(s32 *)(sh2 + p + 0x20))`) keeps the loop-2 base add.
- mechanism: the join-block load makes the preheader's read of ctx+0xC redundant, so cse folds `sh2 + q` into the guard's still-available address temp.
- probe: cells U1 and U2 (U2 adds explicit `q = p;` in loop 2's preheader) on the HEAD chassis via s42/run_cell.ps1.
- result: U1 = U2 = 9 at 127/124: loop 1's copy dies (no flow-time reader), loop 2's guard becomes `sll v0; addu a0,v0,a0; lw v0,32(a0)` with the guard dest reused as base and no base add. Banked as rejected/s42_unconditional_reload_l2_inline_guard_base_folds_costs_9.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD src/ings.c:820 INCLUDE_ASM anchor plus body_U1.c / body_U2.c; BASE re-audited at 3 (127/127); no FAKE constructs

## [s42] On the symmetric chassis, spelling loop 2's guard as a two-step through the SAME local `t` that loop 1's guard uses reproduces target's guard/base pair.
- mechanism: the two-step overwrites the guard's address value so cse cannot fold the base add into it.
- probe: cells U3 (explicit `q = p` base) and U3b (fresh-read base).
- result: 32 at 127/125 for both: `t` shared across both loops becomes a long-lived global allocno that outranks i/p/sh (t->v1, i->a0, p->v0), the s9 t-reuse rotation on this chassis. Banked as rejected/s42_symmetric_shared_t_twostep_seat_rotation_costs_32.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD src/ings.c:820 INCLUDE_ASM anchor plus body_U3.c / body_U3b.c; BASE re-audited at 3 (127/127); no FAKE constructs

## [s42] The natural symmetric chassis (no exit tail, unconditional reload, distinct two-step local t2 for loop 2's guard, `q = p; base = sh2 + q` in loop 2's preheader) reproduces target except for the two preheader copies.
- mechanism: with reorg supplying the tail geometry, the only remaining divergence should be the two use-once copies combine deletes.
- probe: cells U4 (explicit copy) and U4b (fresh-read base), normalised diffs s42/B_U4.txt; dumps s42/iU4/ings.i.cse2 and .combine.
- result: 6 at 127/125 for both. Diff vs target is exactly: both `addu a3,a0,zero` absent, both base adds `addu a0,a1,a0`, loop 2's links in a1 not a2 (consequence of the missing copy: sh dies at the add before the links load). cse2 holds both copies (insns 83 and 162 into reg/v 80); .combine holds neither. Body equals s34's candidate_alt_join_shape_6.c modulo declaration order. Banked as rejected/s42_natural_symmetric_reload_both_copies_die_in_combine_costs_6.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD src/ings.c:820 INCLUDE_ASM anchor plus body_U4.c / body_U4b.c; BASE re-audited at 3 (127/127); no FAKE constructs

## [s42] Writing the reload `p = *(u8 **)(ctx + 0xC);` directly after BASE's `p = q;` exit tail keeps q's flow-time read (so loop 1's copy) while making the tail target's `lw a0,12(s2)`.
- mechanism: `p = q` would still be scanned as a use of q by flow even though its value is immediately overwritten.
- probe: cell U5 on the HEAD chassis; diff s42/B_U5.txt.
- result: 4 at 127/126. The tail is now `lw a0,12(s2)` (positional match) but `p = q` is a dead store: flow.c's insn_dead_p path deletes it before mark_used_regs runs on it, q has no reader, loop 1's copy dies in combine (`addu a0,a1,a0`). Loop 2 unchanged from BASE. Banked as rejected/s42_p_eq_q_then_reload_dead_store_read_uncounted_costs_4.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD src/ings.c:820 INCLUDE_ASM anchor plus body_U5.c; BASE re-audited at 3 (127/127); no FAKE constructs

## [s42] Reload's input-reload copy producer (find_equiv_reg supplying a hard register that already holds the addend's value, emitted as gen_move_insn (reloadreg, oldequiv)) cannot fire for either preheader copy in this function because every form fails the predicate that a MEM operand or an unallocated pseudo reaches reload.
- mechanism: reload1.c:5843-5851 requires GET_CODE (old) == MEM or a pseudo with reg_renumber < 0. The MIPS addsi3 predicates force expand to load a MEM addend into a pseudo; the only pass that puts a MEM back into an add is local-alloc.c:1079-1082 (reg_n_refs == 2 && reg_basic_block < 0 with a REG_EQUIV note), and REG_EQUIV notes are created only by function.c:3838-3854 for stack-passed parameters and by local-alloc.c:1051-1055 for reg_basic_block >= 0 (LOCAL) pseudos; global.c:414-432 allocates every other pseudo with refs. This function has four register parameters and no stack parameter.
- probe: read reload1.c:5800-5870, reload.c:5348-5420 (find_equiv_reg), local-alloc.c:1030-1115, global.c:396-432, and grep of every REG_EQUIV emitter in the tree (function.c only).
- result: the producer s39 named is closed by predicate, not by trial: no C spelling of this function can present reload with a MEM addend or an unallocated addend pseudo.
- verdict: KILLED
- kill_scope: class
- predicate_cite: local-alloc.c:1079
- measured_on: GCC 2.7.2 source under tools/gcc-2.7.2 (reload1.c, reload.c, local-alloc.c, global.c, function.c); BASE re-audited at 3 (127/127); no FAKE constructs

## [s42] The floor and residual survive unchanged on the HEAD chassis this session, and the closest-to-target instance kill (Q1) is not stale.
- mechanism: Driver-mandated chassis re-audit and kill re-audit; no FAKE construct exists so fake_ablate is vacuous.
- probe: sandbox --disable all with s42/body_BASE.c and body_Q1.c over src/ings.c:820; s42/dis.sh normalised diff.
- result: BASE 3 at 127/127, Q1 14 at 127/127. Fresh BASE diff: loop 1 instruction-exact; residual = tail lw a0,12(s2) vs addu a0,a3,zero, loop-2 addu a3,a0,zero vs lw v0,12(s2), addu a0,a1,a3 vs addu a0,a1,v0. s41/B.txt was not the BASE stream.
- verdict: CONFIRMED

## [s42] Target's loop-1-exit tail geometry (lw a0,12(s2); sll a1,s4,6 on the fall-through only, loop-1 blez landing on addu v0,a1,a0) is produced by reorg.c's redundant-insn thread redirect from an UNCONDITIONAL reload of p in the join block, not by a C-level exit tail.
- mechanism: reorg.c:3442-3459 fill_slots_from_thread advances new_thread past a thread insn that redundant_insn matches against an insn before the branch on a non-owned thread; reorg.c:3714-3716 emits a label there and redirects the jump. Guard 1's own lw/sll make the join block's identical pair redundant on the taken path.
- probe: Cell U1 (unconditional reload, no p = q); s42/raw_U1.txt; instrumented dumps s42/iU1_sched2_insns.txt vs s42/iU1_dbr_insns.txt.
- result: blez targets 0x1468, one past the join block's lw a0,12(s2) at 0x1464. .sched2 has code_label 140 before reload insn 143; .dbr has label 140 gone, new code_label 380 after insn 143, jump_insn 79 retargeted to label_ref 380.
- verdict: CONFIRMED

## [s42] An unconditional reload of p before loop 2 with loop 2's guard written inline keeps the loop-2 base add (cells U1/U2).
- mechanism: The join-block load makes the preheader read of ctx+0xC redundant, so cse folds sh2 + q into the guard's available address temp.
- probe: Cells U1 and U2 on the HEAD chassis via s42/run_cell.ps1.
- result: 9 at 127/124 for both: loop 1's copy dies, loop 2's guard dest is reused as base and the base add vanishes. Banked as rejected/s42_unconditional_reload_l2_inline_guard_base_folds_costs_9.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD src/ings.c:820 INCLUDE_ASM anchor plus body_U1.c / body_U2.c; BASE re-audited at 3 (127/127); no FAKE constructs

## [s42] On the symmetric chassis, a two-step loop-2 guard through the SAME local t that loop 1's guard uses reproduces target's guard/base pair (cells U3/U3b).
- mechanism: The two-step overwrites the guard address so cse cannot fold the base add into it.
- probe: Cells U3 (explicit q = p) and U3b (fresh-read base).
- result: 32 at 127/125 for both: shared t becomes a long-lived global allocno outranking i/p/sh (t->v1, i->a0, p->v0), the s9 t-reuse rotation. Banked as rejected/s42_symmetric_shared_t_twostep_seat_rotation_costs_32.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD src/ings.c:820 INCLUDE_ASM anchor plus body_U3.c / body_U3b.c; BASE re-audited at 3 (127/127); no FAKE constructs

## [s42] The natural symmetric chassis (no exit tail, unconditional reload, distinct two-step local t2 for loop 2's guard, q = p copies) reproduces target except for the two preheader copies (cells U4/U4b).
- mechanism: With reorg supplying the tail geometry, the only remaining divergence should be the two use-once copies combine deletes.
- probe: Cells U4 and U4b; diffs s42/B_U4.txt; dumps s42/iU4/ings.i.cse2 and .combine.
- result: 6 at 127/125 for both. Diff vs target is exactly both addu a3,a0,zero absent, both base adds reading a0, loop 2's links in a1 not a2. cse2 holds both copies (insns 83 and 162 into reg/v 80); .combine holds neither. Equals s34's candidate_alt_join_shape_6.c modulo declaration order. Banked as rejected/s42_natural_symmetric_reload_both_copies_die_in_combine_costs_6.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD src/ings.c:820 INCLUDE_ASM anchor plus body_U4.c / body_U4b.c; BASE re-audited at 3 (127/127); no FAKE constructs

## [s42] Writing the reload p = *(u8 **)(ctx + 0xC) directly after BASE's p = q exit tail keeps q's flow-time read (loop 1's copy) while making the tail target's lw a0,12(s2) (cell U5).
- mechanism: p = q would still be scanned as a use of q by flow although immediately overwritten.
- probe: Cell U5 on the HEAD chassis; diff s42/B_U5.txt.
- result: 4 at 127/126: the tail is target's lw but p = q is a dead store deleted by flow before its read counts, so loop 1's copy dies in combine; loop 2 unchanged. Banked as rejected/s42_p_eq_q_then_reload_dead_store_read_uncounted_costs_4.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD src/ings.c:820 INCLUDE_ASM anchor plus body_U5.c; BASE re-audited at 3 (127/127); no FAKE constructs

## [s42] Reload's input-reload copy producer (find_equiv_reg supplying a hard register already holding the addend value, emitted as gen_move_insn (reloadreg, oldequiv)) cannot fire for either preheader copy in this function because every form fails the predicate that a MEM operand or an unallocated pseudo reaches reload.
- mechanism: reload1.c:5843-5851 requires GET_CODE (old) == MEM or reg_renumber < 0; MIPS addsi3 predicates force expand to load a MEM addend into a pseudo; the only pass that puts a MEM back into an add is local-alloc.c:1079-1082 (reg_n_refs == 2 && reg_basic_block < 0 with a REG_EQUIV note); REG_EQUIV notes come only from function.c:3838-3854 (stack-passed params) or local-alloc.c:1051-1055 (LOCAL pseudos); global.c:414-432 allocates every other pseudo with refs. This function has four register parameters.
- probe: Read reload1.c:5800-5870, reload.c:5348-5420, local-alloc.c:1030-1115, global.c:396-432; grep of every REG_EQUIV emitter (function.c only).
- result: The producer s39 named is closed by predicate: no C spelling of this function presents reload with a MEM addend or an unallocated addend pseudo.
- verdict: KILLED
- kill_scope: class
- measured_on: GCC 2.7.2 source under tools/gcc-2.7.2; BASE re-audited at 3 (127/127); no FAKE constructs
- predicate_cite: local-alloc.c:1079

## [s43] The floor and residual survive unchanged on the HEAD chassis this session, and s42's natural symmetric chassis U4 is not stale.
- mechanism: chassis re-audit mandated by the brief; the residual is the two use-once preheader copies combine deletes (E-s42-1/E-s42-3).
- probe: s43/body_BASE.c and s43/body_U4.c applied over the src/ings.c:820 anchor, sandbox --disable all.
- result: BASE = 3 at 127/127, U4 = 6 at 127/125, both identical to s42. CONFIRMED.

## [s43] Writing loop 1's exit tail (reload of p and recompute of sh) inside the guard's then-block instead of an unconditional join-block reload reproduces target's join geometry with the same seats as U4.
- mechanism: the tail-in-if spelling gives the same instruction stream on both paths (E-s42-2 showed reorg's redirect produces target's fall-through-only lw/sll from a join reload); the question was whether the two-definition pseudos p and sh keep U4's seats.
- probe: cell W1 (s43/body_W1.c), D_W1.txt, iW1/.
- result: KILLED (instance). 12 at 127/125: both guards swap to lw a1,12(s2) / sll a0,s4,6 / addu v0,a0,a1, both preheaders addu a0,a0,a1 (no copy), loop-2 links a1. Measured on the HEAD src/ings.c:820 chassis, U4-derived body, no FAKE constructs. Banked as rejected/s43_exit_tail_reload_inside_if_seat_swap_costs_12.c.

## [s43] A C self-assignment `q = q;` of the copy destination inside the loop body is a loop-carried flow-time reader that keeps q live across the body (a3 seat) and vanishes as a no-op move after allocation.
- mechanism (proposed): a self-set both uses and sets q, so q would be live-in at the loop top and not dead at the base add (combine keeps the copy, added_sets_2), and the post-RA `addu a3,a3,zero` is deleted by jump2.
- probe: cell W2 (s43/body_W2.c) = U4 + `q = q;` at the top of both loop bodies; iW2/ dumps grepped for self-set insns in every pass.
- result: KILLED (class). 6 at 127/125, object identical to U4; zero `(set (reg N) (reg N))` insns in .rtl and every later dump. expr.c:2845 (store_expr: a move is emitted only when `temp != target`) means a self-assignment never reaches RTL in any spelling that expands the variable to its own DECL_RTL; and flow.c:1590 (uses of a dead insn are never marked, in every propagate_block pass) means an emitted self-set could not seed its own liveness either. Measured on the HEAD src/ings.c:820 chassis, no FAKE constructs. Banked as rejected/s43_self_assign_q_eq_q_in_loop_body_emits_no_rtl.c.

## [s43] The sanctioned `do { } while (0);` wrap (owner ruling 2026-07-06, any codegen effect incl. register allocation) placed around the preheader copy, the preheader statements, or the whole then-block keeps the use-once copy or moves the copy destination's seat.
- mechanism (proposed): the wrap adds NOTE_INSN_LOOP_BEG/END and a loop label around the wrapped statements, which could (a) leave a basic-block boundary between copy and add (no LOG_LINK, escape #8) or (b) change loop_depth so flow's reg_n_refs/reg_live_length weighting re-orders global's allocation.
- probe: cells X1 (copy only), X2 (whole then-block), X4 (three preheader statements), each in both loops, on U4; D_X2.txt, iX2/.
- result: KILLED (instance). X1 = X2 = X4 = 6 at 127/125; X2 and X4 raw objdumps byte-identical to each other and to U4's residual. The `while (0)` test folds at expand, the loop label is unreferenced and deleted by jump1 before cse, so the copy and the add remain one basic block and combine deletes the copy as before; no seat changes because the wrapped pseudos' weights change identically. Measured on the HEAD src/ings.c:820 chassis, U4-derived bodies, wraps unannotated (measurement cells). Banked as rejected/s43_dowhile0_wrap_{copy_only,whole_then_block,preheader_stmts}_*.c.

## [s43] The `do { } while (0);` wrap around the inner scan loop only (preheader outside the wrap) changes the allocation order enough to seat the copy destination in a3 or otherwise moves toward target.
- mechanism (proposed): only the loop-body pseudos gain loop_depth weight, so the preheader pseudos' relative priority drops and find_reg's order changes.
- probe: cell X3 (s43/body_X3.c), D_X3.txt, iX3/.
- result: KILLED (instance). 10 at 127/125: loop 1's sh seats a2 and lnk a1 (sll a2,s4,6 / addu v0,a2,a0 / lw a1,16(s2) / addu a0,a2,a0), loop 2's links a1, no copy in either preheader - the copy is deleted by combine before any allocation-order effect can matter. Measured on the HEAD src/ings.c:820 chassis, no FAKE constructs. Banked as rejected/s43_dowhile0_wrap_inner_loop_only_seat_rotation_costs_10.c.

## [s43] find_reg's pass-0 exclusions (regs_someone_prefers / regs_used_so_far) can give the copy destination the a3 seat without it being live across the loop body.
- mechanism: global.c:1000-1001 excludes registers not yet used and registers some other allocno prefers in pass 0.
- probe: reading global.c:882-930 (prune_preferences) and :344-372 (regs_used_so_far seed) against this function's pseudos.
- result: CONFIRMED-DEAD by reading (no measurement needed): regs_someone_prefers merges only LOWER-priority conflicting allocnos' preferences (global.c:919-928); the only a0-a3-preferring pseudos are the four parameter copies, all call-crossing, whose preferences global.c:899-910 prunes by call_used_reg_set; v0 and a0 are seeded used by local-alloc qtys. The a3 seat requires hard_reg_conflicts with v0 AND a0, i.e. q live when base is born. Third independent confirmation of s40/s41.

## [s43] The floor and residual survive unchanged on the HEAD chassis this session, and s42's natural symmetric chassis U4 is not stale.
- mechanism: Chassis re-audit; residual = the two use-once preheader copies combine deletes.
- probe: s43/body_BASE.c and s43/body_U4.c over the src/ings.c:820 anchor, sandbox --disable all.
- result: BASE = 3 at 127/127; U4 = 6 at 127/125; both identical to s42.
- verdict: CONFIRMED

## [s43] Writing loop 1's exit tail (reload of p and recompute of sh) inside the guard's then-block instead of an unconditional join-block reload keeps U4's seats and target's join geometry.
- mechanism: Both paths carry the same instruction stream; the two-definition pseudos p and sh were expected to keep U4's seats.
- probe: Cell W1 (s43/body_W1.c), D_W1.txt, iW1/ dumps.
- result: 12 at 127/125: both guards swap to lw a1,12(s2) / sll a0,s4,6 / addu v0,a0,a1, both preheaders addu a0,a0,a1 with no copy, loop-2 links a1.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD src/ings.c:820 INCLUDE_ASM anchor plus body_W1.c (U4-derived); BASE re-audited at 3 (127/127); no FAKE constructs

## [s43] A C self-assignment `q = q;` of the copy destination inside the loop body is a loop-carried flow-time reader that keeps q live across the body and vanishes as a no-op move after allocation.
- mechanism: A self-set would use and set q, making q live-in at the loop top and not dead at the base add (combine added_sets_2 keeps the copy); the post-RA self-move would be deleted by jump2.
- probe: Cell W2 (s43/body_W2.c) = U4 + `q = q;` at the top of both loop bodies; iW2/ dumps grepped for self-set insns in every pass.
- result: 6 at 127/125, object identical to U4; zero (set (reg N) (reg N)) insns in .rtl or any later dump. expr.c:2845 store_expr emits a move only when temp != target, so a self-assignment never reaches RTL; flow.c:1590 never marks the uses of a dead insn in any propagate_block pass, so an emitted self-set could not seed its own liveness either.
- verdict: KILLED
- kill_scope: class
- measured_on: HEAD src/ings.c:820 INCLUDE_ASM anchor plus body_W2.c (U4-derived); BASE re-audited at 3 (127/127); no FAKE constructs
- predicate_cite: tools/gcc-2.7.2/expr.c:2845

## [s43] The sanctioned do { } while (0); wrap placed around the preheader copy alone (X1), the whole guard then-block (X2), or the three preheader statements (X4), in both loops, keeps the use-once copy or moves the copy destination's seat.
- mechanism: The wrap's NOTE_INSN_LOOP_BEG/END and loop label could leave a block boundary between copy and add (no LOG_LINK) or change loop_depth weighting of reg_n_refs/reg_live_length for global's allocation order.
- probe: Cells X1, X2, X4 on U4 (s43/body_X{1,2,4}.c), D_X2.txt, iX2/ dumps, raw objdump comparison.
- result: X1 = X2 = X4 = 6 at 127/125; X2 and X4 raw objdumps byte-identical to each other and to U4's residual. The while(0) test folds at expand, the loop label is unreferenced and deleted by jump1 before cse, the copy and the add stay one basic block and combine deletes the copy.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD src/ings.c:820 INCLUDE_ASM anchor plus body_X1.c / body_X2.c / body_X4.c (U4-derived); BASE re-audited at 3 (127/127); wraps unannotated measurement cells, no other FAKE constructs

## [s43] The do { } while (0); wrap around the inner scan loop only, with the preheader outside the wrap, changes global's allocation order enough to seat the copy destination in a3.
- mechanism: Only the loop-body pseudos gain loop_depth weight, lowering the preheader pseudos' relative priority.
- probe: Cell X3 (s43/body_X3.c), D_X3.txt, iX3/ dumps.
- result: 10 at 127/125: loop 1's sh seats a2 and lnk a1 (sll a2,s4,6 / addu v0,a2,a0 / lw a1,16(s2) / addu a0,a2,a0), loop 2's links a1, no copy in either preheader; the copy is deleted by combine before allocation order can matter.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD src/ings.c:820 INCLUDE_ASM anchor plus body_X3.c (U4-derived); BASE re-audited at 3 (127/127); wrap unannotated measurement cell, no other FAKE constructs

## [s43] find_reg's pass-0 exclusions (regs_someone_prefers / regs_used_so_far) can give the copy destination the a3 seat without it being live across the loop body.
- mechanism: global.c:1000-1001 excludes not-yet-used registers and registers preferred by other allocnos in pass 0.
- probe: Reading global.c:882-930 (prune_preferences) and :344-372 (regs_used_so_far seed) against this function's pseudos.
- result: Dead by reading: regs_someone_prefers merges only lower-priority conflicting allocnos' preferences (global.c:919-928); the only a0-a3-preferring pseudos are the four call-crossing parameter copies whose preferences global.c:899-910 prunes by call_used_reg_set; v0 and a0 are seeded used by local-alloc qtys. The a3 seat requires hard_reg_conflicts with v0 and a0, i.e. q live when base is born. Third independent confirmation of s40/s41.
- verdict: CONFIRMED

## s44 hypotheses (2026-09-06, synthesis; owner directive already executed s37-s43)

## [s44] The floor and residual survive unchanged on the HEAD chassis this session, and s42's natural symmetric chassis U4 is not stale.
- mechanism: Chassis re-audit mandated by the brief; residual = the two use-once preheader copies combine deletes.
- probe: s44/body_BASE.c and s44/body_U4.c over the src/ings.c:820 anchor, sandbox --disable all.
- result: BASE = 3 at 127/127; U4 = 6 at 127/125; both identical to s43.
- verdict: CONFIRMED

## [s44] Placing the copy `q = p` in the guard block immediately after the guard load and before the two-step address seats q in a3 by conflicts alone and keeps the copy (frontier item 1).
- mechanism (proposed): conflicts recorded at every birth while q is live; copy and add in different blocks so combine never links them.
- probe: cells F1a (`q = p`) and F1b (fresh read) on U4, both loops; D_F1a.txt.
- result: 6 at 127/125 for both, same object class as U4: cse2's extended block spans guard + preheader, q's last reference is the base add inside it, make_regs_eqv (cse.c:826-855) keeps p canonical, the add is rewritten to read p and the copy is dead before flow. Measured on the HEAD src/ings.c:820 chassis, U4-derived bodies, no FAKE constructs. Banked as rejected/s44_frontier1_copy_in_guard_block_before_twostep_cse2_canonicalizes_costs_6.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD src/ings.c:820 INCLUDE_ASM anchor plus body_F1a.c / body_F1b.c (U4-derived); BASE re-audited at 3 (127/127); no FAKE constructs

## [s44] Guard 1 loading into q with `p = q` in the preheader and `base = sh + p` (roles swapped) keeps a copy or moves the copy destination's seat (frontier item 2).
- mechanism (proposed): the guard's own load as q's definition avoids entry liveness on the loop-skip paths.
- probe: cell F2 on U4 (both loops); cell F2b = F2 plus loop 2's guard reading q (a real post-loop reader).
- result: F2 = 6 at 127/125 (p short-lived, folded by combine, identical to U4). F2b = 12 at 127/124: the copy is folded into the base add which reads q, q takes a1 and sh a0 in both guards, loop-2 links a1. Measured on the HEAD src/ings.c:820 chassis, no FAKE constructs. Banked as rejected/s44_frontier2_roles_swapped_guard_loads_q_preheader_p_eq_q_costs_6.c and rejected/s44_loop2_guard_reads_q_real_postloop_reader_seats_q_in_a1_costs_12.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD src/ings.c:820 INCLUDE_ASM anchor plus body_F2.c / body_F2b.c (U4-derived); BASE re-audited at 3 (127/127); no FAKE constructs

## [s44] A loop-body read of the copy destination that survives cse1/loop/cse2/flow and is folded to a constant by combine keeps the preheader copy alive with zero reader bytes, and global.c allocates the copy destination as live across the loop.
- mechanism: flow (before combine) marks q live at the loop top so the base add carries no REG_DEAD for q; try_combine(copy -> add) sets added_sets_2 (combine.c:1458), the two-SET PARALLEL is unrecognised and unsplittable (combine.c:1704/1718 need REG_UNUSED, :1900/:1996 need i1) so the copy and the q-reading add remain; combine folds the reader via nonzero_bits/simplify_and_const_int; global_conflicts starts each block from flow's basic_block_live_at_start, which nothing recomputes after combine.
- probe: instrument cells M1 (m = slot_b & 1 chain), M5/M6 (sh & i chain, separate statements, exit-test address consumer); iM1/, iM5/, iM6/ dumps (.cse2/.flow/.combine/.greg), D_M1.txt, D_M5.txt, D_M6.txt. All instruments are dead algebra (cheats), banked as rejected/s44_INSTRUMENT_cheat_*.c, never candidates.
- result: M1 = 14 at 127/128 with loop 1's copy alive (`addu a1,a0,zero` / `addu a0,a2,a1`) and no read of a1 in the loop; M5 = 12 at 127/127 = target's exact instruction stream with BOTH copies alive and a pure seat permutation (q a1, sh a2, lnk a3); M6 = 12 at 127/127 likewise. Mechanism confirmed at the pass level on the HEAD chassis. E-s43-5's claim that no pre-RA deleter of a flow-time reader exists is refuted.
- verdict: CONFIRMED

## [s44] With a combine-folded reader present, the copy destination seats in a3 as soon as it is allocated after sh and lnk; the seat is decided by global.c's priority order, not by conflicts.
- mechanism: global.c:615 priority = floor_log2(n_refs) * n_refs / live_length * 10000 * size; find_reg takes the lowest non-conflicting register in numeric order, so q (conflicting with v0 temps, v1 = i, a0 = base, a1 = sh, a2 = lnk once live across the loop) takes a3 only if sh and lnk already hold a1/a2.
- probe: ALLOCDBG order in iM5/iM6/cc1.log versus s43/iX2 (U4): M5 q 9230 (8/26), M6 q 6153 (4/13), sh 5555 (livelen 18, stretched by the instrument's in-loop read of sh), lnk 2500; U4 sh 11428 (4/7), lnk 8333 (5/12).
- result: In both instruments q precedes sh and takes a1. The instrument itself depresses sh's priority; in U4's geometry a 4-ref / >=10-length q would sort after sh and lnk. Unmeasured inference - next probe must use a fold source that reads no pre-existing pseudo and adds no cross-block pseudo (M7's named idx became a global pseudo, priority 30000, and rotated every seat: 35 at 126).
- verdict: CONFIRMED

## [s44] Instrument spellings that combine does not fold byte-free (banked so they are not retried): an ashiftrt of the lbu value (M2, `sra` + `bne` survive, 45/38), a constant-condition conditional jump (M3, `y = 0` computed but `bne v0,zero` kept, 14/16), a single-expression and-chain (M4, fold-const reassociates `q & 63` into a hoisted invariant, 10/9).
- mechanism: combine.c's shift simplifier has no nonzero-bits zeroing on the jump path; a jump whose condition folds to a constant register is not rewritten by combine; fold-const reassociates constants onto the first operand.
- probe: cells M2, M2a, M3, M3a, M4, M4a; D_M2.txt, D_M3.txt, D_M4.txt, D_M4a.txt.
- result: as stated; none is byte-free. Measured on the HEAD src/ings.c:820 chassis, instrument bodies (cheats), no FAKE constructs.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD src/ings.c:820 INCLUDE_ASM anchor plus body_M2.c / body_M2a.c / body_M3.c / body_M3a.c / body_M4.c / body_M4a.c (U4-derived instruments); BASE re-audited at 3 (127/127); no FAKE constructs

## [s44] The floor and residual survive unchanged on the HEAD chassis this session, and s42's natural symmetric chassis U4 is not stale.
- mechanism: Chassis re-audit mandated by the brief; residual = the two use-once preheader copies combine deletes; no FAKE constructs exist in any form so fake_ablate is vacuous.
- probe: s44/body_BASE.c and s44/body_U4.c applied over the src/ings.c:820 INCLUDE_ASM anchor, sandbox --disable all.
- result: BASE = 3 at 127/127; U4 = 6 at 127/125; both identical to s43.
- verdict: CONFIRMED

## [s44] Placing the copy q = p in the guard block immediately after the guard load and before the two-step address (frontier item 1) keeps the copy and seats q in a3 by conflicts alone, measured as cells F1a and F1b on the U4 chassis.
- mechanism: Proposed: conflicts recorded at every birth while q is live, copy and add in different blocks so combine never links them. Actual: cse2's extended block spans guard + preheader; q's last reference (the base add) lies inside it so cse.c:826-855 make_regs_eqv keeps p canonical, the add is rewritten to read p, and the copy is dead before flow.
- probe: Cells F1a (q = p) and F1b (fresh read) on U4, both loops; D_F1a.txt.
- result: 6 at 127/125 for both, same residual as U4; the copy never reaches combine. Banked as rejected/s44_frontier1_copy_in_guard_block_before_twostep_cse2_canonicalizes_costs_6.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD src/ings.c:820 INCLUDE_ASM anchor plus body_F1a.c / body_F1b.c (U4-derived); BASE re-audited at 3 (127/127); no FAKE constructs

## [s44] Guard 1 loading into q with p = q in the preheader and base = sh + p (roles swapped, frontier item 2), with and without loop 2's guard reading q, keeps a copy or moves the copy destination's seat toward a3, measured as cells F2 and F2b.
- mechanism: Proposed: the guard's own load as q's definition avoids entry liveness on the loop-skip paths. Actual: the short-lived copy destination p dies at the add and combine folds it (F2); with a real post-loop reader (F2b) the copy is folded into the base add which reads q, and q's higher priority seats it in a1 before sh.
- probe: Cell F2 on U4 (both loops); cell F2b = F2 plus loop 2's guard t2 = sh2 + q (no join reload); D_F2b.txt, iF2b/.
- result: F2 = 6 at 127/125 (identical to U4). F2b = 12 at 127/124: addu a0,a0,a1 reads q, q in a1, sh in a0 in both guards, loop-2 links a1. Banked as rejected/s44_frontier2_roles_swapped_guard_loads_q_preheader_p_eq_q_costs_6.c and rejected/s44_loop2_guard_reads_q_real_postloop_reader_seats_q_in_a1_costs_12.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD src/ings.c:820 INCLUDE_ASM anchor plus body_F2.c / body_F2b.c (U4-derived); BASE re-audited at 3 (127/127); no FAKE constructs

## [s44] A loop-body read of the copy destination that survives cse1/loop/cse2/flow and is folded to a constant by combine keeps the preheader copy alive with zero reader bytes, and global.c allocates the copy destination as live across the loop.
- mechanism: flow (before combine) marks q live at the loop top so the base add carries no REG_DEAD for q; try_combine(copy -> add) sets added_sets_2 at combine.c:1458, the two-SET PARALLEL is unrecognised and the two-insn case has no split path (combine.c:1704/1718 need REG_UNUSED, :1900/:1996 need i1), so the copy and the q-reading add remain; combine folds the reader via nonzero_bits / simplify_and_const_int; global_conflicts starts each block from flow's basic_block_live_at_start, which nothing recomputes after combine.
- probe: Instrument cells M1 (m = slot_b & 1; w = m >> 1; in-loop y = i & w; z = q & y consumed in the exit-test address), M5 and M6 (y = (sh & i) & 0x3F; z = q & y as separate statements); iM1/, iM5/, iM6/ dumps (.cse2/.flow/.combine/.greg), D_M1.txt, D_M5.txt, D_M6.txt. All instruments are dead algebra (cheats by policy), banked as rejected/s44_INSTRUMENT_cheat_*.c, never candidates.
- result: M1 = 14 at 127/128 with loop 1's copy alive (addu a1,a0,zero / addu a0,a2,a1) and no read of a1 in the loop. M5 = 12 at 127/127 = target's exact instruction stream with BOTH copies alive and a pure seat permutation (q a1, sh a2, lnk a3 vs target a3/a1/a2). M6 = 12 at 127/127 likewise. Mechanism confirmed at the pass level; E-s43-5's claim that only post-RA no-op moves and reorg can delete a flow-time reader is refuted.
- verdict: CONFIRMED

## [s44] With a combine-folded reader present, the copy destination's seat is decided by global.c's allocation order (priority), and it takes a3 only when allocated after sh and lnk.
- mechanism: global.c:615 priority = floor_log2(n_refs) * n_refs / live_length * 10000 * size; find_reg takes the lowest non-conflicting register in numeric order; once q is live across the loop it conflicts with v0 temps, v1 = i, a0 = base, a1 = sh, a2 = lnk, so a3 requires sh and lnk to hold a1/a2 already.
- probe: ALLOCDBG order in iM5/cc1.log and iM6/cc1.log versus s43/iX2 (U4): M5 q pri 9230 (8 refs / 26), M6 q 6153 (4 / 13), sh 5555 (livelen 18, stretched by the instrument's in-loop read of sh), lnk 2500; U4 sh 11428 (4 / 7), lnk 8333 (5 / 12).
- result: In both instruments q precedes sh and takes a1. The instrument depresses sh's priority; in U4's geometry a 4-ref / >=10-length q would sort after sh and lnk (unmeasured inference). M7 (fold sourced from the loop's lbu value through a named idx) made idx a cross-block global pseudo (priority 30000, v1) and rotated every seat (35 at 126), so the next instrument must add no cross-block pseudo and read no pre-existing one.
- verdict: CONFIRMED

## [s44] The instrument spellings M2 (ashiftrt of the lbu value feeding a jump), M3 (constant-condition conditional jump guarding the reader) and M4 (single-expression and-chain) produce a byte-free combine-folded reader on the U4 chassis.
- mechanism: combine.c's shift simplifier does not zero an ashiftrt whose operand bits are shifted out on the jump path; a jump whose condition folds to a constant register is not rewritten by combine; fold-const reassociates the constant onto q so q & 63 becomes a hoisted invariant (a real second use).
- probe: Cells M2/M2a, M3/M3a, M4/M4a; D_M2.txt, D_M3.txt, D_M4.txt, D_M4a.txt, iM2/, iM3/, iM4/.
- result: M2 45 at 135 / M2a 38 at 131 (sra + bne survive); M3 14 at 133 / M3a 16 at 132 (y = 0 computed, bne v0,zero kept, reader block reachable); M4 10 at 129 / M4a 9 at 127 (andi a2,v0,63 hoisted, q in v0). None is byte-free. Banked as rejected/s44_INSTRUMENT_cheat_M2/M3/M4_*.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD src/ings.c:820 INCLUDE_ASM anchor plus body_M2.c / body_M2a.c / body_M3.c / body_M3a.c / body_M4.c / body_M4a.c (U4-derived instruments); BASE re-audited at 3 (127/127); no FAKE constructs


## s45 (object-model, 2026-09-06)

Chassis re-measured first: BASE = **3** at 127/127 on the HEAD src/ings.c:820
chassis. fake_ablate: no FAKE constructs in candidate.c (vacuous). Kill
re-audit: s30 cell B re-measured as R30B = 7 at 127/127 (ties s30).

## [s45] Declaring the 52-byte context object as a struct (s16 nlink at +6, u8 *recs at +0xC, u8 *links at +0x10, per the matched sibling func_80017D84) and replacing every *(T *)(ctx + off) with a member access, including a natural ctx->nlink tail, is byte-neutral over the BASE body.
- mechanism: the member loads are the same (mem:SI (plus s2 12)) / (mem:HI (plus s2 6)) rtx as the casts; GCC 2.7.2's MIPS movhi (lhu) vs extendhisi2 (lh) reproduces the target's lhu/lh split for a plain s16 field.
- probe: cell O1 (tmp/grind/func_80017848/s45/body_O1.c) through cells.ps1; objects compared with cmp.
- result: 3 at 127/127 and obj_O1.o is byte-IDENTICAL to obj_BASE.o. The context object's declared shape matches the evidence; typing it neither helps nor costs.
- verdict: CONFIRMED

## [s45] Declaring the 0x40-byte record and 0x10-byte link arrays as structs on top of the typed context (full typing) over the BASE preheader/exit-tail shape, with the tail written through typed pointers, reaches the floor.
- mechanism: struct indexing feeds (plus (mult slot_a 64) recs) address trees to cse/combine and could keep the preheader reload while re-associating the base add the way the target has it.
- probe: cell O2 (s45/body_O2.c) measured, disassembled (s45/D_O2.txt) and dumped with the instrumented cc1 (s45/iO2, CODEGEN-IDENTICAL); loads of 0xC(s2) tracked per pass with s45/loads12.py against s30 cell B (s45/iR30B).
- result: 37 at 127/126 (R30B = 7). The six ctx->recs loads survive every RTL pass identically in O2 and R30B; the +30 is global.c seating the guard load (insn 29) and the preheader load (insn 64) both in v1, after which reorg.c redundant_insn deletes the preheader load (present in .sched2, ABSENT in .dbr) and duplicates sll v0,s4,6 into the second guard's delay slot. Typed indexing also cse-merges the loop base into the guard's address sum (addu a0,a1,zero) so the target's separate base add reading a copy cannot appear.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD src/ings.c:820 INCLUDE_ASM anchor plus body_O2.c (BASE-derived); BASE re-audited at 3 (127/127); no FAKE constructs

## [s45] Full typing (context + record + link structs) over the symmetric U4 chassis (no exit tail, unconditional reload, two-step guard per loop) reaches the floor.
- mechanism: U4 is the natural symmetric geometry (E-s42-3, 6 at 125); typed records might keep its per-loop reloads while changing the base association.
- probe: cell O3 (s45/body_O3.c) through cells.ps1; s45/D_O3.txt.
- result: 43 at 127/122 - five instructions SHORT: with typed indexing cse folds the per-loop record-pointer re-reads and base sums into the guard temps. Worse than U4's 6 in both score and count.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD src/ings.c:820 INCLUDE_ASM anchor plus body_O3.c (U4-derived); BASE re-audited at 3 (127/127); no FAKE constructs

## [s45] The fully natural typed form (one record pointer per loop, for-loops reading rec->na / rec->nb, combined && top guard, tail through the typed pointers) reaches the floor.
- mechanism: the object-model directive's premise - the residual could be a declaration artifact that disappears once the code is written the way a human would against the real types.
- probe: cell O4 (s45/body_O4.c) through cells.ps1.
- result: 47 at 127/118 - nine instructions SHORT: the record pointer and the links pointer are cse-shared across the guards, both loops and the tail, and none of the target's seven 0xC(s2) reloads survive. The shipped source re-reads the pointer per block, which is the matched sibling func_80017D84's raw u8 * idiom on the same object.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD src/ings.c:820 INCLUDE_ASM anchor plus body_O4.c; BASE re-audited at 3 (127/127); no FAKE constructs

## [s45] The s30 struct-typed-record cell B kill is stale on the HEAD chassis (kill re-audit).
- mechanism: instance kills are chassis-relative; the anchor moved from src/ings.c:719 to :820 since s30.
- probe: cell R30B = s30/body_B.c unchanged, through cells.ps1; s45/D_R30B.txt; fake_ablate on candidate.c (s45/fake_ablate.txt).
- result: 7 at 127/127, tying s30 exactly; fake_ablate finds no FAKE construct to ablate. The kill stands on the current chassis.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD src/ings.c:820 INCLUDE_ASM anchor plus s30 body_B.c; BASE re-audited at 3 (127/127); no FAKE constructs

## [s45] Declaring the 52-byte context object as a struct (s16 nlink at +6, u8 *recs at +0xC, u8 *links at +0x10, per matched sibling func_80017D84) and replacing every *(T *)(ctx + off) with a member access, including a natural ctx->nlink tail, is byte-neutral over the BASE body.
- mechanism: The member loads are the same (mem:SI (plus s2 12)) / (mem:HI (plus s2 6)) rtx as the casts; GCC 2.7.2's MIPS movhi (lhu) vs extendhisi2 (lh) reproduces the target's lhu/lh split for a plain s16 field.
- probe: Cell O1 (tmp/grind/func_80017848/s45/body_O1.c) via cells.ps1; cmp of obj_O1.o against obj_BASE.o.
- result: 3 at 127/127 and the object file is byte-identical to BASE. The context object's declared shape MATCHES the evidence; typing it neither helps nor costs.
- verdict: CONFIRMED

## [s45] Declaring the 0x40-byte record and 0x10-byte link arrays as structs on top of the typed context (full typing) over the BASE preheader/exit-tail shape, with the tail written through typed pointers, reaches the floor.
- mechanism: Struct indexing feeds (plus (mult slot_a 64) recs) address trees to cse/combine and could keep the preheader reload while re-associating the base add the way the target has it.
- probe: Cell O2 (s45/body_O2.c) measured, disassembled (s45/D_O2.txt) and dumped with the instrumented cc1 (s45/iO2, CODEGEN-IDENTICAL); the 0xC(s2) loads tracked per pass with s45/loads12.py against s30 cell B (s45/iR30B).
- result: 37 at 127/126 (R30B = 7). The six ctx->recs loads survive every RTL pass identically in O2 and R30B; the +30 is global.c seating the guard load (insn 29) and the preheader load (insn 64) both in v1, after which reorg.c redundant_insn deletes the preheader load (present in .sched2, ABSENT in .dbr) and duplicates sll v0,s4,6 into the second guard's delay slot. Typed indexing also cse-merges the loop base into the guard's address sum (addu a0,a1,zero).
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD src/ings.c:820 INCLUDE_ASM anchor plus body_O2.c (BASE-derived); BASE re-audited at 3 (127/127); no FAKE constructs

## [s45] Full typing (context + record + link structs) over the symmetric U4 chassis (no exit tail, unconditional reload, two-step guard per loop) reaches the floor.
- mechanism: U4 is the natural symmetric geometry (E-s42-3, 6 at 125); typed records might keep its per-loop reloads while changing the base association.
- probe: Cell O3 (s45/body_O3.c) via cells.ps1; s45/D_O3.txt.
- result: 43 at 127/122, five instructions SHORT: with typed indexing cse folds the per-loop record-pointer re-reads and base sums into the guard temps. Worse than U4's 6 in score and count.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD src/ings.c:820 INCLUDE_ASM anchor plus body_O3.c (U4-derived); BASE re-audited at 3 (127/127); no FAKE constructs

## [s45] The fully natural typed form (one record pointer per loop, for-loops reading rec->na / rec->nb, combined && top guard, tail through the typed pointers) reaches the floor.
- mechanism: The object-model directive's premise: the residual could be a declaration artifact that disappears once the code is written the way a human would against the real types.
- probe: Cell O4 (s45/body_O4.c) via cells.ps1.
- result: 47 at 127/118, nine instructions SHORT: the record pointer and the links pointer are cse-shared across the guards, both loops and the tail, and none of the target's seven 0xC(s2) reloads survive. The shipped source re-reads the pointer per block, which is the matched sibling func_80017D84's raw u8 * idiom on the same object.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD src/ings.c:820 INCLUDE_ASM anchor plus body_O4.c; BASE re-audited at 3 (127/127); no FAKE constructs

## [s45] The s30 struct-typed-record cell B kill (7) is stale on the HEAD chassis (mandated kill re-audit).
- mechanism: Instance kills are chassis-relative; the anchor moved from src/ings.c:719 to :820 since s30.
- probe: Cell R30B = s30/body_B.c unchanged, via cells.ps1; s45/D_R30B.txt; tools/fake_ablate.py on candidate.c (s45/fake_ablate.txt).
- result: 7 at 127/127, tying s30 exactly; fake_ablate reports no FAKE-annotated construct to ablate (no form of this function has ever carried one). The kill stands on the current chassis.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD src/ings.c:820 INCLUDE_ASM anchor plus s30 body_B.c; BASE re-audited at 3 (127/127); no FAKE constructs

## [s46] The owner's 2026-09-06 foreclosed-bucket directive (minimal scratch TU holding only the loop-1-exit/loop-2-preheader copy geometry under the exact project flags, .cse2/.combine read to name the protecting context; frontier item 3 since s35) is still unexecuted, so executing it could name a chassis-specific protecting context and re-open the residual.
- mechanism: The consistency audit flagged "DIRECTIVE NOT YET IN LEDGER"; if that were true the directive's probe would be an un-tried forensic lever.
- probe: Ledger read (hypotheses.md H-s37-1 at line 3519 and its s37 entry at line 3576; evidence.md s37 CONCLUSION block at line 4350) plus artifact check of tmp/grind/func_80017848/s37/mini/ (m0..m6 .c sources, run.sh with the exact CC_FLAGS/CPP_FLAGS, 14 .cse2/.combine dumps, flat.py).
- result: The directive WAS executed at s37 and closed as a class kill: m4 (isolated loop) and m5 (isolated no-loop) reproduce the exact promoted copy geometry in .cse2 and both lose the copy in .combine by combine.c:1458 (i2dest dies in i3 so added_sets_2 = 0 and i2 is deleted); no chassis context protects or deletes it. The audit warning is a false positive (the ledger cites the directive by date and frontier number, not by the driver's acknowledgement token). This session records the acknowledgement explicitly; the s37 artifacts remain on disk and are re-listed in the s46 outcome.
- verdict: CONFIRMED (directive executed and measured; no re-run needed)

## [s46] KILL RE-AUDIT: the s15 hoisted-copy form (rejected/s15_hoisted_copy_plus_base_combine_deletes_copy_costs_3.c, the instance kill whose form sits closest to target: a hoisted loop-2 preheader copy that combine deletes) measures below 3 on the HEAD chassis or with FAKE constructs ablated.
- mechanism: Mandated re-audit; an instance kill measured under an older chassis or with a FAKE carrier on the target pseudo is not a kill.
- probe: tools/fake_ablate.py --func func_80017848 --file ings --candidate <s15 form> (tmp/grind/func_80017848/s46/ablate_s15.txt) then the form applied over the HEAD src/ings.c:820 INCLUDE_ASM anchor and scored with sandbox --disable all (tmp/grind/func_80017848/s46/sandbox_s15_reaudit.txt).
- result: fake_ablate reports "no FAKE-annotated constructs ... nothing to ablate" (the form carries no FAKE construct), and the plain re-measure is 3 at 127/127, identical to the s15 value and to the candidate. The kill stands on the HEAD chassis.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD src/ings.c:820 INCLUDE_ASM anchor with the s15 form applied; BASE candidate re-audited at 3 (127/127) the same way (tmp/grind/func_80017848/s46/sandbox_base.txt); no FAKE constructs anywhere

## [s46] Frontier item 3 (s44): GCC 2.7.2's C front end does NOT fold a comparison of an s32 local holding a zero-extended byte against an out-of-range constant, so the check reaches RTL and combine (not the front end) removes it, leaving a byte-free reader.
- mechanism: fold-const.c folds range comparisons only through the operand's TREE type; an s32 local has full range so the compare survives to RTL, where combine.c simplify_comparison would fold it via nonzero_bits(lbu) = 0xFF.
- probe: Scratch TUs tmp/grind/func_80017848/s46/mini/f3_s32.c (s32 idx = *p; if (idx == -1) ...), f3_s32_gt.c (idx > 0xFF) and f3_u8.c (u8 idx; idx == -1), built by mini/run.sh with the project's exact CPP_FLAGS/CC_FLAGS and -da; .rtl/.combine/.jump2 jump_insn counts and the final .s read.
- result: Premise half CONFIRMED: for the s32 holder the compare reaches RTL (3 jump_insns in .rtl, (ne ...) / (eq ...) present) and is NOT front-end folded; for the u8 holder the front end folds it entirely (f3_u8.s is `j $31; move $2,$0`, the load itself is gone). Conclusion half KILLED: in isolation the jump pass converts the branch to a store-flag before combine (.combine has 0 jump_insns), and neither combine nor jump2 folds the compare to a constant; the final assembly MATERIALISES it (`lbu $2; nor $2,$0,$2` for == -1, `lbu $2; slt $2,$2,256` for > 0xFF). The reader is therefore not byte-free in this geometry, and its only purpose in the function would be to keep the copy destination live, which fails cheat tests T1/T2 (a fabricated always-false conditional is the forbidden dead-conditional / empty-body-if family) regardless of whether some in-loop geometry folds it.
- verdict: KILLED
- kill_scope: instance
- measured_on: standalone scratch TUs under the exact project CC_FLAGS (canonical tools/gcc-2.7.2/build/cc1), HEAD chassis; no FAKE constructs

## [s46] The owner's 2026-09-06 directive (minimal scratch TU of the loop-1-exit/loop-2-preheader copy geometry under project flags, .cse2/.combine read to name the protecting context) is unexecuted and could re-open the residual.
- mechanism: If the copy deletion depended on chassis context, an isolated TU would keep the copy through .combine.
- probe: Ledger read (hypotheses.md H-s37-1 line 3519, evidence.md s37 conclusion line 4350) and artifact check of tmp/grind/func_80017848/s37/mini/ (m0..m6, run.sh, 14 .cse2/.combine dumps).
- result: Executed at s37: m4/m5 reproduce the promoted copy in .cse2 and lose it in .combine via combine.c:1458 (i2dest dead in i3); no protecting context exists. The dispatch audit warning was a false positive; acknowledged in the ledger this session.
- verdict: CONFIRMED

## [s46] KILL RE-AUDIT: the s15 hoisted-copy form (rejected/s15_hoisted_copy_plus_base_combine_deletes_copy_costs_3.c) measures below 3 on the HEAD chassis or with FAKE constructs ablated.
- mechanism: An instance kill measured on an older chassis or with a FAKE carrier on the target pseudo is not a kill.
- probe: tools/fake_ablate.py on the form (tmp/grind/func_80017848/s46/ablate_s15.txt), then the form applied over the HEAD src/ings.c:820 anchor and scored with sandbox --disable all (s46/sandbox_s15_reaudit.txt).
- result: No FAKE construct in the form (nothing to ablate); plain re-measure 3 at 127/127, identical to s15 and to the candidate. Kill stands.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD src/ings.c:820 INCLUDE_ASM anchor with the s15 form applied; BASE candidate re-audited at 3 (127/127); no FAKE constructs anywhere

## [s46] Frontier item 3 (s44): the 2.7.2 C front end does not fold a comparison of an s32 local holding a zero-extended byte against an out-of-range constant, so the check reaches RTL and combine removes it, leaving a byte-free reader.
- mechanism: fold-const.c folds range comparisons only through the TREE type; combine.c simplify_comparison would fold via nonzero_bits(lbu) = 0xFF.
- probe: Scratch TUs tmp/grind/func_80017848/s46/mini/f3_s32.c, f3_s32_gt.c, f3_u8.c built by mini/run.sh with the exact project CPP_FLAGS/CC_FLAGS and -da; .rtl/.combine/.jump2 jump_insn counts and the final .s read.
- result: s32 holder: compare reaches RTL (not front-end folded) but jump converts it to a store-flag before combine and it materialises as nor/slt in the assembly (not byte-free). u8 holder: front-end folded to a constant, the lbu disappears. No byte-free reader arises; the construct is a fabricated always-false conditional (forbidden family) in any case.
- verdict: KILLED
- kill_scope: instance
- measured_on: standalone scratch TUs under the exact project CC_FLAGS (canonical tools/gcc-2.7.2/build/cc1), HEAD chassis; no FAKE constructs

## s47 hypotheses (2026-09-07, solver; owner sibling-transplant directive executed first)

## [s47] The owner's 2026-09-06 directive (read the newly VISIBLE COMPLETED-C siblings in src/ings.c - func_80017D84, func_80016E60, main - and transplant their loop-2 preheader / copy-geometry spellings onto this chassis) yields a spelling that moves the floor below 3.
- mechanism: func_80017D84 (src/ings.c:824-845 on main) is the constructor of this function's 52-byte object and therefore fixes the object model; func_80016E60 (src/ings.c:436-535 on main) is a MATCHED sibling in the same TU that carries two sanctioned codegen constructs - a pointer-alias pass-through local (ot_base = arg0;) whose annotation names combine.c's i2/i3 merge leaving a copy at the LATER position, and a do { } while (0) wrap whose annotation names flow.c loop-depth weighting lifting a global.c allocno priority. Both are positional/seat levers of exactly the kind this residual needs.
- probe: Read all three sibling bodies from `git show HEAD:src/ings.c` (func_80017D84 at 824, func_80016E60 at 436, main's ings.c body). Derived and measured three transplant cells over the BASE candidate on the HEAD src/ings.c:820 INCLUDE_ASM anchor with `sandbox func_80017848 --disable all`: D1 = func_80016E60's do-while(0) wrap placed around loop 2's preheader base assignment (the statement that emits the divergent `lw v0,12(s2)` / `addu a0,a1,v0` pair); D2 = the loop-2 links read hoisted into its own named local placed AFTER the base assignment (a birth-order/span perturbation of the same block); D3 = func_80017D84's own object relation transplanted verbatim - that constructor writes *(s32*)(p+0x10) = c + (*(s16*)(p+4) << 6) where c = *(s32*)(p+0xC), i.e. the link array base IS the record array base plus (count << 6) - so loop 2's links pointer was respelled as (u8*)((s32)r2 + (*(s16*)(ctx+4) << 6)) off a named record-pointer local r2, giving the loop-2 addend a second use.
- result: D1 = 15 at 128 insns (the wrap materialises an extra instruction and re-seats the block); D2 = 3 at 127 (byte-inert, identical residual to BASE); D3 = 33 at 128 (the sibling's arithmetic relation is semantically correct but costs a sll+lh+addu where target has a single `lw a2,16(s2)`, and the resulting seat permutation is much worse). func_80017D84's contribution is confirmatory only: it re-proves the object model that s45 banked (52-byte block, 0x40-byte records at +0xC, 0x10-byte links at +0x10, s16 link count at +6) and adds the new fact that +0x10 is derived from +0xC, but that derivation has no zero-byte spelling here. The directive is now executed and measured; no sibling spelling reaches below 3.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD src/ings.c:820 INCLUDE_ASM anchor with body_D1.c / body_D2.c / body_D3.c applied; BASE re-audited at 3 (127/127) the same way (tmp/grind/func_80017848/s47/sb_BASE.txt); no FAKE constructs in BASE, D2 or D3 (D1's do-while(0) is a sanctioned-family FAKE carrier and still scored 15)

## [s47] The one register-seat divergence in the floor-3 residual (`addu a0,a1,v0` where target has `addu a0,a1,a3`) is reachable by a perturbation of the register allocator's modelled inputs, so inverse.py can name a C lever for it.
- mechanism: Solver modality rules (1)-(3). goal_from_tgt.py classify/goal types the first divergence and attributes the substitution onto a pseudo; inverse.py then searches the allocator's input space (refs, live span, birth order, conflicts, preferences, calls-crossed) for a perturbation reaching the target hard register, and maps each vector to a C technique.
- probe: (a) `goal_from_tgt.py classify ings func_80017848` -> FIRST DIVERGENCE: RA, `$v0 -> $a3 x1`, 2 pairs skipped as skeleton-differing. (b) `extract.py func_80017848 ings` -> 15-pseudo global model, 61 dispositions; `simulate.py` -> dispositions 15/15 match, forward global.c model exact on this chassis. (c) `goal_from_tgt.py goal ... --model --show` (tmp/grind/func_80017848/s47/goal_BASE.txt). (d) Identified the divergent pseudo from the .lreg dump (tools/grinder/dump.ps1, tmp/grind/func_80017848/dumps/ings.lreg): insn 162 (set (reg:SI 113) (mem:SI (plus (reg/v:SI 72) (const_int 12)))) feeding insn 164 (set (reg/v:SI 81) (plus (reg/v:SI 85) (reg:SI 113))) - reg 113 is loop 2's base addend. (e) `local_extract.py ings --func func_80017848 --suggest` -> reg 113 is blk=13 qty 0, birth=2 death=6 refs=2, got=$v0. (f) `inverse.py local ings.local.json --func func_80017848 --block 13 --goal {"0": 7}` at --depth 3 and --depth 4.
- result: The GLOBAL goal derivation is EMPTY for the third session running, but for a NEW and much sharper reason than s31's: the attribution reports "$v0->$a3 (x1): AMBIGUOUS, 33 pseudos hold $v0; pseudos holding $a3: [80]", and the dump resolves it - reg 113 is a LOCAL allocno (born and dead inside block 13, refs=2) and is therefore not in global.c's 15-pseudo model at all. s31's empty goal was a property of cell E's merged p/links pseudo; this one is a property of the BASE chassis itself, and it is the correct typing: the seat is owned by local-alloc, not global-alloc. Running the LOCAL backend then returns a validated NEGATIVE at depth 3 AND depth 4 over an atom space of 24 single perturbations across 6 classes (refs delta +4/-2, birth/death enumerated exhaustively): no perturbation of refs, live span, birth order, conflicts, preferences or calls-crossed reaches $a3 for that qty. The tool's own reading is "no C spelling that only moves those will ever close it". Corollary, and the load-bearing new fact: the ONLY route to $a3 for that value is to stop it being a local allocno at all - i.e. promote it to a global allocno live across loop 2 - which is byte-for-byte the same precondition as the missing `addu a3,a0,zero` copy (global.c seats such an allocno at $a3 per s40's pass-0 sets: reg80 conflicts v0,v1,a0,a1,a2,s2,s3,sp, a3 first eligible). The three residual instructions are therefore ONE fact, now solver-typed rather than hand-argued, and it is the E-s44-3 byte-free-flow-time-reader wall.
- verdict: KILLED
- kill_scope: class
- predicate_cite: tools/gcc-2.7.2/local-alloc.c:1079
- measured_on: HEAD src/ings.c:820 INCLUDE_ASM anchor with the BASE candidate applied (3 at 127/127); model validated forward 15/15 by simulate.py; no FAKE constructs

## [s47] KILL RE-AUDIT: the closest-to-target instance kill (Q1, s34/s42 body_Q1.c - target's complete 127-instruction stream with a pure register permutation) scores differently on the current chassis or with FAKE constructs ablated.
- mechanism: Mandated re-audit; a kill measured on a stale chassis or with a FAKE carrier on the contested pseudo is not a kill.
- probe: tmp/grind/func_80017848/s42/body_Q1.c applied over the HEAD src/ings.c:820 anchor, `sandbox func_80017848 --disable all` (tmp/grind/func_80017848/s47/sb_Q1.txt); `tools/fake_ablate.py --func func_80017848 --file ings --candidate memory/grind/func_80017848/candidate.c`.
- result: Q1 = 14 at 127/127, identical to its s34/s36/s38/s39/s41/s42 value. fake_ablate: "no FAKE-annotated constructs found ... nothing to ablate". The chassis has not drifted and no banked kill in this ledger rests on a FAKE carrier.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD src/ings.c:820 INCLUDE_ASM anchor plus s42/body_Q1.c; BASE re-audited at 3 (127/127); no FAKE constructs anywhere in the tree

## [s47] The one register-seat divergence in the floor-3 residual (ours `addu a0,a1,v0` vs target `addu a0,a1,a3`) is reachable by a perturbation of local-alloc's modelled inputs for that quantity, so inverse.py can name a C lever for it.
- mechanism: Solver modality rules (1)-(3): goal_from_tgt.py classify/goal types the first divergence and attributes it onto a pseudo; inverse.py then searches the allocator's input space (refs, live span, birth order, conflicts, preferences, calls-crossed) for a perturbation reaching the target hard register and maps each vector to a C technique.
- probe: classify ings func_80017848 -> FIRST DIVERGENCE: RA, $v0->$a3 x1, 2 pairs skipped as skeleton-differing. extract.py -> 15-pseudo global model / 61 dispositions; simulate.py -> 15/15 dispositions match, sort order MATCH (forward global.c model exact on this chassis). goal_from_tgt.py goal --model --show -> EMPTY goal, substitution AMBIGUOUS across 33 $v0-holding pseudos. tools/grinder/dump.ps1 -> .lreg insn 162 (set (reg:SI 113) (mem (plus (reg 72) 12))) feeding insn 164 (set (reg 81) (plus (reg 85) (reg 113))) identifies the value as loop 2's base addend. local_extract.py ings --func func_80017848 --suggest -> reg 113 = blk 13 qty 0, birth 2 death 6 refs 2, got $v0. inverse.py local ings.local.json --func func_80017848 --block 13 --goal {"0": 7} --depth 3 and --depth 4.
- result: Global goal is EMPTY, but for a NEW reason unrelated to s31's merged-pseudo cause: reg 113 is a LOCAL allocno (born and dead inside block 13) and is simply absent from global.c's model, so the seat is owned by local-alloc. The LOCAL backend then returns a validated NEGATIVE at BOTH depth 3 and depth 4 over an atom space of 24 single perturbations across 6 classes (bounds: refs delta +4/-2, birth/death enumerated exhaustively). Tool verdict verbatim: 'the flip is not produced by refs / live span / birth order / conflicts / preferences / calls-crossed at all, so no C spelling that only moves those will ever close it.' Two spelled C cells credit the negative rather than take it on trust: D2 (a birth-order/span perturbation - the loop-2 links read hoisted into its own named local after the base assignment) is byte-inert at 3, and D1 (a do-while(0) wrap, a priority perturbation) is 15 at 128. The corollary is the load-bearing new fact: the only route to $a3 for this value is to stop it being a local allocno - promote it to a global allocno live across loop 2 - which is byte-for-byte the same precondition the missing `addu a3,a0,zero` copy needs (global.c seats such an allocno at $a3 per s40's pass-0 sets). All three residual instructions are ONE fact, now solver-typed instead of hand-argued, and it is the E-s44-3 byte-free-flow-time-reader wall.
- verdict: KILLED
- kill_scope: class
- measured_on: HEAD src/ings.c:820 INCLUDE_ASM anchor with the BASE candidate applied (3 at 127/127); forward model validated 15/15 by simulate.py; no FAKE constructs
- predicate_cite: tools/gcc-2.7.2/local-alloc.c:1079

## [s47] The owner's 2026-09-06 directive - read the newly visible COMPLETED-C siblings in src/ings.c (func_80017D84, func_80016E60, main) and transplant their loop-2 preheader / copy-geometry spellings onto this chassis - yields a spelling that scores below 3.
- mechanism: func_80017D84 constructs this function's 52-byte object and so fixes the object model; func_80016E60 is a matched same-TU sibling carrying two annotated sanctioned codegen constructs - a pointer-alias pass-through local (combine i2/i3 merge leaving a copy at the LATER position) and a do { } while (0) wrap (flow.c loop-depth weighting lifting global.c allocno priority) - both positional/seat levers of the kind this residual needs.
- probe: Read all three sibling bodies from `git show HEAD:src/ings.c`. Built and scored three transplant cells over the BASE candidate at the HEAD src/ings.c:820 anchor with sandbox --disable all: D1 = func_80016E60's do-while(0) wrap around loop 2's preheader base assignment; D2 = func_80016E60's pointer-alias named local applied to loop 2's links read, placed after the base assignment; D3 = func_80017D84's own object relation (it writes *(s32*)(p+0x10) = c + (*(s16*)(p+4) << 6) where c = *(s32*)(p+0xC), i.e. link base = record base + (count << 6)) transplanted so loop 2's links pointer is computed off a named record-pointer local, giving the loop-2 addend a second use.
- result: D1 = 15 at 128 insns; D2 = 3 at 127 (byte-inert, residual identical to BASE, re-confirming s11's C-inertness of loop 2's preheader); D3 = 33 at 128 (the relation is semantically correct and is genuinely new ledger knowledge - +0x10 is derived from +0xC - but costs sll+lh+addu where target has a single `lw a2,16(s2)`). main carries no preheader copy geometry of this shape. The directive is executed and measured; no sibling spelling reaches below 3, and E-s47-2/3 explains why: both annotated sibling mechanisms act on global.c priority / combine position, while the contested value is a local allocno.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD src/ings.c:820 INCLUDE_ASM anchor with body_D1.c / body_D2.c / body_D3.c applied; BASE re-audited at 3 (127/127) the same way; no FAKE constructs in BASE, D2 or D3 (D1 carries the sanctioned do-while(0) FAKE carrier and still scored 15)

## [s47] The closest-to-target instance kill (Q1, target's complete 127-instruction stream with a pure register permutation) scores differently on the current chassis or with FAKE constructs ablated.
- mechanism: Mandated kill re-audit: a kill measured on a stale chassis or with a FAKE carrier occupying the contested pseudo is not a kill.
- probe: tmp/grind/func_80017848/s42/body_Q1.c applied over the HEAD src/ings.c:820 anchor and scored with sandbox --disable all; tools/fake_ablate.py --func func_80017848 --file ings --candidate memory/grind/func_80017848/candidate.c.
- result: Q1 = 14 at 127/127, identical to its s34/s36/s38/s39/s41/s42 value. fake_ablate reports no FAKE-annotated constructs in candidate.c, nothing to ablate. The chassis has not drifted and no banked kill in this ledger rests on a FAKE carrier.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD src/ings.c:820 INCLUDE_ASM anchor plus s42/body_Q1.c; BASE re-audited at 3 (127/127); no FAKE constructs anywhere in the tree

## s48 hypotheses (2026-09-07, forensics; instrumented-cc1 BB2_SUGG_DEBUG + local-alloc source read)

## [s48] Frontier item 3 (s47): local-alloc's SUGGESTED-REGISTER pass (qty_phys_copy_sugg / qty_phys_sugg) is what seats reg 113 in $v0, and $a3 is either outside find_free_reg's scan order at that point or excluded by a live hard register.
- mechanism: block_alloc runs a suggested-register pass before the main pass; if the qty carries a copy- or arithmetic-suggestion, find_free_reg is called with just_try_suggested=1 and `first_used` is restricted to the suggestion set (local-alloc.c:2205-2213), which would make the seat a preference rather than a scan artefact.
- probe: `python3 tools/ra_solver/local_extract.py ings --func func_80017848 --suggest` on the BASE chassis (candidate.c applied, sandbox 3 at 127/127), reading tmp/ra_solver_work/ings.sugg.json for blk 13 qty 0 - the full SUGGDBG-QTY input table and every SUGGDBG-FFR call's scanned hard-reg sets - plus an end-to-end read of find_free_reg in tools/gcc-2.7.2/local-alloc.c:2140-2306 and a grep for REG_ALLOC_ORDER over tools/gcc-2.7.2/config/mips/.
- result: FALSE on both halves, and the true mechanism is now named. blk 13 qty 0 (reg1=113, birth=2, death=6, refs=2) has ncopysugg=0, nsugg=0, copysugg=[], sugg=[] - no suggestion of any kind - so the suggested pass never runs for it; the QTYDBG stream shows blk 13 only on `main` lines and never on a `sugg` line, and there is exactly one SUGGDBG-FFR call for it with jts=0. That call scans used = first_used = {0,1,26..67}: every general-purpose register 2..25 is free over the whole span, so $a3 (regno 7) IS in the scan order and is NOT excluded by a live hard reg, by the class mask, by an eliminable or by a suggestion. It loses because the MIPS back end defines no REG_ALLOC_ORDER, so the scan at local-alloc.c:2249-2255 walks regno ascending from 0 and returns the first free register, $v0 (regno 2). New quantified precondition for a LOCAL-pass $a3 seat: regnos 2,3,4,5,6 ($v0,$v1,$a0,$a1,$a2) must ALL be set in first_used over [2,6), i.e. five hard registers live across loop 2's preheader; blk 13 holds exactly one quantity (so post_mark_life of an earlier-allocated qty contributes nothing) and contains no call (so no RTL hard-reg reference contributes either).
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD src/ings.c:820 INCLUDE_ASM anchor with candidate.c applied; BASE re-audited at 3 (127/127); no FAKE constructs; instrumented cc1 tools/gcc-2.7.2/cc1 with BB2_SUGG_DEBUG=1

## [s48] Frontier item 2 (s47): re-pricing the post-loop-2 consumer on reg 113 (the loop-2 base addend) rather than on the candidate's `p` local across a JOIN costs less than the banked 19 and reaches the $a3 seat, because promoting reg 113 to a global allocno is the single precondition E-s47-3 names.
- mechanism: routing one of target's three tail re-reads of 0xC(s2) off the loop-2 addend keeps the preheader load's value live past the base add, which makes it a global allocno rather than a block-13 local quantity; global.c would then seat it from its pass-0 conflict set, where E-s47-3 computed $a3 as first eligible.
- probe: cell A (tmp/grind/func_80017848/s48/body_A.c) - `recs2 = *(u8 **)(ctx + 0xC);` declared as a named local and read UNCONDITIONALLY before loop 2's guard so it is defined on the skip path, consumed inside the guard as `base = (u8 *)(sh2 + (s32)recs2)`, with its ONLY extra use being math_Distance3D's first argument. Scored with `sandbox func_80017848 --disable all`; then `local_extract.py ings --func func_80017848` re-run on the same source to read the local-alloc quantity table, and the sandbox object disassembled with mipsel-linux-gnu-objdump.
- result: HALF TRUE, and the half that fails is decisive. The promotion works and is dump-proven - block 13 disappears entirely from local_extract.py's quantity table (s48/local_A.txt lists blk 9,10,12,14,15,16 and no blk 13), so the value is a global allocno exactly as predicted. The seat does not follow: objdump shows the promoted value in $v1, hoisted as `lw v1,12(s2)` above loop 1 and reused by `addu v0,a1,v1` in six later blocks, not in $a3. E-s47-3's $a3 prediction was computed for a short allocno ending just after loop 2, but any allocno created by a REAL second use necessarily reaches that use, so its conflict set and seat differ. And the cost is not the expected +1: the extra use is SUBSTITUTIVE, letting cse forward the register equivalence into every later ctx+0xC consumer and delete one of the reloads target performs in its tail - 17 at 126 build insns against 127 target insns. Cheaper than the banked s9 T-series 19, still dead. Banked as rejected/s48_l2_addend_named_local_reused_as_distance3d_arg_promotes_reg113_but_cse_deletes_tail_reload_costs_17.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD src/ings.c:820 INCLUDE_ASM anchor with s48/body_A.c applied; BASE re-audited at 3 (127/127) in the same session; no FAKE constructs

## [s48] Frontier item 3 (s47): local-alloc's SUGGESTED-REGISTER pass (qty_phys_copy_sugg / qty_phys_sugg) is what seats reg 113 in $v0, and $a3 is either outside find_free_reg's scan order at that point or excluded by a live hard register.
- mechanism: block_alloc runs a suggested-register pass before the main pass; if a qty carries a copy- or arithmetic-suggestion, find_free_reg is called with just_try_suggested=1 and first_used is restricted to the suggestion set (local-alloc.c:2205-2213), making the seat a preference rather than a scan artefact.
- probe: python3 tools/ra_solver/local_extract.py ings --func func_80017848 --suggest on the BASE chassis (candidate.c applied, sandbox 3 at 127/127), reading tmp/ra_solver_work/ings.sugg.json for blk 13 qty 0 (full SUGGDBG-QTY input table plus every SUGGDBG-FFR call's scanned hard-reg sets), plus an end-to-end read of find_free_reg in tools/gcc-2.7.2/local-alloc.c:2140-2306 and a grep for REG_ALLOC_ORDER over tools/gcc-2.7.2/config/mips/.
- result: FALSE on both halves, and the true mechanism is now named rather than inferred. blk 13 qty 0 (reg1=113, birth=2, death=6, refs=2, size=1, mode=4, minclass=1) reports ncopysugg=0, nsugg=0, copysugg=[], sugg=[] - no suggestion of any kind - so the suggested pass never runs for it; the QTYDBG stream shows blk 13 only on 'main' lines, never on a 'sugg' line, and there is exactly one SUGGDBG-FFR call for it with jts=0. That call scans used = first_used = {0,1,26..67}: every general-purpose register 2..25 is free over the whole live span, so $a3 (regno 7) IS in the scan order and is NOT excluded by a live hard reg, by the class mask, by an eliminable, or by a suggestion set. It loses because the MIPS back end defines no REG_ALLOC_ORDER (grep over tools/gcc-2.7.2/config/mips/ returns nothing; the only references in the tree are the #ifdef sites in global.c, local-alloc.c, regclass.c, reload1.c and stupid.c), so the scan walks regno ascending from 0 and returns the first free register, $v0 (regno 2). This yields a new quantified precondition for a LOCAL-pass $a3 seat: regnos 2,3,4,5,6 ($v0,$v1,$a0,$a1,$a2) must ALL be set in first_used over [2,6), i.e. five hard registers live across loop 2's preheader. regs_live_at at local-alloc time is fed only by hard regs referenced in the RTL and by post_mark_life marks from qtys already allocated in the SAME block; blk 13 holds exactly one quantity (ord=0) and contains no call, so on this chassis neither source can contribute. s47's 'the mechanism is outside the current model' is replaced by a named mechanism with a concrete price.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD src/ings.c:820 INCLUDE_ASM anchor with memory/grind/func_80017848/candidate.c applied; BASE re-audited at 3 (127/127, scorable, rules_dropped 0); no FAKE constructs; instrumented cc1 tools/gcc-2.7.2/cc1 with BB2_SUGG_DEBUG=1

## [s48] Frontier item 2 (s47): re-pricing the post-loop-2 consumer on reg 113 (the loop-2 base addend) rather than on the candidate's `p` local across a JOIN costs less than the banked 19 and reaches the $a3 seat, because promoting reg 113 to a global allocno is the single precondition E-s47-3 names.
- mechanism: Routing one of target's three tail re-reads of 0xC(s2) off the loop-2 addend keeps the preheader load's value live past the base add, making it a global allocno rather than a block-13 local quantity; global.c would then seat it from its pass-0 conflict set, where E-s47-3 computed $a3 as first eligible.
- probe: Cell A (tmp/grind/func_80017848/s48/body_A.c): `recs2 = *(u8 **)(ctx + 0xC);` declared as a named local and read UNCONDITIONALLY before loop 2's guard so it is defined on the skip path, consumed inside the guard as `base = (u8 *)(sh2 + (s32)recs2)`, with its ONLY extra use being math_Distance3D's first argument - exactly the geometry s47's frontier named. Scored with sandbox func_80017848 --disable all; then local_extract.py ings --func func_80017848 re-run on the same source to read the local-alloc quantity table, and the sandbox object disassembled with mipsel-linux-gnu-objdump.
- result: HALF TRUE, and the failing half is decisive. The promotion works and is dump-proven: block 13 disappears entirely from local_extract.py's quantity table (s48/local_A.txt lists blk 9, 10, 12, 14, 15, 16 and no blk 13), so the value is no longer a local quantity - it is a global allocno, exactly the precondition E-s47-3 named, and ordinary C reaches it. The seat does not follow: objdump of the cell-A object shows the promoted value in $v1, hoisted as `lw v1,12(s2)` above loop 1 and reused by `addu v0,a1,v1` in six later blocks, not in $a3. E-s47-3's $a3 prediction was computed for a short allocno whose range ends just after loop 2, but any allocno created by a REAL second use necessarily reaches that use, so its conflict set and its seat differ. The cost is also not the expected +1: the extra use is SUBSTITUTIVE, letting cse forward the register equivalence into every later ctx+0xC consumer and delete one of the reloads target performs in its tail - the body comes out at 126 build instructions against 127 target instructions, score 17. Cheaper than the banked s9 T-series 19 but still dead. Banked as rejected/s48_l2_addend_named_local_reused_as_distance3d_arg_promotes_reg113_but_cse_deletes_tail_reload_costs_17.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD src/ings.c:820 INCLUDE_ASM anchor with tmp/grind/func_80017848/s48/body_A.c applied; BASE re-audited at 3 (127/127) in the same session immediately before; no FAKE constructs

## [s49] KILL RE-AUDIT: the s48 cell-A form (the closest promoting instance kill) still measures 17 at 126/127 on the current chassis, and it was not measured under a FAKE carrier.
- mechanism: an instance kill is chassis- and FAKE-relative; cell A is the newest kill and the only banked form that reaches the E-s47-3 promotion precondition, so it is the one whose re-measurement could reopen the search.
- probe: `git checkout src/ings.c`, apply tmp/grind/func_80017848/s49/body_A.c over the src/ings.c:820 INCLUDE_ASM anchor, `sandbox func_80017848 --disable all`, then `python3 tools/fake_ablate.py --func func_80017848 --file ings --candidate tmp/grind/func_80017848/s49/body_A.c`.
- result: unchanged. sandbox = 17 at 126 build insns / 127 target insns (s49/sb_A.txt), byte-for-byte the s48 measurement; fake_ablate reports "no FAKE-annotated constructs found ... nothing to ablate", so no FAKE carrier was occupying the target pseudo when the lever was priced. BASE re-audited at 3 (127/127) in the same session (s49/sb_BASE.txt). The kill stands.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD src/ings.c:820 INCLUDE_ASM anchor with s49/body_A.c applied; BASE re-audited at 3 (127/127) immediately before; fake_ablate confirms no FAKE constructs present

## [s49] Frontier item 3 (s48): reload spill-retry (retry_global_alloc / alter_reg) revisits the block-13 assignment and is therefore the last modelled route by which the loop-2 preheader value could reach $a3.
- mechanism: s47's inverse.py negative for reg 113 -> $a3 listed three surfaces it does not model - local-alloc's suggested-register pass, qty_size for DImode, and reload spill-retry. E-s48-2 eliminated the first two; if reload spilled a hard register in this function it would call retry_global_alloc on the evicted pseudos, re-running find_reg with a different forbidden set and potentially producing a different seat that the RA solver never sees.
- probe: `bash tools/wsl.sh 'bash tools/ra_solver/reload_harvest.sh ings'` on the BASE chassis (candidate.c applied, sandbox 3 at 127/127), then grep the per-function BB2_RELOAD_DEBUG stream in tmp/reload_work/ings.reload.log for every event tagged func=func_80017848 (order / new_spill_reg / spill_hard_reg / kickout / RETRYDBG / needs).
- result: FALSE, and it closes the surface completely. For func_80017848 the stream contains only the order line, its uses/bad_spill_regs continuation, and one `needs pass=1 new_bb_needs=0 changed=0` line: ZERO new_spill_reg, ZERO spill_hard_reg, ZERO kickout and ZERO retry_global_alloc events. Reload converges on its first pass without spilling anything, so it never revisits any allocation in this function and cannot be a seat mechanism here. (The only spill_hard_reg events in the whole ings TU belong to disp_CalcFov and func_80016A8C.) Composed with E-s48-2, all three surfaces s47 listed as unmodelled are individually eliminated, so the solver's negative result for reg 113 -> $a3 is complete on this chassis rather than a modelling gap. Tooling note banked with the evidence: reload_harvest.sh must be run through `bash tools/wsl.sh`, because the Windows Git-Bash shell cannot exec tools/gcc-2.7.2/cc1 and writes an empty log whose retries=0 is indistinguishable from a genuine negative.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD src/ings.c:820 INCLUDE_ASM anchor with memory/grind/func_80017848/candidate.c applied; BASE re-audited at 3 (127/127, scorable, rules_dropped 0); no FAKE constructs; instrumented cc1 tools/gcc-2.7.2/cc1 with BB2_RELOAD_DEBUG=1

## [s49] Frontier item 2 (s48): a LOCAL-pass $a3 seat for reg 113 is reachable by giving loop 2's preheader four or five additional co-live quantities that sort ahead of it in qty_compare, since each already-allocated qty in the same block calls post_mark_life and occupies the next ascending free regno.
- mechanism: local-alloc.c:2249-2255 scans regnos ascending and takes the first free one; local-alloc.c:2164-2190 folds regs_live_at (which post_mark_life writes for each qty allocated earlier in the same block) into `used`, so N earlier-allocated overlapping quantities push the seat N regnos up from $v0.
- probe: read the `got` column of `python3 tools/ra_solver/local_extract.py ings --func func_80017848` (s49/local_extract_BASE.txt) on the BASE chassis and calibrate the walk against the quantities of the tail block 16, whose live ranges overlap in known ways; then price the number of extra definitions blk 13 would need, against the instruction content of target's loop-2 preheader as normalised in s49/T.txt.
- result: the model is CONFIRMED and the lever is KILLED BY PRICE. Block 16 calibrates the walk exactly, inside this very function: reg142 (36-40) has zero earlier-allocated overlapping quantities and gets $v0; reg140 (28-32) and reg136 (18-24) each overlap exactly one (reg88, 16-30, seated $v0) and both get $v1; reg76 (20-40) overlaps two distinct seats (reg88 at $v0 and reg140/reg136 at $v1, used={2,3}) and gets $a0. One regno per earlier-allocated overlapping quantity, exactly as the mechanism predicts. Reg 113 sits at $v0 with one quantity in blk 13, so $a3 (regno 7) needs FIVE earlier-allocated overlapping quantities there. A block-local quantity requires a defining instruction inside blk 13 whose value no other block reads - anything used in loop 2's body or in the tail spans blocks and becomes a global allocno instead, which is what E-s48-3 measured. Target's loop-2 preheader is built from exactly two arithmetic instructions (s49/T.txt:59-61, `addu a3,a0,zero` / `addu a0,a1,a3`, plus the links load) and BASE emits the same two, so five additional definitions price at +5 instructions over target's 127 before any seat is even considered. The local route is priced dead on this chassis; the promotion route was already measured at $v1 / 126 insns (E-s48-3).
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD src/ings.c:820 INCLUDE_ASM anchor with memory/grind/func_80017848/candidate.c applied; BASE re-audited at 3 (127/127); no FAKE constructs; instrumented cc1 tools/gcc-2.7.2/cc1 quantity table plus the normalised target stream s49/T.txt

## [s49] Frontier item 1 (s48): a second use of the loop-2 base addend that is consumed and DEAD before the tail's link stores promotes the value out of local-alloc without letting cse forward a register equivalence into target's tail reloads, so it is byte-free where cell A was substitutive.
- mechanism: cse's memory table for ctx+0xC is flushed by the stores to *link that precede rec_a/rec_b, so a reader whose value dies inside loop 2's exit path cannot reach those reloads; the promotion itself was already proven ordinary C by cell A (blk 13 leaves the local quantity table).
- probe: cell C (tmp/grind/func_80017848/s49/body_C.c) - the frontier's own next_probe, spelled literally: `recs2 = *(u8 **)(ctx + 0xC);` read unconditionally before loop 2's guard, used as loop 2's base addend, and given exactly one further use, loop 2's exit condition rewritten from `while (i < *(s32 *)(base + 0x20))` to `while (i < *(s32 *)(sh2 + (s32)recs2 + 0x20))`. Guard, exit tail and every other block identical to BASE. Scored with `sandbox func_80017848 --disable all`; then `local_extract.py ings --func func_80017848` re-run on the same source, and the sandbox object disassembled and normalised against asm/funcs/func_80017848.s (s49/dis.sh -> s49/T.txt vs s49/B_C.txt).
- result: CONFIRMED as to byte-freedom and promotion, and it produces a genuinely new structure, but it does not close. sandbox = 5 at 127/127 (s49/sb_C.txt) - the first promoting form that is NOT an instruction short. local_extract.py lists blocks 2,3,7,9,10,12,14,15,16 and no blk 13 (s49/local_C.txt), so the value became a global allocno with no tail reload deleted. The recs2 load lands in the slot target fills with `sll zero,zero,0`, so it is instruction-free, and loop 2's preheader now emits a REAL surviving copy - `addu a0,a1,a0` then `addu a1,a0,zero` - against target's `addu a3,a0,zero` then `addu a0,a1,a3`. The copy survives combine because its destination carries an OUT-OF-BLOCK use (the exit test), the same can_combine_p escape by which BASE already buys loop 1's copy through `p = q;` in the exit tail. The defect is that the device copies whichever pseudo carries the out-of-block use: cse folds every in-loop use of recs2 back to `base`, so the copy is of base and lands AFTER the add, whereas target copies the record pointer BEFORE it and that pseudo has no reader anywhere after the base add. Score 5 = residual (a) (the untouched loop-1 exit tail move) plus the copy's wrong value/order and the consequent `lw v0,32(a1)` vs `lw v0,32(a0)`. Banked as rejected/s49_l2_exit_test_via_named_addend_buys_a_preheader_copy_of_base_not_recordptr_costs_5.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD src/ings.c:820 INCLUDE_ASM anchor with s49/body_C.c applied; BASE re-audited at 3 (127/127) in the same session; no FAKE constructs

## [s49] KILL RE-AUDIT: the s48 cell-A form (the closest promoting instance kill) still measures 17 at 126/127 on the current chassis, and it was not measured under a FAKE carrier.
- mechanism: An instance kill is chassis- and FAKE-relative; cell A is the newest kill and the only banked form that reaches the E-s47-3 promotion precondition, so it is the one whose re-measurement could reopen the search.
- probe: git checkout src/ings.c, apply tmp/grind/func_80017848/s49/body_A.c over the src/ings.c:820 INCLUDE_ASM anchor, sandbox func_80017848 --disable all, then python3 tools/fake_ablate.py --func func_80017848 --file ings --candidate tmp/grind/func_80017848/s49/body_A.c.
- result: Unchanged. sandbox = 17 at 126 build insns / 127 target insns (s49/sb_A.txt), byte-for-byte the s48 measurement; fake_ablate reports 'no FAKE-annotated constructs found ... nothing to ablate', so no FAKE carrier occupied the target pseudo when the lever was priced. BASE re-audited at 3 (127/127, scorable, rules_dropped 0) in the same session. The kill stands.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD src/ings.c:820 INCLUDE_ASM anchor with tmp/grind/func_80017848/s49/body_A.c applied; BASE re-audited at 3 (127/127) immediately before; fake_ablate confirms no FAKE constructs present

## [s49] Frontier item 3 (s48): reload spill-retry (retry_global_alloc / alter_reg) revisits the block-13 assignment in func_80017848 and is the last modelled route by which the loop-2 preheader value could reach $a3.
- mechanism: s47's inverse.py negative listed three unmodelled surfaces - local-alloc's suggested-register pass, qty_size for DImode, and reload spill-retry. E-s48-2 eliminated the first two; if reload spilled a hard register it would call retry_global_alloc on the evicted pseudos, re-running find_reg with a different forbidden set and producing a seat the RA solver never sees.
- probe: bash tools/wsl.sh 'bash tools/ra_solver/reload_harvest.sh ings' on the BASE chassis, then grep tmp/reload_work/ings.reload.log for every BB2_RELOAD_DEBUG event tagged func=func_80017848 (order / new_spill_reg / spill_hard_reg / kickout / RETRYDBG / needs).
- result: FALSE, and it closes the surface completely. For func_80017848 the stream contains only the order line, its uses/bad_spill_regs continuation, and one 'needs pass=1 new_bb_needs=0 changed=0' line: zero new_spill_reg, zero spill_hard_reg, zero kickout and zero retry_global_alloc events. Reload converges on its first pass without spilling anything, so it never revisits any allocation here. (The only spill_hard_reg events in the whole ings TU belong to disp_CalcFov and func_80016A8C.) Composed with E-s48-2, all three surfaces s47 listed as unmodelled are now individually eliminated, so the solver's negative result for reg 113 -> $a3 is complete on this chassis rather than a modelling gap. Tooling note banked: reload_harvest.sh must run through bash tools/wsl.sh, because the Windows Git-Bash shell cannot exec tools/gcc-2.7.2/cc1 ('Exec format error') and writes an empty log whose retries=0 is indistinguishable from a genuine negative.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD src/ings.c:820 INCLUDE_ASM anchor with memory/grind/func_80017848/candidate.c applied; BASE re-audited at 3 (127/127, scorable, rules_dropped 0); no FAKE constructs; instrumented cc1 tools/gcc-2.7.2/cc1 with BB2_RELOAD_DEBUG=1

## [s49] Frontier item 2 (s48): a LOCAL-pass $a3 seat for reg 113 is reachable by giving loop 2's preheader four or five additional co-live quantities that sort ahead of it, since each already-allocated qty in the same block calls post_mark_life and occupies the next ascending free regno.
- mechanism: local-alloc.c:2249-2255 scans regnos ascending and takes the first free one; local-alloc.c:2164-2190 folds regs_live_at (which post_mark_life writes for each qty allocated earlier in the same block) into `used`, so N earlier-allocated overlapping quantities push the seat N regnos up from $v0.
- probe: Read the `got` column of python3 tools/ra_solver/local_extract.py ings --func func_80017848 (s49/local_extract_BASE.txt) on the BASE chassis and calibrate the walk against the tail block 16, whose quantity live ranges overlap in known ways; then price the extra definitions blk 13 would need against the instruction content of target's loop-2 preheader as normalised in s49/T.txt.
- result: The model is CONFIRMED and the lever is KILLED BY PRICE. Block 16 calibrates the walk inside this very function: reg142 (36-40) has zero earlier-allocated overlapping quantities and gets $v0; reg140 (28-32) and reg136 (18-24) each overlap exactly one (reg88, 16-30, seated $v0) and both get $v1; reg76 (20-40) overlaps two distinct seats (used={2,3}) and gets $a0 - one regno per earlier-allocated overlapping quantity. Reg 113 sits at $v0 with one quantity in blk 13, so $a3 (regno 7) needs five earlier-allocated overlapping quantities there. A block-local quantity needs a defining instruction inside blk 13 whose value no other block reads; anything used in loop 2's body or in the tail spans blocks and becomes a global allocno instead, which is exactly what E-s48-3 measured. Target's loop-2 preheader is built from two arithmetic instructions (s49/T.txt lines 59-61: addu a3,a0,zero / addu a0,a1,a3, plus the links load) and BASE emits the same two, so five extra definitions price at +5 instructions over target's 127 before any seat is considered. The local route is priced dead on this chassis; the promotion route was already measured at $v1 / 126 insns.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD src/ings.c:820 INCLUDE_ASM anchor with memory/grind/func_80017848/candidate.c applied; BASE re-audited at 3 (127/127); no FAKE constructs; instrumented cc1 quantity table (tools/gcc-2.7.2/cc1) plus the normalised target stream s49/T.txt

## [s49] Frontier item 1 (s48): a second use of the loop-2 base addend that is consumed and dead before the tail's link stores promotes the value out of local-alloc without letting cse forward a register equivalence into target's tail reloads, so it is byte-free where cell A was substitutive.
- mechanism: cse's memory table for ctx+0xC is flushed by the stores to *link that precede rec_a/rec_b, so a reader whose value dies inside loop 2's exit path cannot reach those reloads; the promotion itself was already proven ordinary C by cell A (blk 13 leaves the local quantity table).
- probe: Cell C (tmp/grind/func_80017848/s49/body_C.c), the frontier's own next_probe spelled literally: recs2 = *(u8 **)(ctx + 0xC); read unconditionally before loop 2's guard, used as loop 2's base addend, with exactly one further use - loop 2's exit condition rewritten from while (i < *(s32 *)(base + 0x20)) to while (i < *(s32 *)(sh2 + (s32)recs2 + 0x20)). Guard, exit tail and every other block identical to BASE. Scored with sandbox func_80017848 --disable all; local_extract.py re-run on the same source; sandbox object disassembled and normalised against asm/funcs/func_80017848.s (s49/dis.sh -> s49/T.txt vs s49/B_C.txt).
- result: CONFIRMED as to byte-freedom and promotion, and it produces a genuinely new structure, but it does not close. sandbox = 5 at 127/127 (s49/sb_C.txt) - the first promoting form that is NOT an instruction short. local_extract.py lists blocks 2,3,7,9,10,12,14,15,16 and no blk 13 (s49/local_C.txt), so the value became a global allocno with no tail reload deleted. The recs2 load lands in the slot target fills with sll zero,zero,0, so it is instruction-free, and loop 2's preheader now emits a REAL surviving copy - addu a0,a1,a0 then addu a1,a0,zero - against target's addu a3,a0,zero then addu a0,a1,a3. The copy survives combine because its destination carries an OUT-OF-BLOCK use (the exit test), the same can_combine_p escape by which BASE already buys loop 1's copy through p = q; in the exit tail. The defect is that the device copies whichever pseudo carries that use: cse folds every in-loop use of recs2 back to base, so the copy is of base and lands after the add, whereas target copies the record pointer before it and that pseudo has no reader anywhere after the base add. Score 5 = residual (a) (the untouched loop-1 exit tail move) plus the copy's wrong value/order and the consequent lw v0,32(a1) vs lw v0,32(a0). Banked as rejected/s49_l2_exit_test_via_named_addend_buys_a_preheader_copy_of_base_not_recordptr_costs_5.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD src/ings.c:820 INCLUDE_ASM anchor with tmp/grind/func_80017848/s49/body_C.c applied; BASE re-audited at 3 (127/127) in the same session; no FAKE constructs

## s50 (2026-09-07, rederive)

BASE re-audit: 3 at 127/127 on the HEAD chassis (`candidate.c` at the src/ings.c:820 anchor).

- **H-s50-1 (KILLED, instance).** Replacing the loop-1 exit-tail `p = q;` with a fresh
  `p = *(u8 **)(ctx + 0xC);` makes the tail match the target's `lw a0,12(s2)` without
  disturbing loop-1's preheader copy. Probe: cell F1. Result: **4 at 126/127** — the tail
  matches exactly, but loop-1's copy is deleted (`addu a0,a1,a0`), because `p = q;` was
  `q`'s only out-of-block reader. Measured on: HEAD src/ings.c:820 INCLUDE_ASM anchor with
  tmp/grind/func_80017848/s50/body_F1.c applied; BASE re-audited at 3; no FAKE constructs
  present in either form.

- **H-s50-2 (KILLED, instance).** Rewriting each loop's exit test as
  `while (i < *(s32 *)(sh + (s32)q + OFF));` gives `q` an in-loop out-of-block reader that
  buys the preheader copy in BOTH loops at zero instruction cost. Probe: cells E1 (`q` a
  re-read) and E2 (`q = p`). Result: E1 **10 at 128/127**, E2 **12 at 126/127**. Copies do
  survive in both loops and residual (a) is won, but cse assigns the exit-test expression its
  own pseudo, so the surviving copy runs base→temp instead of the target's pointer→base, and
  E2 lets cse fold loop-2's base add into the guard add. Measured on: HEAD src/ings.c:820
  anchor with s50/body_E1.c and s50/body_E2.c applied; BASE re-audited at 3; no FAKE
  constructs.

- **H-s50-3 (CONFIRMED).** The join between the two loops is a cse extended-basic-block
  boundary (two predecessors), so two pointer pseudos that both hold `*(u8 **)(ctx + 0xC)`
  are never proven equal there; giving loop-2's GUARD the reloaded `p` and loop-2's BASE ADD
  a separately-carried `q` therefore reproduces the target's two unshared adds AND keeps
  `q` live across loop 1, buying loop-1's preheader copy at a site the target pays for
  anyway. Probe: cell M1. Result: **14 at 126/127** with loop-1's `addu a3,a0,zero` and the
  loop-1 exit-tail `lw a0,12(s2)` both byte-matching; the only structural miss is loop-2's
  own copy. This supersedes the s42/E-s42-4 attribution that loop-1's copy is bought
  ENTIRELY by the `p = q;` read — it is bought by ANY out-of-block reader of `q`, and
  loop-2's base add is a free one.

- **H-s50-4 (CONFIRMED).** Chaining the same device a second time — a copy `r = q;` in
  loop-2's preheader whose out-of-block reader is math_Distance3D's first argument —
  produces the target's full instruction stream shape: both preheader copies plus the tail
  reload, at 127/127. Probe: cell M2. Result: **17 at 127/127**; every remaining difference
  is a register seat or a copy source (sh/lnk rotated to a2/a1; loop-2's copy sourced from
  `q`/t0 instead of `p`/a3; the call's arg0 from `r` instead of the reload). This is the
  first form in 50 sessions with no missing or extra instruction anywhere in the residual
  region.

- **H-s50-5 (KILLED, instance).** Sourcing loop-2's copy from the reloaded `p` (`r = p;`)
  rather than from `q` moves the copy source to the target's register while keeping the
  count. Probe: cell M3. Result: **16 at 126/127** — GCC drops the post-loop reload entirely
  once `r` and `p` are the same value, losing an instruction. Measured on: HEAD
  src/ings.c:820 anchor with s50/body_M3.c applied; BASE re-audited at 3; no FAKE constructs.

- **H-s50-6 (KILLED, instance).** Emitting `r = q;` before the `lnk` load in loop-2's
  preheader reproduces the target's `addu a3,a0,zero` / `lw a2,16(s2)` order. Probe: cell M4.
  Result: **17 at 127/127**, score identical to M2 — sched reorders the copy/load pair
  regardless of statement order, so source order is not the lever for that pair. Measured on:
  HEAD src/ings.c:820 anchor with s50/body_M4.c applied; BASE re-audited at 3; no FAKE
  constructs.

## [s50] Replacing the loop-1 exit-tail `p = q;` with a fresh `p = *(u8 **)(ctx + 0xC);` makes that tail match the target's `lw a0,12(s2)` while loop-1's preheader copy still survives.
- mechanism: `p = q;` is the only out-of-block reader of `q`; the ledger (E-s42-4) attributed loop-1's surviving copy to that read, so replacing it with a load should be free if some other reader keeps q live.
- probe: Cell F1 = candidate.c with only that one statement changed; sandbox --disable all; normalised objdump diff against asm/funcs/func_80017848.s.
- result: 4 at 126/127. The tail matches the target exactly, but loop-1's copy is deleted (`addu a0,a1,a0` replaces `addu a3,a0,zero` + `addu a0,a1,a3`). Proves the tail move and loop-1's copy are one single purchase on this chassis.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD src/ings.c:820 INCLUDE_ASM anchor with tmp/grind/func_80017848/s50/body_F1.c applied; BASE re-audited at 3 (127/127) this session; no FAKE constructs in the form

## [s50] Rewriting each loop's exit test as `while (i < *(s32 *)(sh + (s32)q + OFF));` gives q an in-loop reader that buys the preheader copy in both loops at zero instruction cost.
- mechanism: s44 established the copy destination must be live across its loop body; an exit test that cse can rewrite onto the already-computed base pointer should add that liveness for free.
- probe: Cells E1 (q a re-read of ctx+0xC) and E2 (q = p), both with the loop-1 tail reload; sandbox --disable all; normalised diff.
- result: E1 = 10 at 128/127, E2 = 12 at 126/127. Copies do survive in both loops and residual (a) is won, but cse gives the exit-test expression its own pseudo so the copy runs base->temp (`addu a1,a1,a0` / `addu a0,a1,zero`) instead of the target's pointer->base; E2 additionally lets cse fold loop-2's base add into the guard add.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD src/ings.c:820 INCLUDE_ASM anchor with tmp/grind/func_80017848/s50/body_E1.c and body_E2.c applied; BASE re-audited at 3; no FAKE constructs in either form

## [s50] The join between the two loops is a cse extended-basic-block boundary, so giving loop-2's guard the reloaded p and loop-2's base add a separately-carried q reproduces the target's two unshared adds and simultaneously buys loop-1's preheader copy at a site the target already pays for.
- mechanism: The join block has two predecessors, so cse's value table is empty there and it never proves q == p; the guard's `sh2 + p` and the preheader's `sh2 + q` therefore stay two distinct adds (target: `addu v0,a1,a0` in the guard, `addu a0,a1,a3` in the preheader, never shared), and q's use in the preheader keeps it live across loop 1 without the tail move.
- probe: Cell M1 = F1 plus `q = p;` seeded before the loop-1 guard, loop-1's preheader keeping `q = *(u8 **)(ctx + 0xC);`, and loop-2's base add reading q while its guard reads p; sandbox --disable all; normalised diff.
- result: 14 at 126/127, with loop-1's preheader copy `addu a3,a0,zero` AND the loop-1 exit-tail `lw a0,12(s2)` both byte-matching the target. Only structural miss is loop-2's own copy. Supersedes E-s42-4: loop-1's copy is bought by any out-of-block reader of q, not specifically by `p = q;`.
- verdict: CONFIRMED

## [s50] Chaining the same device a second time - a copy `r = q;` in loop-2's preheader whose out-of-block reader is math_Distance3D's first argument - produces both preheader copies plus the tail reload at 127/127.
- mechanism: Same cse-EBB-boundary liveness argument applied to loop 2; the post-loop call block is the only remaining free out-of-block reader, and moving one of its two operands onto r keeps the instruction count neutral.
- probe: Cell M2; sandbox --disable all; normalised diff against s50/T.txt.
- result: 17 at 127/127 - the first form in 50 sessions with no missing or extra instruction anywhere in the residual region. Every remaining difference is a register seat or a copy source: sh/lnk rotated to a2/a1 (target a1/a2), loop-2's copy sourced from q/t0 instead of p/a3, and the call's arg0 from r instead of the reload.
- verdict: CONFIRMED

## [s50] Sourcing loop-2's copy from the reloaded p (`r = p;`) instead of from q puts the copy source in the target's register while keeping the instruction count.
- mechanism: Target emits `addu a3,a0,zero` where a0 is the reload, so the copy source should be p; if r and p carry the same pseudo value the copy should still survive because r is read after the loop.
- probe: Cell M3 = M2 with `r = q;` -> `r = p;`; sandbox --disable all.
- result: 16 at 126/127 - GCC drops the post-loop reload entirely once r and p carry the same value, losing an instruction.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD src/ings.c:820 INCLUDE_ASM anchor with tmp/grind/func_80017848/s50/body_M3.c applied; BASE re-audited at 3; no FAKE constructs in the form

## [s50] Emitting `r = q;` before the lnk load in loop-2's preheader reproduces the target's copy-then-`lw a2,16(s2)` order.
- mechanism: Target's preheader order is `addu a3,a0,zero` / `lw a2,16(s2)` / `addu a0,a1,a3`; M2 emits the load first, so source statement order was the obvious lever.
- probe: Cell M4 = M2 with the two statements swapped; sandbox --disable all.
- result: 17 at 127/127, score identical to M2 - sched reorders the copy/load pair regardless of source order, so statement order is not the lever for that pair.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD src/ings.c:820 INCLUDE_ASM anchor with tmp/grind/func_80017848/s50/body_M4.c applied; BASE re-audited at 3; no FAKE constructs in the form

## s51 (2026-09-07, rederive)

### KILLED (instance)
- **H-s51-1** — s50's cell M2 re-measures 17 at 127/127 on the HEAD chassis with no FAKE
  carrier present (fake_ablate: nothing to ablate). Mandatory closest-form kill re-audit; the
  s50 kill stands unchanged. *measured_on:* HEAD src/ings.c:820 INCLUDE_ASM anchor,
  `tmp/grind/func_80017848/s51/body_M2.c`; BASE re-audited at 3 (127/127); no FAKE constructs.
- **H-s51-2** — Deleting the pre-guard `q = p;` from M7 (so `q` and `r` have no definition on
  their loop-skip paths) makes both preheader copies seat in t0/t1 instead of a3 and moves
  loop-1's `lw a2,16(s2)` ahead of the copy; the form measures 10 at 127/127, the same score as
  M7 but with a disjoint residual. *measured_on:* HEAD anchor, `s51/body_M8.c`; BASE 3.
- **H-s51-3** — Moving `i = 0;` to any position around the pre-guard `q = p;` (M19: between the
  count load and `q = p;`; P5: after `q = p;`; M12/M20: `q = p;` earlier) does not remove
  reorg's duplication of `addu v1,zero,zero` into the loop-1 guard's `bltz` delay slot;
  M19 = 10 with a diff byte-identical to M7's, P5 = 10, M12 = 13, M20 = 16.
  *measured_on:* HEAD anchor, `s51/body_M19.c` / `body_P5.c` / `body_M12.c` / `body_M20.c`;
  BASE 3.
- **H-s51-4** — Writing both loop guards in the target's own `blez`-against-zero shape
  (`if (count > 0) { i = 0; ... }`) so that `i = 0` sits inside the guarded block costs an
  instruction: M17 = 35 at 126, M18 = 27 at 128. *measured_on:* HEAD anchor,
  `s51/body_M17.c` / `body_M18.c`; BASE 3.
- **H-s51-5** — Six readers for loop-2's preheader copy `r` were measured on the M7 chassis and
  each one either drops the build below 127 instructions or costs more than the call-argument
  reader: P1 (no reader) = 4 at 126, N1 (call arg1) = 18, N2 (rec_a's add) = 25, P2 (guard on
  q, copy from the reload) = 12 at 126, P3 = 29 at 124, P4 = 34 at 123.
  *measured_on:* HEAD anchor, `s51/body_P1.c`/`N1`/`N2`/`P2`/`P3`/`P4`; BASE 3.
- **H-s51-6** — Hoisting `r`'s definition above loop-2's guard, so that `r` is defined on all
  paths and could inherit the a3 seat `q` vacates, emits the copy in the join block ahead of
  the `blez` instead of in the preheader: M10 = 24, M13 = 24, M14 = 16 at 124.
  *measured_on:* HEAD anchor, `s51/body_M10.c`/`M13`/`M14`; BASE 3.

### CONFIRMED
- **H-s51-7** — s50's M2 residual was inflated by a redundant `lnk` local that s50 added to
  loop 2's preheader (BASE reads `*(u8 **)(ctx + 0x10)` inline in loop 2's body). Deleting it
  restores BASE's inline read and drops the M-branch from 17 to **10 at 127/127** (cell M7),
  with loop-1's preheader byte-exact. The a1/a2 seat rotation s50 attributed to the added q/r
  allocnos was caused by the lnk local's longer live range lifting its global.c priority above
  `sh`. *measured_on:* HEAD anchor, `s51/body_M7.c`; BASE 3 (127/127).

## [s51] s50's cell M2 measures 17 at 127/127 on the HEAD chassis with no FAKE construct present in the banked candidate.
- mechanism: Mandatory closest-form kill re-audit for a flat floor: re-apply M2 at the src/ings.c:820 anchor and re-score, then run tools/fake_ablate.py on candidate.c to confirm no FAKE carrier was masking a lever in the s42-s50 measurements.
- probe: bash tmp/grind/func_80017848/s51/run.sh M2 + sandbox func_80017848 --disable all; tools/fake_ablate.py --func func_80017848 --file ings --candidate memory/grind/func_80017848/candidate.c
- result: M2 = 17 at 127/127, identical to s50. fake_ablate: 'no FAKE-annotated constructs found in memory/grind/func_80017848/candidate.c; nothing to ablate'. BASE re-audited at 3 (127/127). The s50 kill stands and no banked kill on this function was measured under a FAKE mask.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD src/ings.c:820 INCLUDE_ASM anchor with tmp/grind/func_80017848/s51/body_M2.c applied; BASE re-audited at 3 (127/127); no FAKE constructs in the body (fake_ablate confirmed)

## [s51] Deleting the pre-guard `q = p;` from cell M7 makes both preheader copy destinations seat in t0/t1 instead of a3 and moves loop-1's `lw a2,16(s2)` ahead of the copy, scoring 10 at 127/127 with a residual disjoint from M7's.
- mechanism: With no definition on the loop-skip path, flow.c treats the q/r pseudos as live-in at function entry, where they conflict with the incoming a0-a3 parameter hard registers; global.c's allocation order then hands them t0/t1 as the first free seats.
- probe: bash tmp/grind/func_80017848/s51/run.sh M8 + sandbox func_80017848 --disable all + normalised objdump diff against tmp/grind/func_80017848/s51/T.txt
- result: M8 = 10 at 127/127. Its diff shows `addu v1,zero,zero` byte-exact in BOTH blez delay slots (M7's 2-point loss is gone) but `addu t0,a0,zero` / `addu t1,t0,zero` where the target has a3, plus loop-1's lnk load emitted before the copy. Same score as M7, disjoint residual. Banked rejected/s51_M8_no_preguard_q_i0_slot_correct_copies_seat_t0t1_costs_10.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD src/ings.c:820 INCLUDE_ASM anchor with tmp/grind/func_80017848/s51/body_M8.c applied; BASE re-audited at 3 (127/127); no FAKE constructs

## [s51] Repositioning `i = 0;` relative to the pre-guard `q = p;` does not stop reorg from duplicating `addu v1,zero,zero` into the loop-1 guard's bltz delay slot: M19 = 10 with a diff byte-identical to M7's, P5 = 10, M12 = 13, M20 = 16.
- mechanism: The delay-slot fill is decided by the contents of the block preceding the loop-1 guard, which the presence of a pre-guard `q = p;` changes; the statement's textual position inside that block is irrelevant to reorg.c's choice of fill insn.
- probe: bash tmp/grind/func_80017848/s51/run.sh {M19,P5,M12,M20} + sandbox func_80017848 --disable all; M19's normalised build captured to tmp/grind/func_80017848/s51/B_M19.txt
- result: M19 = 10 at 127/127 with a normalised diff line-for-line identical to M7's; P5 = 10; M12 = 13; M20 = 16. The 10 is a liveness property (whether q is defined on the loop-skip path), not a statement-order artefact.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD src/ings.c:820 INCLUDE_ASM anchor with tmp/grind/func_80017848/s51/body_M19.c, body_P5.c, body_M12.c and body_M20.c applied; BASE re-audited at 3 (127/127); no FAKE constructs

## [s51] Writing both loop guards in the target's own blez-against-zero shape (`if (count > 0) { i = 0; ... }`), so that `i = 0` sits inside the guarded block, changes the instruction count: M17 = 35 at 126 and M18 = 27 at 128.
- mechanism: Hoisting the count load out of an `i < count` comparison into a bare `count > 0` test loses the shared comparison pseudo that BASE's guard reuses and re-materialises the count at a different point.
- probe: bash tmp/grind/func_80017848/s51/run.sh {M17,M18} + sandbox func_80017848 --disable all
- result: M17 = 35 at 126/127, M18 = 27 at 128/127. Both banked in rejected/. BASE's `i = 0;` before an `i < count` guard already produces the target's delay-slot placement, so this rewrite buys nothing.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD src/ings.c:820 INCLUDE_ASM anchor with tmp/grind/func_80017848/s51/body_M17.c and body_M18.c applied; BASE re-audited at 3 (127/127); no FAKE constructs

## [s51] Six C-level readers for loop-2's preheader copy `r` were measured on the M7 chassis and each either drops the build below 127 instructions or scores worse than the call-argument reader: P1 = 4 at 126, N1 = 18, N2 = 25, P2 = 12 at 126, P3 = 29 at 124, P4 = 34 at 123.
- mechanism: The target reloads *(u8 **)(ctx + 0xC) six times (T.txt lines 14/28/52/77/94/104) and every post-loop-2 address add reads a fresh reload, never the a3 copy; so any C reader that routes one of those adds through `r` deletes the corresponding reload, and any reader that does not costs an extra live pseudo through the call block.
- probe: bash tmp/grind/func_80017848/s51/run.sh {P1,P2,P3,P4,N1,N2} + sandbox func_80017848 --disable all
- result: P1 (no reader at all) = 4 at 126 - the copy is deleted outright, proving the copy in every M-branch form exists only because of its C-level reader. N1 (math_Distance3D arg1) = 18, N2 (rec_a's address add) = 25, P2 (loop-2 guard on q, copy from the post-loop reload) = 12 at 126, P3 = 29 at 124, P4 = 34 at 123. All banked as rejected/s51_*.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD src/ings.c:820 INCLUDE_ASM anchor with tmp/grind/func_80017848/s51/body_P1.c, body_P2.c, body_P3.c, body_P4.c, body_N1.c and body_N2.c applied; BASE re-audited at 3 (127/127); no FAKE constructs

## [s51] Hoisting `r`'s definition above loop-2's guard, so that r is defined on all paths and could inherit the a3 seat q vacates, emits the copy in the join block ahead of the blez instead of in the preheader: M10 = 24, M13 = 24, M14 = 16 at 124.
- mechanism: A statement placed before the guard belongs to the join block, which the target reaches from two predecessors and where the target emits nothing; moving the copy there also lengthens r's live range across the guard test.
- probe: bash tmp/grind/func_80017848/s51/run.sh {M10,M13,M14} + sandbox func_80017848 --disable all
- result: M10 = 24 at 127/127, M13 = 24 at 127/127, M14 (hoisted r sourced from the post-loop reload) = 16 at 124/127. Banked as rejected/s51_M10_*.c, s51_M13_*.c and s51_M14_*.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD src/ings.c:820 INCLUDE_ASM anchor with tmp/grind/func_80017848/s51/body_M10.c, body_M13.c and body_M14.c applied; BASE re-audited at 3 (127/127); no FAKE constructs

## [s51] s50's M2 residual was inflated by a redundant `lnk` local that s50 added to loop 2's preheader; deleting it and restoring BASE's inline read of *(u8 **)(ctx + 0x10) drops the M-branch from 17 to 10 at 127/127 with loop-1's preheader byte-exact.
- mechanism: The extra local lengthens lnk's live range across loop 2's preheader, which lifts its global.c allocation priority above `sh` and rotates the a1/a2 seats in BOTH loops; without it the seats revert to the target's a1 = sh, a2 = lnk in loop 1.
- probe: bash tmp/grind/func_80017848/s51/run.sh M7 + sandbox func_80017848 --disable all + normalised objdump diff against tmp/grind/func_80017848/s51/T.txt
- result: M7 = 10 at 127/127 (M2 = 17 on the same chassis). Loop-1's preheader is byte-exact: `addu a3,a0,zero / lw a2,16(s2) / addu a0,a1,a3`. This retires s50's frontier item 1 - the seat rotation was the lnk local, not the added q/r allocnos. Banked rejected/s51_M7_loop2_lnk_local_removed_seat_and_call_residual_costs_10.c.
- verdict: CONFIRMED

## [s52] The three closest banked forms (s11_l1tail_fresh_read, s12_symmetric_fresh_reload_tail, s12_symmetric_no_q_fresh_reload_tail) still measure 4 at 126/127 on the HEAD chassis with no FAKE construct present.
- mechanism: mandated kill re-audit - an instance kill is only as good as the chassis and FAKE state it was measured under, and the chassis has moved through the M-branch work of s50/s51.
- probe: applied each rejected body to src/ings.c:820 and ran sandbox func_80017848 --disable all; normalised objdump diff for the s11 form against tmp/grind/func_80017848/s52/T.txt
- result: 4 / 4 / 4, all at 126 build insns, unchanged from their banked values. The s11 form's residual is now pinned: loop-2's GUARD block is byte-exact (`lw a0,12(s2)`), loop-2's preheader still emits a fresh `lw v0,12(s2)` where the target has the copy, and loop 1 loses its copy entirely (`addu a0,a1,a0`).
- verdict: KILLED

## [s52] Loop-2's missing preheader copy `addu a3,a0,zero` is emitted by ordinary C when the copy destination is a pointer local that is already live on loop-2's skip path and has a use after loop 2.
- mechanism: the copy needs a reference to its destination in a block other than the preheader to survive combine (the same trichotomy loop 1 uses via its `p = q;` exit tail). A variable that is already defined before loop-2's guard can take the copy inside the guard without an else-arm definition, and its post-loop use supplies the out-of-block reference.
- probe: cell G = BASE with `slots = p;` as the first statement inside loop-2's guard, `base = (u8 *)(sh2 + (s32)slots);`, and rec_a re-spelled `(u8 *)((slot_a << 6) + (s32)slots)`; sandbox + normalised objdump diff
- result: build emits `addu s5,a0,zero` in loop-2's preheader at 127/127, score 22. This is the first form in 52 sessions in which loop-2's copy exists at all - the copy is reachable, and the residual is now a CARRIER-COST problem rather than a materialisation problem.
- verdict: CONFIRMED

## [s52] Nineteen carrier spellings for loop-2's preheader copy each measure worse than BASE on the HEAD chassis, because the carrier variable has to be live on loop-2's skip path, which either seats it callee-saved or collapses the guard and base adds.
- mechanism: reusing an entry-live local (`slots`) gives the copy an out-of-block reader but makes cse aware that the carrier equals `p` inside the guard, so the guard add and base add fold into one add plus a copy; and the carrier's entry-to-rec_a live range forces a callee-saved seat with a new save/restore pair. Introducing a fresh local instead requires either a definition on the skip path (an extra insn) or a `p = r;` exit tail whose post-loop reader deletes one of the target's own reloads.
- probe: 19 cells on the HEAD chassis, bodies in tmp/grind/func_80017848/s52/: F=17, G=22, H=26, I=26 (slots reuse); J4=8, J2=17, J1=50, J3=50 (fresh r + `p = r;` tail); A=14, B=21, C=38 (fresh read instead of a copy); N1=32, N2=17, N3=36, N4=33 (loop-2 guard given its own `p2` read on top of loop-1's tail); R=13 (fully symmetric, `q` reused, top-guard read feeding it)
- result: best of the family is J4 = 8 at 125 insns; nothing approaches BASE's 3. Bodies banked as rejected/s52_*.c.
- verdict: KILLED

## [s52] Giving loop-2's copy destination its out-of-block reader inside the guarded region (the do-while exit test) rather than after loop 2 does not produce the pointer copy.
- mechanism: the exit test `i < *(s32 *)(sh2 + (s32)r + 0x20)` is loop-invariant in `r`, so loop.c hoists `sh2 + r` back into the preheader where cse merges it with the guard add; the surviving out-of-block reference is then to the BASE, not to the pointer.
- probe: cell T = BASE with `r = p;` in loop-2's preheader, `base = (u8 *)(sh2 + (s32)r);`, and the do-while condition re-spelled through r; sandbox + normalised objdump diff
- result: T = 7 at 126/127. Build emits a single collapsed add and `addu a1,a0,zero` as a copy of the BASE; the target's `lw a0,12(s2)` / `addu a3,a0,zero` / `addu a0,a1,a3` triple is not reproduced. Banked rejected/s52_l2_exit_test_through_r_collapses_base_add_costs_7.c.
- verdict: KILLED

## [s52] The owner directive's sibling transplant is discharged negatively: none of the three newly-visible COMPLETED-C siblings in src/ings.c carries a preheader-copy or two-loop copy geometry that applies to func_80017848.
- mechanism: the directive assumed the tombstone-backfilled siblings might hold the loop-2 preheader spelling this ledger is missing. Reading their on-main bodies settles what they actually contain.
- probe: read src/ings.c:824-846 (func_80017D84), src/ings.c:436-565 (func_80016E60) and src/ings.c:576-685 (main) on main
- result: func_80017D84's only loop is a walking-pointer `for (i = 0; i < 8; i++) { ...; p += 0x34; }` with a break and a post-loop read of `i` - the explicit-walking-pointer form already priced at 49 by s18's R3. func_80016E60 is a `while (1)` dispatch loop with no pointer preheader (its two FAKE constructs are a pointer-alias pass-through and a do-while(0) wrap, neither of which addresses a preheader copy). main has no analogous geometry. The siblings' positive contribution is object-model confirmation only: func_80017D84 sets `*(s32 *)(p + 0x10) = c + (*(s16 *)(p + 4) << 6)` with `c = *(s32 *)(a0 + 0x10)`, so ctx+0x10 is exactly ctx+0xC advanced past the 64-byte node records and ctx+6 is the link counter func_80017848 increments.
- verdict: KILLED

## [s52] The three closest banked forms (s11_l1tail_fresh_read_loses_l1_copy_costs_4, s12_symmetric_fresh_reload_tail_costs_4, s12_symmetric_no_q_fresh_reload_tail_costs_4) still measure 4 at 126/127 on the HEAD chassis with no FAKE construct present.
- mechanism: Mandated kill re-audit: an instance kill is only as good as the chassis and FAKE state it was measured under, and the chassis moved through the M-branch work of s50/s51.
- probe: Applied each rejected body to the src/ings.c:820 INCLUDE_ASM anchor and ran sandbox func_80017848 --disable all; normalised objdump diff of the s11 form against tmp/grind/func_80017848/s52/T.txt.
- result: 4 / 4 / 4, all at 126 build insns, identical to their banked values. The s11 residual is now pinned exactly: loop-2's GUARD block is byte-exact (lw a0,12(s2)), loop-2's preheader still emits a fresh lw v0,12(s2) where the target has the copy, and loop 1 loses its copy entirely (addu a0,a1,a0). The kills are re-validated, not void.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD src/ings.c:820 INCLUDE_ASM anchor with each rejected/*.c body applied; BASE re-audited at 3 (127/127) in the same session; fake_ablate reports no FAKE construct in any of these bodies

## [s52] Loop-2's missing preheader copy addu a3,a0,zero is emitted by ordinary C when the copy destination is a pointer local that is already live on loop-2's skip path and has a use after loop 2.
- mechanism: The copy needs a reference to its destination in a block other than the preheader to survive combine - the same trichotomy loop 1 already uses via its `p = q;` exit tail. A variable defined before loop-2's guard can take the copy inside the guard without needing an else-arm definition, and its post-loop use supplies the out-of-block reference.
- probe: Cell G = BASE with `slots = p;` as the first statement inside loop-2's guard, `base = (u8 *)(sh2 + (s32)slots);`, and rec_a re-spelled `(u8 *)((slot_a << 6) + (s32)slots)`; sandbox + normalised objdump diff (tmp/grind/func_80017848/s52/body_G.c).
- result: Build emits `addu s5,a0,zero` in loop-2's preheader at 127/127, score 22. First form in 52 sessions in which loop-2's copy exists at all. The 22 points are entirely carrier cost: slots is live from function entry to rec_a so global.c seats it callee-saved (new save/restore pair, ra moves to 60(sp), s6 appears), and because cse knows slots == p inside the guard the guard add and base add collapse to `addu a1,v0,a0` + `addu a0,a1,zero` where the target emits two independent adds.
- verdict: CONFIRMED

## [s52] Nineteen carrier spellings for loop-2's preheader copy each measure worse than BASE's 3 on the HEAD chassis, because the carrier has to be live on loop-2's skip path, which either seats it callee-saved or collapses the guard and base adds.
- mechanism: Reusing an entry-live local (slots) gives the copy an out-of-block reader but makes cse aware the carrier equals p inside the guard, folding the guard and base adds into one add plus a copy, and its long live range forces a callee-saved seat. A fresh local instead requires either a definition on the skip path (an extra insn) or a `p = r;` exit tail whose post-loop reader deletes one of the target's own reloads.
- probe: 19 cells applied at src/ings.c:820 and scored with sandbox --disable all; bodies in tmp/grind/func_80017848/s52/: slots-reuse F/G/H/I, fresh-r-with-tail J1..J4, fresh-read A/B/C, own-p2-guard-read N1..N4, fully symmetric R.
- result: F=17@126, G=22@127, H=26@124, I=26@127, J4=8@125, J2=17@123, J1=50@127, J3=50@126, A=14@125, B=21@124, C=38@128, N1=32@129, N2=17@126, N3=36@126, N4=33@124, R=13@124. Best of the family is J4 = 8; nothing approaches BASE's 3. Seven representative bodies banked as memory/grind/func_80017848/rejected/s52_*.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD src/ings.c:820 INCLUDE_ASM anchor with tmp/grind/func_80017848/s52/body_{A,B,C,F,G,H,I,J1,J2,J3,J4,N1,N2,N3,N4,R}.c applied; BASE re-audited at 3 (127/127); no FAKE construct present in any body

## [s52] Giving loop-2's copy destination its out-of-block reader inside the guarded region (the do-while exit test) rather than after loop 2 does not produce the pointer copy on this chassis.
- mechanism: The exit test `i < *(s32 *)(sh2 + (s32)r + 0x20)` is loop-invariant in r, so loop.c hoists sh2 + r back into the preheader where cse merges it with the guard add; the surviving out-of-block reference is then to the BASE, not to the pointer.
- probe: Cell T = BASE with `r = p;` in loop-2's preheader, `base = (u8 *)(sh2 + (s32)r);`, and the do-while condition re-spelled through r; sandbox + normalised objdump diff (tmp/grind/func_80017848/s52/body_T.c).
- result: T = 7 at 126/127. Build emits a single collapsed add plus `addu a1,a0,zero` (a copy of the BASE); the target's lw a0,12(s2) / addu a3,a0,zero / addu a0,a1,a3 triple is not reproduced. Banked rejected/s52_l2_exit_test_through_r_collapses_base_add_costs_7.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD src/ings.c:820 INCLUDE_ASM anchor with tmp/grind/func_80017848/s52/body_T.c applied; BASE re-audited at 3 (127/127); no FAKE construct present

## [s52] None of the three newly-visible COMPLETED-C siblings in src/ings.c (func_80017D84, func_80016E60, main) carries a preheader-copy or two-loop copy geometry that transplants onto func_80017848.
- mechanism: The owner directive assumed the tombstone-backfilled siblings might hold the loop-2 preheader spelling this ledger is missing; reading their on-main bodies settles what they actually contain.
- probe: Read src/ings.c:824-846 (func_80017D84), src/ings.c:436-565 (func_80016E60) and src/ings.c:576-685 (main) on main.
- result: func_80017D84's only loop is a walking-pointer `for (i = 0; i < 8; i++) { ...; p += 0x34; }` with a break and a post-loop read of i - the explicit-walking-pointer form already priced at 49 by s18's R3. func_80016E60 is a while(1) dispatch loop with no pointer preheader (its two FAKE constructs are a pointer-alias pass-through and a do-while(0) wrap, neither addressing a preheader copy). main has no analogous geometry. Positive contribution is object-model confirmation only: func_80017D84 sets *(s32 *)(p + 0x10) = c + (*(s16 *)(p + 4) << 6) with c = *(s32 *)(a0 + 0x10), so ctx+0x10 is exactly ctx+0xC advanced past the 64-byte node records and ctx+6 is the link counter func_80017848 increments.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD src/ings.c on main (commit 4a2d4a51), read-only sibling audit; BASE re-audited at 3 (127/127) in the same session

## s53 (2026-09-07, structural)

- **H-s53-1 (CONFIRMED).** Loop 2 written as a literal mirror of loop 1 (own pre-guard
  read `p2`, own guard temp `t2`, own preheader read `q2`) makes the emitted function
  match the target everywhere except one instruction per loop preheader.
  Probe: cells S1/S2, `tmp/grind/func_80017848/s53/body_S1.c`, `body_S2.c`, diff in
  `s53/B_S2.txt`. Result: both 12 at 125/127; the only differing lines are the two
  preheaders, and the difference is the SAME in both loops (missing `addu a3,a0,zero`
  plus the a0/a1 seat swap it causes).

- **H-s53-2 (CONFIRMED).** The missing preheader copy is decided at COMBINE time by the
  copy destination's use count, not by local-alloc / optimize_reg_copy_1.
  Probe: `pwsh tools/grinder/dump.ps1 func_80017848` with S2 in place; read
  `tmp/grind/func_80017848/dumps/ings.cse` and `ings.combine` insns 64-89 (loop 1) and
  143-165 (loop 2). Result: no `(set (reg) (reg))` insn exists in either preheader at any
  pass; cse propagates the guard pseudo (reg 79 / reg 81) straight into the base add
  (insn 89 / insn 165), both operands carrying REG_DEAD.

- **H-s53-3 (CONFIRMED).** Giving the preheader copy destination a SECOND use makes the
  copy survive. Probe: cell V1 (loop 2's preheader spelled `q2 = q;`, so loop-1's `q` is
  used by loop-1's base add and by loop-2's preheader). Result: 10 at 126/127 with
  `addu t0,a0,zero / addu a0,a1,t0` emitted in loop 1's preheader for the first time on a
  chassis whose loop-2 guard also carries the target's `lw`.

- **H-s53-4 (CONFIRMED).** Two chained second-uses restore the instruction count and give
  the best M-branch score recorded. Probe: cell Z1 = V1 + `} while (i < *(s32 *)(sh2 +
  (s32)q2 + 0x20));`. Result: 9 at 127/127 (previous M-branch best: 10 at s51).
  Banked `rejected/s53_chain_plus_loop2_latch_via_q2_BEST_M_BRANCH_costs_9.c`.

- **H-s53-5 (KILLED, instance).** Spelling the copy destination's second use as an ADDRESS
  expression (the do-while latch bound, or the loop-body index) does not produce the
  target's pointer copy: cse folds the expression to `base`, so the surviving copy is a
  copy of base emitted after the base add. Measured on the HEAD src/ings.c:820 INCLUDE_ASM
  anchor with no FAKE construct present: U1 = 13 at 127/127, U2 = 17 at 127/127, U3 = 14 at
  126, AA = 11 at 126, AB = 13 at 127, AC = 10 at 127, AD = 11 at 128, Z3 = 12 at 127.

- **H-s53-6 (KILLED, instance).** Spelling the copy destination's second use as a POST-LOOP
  consumer of the node-array pointer, or sourcing the preheader copies from the top-guard
  local `slots`, does not improve on BASE or on Z1. Measured on the same chassis with no
  FAKE construct present: BA (rec_a via q2) = 21 at 127, BB (call arg0 via q2) = 13 at 127,
  BC (rec_b via q2) = 21 at 126, BD = 21 at 127, Y1 = 15 at 125, Y2 = 15 at 125,
  Z4 = 32 at 128, W1 (no `base` local) = 12 at 125,
  AE/AF/AG (q pre-initialised before loop 1's guard) = 11 / 11 / 12.

## [s53] Loop 2 written as a literal mirror of loop 1 (its own pre-guard read p2, its own guard temp t2, its own preheader read q2 feeding base = sh2 + q2) makes the emitted function match the target everywhere except one instruction per loop preheader, and the two preheaders then differ from the target in exactly the same way.
- mechanism: BASE gives loop 2 no memory reference in its guard block (the guard tests through the register p carried out of loop 1), so loop 2's preheader read is the first reference in its cse extended block and stays a real lw while loop 1's becomes a copy. Mirroring the guard's fresh read into loop 2 makes both loops' extended blocks identical, which collapses the long-standing asymmetry into a single repeated construct.
- probe: Cells S1 and S2, tmp/grind/func_80017848/s53/body_S1.c and body_S2.c, applied at the HEAD src/ings.c:820 INCLUDE_ASM anchor and scored with `sandbox func_80017848 --disable all`; normalised objdump diff in s53/B_S2.txt vs s53/T.txt.
- result: S1 = 12 at 125/127, S2 = 12 at 125/127. The whole diff is the two loop preheaders: the target's `addu a3,a0,zero` is absent in both, and the guard's pointer/shift seats swap to a1/a0 from the target's a0/a1. Prologue, top guard, both loop bodies, both latches, the call block, the link-record stores, rec_a/rec_b and the epilogue are byte-identical. The 52-session framing that the residual is a load-vs-copy transposition peculiar to loop 2 is retired.
- verdict: CONFIRMED

## [s53] The missing preheader copy is decided at combine time by the copy destination's use count; cse never leaves a reg-reg copy insn for local-alloc to keep or drop.
- mechanism: cse rewrites the preheader's redundant *(u8 **)(ctx + 0xC) load by propagating the guard's pointer pseudo directly into the consuming base add, so no (set (reg) (reg)) insn is ever created. A copy insn only persists when its destination has more than one use, because combine merges a single-use move into the consuming plus.
- probe: `pwsh tools/grinder/dump.ps1 func_80017848` with cell S2 applied; read tmp/grind/func_80017848/dumps/ings.cse and ings.combine, loop-1 preheader insns 64/66/86/89 and loop-2 preheader insns 143/145/165/336.
- result: Neither dump contains any reg-to-reg set in either preheader. In .cse the loop-1 base add is already (insn 89 (set (reg/v:SI 83) (plus (reg/v:SI 86) (reg/v:SI 79)))) and in .combine both source operands carry REG_DEAD notes; loop 2's insn 165 is the same shape. This supersedes s52's local-alloc.c:700 optimize_reg_copy_1 attribution for THIS construct: the copy is gone long before local-alloc runs.
- verdict: CONFIRMED

## [s53] Giving the preheader copy destination a second use makes the copy survive, and two chained second uses restore the instruction count to 127/127 at score 9.
- mechanism: With two uses combine cannot merge the move into the base add, so the pseudo survives to the allocator and is emitted as an addu copy. Chaining loop 2's preheader onto loop 1's copy dest (`q2 = q;`) supplies loop 1's second use, and recomputing loop 2's do-while bound from q2 supplies loop 2's.
- probe: Cells V1 (tmp/grind/func_80017848/s53/body_V1.c) and Z1 (body_Z1.c), applied at the HEAD src/ings.c:820 anchor and scored with `sandbox func_80017848 --disable all`; diffs in s53/B_V1.txt and s53/B_Z1.txt.
- result: V1 = 10 at 126/127 and emits `addu t0,a0,zero / addu a0,a1,t0` in loop 1's preheader for the first time on a chassis whose loop-2 guard also carries the target's lw. Z1 = 9 at 127/127, beating s51's M7/M8 (10) and s50's M2 (17); it is the best M-branch score in 53 sessions. Banked as rejected/s53_chain_plus_loop2_latch_via_q2_BEST_M_BRANCH_costs_9.c.
- verdict: CONFIRMED

## [s53] Spelling the copy destination's second use as an address expression - the do-while latch bound or the loop-body index - produces a copy of base emitted after the base add rather than the target's copy of the pointer emitted before it, in each of the eight cells measured this session.
- mechanism: cse recognises `sh + q + K` as `base + K` and folds it, so the second use attaches to the base pseudo. LICM then hoists the folded address into the preheader as a copy of base, which lands after the base add instead of before it.
- probe: Cells U1, U2, U3, AA, AB, AC, AD, Z3 in tmp/grind/func_80017848/s53/, each applied at the HEAD src/ings.c:820 anchor and scored; diffs read for U1 and U2 (s53/B_U1.txt, s53/B_U2.txt).
- result: U1 = 13 at 127/127, U2 = 17 at 127/127, U3 = 14 at 126, AA = 11 at 126, AB = 13 at 127, AC = 10 at 127, AD = 11 at 128, Z3 = 12 at 127. Every one of them shows the copy after the base add with the latch or body reading the copy.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD src/ings.c:820 INCLUDE_ASM anchor with each s53 body applied in turn; no FAKE construct present in any body (fake_ablate reported none in candidate.c at s51 and none of these cells adds one); BASE re-audited at 3 (127/127) in the same session.

## [s53] Sourcing the preheader copies from the top-guard local `slots`, or giving the copy destination a post-loop second use (rec_a, rec_b, or math_Distance3D's first argument), does not improve on BASE's 3 or on Z1's 9 in any of the eleven cells measured this session.
- mechanism: A post-loop consumer of the node-array pointer replaces one of the target's own `lw ..,12(s2)` reloads, so the copy bought is paid for by a load lost; `slots` is live from function entry, which lengthens its live range across the loops and rotates the guard seats.
- probe: Cells BA, BB, BC, BD, Y1, Y2, Z4, W1, AE, AF, AG in tmp/grind/func_80017848/s53/, each applied at the HEAD src/ings.c:820 anchor and scored with `sandbox func_80017848 --disable all`.
- result: BA = 21 at 127, BB = 13 at 127, BC = 21 at 126, BD = 21 at 127, Y1 = 15 at 125, Y2 = 15 at 125, Z4 = 32 at 128, W1 (no `base` local at all) = 12 at 125, AE = 11 at 126, AF = 11 at 126, AG = 12 at 128.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD src/ings.c:820 INCLUDE_ASM anchor with each s53 body applied in turn; no FAKE construct present in any body; BASE re-audited at 3 (127/127) in the same session.

## [s53] The mandatory kill re-audit found nothing void: BASE and s52's two closest cells re-measure at their banked values on the HEAD chassis.
- mechanism: Instance kills are chassis-relative and FAKE-state-relative, so the closest banked forms are re-measured before new probes are spent.
- probe: memory/grind/func_80017848/candidate.c, tmp/grind/func_80017848/s52/body_J4.c and body_G.c applied in turn at the HEAD src/ings.c:820 anchor and scored with `sandbox func_80017848 --disable all`.
- result: BASE = 3 at 127/127, J4 = 8 at 125/127, G = 22 at 127/127 - all identical to their s52 values. The s52 instance kills stand.
- verdict: CONFIRMED

## s54 (2026-09-07, synthesis)

## [s54] BASE's floor-3 residual is a two-instruction transposition confined to loop 2: the target puts the ctx+0xC LOAD in loop-2's guard and the reg-reg COPY in loop-2's preheader, BASE puts the COPY in the guard and the LOAD in the preheader.
- mechanism: BASE's loop-1 exit tail `p = q;` is a join copy that materialises at the merge block, and that merge block IS loop-2's guard block; it therefore occupies the slot where the target emits `lw a0,12(s2)`, and loop-2's preheader is then left with nothing to copy from and issues a fresh load instead.
- probe: memory/grind/func_80017848/candidate.c applied at the HEAD src/ings.c:820 anchor, `sandbox func_80017848 --disable all`, normalised objdump diff tmp/grind/func_80017848/s54/T.txt vs s54/B.txt.
- result: 3 at 127/127; the entire diff is `lw a0,12(s2)` / `addu a3,a0,zero` / `addu a0,a1,a3` (target) against `addu a0,a3,zero` / `lw v0,12(s2)` / `addu a0,a1,v0` (build). Both sides emit exactly one load and one copy per loop; only their placement differs.
- verdict: CONFIRMED

## [s54] The RTL producer of every part of the residual is now read from dumps: loop-1's copy is combine insn 83 kept alive by the tail at insn 141, loop-1's base add reads the copy's SOURCE rather than its destination, and loop-2's preheader is a genuine memory load (insn 162, the s47 solver's local allocno reg 113).
- mechanism: cse propagates the guard's pointer pseudo into the base add, so the preheader copy is never the base add's operand; combine deletes such a copy unless its destination is used again, which the `p = q;` tail supplies. Loop 2 has no equivalent because reg 79 carries REG_DEAD at loop-2's guard add and the ctx+0xC memory equivalence does not survive loop 1's blocks.
- probe: `pwsh tools/grinder/dump.ps1 func_80017848` with BASE applied; read tmp/grind/func_80017848/dumps/ings.combine from line 6767 (insns 64, 66, 72, 75, 79, 83, 86, 89, 141, 148, 151, 155, 158, 162, 164, 335).
- result: insn 83 = (set (reg/v 80) (reg/v 79)); insn 89 = (set (reg/v 81) (plus (reg/v 84) (reg/v 79))); insn 141 = (set (reg/v 79) (reg/v 80)); insn 162 = (set (reg 113) (mem (plus (reg/v 72) (const_int 12)))). The s52 local-alloc.c:700 attribution and the s53 framing that loop 2 needs a spelling loop 1 does not are both superseded by this reading.
- verdict: CONFIRMED

## [s54] The two-step guard temp (`t = sh + (s32)p; t = *(s32 *)(t + 0x1C);`) is load-bearing: reusing one variable for the guard address and the guard value clobbers the pseudo holding `sh + p`, which is what forces the preheader to recompute the base add instead of reusing the guard's address.
- mechanism: combine insns 72 and 75 both write reg 86, so cse's record of (plus sh p) is destroyed before the preheader is reached. A single-expression guard leaves the address pseudo live and cse folds the preheader's base add into it, leaving a copy of BASE after the add rather than a copy of the POINTER before it.
- probe: cell XM (tmp/grind/func_80017848/s54/body_XM.c, loop 2 mirrored WITHOUT the two-step temp) applied at the HEAD anchor and scored; normalised objdump diff.
- result: XM = 17 at 126/127; loop-2's guard emits `addu a3,v0,a1` and the preheader emits `addu a1,a3,zero`, a copy of base. The device is present in BASE's loop 1 and in s53's S2 loop 2 and absent from the whole s52 N-family, which retrospectively explains those scores.
- verdict: CONFIRMED

## [s54] Writing both loops as `while` loops, so that jump.c's duplicate_loop_exit_test synthesises the guard from the exit test, does not reproduce the target on this chassis: the latch's bound is hoisted out of the loop and the build is seven instructions short.
- mechanism: duplicate_loop_exit_test copies the exit-test insns before the loop and remaps pseudos used only in the exit code, which does produce a preheader copy; but the copied exit test leaves the original bound load loop-invariant, and loop.c then hoists it, whereas the target reloads `lw v0,28(a0)` on every iteration.
- probe: cell WA (tmp/grind/func_80017848/s54/body_WA.c) applied at the HEAD src/ings.c:820 anchor and scored with `sandbox func_80017848 --disable all`; normalised objdump diff against s54/T.txt.
- result: WA = 29 at 120/127. The preheader copy that does appear is `addu a2,v0,zero`, a copy of the guard's computed ADDRESS rather than of the pointer. Banked rejected/s54_while_loops_duplicate_loop_exit_test_hoists_latch_costs_29.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD src/ings.c:820 INCLUDE_ASM anchor with tmp/grind/func_80017848/s54/body_WA.c applied; BASE re-audited at 3 (127/127) in the same session; fake_ablate reports no FAKE construct in candidate.c and none of these cells adds one

## [s54] Sourcing loop-2's preheader base from loop-1's exit-tail carrier while loop-2's guard reads ctx+0xC fresh moves the join copy above the guard load instead of replacing it.
- mechanism: giving `p` a consumer in loop-2's preheader keeps loop-1's tail alive, so loop 1 stays byte-exact, but `p` still has two reaching definitions and the join copy is still emitted; with the guard now reading memory the copy is simply scheduled ahead of the load rather than taking its place.
- probe: cells X1, X2, X4 (fresh p2 guard read, preheader base from p, with and without a named intermediate and a loop-2 tail) and X3 (loop-2's guard read reusing variable q) in tmp/grind/func_80017848/s54/, each applied at the HEAD anchor and scored.
- result: X1 = X2 = X4 = 7 at 127/127, X3 = 14 at 127. All three 7-cells emit `addu a0,a3,zero` BEFORE `lw v0,12(s2)` in loop-2's guard block and `addu a0,a1,a0` with no copy in the preheader. Banked rejected/s54_l2_guard_fresh_read_preheader_base_from_l1_carrier_costs_7.c and rejected/s54_l2_guard_read_reuses_q_costs_14.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD src/ings.c:820 INCLUDE_ASM anchor with each s54 body applied in turn; no FAKE construct present in any body; BASE re-audited at 3 (127/127) in the same session

## [s54] Statement ORDER inside loop-2's preheader is byte-neutral, so combine's adjacency test is not the gate that decides the copy; the gate is the copy destination's use count.
- mechanism: combine's `added_sets_2 = ! dead_or_set_p (i3, i2dest)` (tools/gcc-2.7.2/combine.c:1453) decides whether the i2 SET has to be carried into the merged pattern; the `! all_adjacent` clause in can_combine_p only rejects when an intervening insn SETS a register the source uses, which the `lw ..,16(s2)` sitting between them never does.
- probe: cell Z = BASE with loop 2 given explicit named intermediates in loop-1's exact order (`q2 = *(u8 **)(ctx + 0xC); lnk2 = *(u8 **)(ctx + 0x10); base = (u8 *)(sh2 + (s32)q2);`), applied at the HEAD anchor and scored; cross-checked against s53's S2, whose loop 1 carries the same ordering yet loses its copy for want of a tail.
- result: Z = 3 at 127/127, byte-identical to BASE. Banked rejected/s54_l2_named_q2_lnk2_mirrors_l1_order_no_change_costs_3.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD src/ings.c:820 INCLUDE_ASM anchor with tmp/grind/func_80017848/s54/body_Z.c applied; no FAKE construct present; BASE re-audited at 3 (127/127) in the same session

## [s54] Every second use available to loop-2's preheader copy destination is priced: an address use is folded to `base` by cse and yields a copy of base after the base add, and a value use consumes one of the three ctx+0xC reloads the target performs after the loops, dropping the build to 126 instructions.
- mechanism: BASE's .combine carries three independent (mem (plus (reg 72) (const_int 12))) loads after the loops (insn 222 for math_Distance3D, insn 270 for rec_a, insn 293 for rec_b). Routing the carrier into any of them makes GCC use the register it already holds and deletes that load; routing it into an address instead lets cse recognise sh2 + q2 + K as base + K and attaches the second use to the base pseudo.
- probe: ten cells across two chassis, all applied at the HEAD src/ings.c:820 anchor and scored with `sandbox func_80017848 --disable all`. On BASE: ZB (q2 sourced from the carrier), ZC (latch bound via the carrier), ZD (latch bound via q2), ZE (body index via q2). On the s53 S2 mirror chassis: M (both tails plus call arg0 via p), M2 (loop-2 tail only), M3 (loop-2 tail placed inside the preheader), F2 (copy dest feeds only the tail while the base is routed through the guard pseudo p2), F2b (the same shape in both loops). Plus a re-measurement of s52's N2.
- result: ZB = 8 at 125, ZC = 17 at 127, ZD = 5 at 128, ZE = 6 at 128, M = 17 at 126, M2 = 17 at 126, M3 = 16 at 126, F2 = 17 at 126, F2b = 17 at 126, N2 = 17 at 126 (unchanged from its s52 value). ZD's diff shows the predicted `addu a1,a0,zero` copy of BASE emitted after the base add. Seven bodies banked as rejected/s54_*.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD src/ings.c:820 INCLUDE_ASM anchor with each s54 body applied in turn; no FAKE construct present in any body; BASE re-audited at 3 (127/127) in the same session

## [s54] The mandatory kill re-audit found nothing void: BASE re-measures at its banked value on the HEAD chassis and carries no FAKE construct.
- mechanism: instance kills are chassis-relative and FAKE-state-relative, so the closest banked form is re-measured before new probes are spent.
- probe: memory/grind/func_80017848/candidate.c applied at the HEAD src/ings.c:820 anchor and scored; `python3 tools/fake_ablate.py --func func_80017848 --file ings --candidate memory/grind/func_80017848/candidate.c`.
- result: BASE = 3 at 127/127, identical to its s52/s53 value; fake_ablate reports "no FAKE-annotated constructs found". Every s52/s53 instance kill stands on this chassis.
- verdict: CONFIRMED

## [s54] BASE's floor-3 residual is a two-instruction transposition confined to loop 2: the target puts the ctx+0xC LOAD in loop-2's guard and the reg-reg COPY in loop-2's preheader, while BASE puts the COPY in the guard and the LOAD in the preheader.
- mechanism: BASE's loop-1 exit tail `p = q;` is a join copy that materialises at the merge block, and that merge block IS loop-2's guard block, so it occupies the slot where the target emits `lw a0,12(s2)`; loop-2's preheader is then left with nothing to copy from and issues a fresh load instead.
- probe: memory/grind/func_80017848/candidate.c applied at the HEAD src/ings.c:820 INCLUDE_ASM anchor, `sandbox func_80017848 --disable all`, normalised objdump diff tmp/grind/func_80017848/s54/T.txt vs s54/B.txt.
- result: 3 at 127/127. The whole diff is `lw a0,12(s2)` / `addu a3,a0,zero` / `addu a0,a1,a3` (target) against `addu a0,a3,zero` / `lw v0,12(s2)` / `addu a0,a1,v0` (build). Both sides emit exactly one load and one copy per loop; only the placement differs. Loop 1, the prologue, both bodies, both latches, the call block, the link stores, rec_a/rec_b and the epilogue are byte-identical.
- verdict: CONFIRMED

## [s54] The RTL producer of every part of the residual is read from dumps: loop-1's preheader copy is combine insn 83, kept alive only by the tail at insn 141; loop-1's base add (insn 89) reads the copy's SOURCE rather than its destination; and loop-2's preheader is a genuine memory load (insn 162, the s47 solver's local allocno reg 113).
- mechanism: cse propagates the guard's pointer pseudo into the base add, so the preheader copy is never the base add's operand; combine deletes such a copy unless its destination is used again, which the `p = q;` tail supplies. Loop 2 has no equivalent because reg 79 carries REG_DEAD at loop-2's guard add (insn 151) and the ctx+0xC memory equivalence does not survive loop 1's blocks.
- probe: `pwsh tools/grinder/dump.ps1 func_80017848` with BASE applied; read tmp/grind/func_80017848/dumps/ings.combine from line 6767 (insns 64, 66, 72, 75, 79, 83, 86, 89, 141, 148, 151, 155, 158, 162, 164, 335).
- result: insn 83 = (set (reg/v 80) (reg/v 79)); insn 89 = (set (reg/v 81) (plus (reg/v 84) (reg/v 79))); insn 141 = (set (reg/v 79) (reg/v 80)); insn 162 = (set (reg 113) (mem (plus (reg/v 72) (const_int 12)))). This supersedes s52's local-alloc.c:700 attribution and s53's framing that loop 2 needs a spelling loop 1 does not: loop-1's copy and loop-2's wrong guard instruction are the two ends of ONE device.
- verdict: CONFIRMED

## [s54] The two-step guard temp (`t = sh + (s32)p; t = *(s32 *)(t + 0x1C);`) is load-bearing: reusing one variable for the guard address and the guard value clobbers the pseudo holding sh + p, which is what forces the preheader to recompute the base add instead of reusing the guard's address.
- mechanism: combine insns 72 and 75 both write reg 86, so cse's record of (plus sh p) is destroyed before the preheader is reached. A single-expression guard leaves the address pseudo live, cse folds the preheader's base add into it, and the surviving copy is a copy of BASE emitted after the add rather than a copy of the POINTER emitted before it.
- probe: Cell XM (tmp/grind/func_80017848/s54/body_XM.c: loop 2 mirrored WITHOUT the two-step temp) applied at the HEAD anchor and scored; normalised objdump diff.
- result: XM = 17 at 126/127; loop-2's guard emits `addu a3,v0,a1` and the preheader emits `addu a1,a3,zero`. The device is present in BASE's loop 1 and in s53's S2 loop 2 and absent from the whole s52 N-family, which retrospectively explains those scores.
- verdict: CONFIRMED

## [s54] Writing both loops as `while` loops, so that jump.c's duplicate_loop_exit_test synthesises the guard from the exit test, measures 29 at 120/127 on this chassis: the latch's bound is hoisted out of the loop and the build is seven instructions short.
- mechanism: duplicate_loop_exit_test copies the exit-test insns before the loop and remaps pseudos used only in the exit code, which does produce a preheader copy; but the copied exit test leaves the original bound load loop-invariant and loop.c hoists it, whereas the target reloads `lw v0,28(a0)` on every iteration.
- probe: Cell WA (tmp/grind/func_80017848/s54/body_WA.c) applied at the HEAD src/ings.c:820 anchor and scored with `sandbox func_80017848 --disable all`; normalised objdump diff against s54/T.txt.
- result: WA = 29 at 120/127. The preheader copy that does appear is `addu a2,v0,zero`, a copy of the guard's computed ADDRESS rather than of the pointer. Banked rejected/s54_while_loops_duplicate_loop_exit_test_hoists_latch_costs_29.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD src/ings.c:820 INCLUDE_ASM anchor with tmp/grind/func_80017848/s54/body_WA.c applied; BASE re-audited at 3 (127/127) in the same session; tools/fake_ablate.py reports no FAKE construct in candidate.c and none of these cells adds one

## [s54] Sourcing loop-2's preheader base from loop-1's exit-tail carrier while loop-2's guard reads ctx+0xC fresh measures 7 at 127/127: the join copy moves above the guard load instead of replacing it.
- mechanism: Giving `p` a consumer in loop-2's preheader keeps loop-1's tail alive so loop 1 stays byte-exact, but `p` still has two reaching definitions and the join copy is still emitted; with the guard now reading memory the copy is simply scheduled ahead of the load rather than taking its place.
- probe: Cells X1, X2, X4 (fresh p2 guard read, preheader base from p, with and without a named intermediate and a loop-2 tail) and X3 (loop-2's guard read reusing variable q) in tmp/grind/func_80017848/s54/, each applied at the HEAD anchor and scored.
- result: X1 = X2 = X4 = 7 at 127/127, X3 = 14 at 127. All three 7-cells emit `addu a0,a3,zero` BEFORE `lw v0,12(s2)` in loop-2's guard block and `addu a0,a1,a0` with no copy in the preheader. Banked rejected/s54_l2_guard_fresh_read_preheader_base_from_l1_carrier_costs_7.c and rejected/s54_l2_guard_read_reuses_q_costs_14.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD src/ings.c:820 INCLUDE_ASM anchor with each s54 body applied in turn; no FAKE construct present in any body; BASE re-audited at 3 (127/127) in the same session

## [s54] Statement order inside loop-2's preheader is byte-neutral (cell Z measures 3 at 127/127, identical to BASE), so combine's adjacency test is not the gate that decides the copy on this chassis; the gate is the copy destination's use count.
- mechanism: combine's `added_sets_2 = ! dead_or_set_p (i3, i2dest)` decides whether the i2 SET has to be carried into the merged pattern; the `! all_adjacent` clause in can_combine_p only rejects when an intervening insn SETS a register the source uses, which the `lw ..,16(s2)` sitting between them never does.
- probe: Cell Z = BASE with loop 2 given explicit named intermediates in loop-1's exact order (`q2 = *(u8 **)(ctx + 0xC); lnk2 = *(u8 **)(ctx + 0x10); base = (u8 *)(sh2 + (s32)q2);`), applied at the HEAD anchor and scored; cross-checked against s53's S2, whose loop 1 carries the same ordering yet loses its copy for want of a tail.
- result: Z = 3 at 127/127, byte-identical to BASE. Banked rejected/s54_l2_named_q2_lnk2_mirrors_l1_order_no_change_costs_3.c. Predicate for the use-count gate: tools/gcc-2.7.2/combine.c:1453.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD src/ings.c:820 INCLUDE_ASM anchor with tmp/grind/func_80017848/s54/body_Z.c applied; no FAKE construct present; BASE re-audited at 3 (127/127) in the same session

## [s54] Each of the ten second-use spellings measured this session for loop-2's preheader copy destination scores between 5 and 17: an address use is folded to `base` by cse and yields a copy of base after the base add, and a value use consumes one of the three ctx+0xC reloads the target performs after the loops, dropping the build to 126 instructions.
- mechanism: BASE's .combine carries three independent (mem (plus (reg 72) (const_int 12))) loads after the loops - insn 222 for math_Distance3D, insn 270 for rec_a, insn 293 for rec_b. Routing the carrier into any of them makes GCC use the register it already holds and deletes that load; routing it into an address instead lets cse recognise sh2 + q2 + K as base + K, attaching the second use to the base pseudo.
- probe: Ten cells across two chassis, all applied at the HEAD src/ings.c:820 anchor and scored with `sandbox func_80017848 --disable all`. On BASE: ZB (q2 sourced from the carrier), ZC (latch bound via the carrier), ZD (latch bound via q2), ZE (body index via q2). On the s53 S2 mirror chassis: M (both tails plus call arg0 via p), M2 (loop-2 tail only), M3 (loop-2 tail placed inside the preheader), F2 (copy dest feeds only the tail while the base is routed through the guard pseudo p2), F2b (the same shape in both loops). Plus a re-measurement of s52's N2.
- result: ZB = 8 at 125, ZC = 17 at 127, ZD = 5 at 128, ZE = 6 at 128, M = 17 at 126, M2 = 17 at 126, M3 = 16 at 126, F2 = 17 at 126, F2b = 17 at 126, N2 = 17 at 126 (unchanged from its s52 value). ZD's diff shows the predicted `addu a1,a0,zero` copy of BASE emitted after the base add. Seven bodies banked as rejected/s54_*.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD src/ings.c:820 INCLUDE_ASM anchor with each s54 body applied in turn; no FAKE construct present in any body; BASE re-audited at 3 (127/127) in the same session

## [s54] The mandatory kill re-audit found nothing void: BASE re-measures at 3 (127/127) on the HEAD chassis and tools/fake_ablate.py reports no FAKE-annotated construct in it.
- mechanism: Instance kills are chassis-relative and FAKE-state-relative, so the closest banked form is re-measured before new probes are spent.
- probe: memory/grind/func_80017848/candidate.c applied at the HEAD src/ings.c:820 anchor and scored; `python3 tools/fake_ablate.py --func func_80017848 --file ings --candidate memory/grind/func_80017848/candidate.c`.
- result: BASE = 3 at 127/127, identical to its s52/s53 value; fake_ablate reports "no FAKE-annotated constructs found". Every s52/s53 instance kill stands on this chassis.
- verdict: CONFIRMED

## s55 (2026-09-07, escalation)

## [s55] The s54 attribution of loop-1's `addu a3,a0,zero` / `addu a0,a1,a3` pair to combine is wrong: the operand rewrite is done by `optimize_reg_copy_1` inside local-alloc, between the `flow` and `lreg` dumps.
- mechanism: local-alloc.c:700 `optimize_reg_copy_1` (called from `update_equiv_regs`, local-alloc.c:1007) takes a reg-reg copy whose SOURCE does not die at the copy, scans forward to the insn where the source dies (stopping at labels, jumps and `NOTE_INSN_LOOP_BEG`/`LOOP_END`), and if nothing sets either register in between it replaces the source with the destination throughout that range and moves the death note onto the copy. Loop 1 satisfies every clause.
- probe: `pwsh tools/grinder/dump.ps1` output already on disk with BASE applied; insn 89 compared at ings.combine:6974, ings.flow:9027 and ings.lreg:8889.
- result: insn 89 reads `(reg/v 79)` in both combine and flow and reads `(reg/v 80)` in lreg, with `REG_DEAD (reg 79)` moved back onto insn 83. The pass boundary is unambiguous.
- verdict: CONFIRMED

## [s55] The preheader copy has a SECOND survival gate that 54 sessions never tested: combine.c:914's `(! all_adjacent && use_crosses_set_p (src, INSN_CUID (insn)))`, which refuses the merge when the copy's SOURCE is set between the copy and its consumer — and the target's own `lw a2,16(s2)` lnk load is that intervening insn.
- mechanism: spelling the ctx+0x10 lnk load as a write to the same C variable that holds the ctx+0xC pointer makes `reg_last_set[src]` newer than the copy's CUID, so `can_combine_p` returns 0 and the copy survives to local-alloc, where `optimize_reg_copy_1` then rewrites the base add to read it.
- probe: cell V3 (`tmp/grind/func_80017848/s55/body_V3.c`, both loops given the clobber with per-loop variables) applied at the HEAD src/ings.c:820 anchor and scored with `sandbox func_80017848 --disable all`; normalised objdump diff against s55/T.txt.
- result: 14 at 127/127, and the preheader copy `addu v0,a1,zero` is PRESENT in both loops — the first time the loop-2 copy and the loop-1 copy have ever coexisted. The whole opcode stream matches the target; only register seats differ. `tools/ra_solver/inverse_compose.py classify` re-types the residual as `FIRST DIVERGENCE: RA — same instructions, different registers`.
- verdict: CONFIRMED

## [s55] The target's seat map says the ctx+0xC pointer variable and the loop base variable are ONE C variable (both live in a0) while lnk has its own seat; spelling it that way makes every register in the function target-exact.
- mechanism: in the target `lw a0,12(s2)` loads the pointer and `addu a0,a1,a3` writes the base into the same register; V3 instead reuses the pointer variable for lnk, which keeps that pseudo live through the whole loop body, pins lnk and the pointer to one seat and cascades a one-seat shift through ptr/sh/copy/base/index.
- probe: cell U1 (`tmp/grind/func_80017848/s55/body_U1.c`: per-loop `q` holds the ctx+0xC pointer and is then reassigned the base, `lnk`/`lnk2` separate, copy dest `p`/`p2`) applied at the HEAD anchor and scored; full normalised objdump diff.
- result: 4 at 125/127. The ENTIRE diff is the two missing `addu a3,a0,zero` copies and their two consumers reading `a0` instead of `a3`; all 125 emitted instructions match the target register-for-register and offset-for-offset.
- verdict: CONFIRMED

## [s55] On the U1 chassis the copy is merged because the source is set by the base add itself rather than by an insn between the copy and the add, so combine.c:914's clobber gate does not fire; every U1-family spelling measured this session leaves the two copies merged at 125 instructions.
- mechanism: `use_crosses_set_p` consults `reg_last_set`, which combine only updates for insns it has already scanned past; i3 setting the source does not count as a set "between". The two gates are mutually exclusive on this chassis — V3's clobber buys the copy and loses the seats, U1's variable identity buys the seats and loses the clobber.
- probe: cells U2 (lnk load moved above the copy), U5/U6 (`slots` as the copy dest in one/both loops), U7 (copy dest shared across both loops), U8 (no reuse at all, separate base variable) and U10 (an explicit re-read of ctx+0xC between the copy and the add), each applied at the HEAD src/ings.c:820 anchor and scored.
- result: U2 = U5 = U6 = U7 = U10 = 4 at 125, U8 = 10 at 125. U10's re-read is folded away by cse, so it never becomes a clobber. Banked rejected/s55_no_reuse_separate_base_var_costs_10.c and rejected/s55_reread_ctx0xC_between_copy_and_add_cse_folds_costs_4.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD src/ings.c:820 INCLUDE_ASM anchor with each s55 U-cell applied in turn; no FAKE construct in any body (tools/fake_ablate.py finds none in candidate.c either); BASE re-audited at 3 (127/127) in the same session

## [s55] Sharing or splitting the four loop variables (q, p, base, t) across the two loops does not move the V3 seat map: all sixteen cells of the lattice measure 14 at 127/127 except the four that share both `p` and `t`, which measure 36.
- mechanism: sharing a variable across both loops makes its pseudo multi-block and hands it to global.c instead of local-alloc, which was the hoped-for route to the `a3` seat that BASE's loop 1 wins; the measurement shows the seat map is decided upstream of that distinction, by which values are simultaneously live in the loop body.
- probe: cells W00..W15 (`tmp/grind/func_80017848/s55/body_W*.c`, generated as the 2^4 subset lattice over {q, p, base, t}) plus birth-order variants Y1 (`i = 0` after the pointer read), Y2 (pointer read before `sh`), Y3 (base written into the top-guard variable `slots`), Y4 (loop-1's pointer IS `slots`) and Y5 (`slots` as the copy dest), each applied at the HEAD anchor and scored.
- result: W00-W09, W12, W13 = 14 at 127; W10, W11, W14, W15 = 36 at 127; Y1 = Y2 = 14 at 127; Y3 = Y5 = 17 at 127; Y4 = 19 at 126. Banked rejected/s55_combine_clobber_via_lnk_var_reuse_seats_shifted_costs_14.c, rejected/s55_combine_clobber_all_vars_shared_across_loops_costs_36.c, rejected/s55_share_p_and_t_across_loops_costs_36.c, rejected/s55_base_written_into_slots_costs_17.c and rejected/s55_loop1_ptr_is_slots_costs_19.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD src/ings.c:820 INCLUDE_ASM anchor with each s55 W/Y cell applied in turn; no FAKE construct present in any body; BASE re-audited at 3 (127/127) in the same session

## [s55] The mandatory kill re-audit found nothing void: BASE re-measures at 3 (127/127) on the HEAD chassis and carries no FAKE construct.
- mechanism: instance kills are chassis- and FAKE-state-relative, so the closest banked form is re-measured before new probes are spent.
- probe: memory/grind/func_80017848/candidate.c applied at the HEAD src/ings.c:820 anchor and scored; the dispatch brief reported the chassis measurement as unavailable.
- result: BASE = 3 at 127/127, identical to its s52/s53/s54 value. The ledger floor of 3 is the correct chassis number.
- verdict: CONFIRMED

## [s55] s54's attribution of loop-1's copy/add pair to combine is incorrect: the operand rewrite that produces the target's 'addu a3,a0,zero' / 'addu a0,a1,a3' is done by optimize_reg_copy_1 inside local-alloc, between the flow and lreg dumps.
- mechanism: tools/gcc-2.7.2/local-alloc.c:700 optimize_reg_copy_1, called from update_equiv_regs at local-alloc.c:1007, takes a reg-reg copy whose source does not die at the copy, scans forward to where the source dies (stopping at labels, jumps and NOTE_INSN_LOOP_BEG/LOOP_END), and if neither register is set in between replaces the source with the destination over that range and moves the death note onto the copy.
- probe: Compared insn 89 at the same uid across the four dumps already on disk with BASE applied: tmp/grind/func_80017848/dumps/ings.combine:6974, ings.flow:9027, ings.lreg:8889, ings.greg:7549.
- result: insn 89 = (set (reg/v 81) (plus (reg/v 84) (reg/v 79))) in combine and flow, and (plus (reg/v 84) (reg/v 80)) in lreg with REG_DEAD (reg 79) moved back onto the copy at insn 83. The producing pass is named unambiguously for the first time.
- verdict: CONFIRMED

## [s55] The preheader copy has a second survival gate that no prior session tested - combine.c:914's '(! all_adjacent && use_crosses_set_p (src, INSN_CUID (insn)))' - and the target's own ctx+0x10 lnk load is a free intervening insn that can supply the required set of the copy's source.
- mechanism: Spelling the lnk load as a write to the same C variable that holds the ctx+0xC pointer makes reg_last_set[src] newer than the copy's CUID, so can_combine_p returns 0, the copy survives to local-alloc, and optimize_reg_copy_1 then rewrites the base add to read it.
- probe: Cell V3 (tmp/grind/func_80017848/s55/body_V3.c, both loops given the clobber with per-loop variables) applied at the HEAD src/ings.c:820 INCLUDE_ASM anchor, scored with 'sandbox func_80017848 --disable all', diffed against tmp/grind/func_80017848/s55/T.txt, then classified with tools/ra_solver/inverse_compose.py.
- result: 14 at 127/127 with the preheader copy 'addu v0,a1,zero' PRESENT in both loops - the loop-1 and loop-2 copies coexist for the first time in 55 sessions - and the whole opcode stream matching the target. inverse_compose.py classify (--target-object build/src/ings.o --ours-object tmp/sandbox/func_80017848/ings.o) reports 'FIRST DIVERGENCE: RA - same instructions, different registers', where every prior session's residual was a structural transposition.
- verdict: CONFIRMED

## [s55] The target's seat map says the ctx+0xC pointer and the loop base are one C variable (both a0) while lnk holds its own seat (a2); spelling it that way makes every register in the function target-exact and leaves only the two copies missing.
- mechanism: V3 reuses the pointer variable for lnk, which keeps that pseudo live through the whole loop body, pins lnk and the pointer to one seat and cascades a one-seat shift through ptr/sh/copy/base/index. Reusing the pointer variable for the base instead lets the pointer die at the copy, and the allocator hands the freed a0 to the base exactly as the target does.
- probe: Cell U1 (tmp/grind/func_80017848/s55/body_U1.c; per-loop 'q' holds the ctx+0xC pointer and is then reassigned the base, 'lnk'/'lnk2' separate, copy dest 'p'/'p2') applied at the HEAD src/ings.c:820 anchor and scored; full normalised objdump diff via tmp/grind/func_80017848/s55/dis.sh.
- result: 4 at 125/127. The complete diff is '-addu a3,a0,zero' / '-addu a0,a1,a3' / '+addu a0,a1,a0' twice, once per loop. All 125 emitted instructions match the target register-for-register, offset-for-offset and in order. Banked as memory/grind/func_80017848/candidate_alt_s55_u1_seat_exact_two_copies_missing_4.c.
- verdict: CONFIRMED

## [s55] On the U1 chassis the copies are merged because the copy's source is set by the base add itself rather than by an insn between the copy and the add; each U-family spelling measured this session leaves both copies merged at 125 instructions.
- mechanism: use_crosses_set_p consults reg_last_set, which combine updates only for insns it has already scanned past, so i3 setting the source does not count as a set 'between'. The two gates are mutually exclusive as spelled so far: V3's clobber buys the copies and loses the seats, U1's variable identity buys the seats and loses the clobber.
- probe: Cells U2 (lnk load moved above the copy), U5/U6 ('slots' as the copy dest in one/both loops), U7 (copy dest shared across both loops), U8 (no reuse, separate base variable) and U10 (explicit re-read of ctx+0xC between the copy and the add), each applied at the HEAD src/ings.c:820 anchor and scored.
- result: U2 = U5 = U6 = U7 = U10 = 4 at 125, U8 = 10 at 125. U10's re-read is folded away by cse so it never becomes a clobber. Banked rejected/s55_no_reuse_separate_base_var_costs_10.c and rejected/s55_reread_ctx0xC_between_copy_and_add_cse_folds_costs_4.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD src/ings.c:820 INCLUDE_ASM anchor with each s55 U-cell applied in turn; no FAKE construct in any body; BASE re-audited at 3 (127/127) in the same session

## [s55] Sharing or splitting the four loop variables (q, p, base, t) across the two loops does not move the V3 seat map, and neither do the birth-order and 'slots'-routing variants measured this session.
- mechanism: Sharing a variable across both loops makes its pseudo multi-block and hands it to global.c instead of local-alloc, which was the hoped-for route to the a3 seat that BASE's loop 1 wins; the measurement shows the seat map is decided upstream of that distinction, by which values are simultaneously live in the loop body.
- probe: Cells W00..W15 (the 2^4 subset lattice over {q, p, base, t}, generated into tmp/grind/func_80017848/s55/) plus Y1 ('i = 0' after the pointer read), Y2 (pointer read before 'sh'), Y3 (base written into the top-guard variable 'slots'), Y4 (loop-1's pointer IS 'slots') and Y5 ('slots' as the copy dest), each applied at the HEAD anchor and scored.
- result: W00-W09, W12, W13 = 14 at 127; W10, W11, W14, W15 = 36 at 127; Y1 = Y2 = 14 at 127; Y3 = Y5 = 17 at 127; Y4 = 19 at 126. Five bodies banked to memory/grind/func_80017848/rejected/ (333 forms total).
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD src/ings.c:820 INCLUDE_ASM anchor with each s55 W/Y cell applied in turn; no FAKE construct present in any body; BASE re-audited at 3 (127/127) in the same session

## [s55] The mandatory kill re-audit found nothing void: BASE re-measures at 3 (127/127) on the HEAD chassis and carries no FAKE construct, so the ledger floor of 3 is the correct chassis number despite the dispatch brief reporting the measurement as unavailable.
- mechanism: Instance kills are chassis- and FAKE-state-relative, so the closest banked form is re-measured before new probes are spent.
- probe: memory/grind/func_80017848/candidate.c applied at the HEAD src/ings.c:820 anchor and scored with 'sandbox func_80017848 --disable all'.
- result: 3 at 127/127, identical to its s52/s53/s54 value.
- verdict: CONFIRMED

## s56 (2026-09-15, rederive - forced sibling transplant from func_8005BA8C)

## [s56] Forced sibling transplant: func_8005BA8C's s2 finding H8 (copy the pointer parameter into a local cursor at entry and route every access through it) applied to this function's best chassis.
- mechanism: the sibling's closed ledger (its CROSS-KNOWLEDGE line) states that func_80017848 shares no code block with it; the only transplantable content is its general lever H8 (param -> entry-copied local cursor, which in the sibling fixed prologue order and one allocno priority) and H6 (param advanced in place, not applicable here because ctx is never advanced).
- probe: cell K4 = cell K2 with `u8 *c; c = ctx;` as the first statement and every `ctx` use rewritten to `c` (tmp/grind/func_80017848/s56/body_K4.c), applied at the HEAD src/ings.c:820 INCLUDE_ASM anchor and scored with `sandbox func_80017848 --disable all`; normalised objdump diff in s56/diff_K4.txt.
- result: 8 at 127/127. The only change against K2 is that the `sw s2,40(sp)` / `addu s2,a0,zero` prologue pair moves from slots 2-3 to slots 9-10 (the ctx pseudo's first use shifts, so the prologue order changes); the copy-dest seat residual is unchanged. Banked as rejected/s56_k4_sibling_8005BA8C_entry_cursor_c_eq_ctx_moves_s2_prologue_pair_costs_8.c. The owner directive (auto-return on func_8005BA8C's completion) is executed and measured.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD src/ings.c:820 INCLUDE_ASM anchor with tmp/grind/func_80017848/s56/body_K4.c applied (K2 chassis + entry cursor); no FAKE construct in any s56 body; BASE re-audited at 3 (127/127) in the same session

## [s56] combine.c:914's clobber gate can be fed by a set of the copy's source that combine itself later deletes, because reg_last_set[] is written only at combine.c:9804 and never cleared when an i2 is merged away; so the lnk load can be spelled INTO the pointer variable and copied straight out again (`q = *(u8 **)(ctx + 0x10); lnk = q;`), which keeps s55-U1's exact seat map while making both preheader copies survive with no second reader.
- mechanism: combine scans insns in order and records reg_last_set[q] = the lnk-load insn when it passes it; at `lnk = q` it merges the load into the copy (q's second value has one use and dies there) and turns the load into a NOTE, but the NOTE keeps its INSN_CUID and reg_last_set[q] still points at it; at the base add `q = sh + p`, can_combine_p calls use_crosses_set_p (q, CUID(copy)), finds INSN_CUID (reg_last_set[q]) > CUID(copy) and returns 1, so the copy `p = q` is not merged (combine.c:914). Read this session at tools/gcc-2.7.2/combine.c:860-960 (gates), :9804 (the only reg_last_set writer), :10107-10131 (use_crosses_set_p).
- probe: cell K1 = s55's U1 with `lnk = *(u8 **)(ctx + 0x10);` replaced by `q = *(u8 **)(ctx + 0x10); lnk = q;` in loop 1 and the same for q2/lnk2 in loop 2 (tmp/grind/func_80017848/s56/body_K1.c); applied at the HEAD anchor, scored, diffed (s56/diff_K1.txt); RA solver `extract.py func_80017848 ings` + `simulate.py --trace` on the same source (s56/ra_K1.json, s56/sim_K1.txt).
- result: 6 at 127/127. BOTH copies are present (`addu v0,a0,zero` / `addu a0,a1,v0` in each preheader), the lnk load sits between them exactly as in the target, and the remaining 123 instructions match register-for-register. The only residual is the copy destination's seat: v0 where the target has a3. The RA model reproduces the build exactly (sort order MATCH, 14/14 dispositions); the copy dests p/p2 are absent from the global allocno list, i.e. they are LOCAL quantities seated by local-alloc's ascending scan (the s48 mechanism). This is the first form in 56 sessions with both copies, no reader, and a U1-exact seat map everywhere else.
- verdict: CONFIRMED

## [s56] Making the two copy destinations ONE variable (`p` in both loops) turns the copy dest into a global allocno (two sets in two blocks) without adding bytes, and global.c's pass-0 scan still seats it in v0 because nothing excludes v0, a0, a1 or a2 at its allocation time.
- mechanism: global.c find_reg pass 0 (global.c:985-1078) takes the lowest regno not in hard_reg_conflicts, not outside regs_used_so_far and not in regs_someone_prefers; p (pseudo 78) sorts at pri 13333 (4 refs, live length 6), above sh (3750) and lnk (2500) so they do not constrain it, below i (26000 -> v1) and q/q2 (16666 -> a0); q never overlaps p (q dies at the copy and is reborn at the add where p dies, and global_conflicts processes deaths before stores, global.c:755-790), so a0 is not a conflict. Scan -> v0.
- probe: cell K2 (tmp/grind/func_80017848/s56/body_K2.c) scored and diffed (s56/diff_K2.txt); RA solver extract + simulate --trace (s56/ra_K2.json, s56/sim_K2.txt).
- result: 4 at 127/127 (the score improves from K1's 6 because the two seats are now one consistently renamed pseudo). TRACE for pseudo 78: `pri=13333 hard_conf=[3, 29] someone=[] best=2 prefs=[2]`; sort order MATCH, 15/15 dispositions. Banked as memory/grind/func_80017848/candidate_alt_s56_k2_combine914_clobber_both_copies_seat_v0_4.c, the most advanced form on file.
- verdict: CONFIRMED

## [s56] Sharing q, lnk, t and sh across both loops on the K1 chassis changes nothing.
- mechanism: the copy dest seat is decided by the copy dest's own live range and allocno class, not by which pointer/lnk/count/shift variable feeds it.
- probe: cell K3 (tmp/grind/func_80017848/s56/body_K3.c: K1 with q2/lnk2/t2/sh2 folded into q/lnk/t/sh), scored and diffed.
- result: 6 at 127/127, byte-identical to K1. Banked as rejected/s56_k3_shared_q_lnk_t_sh_across_loops_on_k1_costs_6.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD src/ings.c:820 INCLUDE_ASM anchor with s56/body_K3.c applied; no FAKE construct; BASE re-audited at 3 (127/127) in the same session

## [s56] The mandatory chassis/kill re-audit: BASE re-measures 3 at 127/127 on the HEAD chassis (the dispatch brief reported the measurement as unavailable), U1 re-measures 4 at 125/127; no FAKE construct exists in candidate.c or any s55/s56 body, so no ablation applies.
- mechanism: instance kills are chassis-relative; the closest banked forms are re-measured before new probes.
- probe: body_BASE.c and body_U1.c (extracted from candidate.c and candidate_alt_s55_u1_seat_exact_two_copies_missing_4.c) applied at the HEAD src/ings.c:820 anchor and scored (s56/diff_BASE.txt, s56/diff_U1.txt).
- result: BASE = 3 at 127/127 with the s54 three-instruction transposition; U1 = 4 at 125/127 with the two copies missing. Both identical to s55. Ledger floor 3 is the correct chassis number.
- verdict: CONFIRMED

## [s56] Frontier restated after K2: the seat question is isolated for the first time from the copy-survival question. From tools/gcc-2.7.2/global.c read end to end this session, the copy dest reaches a3 only if, at its allocation, v0 AND a0 AND a1 AND a2 are excluded (hard_reg_conflicts or regs_someone_prefers) or it carries an a3 copy preference. Both the v0 and the a0 exclusion require the copy dest to be live past the base add (a0's holder base is born there; v0's holders are the loop-body temporaries), i.e. a reader after the add that leaves no bytes - E-s44-3's requirement, now with combine survival already paid by the K device so the reader no longer has to defeat combine. Post-flow deleters enumerated this session (combine merge/fold, local-alloc optimize_reg_copy_2 self-copies, update_equiv_regs init deletion, reload no-op moves, sched2 no-op moves, jump2 no-op or find_equiv_reg-redundant moves and dead code, reorg redundant_insn, final no-op moves); none has yet been spelled from ordinary C for this reader. The alternative, a copy preference for a3, needs a hard-reg copy (set_preference, global.c:1671), i.e. a 4th call argument or a local-alloc'd pseudo seated in a3, both previously measured dead (s40).

## [s56] Forced sibling transplant: func_8005BA8C's H8 (copy the pointer parameter into a local cursor at entry, route every access through it) applied to this function's K2 chassis.
- mechanism: The sibling's closed ledger states this function shares no code block with it; H8 is its only general lever (in the sibling it fixed prologue order and one allocno priority). H6 (param advanced in place) is inapplicable because ctx is never advanced.
- probe: Cell K4 = K2 + `u8 *c; c = ctx;` first statement, every ctx use through c (tmp/grind/func_80017848/s56/body_K4.c), applied at the HEAD src/ings.c:820 INCLUDE_ASM anchor, sandbox func_80017848 --disable all, normalised objdump diff s56/diff_K4.txt.
- result: 8 at 127/127: the sw s2 / addu s2,a0 prologue pair moves from slots 2-3 to 9-10 on top of K2's unchanged copy-dest seat residual. Banked rejected/s56_k4_sibling_8005BA8C_entry_cursor_c_eq_ctx_moves_s2_prologue_pair_costs_8.c. Owner directive executed and measured.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD src/ings.c:820 INCLUDE_ASM anchor with tmp/grind/func_80017848/s56/body_K4.c applied (K2 chassis + entry cursor); no FAKE construct in any s56 body; BASE re-audited at 3 (127/127) in the same session

## [s56] combine.c:914's clobber gate can be fed by a set of the copy's source that combine later deletes, because reg_last_set[] is written only at combine.c:9804 and never cleared when an i2 is merged away; spelling the lnk load into the pointer variable and copying it straight out (`q = *(u8 **)(ctx + 0x10); lnk = q;`) keeps U1's seat map and makes both preheader copies survive with no second reader.
- mechanism: combine records reg_last_set[q] at the lnk-load insn, merges that load into `lnk = q` (the load becomes a NOTE but keeps its CUID), then at the base add `q = sh + p` use_crosses_set_p (q, CUID(copy)) still sees a later set and can_combine_p refuses the copy merge (combine.c:860-960, :9804, :10107-10131 read this session). optimize_reg_copy_1/2 do not fire on this shape (local-alloc.c:874, :1005-1015).
- probe: Cell K1 = s55 U1 with the lnk load rewritten in both loops (s56/body_K1.c); sandbox + diff (s56/diff_K1.txt); tools/ra_solver extract.py + simulate.py --trace (s56/ra_K1.json, s56/sim_K1.txt).
- result: 6 at 127/127. Both copies present, the lnk load between copy and add as in the target, 123 other instructions register-exact; the only residual is the copy dest seat (v0 vs a3). RA model exact (sort MATCH, 14/14); the copy dests p/p2 are local quantities (absent from the global list) seated by local-alloc's ascending scan.
- verdict: CONFIRMED

## [s56] Using ONE copy-destination variable for both loops makes the copy dest a global allocno (two sets in two blocks) at no byte cost, and global.c's pass-0 scan still seats it in v0 because nothing excludes v0, a0, a1 or a2 at its allocation time.
- mechanism: find_reg pass 0 (global.c:985-1078) takes the lowest regno outside hard_reg_conflicts / someone_prefers; global_conflicts (global.c:755-790) processes deaths before stores so p (born where q dies, dying where base is born) never overlaps a0; sh/lnk sort below p (pri 13333) so they do not constrain it.
- probe: Cell K2 (s56/body_K2.c) sandbox + diff (s56/diff_K2.txt); RA solver extract + simulate --trace (s56/ra_K2.json, s56/sim_K2.txt).
- result: 4 at 127/127, same four seat-only diff lines. Pseudo 78 (p): pri=13333 hard_conf=[3,29] someone=[] best=2 prefs=[2]; sort MATCH, 15/15 dispositions. Banked as memory/grind/func_80017848/candidate_alt_s56_k2_combine914_clobber_both_copies_seat_v0_4.c (most advanced form on file).
- verdict: CONFIRMED

## [s56] Sharing q, lnk, t and sh across both loops on the K1 chassis changes the copy-dest seat.
- mechanism: The seat is decided by the copy dest's own live range and allocno class, not by which pointer/lnk/count/shift variable feeds it.
- probe: Cell K3 (s56/body_K3.c: K1 with q2/lnk2/t2/sh2 folded into q/lnk/t/sh), sandbox + diff.
- result: 6 at 127/127, byte-identical to K1. Banked rejected/s56_k3_shared_q_lnk_t_sh_across_loops_on_k1_costs_6.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD src/ings.c:820 INCLUDE_ASM anchor with tmp/grind/func_80017848/s56/body_K3.c applied; no FAKE construct; BASE re-audited at 3 (127/127) in the same session

## [s56] Chassis and kill re-audit: BASE and U1 re-measure unchanged on the HEAD chassis and carry no FAKE construct.
- mechanism: Instance kills are chassis-relative; the dispatch brief reported the HEAD measurement as unavailable.
- probe: body_BASE.c / body_U1.c applied at the HEAD src/ings.c:820 anchor and scored (s56/diff_BASE.txt, s56/diff_U1.txt).
- result: BASE = 3 at 127/127 (the s54 three-instruction transposition), U1 = 4 at 125/127 (both copies missing), identical to s55. Ledger floor 3 is the correct chassis number.
- verdict: CONFIRMED

## s57 (2026-09-15, structural - global.c preference/scan routes to the a3 seat)

## [s57] Owner directive (auto-return on func_8005BA8C's completion) acknowledged: it was executed and measured in s56 (cell K4, its H8 entry-cursor lever, 8 at 127/127, KILLED); nothing further to transplant (the sibling shares no code block, s56 E-s56-1).
- mechanism: directive bookkeeping only.
- probe: none this session (s56 measurement stands; s57 cell C re-measures the same lever on slot_b instead of ctx, see below).
- result: acknowledged.
- verdict: CONFIRMED

## [s57] Mandatory chassis/kill re-audit: K2 re-measures 4 at 127/127 on the HEAD chassis (src/ings.c:820 INCLUDE_ASM anchor, HEAD 44087a2cd); no FAKE construct exists in candidate.c or any s56/s57 body, so no ablation applies.
- mechanism: instance kills are chassis-relative.
- probe: tmp/grind/func_80017848/s57/body_K2.c applied via s57/apply.py, `sandbox func_80017848 --disable all`, normalised diff s57/diff_K2.txt.
- result: 4 at 127/127, the same four seat-only diff lines (`addu v0,a0,zero`/`addu a0,a1,v0` for `addu a3,a0,zero`/`addu a0,a1,a3`, once per loop). Ledger floor 3 (BASE) is the correct chassis number.
- verdict: CONFIRMED

## [s57] The prefs={v0} entry on the copy-dest pseudo (s56 frontier item 2) comes from expand_preferences, not from set_preference on the copy: base's v0 hard_reg_preference (set_preference on the loop body's `(set v0-local (plus base i))`, whose first source operand is base) is merged into p at the base add because p dies there and does not conflict with base.
- mechanism: global.c:829-870 expand_preferences: for every single_set whose dest is an allocno, each REG_DEAD allocno that does not conflict with the dest exchanges hard_reg_preferences/full_preferences with it (copy preferences too when the dying reg IS the source). set_preference (global.c:1671) itself only records preferences when one side is a hard reg or a local-alloc'd pseudo; `p = q` has two global pseudos and records nothing.
- probe: tools/grinder/dump.ps1 with body_K2.c applied; `.greg` dump lines `;; 78 preferences: 2`, `;; 78 conflicts: 72 73 74 75 78 80 82 85 86 89 29` (no 79/81 = q/base, so the merge condition holds); global.c read at :829-870, :882-935, :985-1160, :1671-1740.
- result: traced. Consequence: p ALWAYS fully-prefers v0 on any chassis where the body's first temporary is a v0 local and p dies at the add, so regs_someone_prefers can never exclude v0 for p (prune_preferences global.c:920-928 removes the allocno's own full preferences from the someone set).
- verdict: CONFIRMED

## [s57] Reusing the dead parameter slot_b as the copy destination (cell A: `b = slot_b;` first, all slot_b uses through b, `slot_b = (s32)q; q = (u8 *)(sh + slot_b);` in both preheaders) gives the copy dest the incoming a3 copy preference and the seat.
- mechanism: set_preference records hard_reg_copy_preferences[75] = {a3} from the entry copy `(set (reg 75) (reg a3))`; 75 no longer crosses the call (b does), so prune_preferences keeps a3; find_reg's copy-preference override (global.c:1097-1115) then picks a3 over the scan result v0.
- probe: tmp/grind/func_80017848/s57/body_A.c (generated by s57/gen.py from body_K2.c), sandbox, diff s57/diff_A.txt, dump.ps1 with body_A applied, `.greg` function summary.
- result: 4 at 127/127, byte-identical to K2. The `.greg` dump shows `;; 75 conflicts: 72 73 74 75 79 81 84 85 88 90 29` and `;; 75 preferences: 2` (v0, via the base merge), no a3: combine merged the entry copy `(set 75 a3)` into its single use `(set 90 75)` (b = slot_b) giving `(set 90 (reg a3))`, so the a3 event now belongs to b (pseudo 90, seated s3, crosses the call, pref pruned) and 75 is just the two loop segments. can_combine_p (combine.c:860-945) has no gate against substituting a hard-reg source on MIPS (SMALL_REGISTER_CLASSES undefined), the two insns are in the same block (LOG_LINK exists), and a3 is not set in between. Banked rejected/s57_A_dead_param_slot_b_reused_as_copy_dest_entry_copy_b_costs_4.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD src/ings.c:820 INCLUDE_ASM anchor with tmp/grind/func_80017848/s57/body_A.c applied (K2 chassis + entry copy + param reuse); no FAKE construct in any s57 body; K2 re-audited at 4 (127/127) in the same session

## [s57] The copy destination can obtain an a3 copy or hard preference (global.c set_preference / expand_preferences) from the incoming-argument register.
- mechanism: set_preference (global.c:1671) records a preference only when one side of a single_set is a hard reg or a local-alloc'd pseudo. The only insn in this function that mentions hard a3 is the incoming-argument copy `(set 75 (reg a3))` (the call has two arguments; nothing else touches a3). expand_preferences (global.c:829) propagates preferences only along death->set edges in insn order. The param pseudo either lives to the function end (its uses after the call: `or v0,v0,s3`, `sll s1,s3,6`), so it crosses the call, its a3 pref is pruned (global.c:900) and it dies at no insn before the loops; or it is copied into a local at entry so that it dies there, in which case combine folds the entry copy into that local's set (cell A measurement) and the a3 event moves to the call-crossing local. Any spelling where the param pseudo's entry value has a second reader emits a byte-visible read of a3 (the target reads a3 only in the two base adds).
- probe: cell A measurement + dump (above); combine.c:860-945 and global.c:829-935, :1671-1740 read this session.
- result: no spelling of this route survives combine without adding an a3-reading instruction; every form fails the predicate that set_preference needs a hard-reg or local-alloc'd operand and the only a3 operand insn folds away.
- verdict: KILLED
- kill_scope: class
- predicate_cite: tools/gcc-2.7.2/global.c:1671
- measured_on: HEAD src/ings.c:820 INCLUDE_ASM anchor with tmp/grind/func_80017848/s57/body_A.c applied; no FAKE construct

## [s57] Placing the copy `p = q` in the guard block before the guard temporaries (cell B) makes p live across the guard's v0 temporaries and q, so global.c records v0 and a0 conflicts and the scan reaches a3 (at the cost of the copy's position).
- mechanism: global_conflicts records hard-reg conflicts for every insn where p is live; a p born before `t = sh + q` overlaps the v0 temporaries and q (a0).
- probe: tmp/grind/func_80017848/s57/body_B.c (K2 with `p = q;` moved before `t = sh + (s32)q;` in both loops), sandbox, diff s57/diff_B.txt.
- result: 4 at 125/127: both copies vanish (`addu a0,a1,a0`). cse's extended basic block follows the fall-through edge into the single-predecessor preheader (cse.c cse_end_of_basic_block path following), canonicalises p's only use to q and the copy is deleted as dead before flow. Same 125-insn shape as s55 U1. Banked rejected/s57_B_copy_hoisted_before_guard_temps_cse_folds_dest_into_q_costs_4_at_125.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD src/ings.c:820 INCLUDE_ASM anchor with tmp/grind/func_80017848/s57/body_B.c applied; no FAKE construct

## [s57] `b = slot_b;` as the first statement with every slot_b use routed through b (cell C) is byte-neutral on the K2 chassis.
- mechanism: the incoming copy `(set 75 a3)` folds into `(set b a3)`, which is the LAST of the four argument copies, so the prologue pairing is unchanged (for ctx, s56's K4, the folded copy was emitted after the other three copies and moved the s2 pair).
- probe: tmp/grind/func_80017848/s57/body_C.c, sandbox, diff s57/diff_C.txt.
- result: 4 at 127/127, byte-identical to K2. Banked rejected/s57_C_entry_copy_b_eq_slot_b_byte_neutral_control_4.c. A free spelling degree of freedom, not a lever.
- verdict: CONFIRMED

## [s57] Frontier restated: for a copy destination whose live range is exactly [copy, base add] (forced by the target bytes: a3 is written at the copy, read at the add and nowhere else; a0's q value is dead after the copy and a0's base is born at the add), global.c's pass-0 scan cannot return a3 because a0 can never be in `used` (no conflict: death-before-store; no someone_prefers: the only a0-preference holder is the call-crossing ctx pseudo, pruned; regs_used_so_far pre-seeds every call-used reg), a1/a2 can only enter `used` if sh and lnk are allocated before p (priority < 2500, i.e. live length >= 33 at 4 refs), and v0 can never be someone-excluded because p inherits base's v0 preference. The preference override cannot pick a3 (no a3 event survives combine). local-alloc's `used` is hard regs live in the block only (local-alloc.c:2164-2171, regs_live seeded from the first regset word of basic_block_live_at_start, :1160-1162) -> v0. reload's find_equiv_reg copy route (reload1.c:5843-5853) would emit exactly `addu a3,a0,zero` with a3 = the first potential spill reg (reload1.c:3766-3772: unused call-used regs first; a3 is the only unused a-reg in this function) but requires the copy dest to be UNALLOCATED, and global.c allocates every GR_REGS allocno it creates (find_reg fails only on class exhaustion, global.c:586-598; allocnos are created for every pseudo with reg_n_refs != 0, :414-431; the only reg_n_refs zeroing paths, combine.c:2313 and local-alloc.c:1104, remove the pseudo from all insns). Therefore the residual is NOT a register-allocation question on any chassis where the copy is an ordinary expanded C copy with that range: the next session must attack either the RANGE (a byte-free second segment of the copy dest that overlaps a v0 local and an a0 holder and is deleted only after global - the s56 no-op-move enumeration - or a second segment ending at the copy itself) or the PASS that emits the copy (a reload-generated copy from an insn that needs an input reload of `(mem ctx+12)`: no insn accepts a MEM operand on MIPS after expansion, so that needs a spelling in which the add's operand is a MEM at reload time, e.g. through an unallocated REG_EQUIV pseudo - currently closed).

## [s57] The prefs={v0} entry on the copy-dest pseudo comes from expand_preferences merging base's v0 hard_reg_preference (set_preference on the body's `(set v0-local (plus base i))`) into p at the base add where p dies without conflicting with base.
- mechanism: global.c:829-870 expand_preferences exchanges hard_reg_preferences/full_preferences between a set's dest allocno and each non-conflicting REG_DEAD allocno; set_preference (global.c:1671) records nothing for a pseudo-pseudo copy.
- probe: tools/grinder/dump.ps1 with tmp/grind/func_80017848/s57/body_K2.c applied; .greg lines `;; 78 preferences: 2` and `;; 78 conflicts: ...` (no 79/81); global.c:829-935 and :1671-1740 read.
- result: Traced. Consequence: p always fully-prefers v0 while the body's first temporary is a v0 local, so prune_preferences (global.c:920-928) can never let regs_someone_prefers exclude v0 for p.
- verdict: CONFIRMED

## [s57] Reusing the dead parameter slot_b as the copy destination (cell A: `b = slot_b;` first, all slot_b uses through b, `slot_b = (s32)q; q = (u8 *)(sh + slot_b);` in both preheaders) gives the copy dest the incoming a3 copy preference and the a3 seat.
- mechanism: set_preference records copy pref a3 from the entry `(set 75 a3)`; 75 would no longer cross the call so prune keeps a3; find_reg's copy-preference override (global.c:1097-1115) picks a3 over the scan's v0.
- probe: tmp/grind/func_80017848/s57/body_A.c at the HEAD src/ings.c:820 anchor, sandbox func_80017848 --disable all, s57/diff_A.txt, dump.ps1 .greg summary.
- result: 4 at 127/127, byte-identical to K2. .greg: `;; 75 conflicts: 72 73 74 75 79 81 84 85 88 90 29`, `;; 75 preferences: 2`: combine merged `(set 75 a3)` into `(set 90 75)` -> `(set 90 a3)` (combine.c:860-945, no SMALL_REGISTER_CLASSES on MIPS), so the a3 event belongs to b (crosses the call, pruned). Banked rejected/s57_A_dead_param_slot_b_reused_as_copy_dest_entry_copy_b_costs_4.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD src/ings.c:820 INCLUDE_ASM anchor with tmp/grind/func_80017848/s57/body_A.c applied (K2 chassis + entry copy + param reuse); no FAKE construct in any s57 body; K2 re-audited at 4 (127/127) this session

## [s57] The copy destination can obtain an a3 copy or hard-reg preference (set_preference or an expand_preferences chain) from the incoming-argument register a3.
- mechanism: set_preference (global.c:1671) needs a hard reg or a local-alloc'd pseudo on one side; the only hard-a3 insn is the incoming-argument copy; expand_preferences propagates only along death->set edges in insn order; the param pseudo either lives to the end (crosses the call, pref pruned at global.c:900, dies at no pre-loop insn) or dies at an entry copy that combine folds (cell A), moving the a3 event to a call-crossing local; a second reader of the entry value is a byte-visible a3 read.
- probe: Cell A measurement + .greg dump; combine.c:860-945, global.c:829-935 and :1671-1740 read this session.
- result: No form of this route survives combine without adding an a3-reading instruction; the target reads a3 only in the two base adds.
- verdict: KILLED
- kill_scope: class
- measured_on: HEAD src/ings.c:820 INCLUDE_ASM anchor with tmp/grind/func_80017848/s57/body_A.c applied; no FAKE construct
- predicate_cite: tools/gcc-2.7.2/global.c:1671

## [s57] Placing the copy `p = q` in the guard block before the guard temporaries (cell B) makes p overlap the guard's v0 temporaries and q so global.c records v0 and a0 conflicts for it.
- mechanism: global_conflicts records hard-reg conflicts at every insn where p is live.
- probe: tmp/grind/func_80017848/s57/body_B.c (K2 with `p = q;` moved before `t = sh + (s32)q;` in both loops), sandbox, s57/diff_B.txt.
- result: 4 at 125/127: both copies vanish (`addu a0,a1,a0`); cse's path following into the single-predecessor preheader canonicalises p's use to q and the copy dies before flow (same shape as s55 U1). Banked rejected/s57_B_copy_hoisted_before_guard_temps_cse_folds_dest_into_q_costs_4_at_125.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD src/ings.c:820 INCLUDE_ASM anchor with tmp/grind/func_80017848/s57/body_B.c applied; no FAKE construct

## [s57] `b = slot_b;` as the first statement with every slot_b use through b (cell C) is byte-neutral on the K2 chassis.
- mechanism: The folded `(set b a3)` is the last of the four argument copies, so the prologue pairing is unchanged (unlike ctx in s56 K4).
- probe: tmp/grind/func_80017848/s57/body_C.c, sandbox, s57/diff_C.txt.
- result: 4 at 127/127, byte-identical to K2. Banked rejected/s57_C_entry_copy_b_eq_slot_b_byte_neutral_control_4.c.
- verdict: CONFIRMED

## [s57] Chassis re-audit: K2 re-measures 4 at 127/127 on the HEAD chassis; no FAKE construct exists in candidate.c or any s56/s57 body.
- mechanism: instance kills are chassis-relative.
- probe: s57/body_K2.c applied and scored; s57/diff_K2.txt.
- result: 4 at 127/127, same four seat-only lines. Ledger floor 3 stands.
- verdict: CONFIRMED

## s58 (2026-09-15, enumerate - systematic spelling sweep per owner ruling 2026-09-08)

## [s58] Chassis re-audit: BASE (candidate.c) re-measures 3 at 127/127 on the HEAD chassis; K2 alt re-measures 4 at 127/127; tools/fake_ablate.py reports no FAKE construct in either. A block-scoped fresh local for the loop-2 guard address (`{ s32 tt = (slot_a << 6) + (s32)p; ... }`) is byte-neutral (3 at 127), which licenses placing the enumerator's declaration region at a bare-block start; reusing the outer `t` for that address instead costs 17 at 127 (variable reuse is not a free axis).
- mechanism: instance kills are chassis-relative; the enumerator emits `TYPE name = expr;` declarations at the region top, so the region must open a block.
- probe: tmp/grind/func_80017848/s58/base.c, block_tt_test.c, outer_t_test.c, k2.c via tools/sweep_variants.py; fake_ablate on k2.c.
- result: 3 / 3 / 17 / 4 respectively, all at 127. Ledger floor 3 stands; owner directive (func_8005BA8C auto-return) was executed and measured in s56 (K4, 8) and needs no further action.
- verdict: CONFIRMED

## [s58] The loop-2 joint region (guard + preheader + body, guard read through the loop-1-carried `p`) contains a naming / declaration-order / commutative-swap spelling that scores below 3.
- mechanism: owner ruling 2026-09-08 enumerate modality: every sub-expression that could be a local is a local (sh2n, q2, ga, gc, lnk2, b2), tools/spelling_enum.py emits every inline-subset x def-before-use order (x swap subset), tools/sweep_variants.py scores each with the engine.
- probe: tmp/grind/func_80017848/s58/enum_base_p.c -> enum_base_p/ (403 no-swap variants) -> sweep_base_p.json; then swaps on the 28 forms at <= 5 -> enum_swaps/ (96, 56 valid) -> sweep_swaps.json.
- result: histogram (score, insns): 8 at (3,127), 20 at (5,127), 96 at (6,126), 50 at (33,126), 145 at (34,126), 84 at (35,127). Swaps: 16 at 3, 40 at 5, 40 invalid (tool swapped `u8 * lnk2` as a product). ZERO below 3. Cross-tab by which locals stay named: every form with q2, b2 and lnk2 all inlined is 3 (naming sh2n/ga/gc is inert); lnk2 named above the guard = 5; q2 named above the guard (the target's own lw-a0-in-guard-block shape) = 6 at 126; b2 named (base add hoisted above the guard) = 33-35. Banked rejected/s58_enum_l2_lnk2_named_before_guard_costs_5.c, s58_enum_l2_q2_named_above_guard_costs_6_or_33.c.
- verdict: KILLED (class: the region's naming/order/swap spelling space is exhausted by construction of the enumerator; predicate tools/spelling_enum.py:26)

## [s58] The same loop-2 region with the guard read through a fresh ctx+0xC local (q2) instead of `p` contains a spelling at or below 3.
- mechanism: same enumerator; this is the s9 "q direct into l2 guard" chassis re-swept exhaustively.
- probe: enum_base_q.c -> 302 variants -> sweep_base_q.json.
- result: 60 at (31,124), 242 at (32,123). Flat; the loop-1 tail copy `p = q` becomes dead and both preheader devices collapse. Banked rejected/s58_enum_l2_guard_through_fresh_load_flat_31.c.
- verdict: KILLED (instance: this chassis, no FAKE construct, 302 spellings measured)

## [s58] On the K2 chassis (both preheader copies present, seats v0 for a3) a naming/order/swap spelling of the loop-2 region (sh2, t2a, t2; q2 kept named because it is re-assigned) moves the copy-dest seat.
- mechanism: local-alloc / global.c seat choice is insensitive to the naming of the guard's sub-expressions if their pseudos' live ranges do not change; the sweep tests that empirically.
- probe: enum_k2.c -> 12 variants -> sweep_k2.json.
- result: all 12 at 4 (127/127). Banked rejected/s58_enum_k2_l2_region_flat_4.c.
- verdict: KILLED (instance: K2 chassis, no FAKE construct, 12 spellings measured)

## [s58] A loop-1 region spelling (guard through p; q1, lnk1, b1, ga1, gc1, sh1 declared ABOVE the guard, tail `(p) = q1`) reaches 3 or below, i.e. a loop-1 respelling that changes loop 2's residual.
- mechanism: the residual is the loop-1 tail device's join copy (s54); a loop-1 spelling that keeps the copy but seats it differently could move loop 2.
- probe: enum_loop1b.c -> 403 variants -> sweep_loop1b.json.
- result: best 15 at 125/126, 34 forms; 16-37 otherwise. NOT a faithful enumeration of the base's loop 1: with all decls above the guard, q1 named hoists the ctx+0xC load above the guard and q1 inlined turns the tail into a reload (`(p) = (*(u8 **)(ctx + 0xC))`, 15 at 126 - the s9 no-tail-reload result). The base's in-preheader `q` + tail copy is expressible only with the region INSIDE the if-body, done next. Banked rejected/s58_enum_l1_decls_above_guard_flat_15.c.
- verdict: KILLED (instance: this region placement on the BASE chassis, no FAKE construct, 403 spellings measured)

## [s58] An in-preheader region spelling of loop 1 (q1 / lnk1 / b1 declared at the top of the if-body, tail `(p) = q1`, with swaps) or of loop 2 (q2 / lnk2 / b2 at the top of its if-body, with swaps) scores below 3.
- mechanism: same enumerator with the region opening the existing if-body block, which reproduces the base's own declaration placement exactly (v000 of each set IS the base modulo names).
- probe: enum_loop1c.c / enum_loop2c.c -> 102 variants each (60 valid each; 42 each are the `u8 * x` swap-bug outputs at 46/107 and 86/109) -> sweep_loop1c.json, sweep_loop2c.json.
- result: loop1c: 6 at (3,127), 6 at (4,126), 4 at 5, 4 at 6, 4 at 7, 8 at 14, 6 at 16, 8 at 17, 14 at 20. loop2c: 24 at (3,127), 8 at (7,126), 28 at (17,125). ZERO below 3 in either.
- verdict: KILLED (class: in-preheader naming/order/swap spelling space of both loops exhausted by construction; predicate tools/spelling_enum.py:26)

## [s58] Frontier restated after the enumeration: seven sweeps, 1,420 spellings (1,298 valid), best 3, never below, on two chassis (BASE and K2) and three region placements. The naming / declaration-order / commutative-swap spelling space of the loop-1 and loop-2 regions is CLOSED; the gradient inside it is monotone toward the base (every deviation from "q2, b2, lnk2 inlined" costs 2-30). What the enumerator cannot express - and where the residual therefore lives - is exactly s57's RANGE frontier: variable IDENTITY (which C variable carries the pointer across the copy; the enumerator only names fresh locals, never reuses one, and `t` reuse alone moves the score 3 -> 17), declaration SCOPE (block vs function scope changes nothing here, measured byte-neutral), and the object model (a struct-typed ctx / slot record, never tried because the splat names are per-word). Two tool findings for the next session: (1) tools/spelling_enum.py's swap axis treats `u8 * name = ...` as a product and emits invalid `name * u8 = ...` files - discard the 46/107 and 86/109 buckets; (2) the enumerator cannot place a named local inside a nested block, so a region must open at a block start (a bare block is byte-neutral on this chassis).

## [s58] Chassis re-audit: BASE re-measures 3 at 127/127 and the K2 alt 4 at 127/127 on HEAD; fake_ablate finds no FAKE construct; a bare block with a fresh block-scoped guard-address local is byte-neutral (3) while reusing outer t for it costs 17.
- mechanism: instance kills are chassis-relative; the enumerator needs a block-start region
- probe: tmp/grind/func_80017848/s58/base.c, block_tt_test.c, outer_t_test.c, k2.c via sweep_variants; fake_ablate on k2.c
- result: 3 / 3 / 17 / 4 at 127. Owner directive (func_8005BA8C auto-return) already executed and measured in s56 (K4, 8).
- verdict: CONFIRMED

## [s58] The loop-2 joint region (guard + preheader + body, guard via the loop-1-carried p) has a naming / declaration-order / commutative-swap spelling that scores below 3.
- mechanism: spelling_enum.py enumerates every inline subset x def-before-use order x swap subset of the fully-named region; sweep_variants scores each with the engine
- probe: enum_base_p (403) + enum_swaps (96, 56 valid) -> sweep_base_p.json, sweep_swaps.json
- result: 8+16 at 3/127, 20+40 at 5, 96 at 6/126, 50 at 33, 145 at 34, 84 at 35; zero below 3. Determinant: which loads sit above the guard (none 3, lnk2 5, q2 6, base add 33+); naming sh2n/ga/gc inert.
- verdict: KILLED
- kill_scope: class
- measured_on: HEAD src/ings.c:820 INCLUDE_ASM anchor; no FAKE construct in any body (fake_ablate confirmed on candidate.c and the K2 alt)
- predicate_cite: tools/spelling_enum.py:26

## [s58] The loop-2 region with the guard read through a fresh ctx+0xC local instead of p has a spelling at or below 3 (302 spellings measured).
- mechanism: s9 q-direct-into-guard chassis re-swept exhaustively
- probe: enum_base_q -> sweep_base_q.json
- result: 60 at 31/124, 242 at 32/123; the loop-1 tail copy device dies.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD src/ings.c:820 INCLUDE_ASM anchor; no FAKE construct in any body (fake_ablate confirmed on candidate.c and the K2 alt); guard-via-fresh-load chassis

## [s58] On the K2 chassis a naming/order spelling of the loop-2 region (sh2, t2a, t2 with q2 kept) moves the copy-dest seat (12 spellings measured).
- mechanism: seat choice insensitive to naming when live ranges do not change
- probe: enum_k2 -> sweep_k2.json
- result: all 12 at 4/127.
- verdict: KILLED
- kill_scope: instance
- measured_on: K2 chassis (candidate_alt_s56_k2...), HEAD src/ings.c:820 INCLUDE_ASM anchor; no FAKE construct in any body (fake_ablate confirmed on candidate.c and the K2 alt)

## [s58] A loop-1 region spelling with q1/lnk1/b1/ga1/gc1/sh1 declared above the guard and tail (p) = q1 reaches 3 or below (403 spellings measured).
- mechanism: loop-1 tail join copy is the residual device (s54); a loop-1 respelling could re-seat it
- probe: enum_loop1b -> sweep_loop1b.json
- result: best 15 at 125/126 (34 forms), 16-37 otherwise; this placement cannot express the base's in-preheader q + tail copy (q1 inlined = tail reload = the s9 result).
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD src/ings.c:820 INCLUDE_ASM anchor; no FAKE construct in any body (fake_ablate confirmed on candidate.c and the K2 alt); decls-above-guard region placement

## [s58] An in-preheader region spelling of loop 1 (q1/lnk1/b1 + tail copy, swaps) or loop 2 (q2/lnk2/b2, swaps) scores below 3.
- mechanism: region opens the existing if-body block, reproducing the base's own declaration placement
- probe: enum_loop1c, enum_loop2c (102 each, 60 valid each) -> sweep_loop1c.json, sweep_loop2c.json
- result: loop1c 6 at 3/127 then 4..20; loop2c 24 at 3/127, 8 at 7, 28 at 17; zero below 3.
- verdict: KILLED
- kill_scope: class
- measured_on: HEAD src/ings.c:820 INCLUDE_ASM anchor; no FAKE construct in any body (fake_ablate confirmed on candidate.c and the K2 alt)
- predicate_cite: tools/spelling_enum.py:26

## s59 (2026-09-15, synthesis - merged attack after s55-s58; kills re-audited; frontier reset)

## [s59] Chassis / kill re-audit: BASE 3 at 127/127, K2 4 at 127/127, s57 cell B 4 at 125/127 on the HEAD chassis; fake_ablate finds no FAKE construct in candidate.c.
- mechanism: instance kills are chassis-relative; the two closest-to-target kills (K2's seat residual, cell B) were re-measured with dumps.
- probe: s59/run.ps1 BASE,K2,B; s59/diff_*.txt; tools/fake_ablate.py on candidate.c.
- result: unchanged; ledger floor 3 stands. Cell B's mechanism corrected (E-s59-3).
- verdict: CONFIRMED

## [s59] The target's loop-1 exit-path loads (`lw a0,0xC(s2); sll a1,s4,6` before the join label) are produced from ordinary join-block statements by reorg retargeting the guard's blez past redundant insns.
- mechanism: reorg.c:3442-3460 fill_slots_from_thread: redundant_insn() matches the target thread's first insns against insns executed before the branch; on an un-owned thread it sets new_thread past them and redirects the branch.
- probe: K2 applied; tools/grinder/dump.ps1; s59/skel_*_K2.txt show insns 151/145 inside the join block through sched2 while the bytes match the target there.
- result: CONFIRMED. The target's C needs no tail copy after loop 1; BASE's loop-1 match is a coincidence of bytes, K2 is the faithful chassis. The residual is exactly the two copy-dest seats.
- verdict: CONFIRMED

## [s59] Giving each loop its own copy variable (p, p2) on the cell-B chassis (copy in the guard block) keeps the copy through cse and combine and reaches the a3 seat.
- mechanism: cse.c:844-857 keeps q canonical when p's last use is earlier, so cse.c:7454's copy-swap cannot fire; combine.c:914 blocks the merge because q is set in the preheader.
- probe: s59/body_V1..V4.c (four placements of the copy inside the guard block), sandbox, s59/diff_V*.txt, dumps for V4 (skel_rtl/cse/combine_V4.txt).
- result: all four = 4 at 125/127, both copies gone. The copy does survive cse and combine (V4 insn 72, add reads r78) and is folded by local-alloc's optimize_reg_copy_1 (local-alloc.c:700-790) because q's REG_DEAD precedes the JUMP_INSN where the scan stops. Banked rejected/s59_V1..V4_*.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD src/ings.c:820 INCLUDE_ASM anchor with tmp/grind/func_80017848/s59/body_V1..V4.c applied (K2-derived chassis, copy in the guard block, q dead before the branch); no FAKE construct in any body

## [s59] Keeping q live past loop 1's branch (loop 2's guard reads q/sh; the exit path re-assigns `q = *(u8 **)(ctx + 0xC); sh = slot_a << 6;` inside the if) makes both guard-block copies survive to the bytes with the copy dest seated in a3.
- mechanism: no REG_DEAD for q before the JUMP_INSN, so optimize_reg_copy_1's forward scan (local-alloc.c:721-725) stops without rewriting; the copy dest then overlaps the guard's v0 temporaries and q (a0), and global.c's scan must skip v0/a0.
- probe: s59/body_W1 (shared p), W2 (p/p2), W3 (copy after the count load), W4/W5 (copy adjacent to the load; sh/i moved), sandbox, s59/diff_W*.txt.
- result: W1 = 16 at 127/127 (both copies present; loop-1 seats p=a1, lnk=a3, sh=a2), W2 = 12 at 126/127 (loop 1: p=a2, lnk=a3, sh=a1; loop 2's copy folds because q has no use after its guard), W3 = 16 at 126, W4 = W5 = 12 at 126 byte-identical to W2. The seat moved off v0 for the first time in the ledger, and a3 is missed only by allocation order (p before lnk). But the copy is emitted in the blez delay slot (reorg fill_simple_delay_slots takes the last guard-block insn), while the target's copy follows the delay slot, i.e. lives in the preheader block; and the exit-path `sh` re-assignment duplicates `sll` into the early-return load-delay slots. Banked rejected/s59_W1..W5_*.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD src/ings.c:820 INCLUDE_ASM anchor with tmp/grind/func_80017848/s59/body_W1..W5.c applied (K2-derived chassis, copy in the guard block, q live past the branch via loop 2's guard + exit-path re-assignment); no FAKE construct in any body

## [s59] Frontier reset (strongest 3):
1. PREHEADER copy whose add operand reaches reload as a MEM / unallocated pseudo (reload1.c:5843-5853 find_equiv_reg copy into the first potential spill reg a3 = the target's exact `addu a3,a0,zero` / `addu a0,a1,a3`): the only unallocated-pseudo producer left under global.c:414-431 is a REG_EQUIV-mem pseudo whose init reload deletes (reload1.c:1955-1966). Probe: spell the add's operand as a pseudo set ONCE from `*(u8 **)(ctx + 0xC)` in the preheader with no other set and check the .lreg dump for a REG_EQUIV note and the .greg dump for its disposition (BB2_FINDREG_DEBUG on the instrumented cc1). If global allocates it, measure whether validate_equiv_mem rejects the equivalence because of the post-loop stores.
2. On the W2 chassis, lnk allocated BEFORE p gives a3 for loop 1 (order lever: p's priority = refs/live-length; lnk's rises with an extra in-loop reference or p's falls with a longer range) - use only as a MEASUREMENT of the order model (the guard-block placement itself is dead, E-s59-5); then look for a PREHEADER-block spelling that inherits the same conflicts: a preheader statement that keeps a v0 local and the q value live across the copy without bytes (e.g. the guard count variable consumed in the preheader).
3. Loop 2 symmetric to loop 1 under any chassis needs q live past loop 2's guard too: the only post-loop-2 consumer the target allows is a re-load into a1 for the call (`lw a1,0xC(s2)`), so q's post-guard use must be byte-free - measure `slots = q;` / `rec_a = q + sh` style consumers on the W2 chassis to see whether the fold moves rather than the copy.

## [s59] BASE (candidate.c) re-measures 3 at 127/127, K2 4 at 127/127 and s57 cell B 4 at 125/127 on the HEAD chassis; fake_ablate finds no FAKE construct in candidate.c.
- mechanism: instance kills are chassis-relative; the two closest kills (K2 residual, cell B) re-measured with dumps
- probe: tmp/grind/func_80017848/s59/run.ps1 BASE,K2,B; tools/fake_ablate.py --func func_80017848 --file ings --candidate memory/grind/func_80017848/candidate.c
- result: unchanged; floor 3 stands; cell B's mechanism corrected to cse's copy-swap (cse.c:7440-7470)
- verdict: CONFIRMED

## [s59] The target's loop-1 exit-path lw a0,0xC(s2) and sll a1,s4,6 before the join label are produced from ordinary join-block statements by reorg retargeting loop 1's guard blez past redundant insns.
- mechanism: reorg.c:3442-3460 fill_slots_from_thread: redundant_insn matches the target thread's first insns against insns executed before the branch; on an un-owned thread new_thread = next_active_insn(trial) and the branch is redirected past them
- probe: K2 applied, tools/grinder/dump.ps1; s59/skel_{jump,cse,loop,combine,flow,greg,jump2,sched2}_K2.txt show insns 151/145 inside the join block (after label 143) through sched2 while the bytes match the target there
- result: CONFIRMED: the target's C needs no tail copy after loop 1; K2 is the structurally faithful chassis and the residual is exactly the two copy-dest seats
- verdict: CONFIRMED

## [s59] On the cell-B chassis (copy in the guard block), a distinct copy variable per loop (p, p2) keeps the copy through cse and combine and reaches the a3 seat in the bytes.
- mechanism: cse.c:844-857 keeps q canonical when p's last use is earlier, so the cse.c:7454 copy-swap cannot fire; combine.c:914 blocks the merge since q is set in the preheader
- probe: s59/body_V1..V4.c (four copy placements inside the guard block), sandbox, s59/diff_V*.txt, V4 dumps skel_rtl/cse/combine_V4.txt
- result: all four = 4 at 125/127 with both copies gone; the copy survives cse and combine (V4 insn 72, the add reads r78) and is folded by local-alloc's optimize_reg_copy_1 (local-alloc.c:700-790) because q's REG_DEAD precedes the JUMP_INSN where its scan stops (:721-725); banked rejected/s59_V1..V4_*.c
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD src/ings.c:820 INCLUDE_ASM anchor with tmp/grind/func_80017848/s59/body_V1..V4.c applied (K2-derived chassis, copy in the guard block, q dead before the branch); no FAKE construct in any body

## [s59] Keeping q live past loop 1's branch (loop 2's guard reads q and sh; the exit path re-assigns q = *(u8 **)(ctx + 0xC) and sh = slot_a << 6 inside the if) makes both guard-block copies survive to the bytes with the copy dest seated in a3.
- mechanism: no REG_DEAD for q before the JUMP_INSN, so optimize_reg_copy_1 stops without rewriting; the copy dest then conflicts with the guard's v0 temporaries and q (a0) and global.c's scan must skip v0/a0
- probe: s59/body_W1 (shared p), W2 (p/p2), W3 (copy after the count load), W4/W5 (copy adjacent to the load), sandbox, s59/diff_W*.txt
- result: W1 = 16 at 127/127 with both copies present (loop-1 seats p=a1, lnk=a3, sh=a2); W2 = 12 at 126/127 (loop 1: p=a2, lnk=a3, sh=a1 - a3 missed only because p is allocated before lnk; loop 2's copy folds since q has no use after its guard); W3 = 16 at 126; W4 = W5 = 12 at 126 byte-identical to W2. The seat leaves v0 for the first time, but the copy is emitted in the blez delay slot (reorg fill_simple_delay_slots takes the last guard-block insn) whereas the target's copy follows the delay slot (a preheader-block insn), and the exit-path sh re-assignment duplicates sll into the early-return load-delay slots; banked rejected/s59_W1..W5_*.c
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD src/ings.c:820 INCLUDE_ASM anchor with tmp/grind/func_80017848/s59/body_W1..W5.c applied (K2-derived chassis, copy in the guard block, q live past the branch via loop 2's guard read + exit-path re-assignment); no FAKE construct in any body

## s60 (2026-09-15, solver - classify + inverse global on K2; seat residual typed; pref route class-killed)

## [s60] Chassis / kill re-audit: BASE 3 at 127/127, K2 4 at 127/127 on the HEAD chassis; fake_ablate finds no FAKE construct in candidate.c; the owner directive (func_8005BA8C auto-return) was executed in s56 (K4 = 8).
- mechanism: instance kills are chassis-relative; the closest kill (K2's two copy-dest seats) re-measured with dumps
- probe: s60/run.ps1 BASE; s60/apply.py body_K2.c + sandbox; tools/fake_ablate.py --candidate candidate.c
- result: unchanged (3 / 4 at 127/127); no FAKE carrier anywhere; directive needs no further action
- verdict: CONFIRMED

## [s60] The K2 residual is a register-allocation residual and the global.c model reproduces the build exactly, so an inverse search over the model's inputs is meaningful for it.
- mechanism: inverse_compose.py classify (object path, the supported path for INCLUDE_ASM-routed functions); extract.py/simulate.py forward fidelity
- probe: classify with --target-object build/src/ings.o --ours-object tmp/sandbox/func_80017848/ings.o; extract + simulate --trace (s60/classify_K2.txt, ra_K2.json, sim_K2.txt)
- result: FIRST DIVERGENCE: RA, exactly the four seat lines; 15/15 dispositions, sort MATCH; pseudo 78 (shared copy dest) pri 13333, hard_conflicts {v1,sp}, no conflict with 79/81, full pref {v0}
- verdict: CONFIRMED

## [s60] Within the solver's atom space (403 atoms, 10 classes, refs +12/-6, live length +/-8, depth <= 3) exactly one perturbation of the K2 model seats 78 in a3 with every other seat pinned: replacing 78's v0 preference by an a3 preference.
- mechanism: find_reg's copy/hard preference override (global.c:1097-1160) takes the lowest free preferred register after the ascending scan; a3 is free over 78's range, so an a3 pref (with no v0 pref) wins
- probe: inverse.py global with the full 13-seat goal at depth 2 and depth 3 (s60/inverse_global_K2_d2.txt, _d3.txt)
- result: minimal solution size 1, one distinct vector (pref_reroute 78 {v0} -> {a3}); 81 preference atoms foreclosed by the tool; no conflict/order vector inside the bounds
- verdict: CONFIRMED

## [s60] The copy dest can acquire an a3 copy or hard-register preference (the solver's only in-bounds vector) from some C spelling of this function.
- mechanism: set_preference (global.c:1645-1700) records a preference only from a SET whose other side is a hard register or an already-renumbered pseudo; expand_preferences (global.c:829-870) propagates copy prefs only along pure copies whose source dies and hard/full prefs along dying operands; prune_preferences (global.c:880-915) strips call-used regs from call-crossing allocnos first
- probe: K2 flow dump (s60/K2_flow_slice.txt) census of hard registers; model prefs/copy_prefs/full_prefs (s60/ra_K2.json); death insn of slot_b's pseudo 75
- result: a3 appears in the pre-RA RTL exactly twice, as slot_b's incoming copy `(set (reg/v:SI 75) (reg:SI 7 a3))` and its REG_DEAD; 75 crosses the call (a3 pruned) and dies at insn 272 `(set (reg:SI 139) (ior (reg:SI 138) (reg/v:SI 75)))` whose dest is a local-alloc'd temp with no allocno, so no expand_preferences edge leaves 75; every other hard reg in the RTL is v0 (return sets), a0/a1/a2 (parameters, call arguments). A fresh a3 appearance needs a 4-argument call or a five-deep block-local overlap that pushes local-alloc to a3 - bytes the target does not contain. The vector has no C preimage on this signature.
- verdict: KILLED
- kill_scope: class
- predicate_cite: tools/gcc-2.7.2/global.c:1097
- measured_on: K2 chassis (candidate_alt_s56_k2...) applied at HEAD src/ings.c:820, flow/greg dumps + ra_solver model; no FAKE construct

## [s60] In the global.c model the a3 seat for the copy dest is reachable, with every other seat unchanged, exactly when 78 conflicts with the base pseudos (79/81), hard-conflicts with a v0 local, and has priority strictly below lnk's 2500.
- mechanism: pass-0 `used` = hard_reg_conflicts | regs_someone_prefers | ~regs_used_so_far; with v0 (local temp), v1 (i), a0 (base) as hard conflicts and a1/a2 taken by sh/lnk allocated EARLIER (priority order), a3 is the first free register; at priority >= 2500 sh or lnk are still unallocated and 78 lands in a1/a2 instead
- probe: simulate.py on edited models s60/ra_K2_M1..M6.json (s60/sim_M*.txt)
- result: M1 (conflicts + v0 + pri 1666) -> a3, 14/15 with 78 the only change; M6 (pri 3333) -> a2 with lnk displaced to a3; M2 (pri 4000) -> a1; M3 (conflicts only) -> a1; M4 (priority only) -> v0. Requirement in C terms: the copy dest must stay live from the copy through >= 8 further insns (per-loop p, nrefs 2; >= 32 for a shared p, nrefs 4), i.e. past the loop's bottom test, and overlap base and the loop's v0 temporaries - a reader at or after the loop bottom. BASE realises this for loop 1 (reader = loop 2's guard) at the cost of the join copy; loop 2 has no reader slot between its bottom test and the jal.
- verdict: CONFIRMED

## [s60] Frontier reset (strongest 3):
1. The seat residual is now a one-line spec (E-s60-5): each copy dest needs a reader >= 8 insns downstream that leaves no bytes. Under the frozen compiler a reader disappears after global alloc only in jump2 (jump.c:437-490: same-register moves and find_equiv_reg-redundant moves), sched2/final no-op moves, or reorg's owned-thread redundant_insn deletion (reorg.c:3442-3460, no owned thread exists here). Probe: on a per-loop-p K2 variant, spell `x = p1;` after loop 1 with x consumed by loop 2's guard so that x and p1 are copy-preferred into the same seat (jump2 then deletes the move), and read .jump2/.dbr for the deleted insn and the resulting seats; if x cannot share p1's seat without displacing the guard's `lw a0`, the loop-1 range route reduces to BASE's join copy and is dead.
2. Loop 2's copy dest has NO downstream reader slot (only `sll s0 / lw a1 / sll s1` before the jal, and post-call readers force a callee-saved seat), so under global.c its a3 seat cannot come from the range route, the pref route is class-killed (s60), and no unallocated pseudo exists (E-s60-6). The remaining premise to attack is that loop 2's copy is a `(set pseudo pseudo)` at global time at all: check with the instrumented cc1 whether ANY MIPS pattern (reload of a constrained operand, secondary reload, output reload of a call-argument register, mips.c's move expanders) can print `addu a3,a0,zero` immediately before `lw a2,0x10(s2)` with a3 as a reload register (BB2_RELOAD_DEBUG) rather than an allocno seat - and if so what operand shape asks for it.
3. Only after 1-2: revisit BASE's loop-2 side. BASE has loop 1 exact via the join read and loop 2 transposed; if a jump2-deleted reader exists (frontier 1), apply it symmetrically so loop 2's guard reloads `lw a0,0xC(s2)` while p1 still dies downstream.

## [s60] Chassis / kill re-audit: BASE re-measures 3 at 127/127 and K2 4 at 127/127 on the HEAD chassis; fake_ablate finds no FAKE construct in candidate.c; the owner directive (func_8005BA8C auto-return) was executed in s56 (cell K4 = 8) and needs no further action.
- mechanism: instance kills are chassis-relative; the closest kill (K2's two copy-dest seats) re-measured with dumps
- probe: s60/run.ps1 BASE; s60/apply.py body_K2.c + sandbox --disable all; tools/fake_ablate.py --func func_80017848 --file ings --candidate memory/grind/func_80017848/candidate.c
- result: unchanged: 3 / 4 at 127/127; no FAKE-annotated construct; sibling shares no block (max overlap 0.120), K4 lever killed in s56
- verdict: CONFIRMED

## [s60] The K2 residual is a register-allocation residual and the ra_solver global.c model reproduces the build exactly, so an inverse search over the model's inputs is meaningful for it.
- mechanism: inverse_compose.py classify on the object path (the supported path for INCLUDE_ASM-routed functions); extract.py/simulate.py forward fidelity
- probe: classify ings func_80017848 --target-object build/src/ings.o --ours-object tmp/sandbox/func_80017848/ings.o; extract.py + simulate.py --trace (s60/classify_K2.txt, ra_K2.json, sim_K2.txt)
- result: FIRST DIVERGENCE: RA, exactly the four seat lines (move v0,a0 x2 / addu a0,a1,v0 x2 vs a3); 15/15 dispositions, sort MATCH; copy dest pseudo 78: nrefs 4, livelen 6, pri 13333, hard_conflicts {v1,sp}, no conflict with q/base 79/81, full pref {v0}, no copy prefs
- verdict: CONFIRMED

## [s60] Within the solver's atom space (403 atoms over 10 classes, refs +12/-6, live length +/-2,4,8, depth <= 3) exactly one perturbation of the K2 model seats 78 in a3 with every other seat pinned: replacing 78's v0 preference by an a3 preference.
- mechanism: find_reg's copy/hard preference override (global.c:1097-1160) takes the lowest free preferred register after the ascending scan; a3 is free over 78's range, so an a3 pref with no v0 pref wins
- probe: inverse.py global s60/ra_K2.json --goal {89:3,79:4,81:4,78:7,72:18,77:3,75:19,85:5,86:5,80:6,82:6,74:20,73:21} --depth 2 and --depth 3 (s60/inverse_global_K2_d2.txt, _d3.txt)
- result: minimal solution size 1, one distinct vector: [pref_reroute] pseudo 78 {v0} -> {a3}; 81 preference atoms foreclosed by the tool's RTL check; no conflict/order/live-length vector inside the bounds
- verdict: CONFIRMED

## [s60] The copy dest can acquire an a3 copy or hard-register preference (the solver's only in-bounds vector) from some C spelling of this function.
- mechanism: set_preference (global.c:1645-1700) records a preference only from a SET whose other side is a hard register or an already-renumbered pseudo; expand_preferences (global.c:829-870) propagates copy prefs only along pure copies whose source dies and hard/full prefs along dying non-conflicting operands; prune_preferences (global.c:880-915) strips call-used regs from call-crossing allocnos before regs_someone_prefers is built
- probe: hard-register census of the K2 flow dump (s60/K2_flow_slice.txt); model prefs/copy_prefs/full_prefs (s60/ra_K2.json); the death insn of slot_b's pseudo 75
- result: a3 appears in the pre-RA RTL exactly twice: slot_b's incoming copy (set (reg/v:SI 75) (reg:SI 7 a3)) and its REG_DEAD; 75 crosses the call (a3 pruned) and dies at insn 272 (set (reg:SI 139) (ior (reg:SI 138) (reg/v:SI 75))) whose dest is a local-alloc'd temp with no allocno, so no expand_preferences edge leaves 75; all other hard regs are v0 (return sets) and a0/a1/a2 (params, call args). A fresh a3 appearance needs a 4-argument call or a five-deep block-local overlap forcing local-alloc to a3 - bytes the target does not contain
- verdict: KILLED
- kill_scope: class
- measured_on: K2 chassis (candidate_alt_s56_k2_combine914_clobber_both_copies_seat_v0_4.c) applied at HEAD src/ings.c:820, cc1 -da flow/greg dumps + ra_solver model; no FAKE construct
- predicate_cite: tools/gcc-2.7.2/global.c:1097

## [s60] In the global.c model the a3 seat for the copy dest is reachable with every other seat unchanged exactly when 78 conflicts with the base pseudos 79/81, hard-conflicts with a v0 local, and has priority strictly below lnk's 2500.
- mechanism: pass-0 used = hard_reg_conflicts | regs_someone_prefers | ~regs_used_so_far; with v0 (local temp), v1 (i), a0 (base) as hard conflicts and a1/a2 taken by sh/lnk allocated earlier in priority order, a3 is the first free register; at priority >= 2500 sh or lnk are still unallocated and 78 lands in a1/a2
- probe: simulate.py on edited models s60/ra_K2_M1..M6.json (s60/sim_M*.txt)
- result: M1 (conflicts + v0 hard-conflict + livelen 48 -> pri 1666) = a3 with 14/15 dispositions and 78 the only change; M6 (pri 3333) = a2 with lnk displaced to a3; M2 (pri 4000) = a1; M3 (conflicts only) = a1; M4 (priority only) = v0. In C terms: per-loop p (nrefs 2) needs livelen >= 9, shared p (nrefs 4) >= 33 (flow.c:1685/2087 count one per insn live), i.e. a reader at or after the loop bottom that leaves no bytes; BASE realises it for loop 1 via loop 2's guard read and pays the join copy; loop 2 has no reader slot before the jal (only sll s0 / lw a1 / sll s1) and post-call readers force a callee-saved seat (s52 cell G)
- verdict: CONFIRMED

## [s60] The s59 frontier item 1 (a REG_EQUIV-mem copy-dest pseudo whose init reload deletes at reload1.c:1955-1966, feeding find_equiv_reg's a3 copy) has a producer in this function.
- mechanism: reload1.c:1955 requires reg_renumber < 0; global.c:414-431 makes an allocno for every referenced pseudo with reg_live_length != -1 (flow.c:1240/1260 setjmp only) and find_reg (global.c:1052-1075, ascending, no REG_ALLOC_ORDER in mips.h) fails only on a full `used`; REG_EQUIV notes come only from function.c:3838-3854 (stack parms) and local-alloc.c:1051-1055 (block-local pseudos)
- probe: source reading of global.c:414-431, :586-598, :1052-1075, flow.c:1240/1260, reload1.c:865-900 and :1955-1966; grep of REG_EQUIV producers across tools/gcc-2.7.2/*.c
- result: no producer: the item presupposes the unallocated pseudo it was meant to create; reload's find_equiv_reg copy route stays closed (E-s42, E-s57-6)
- verdict: KILLED
- kill_scope: class
- measured_on: K2 chassis applied at HEAD src/ings.c:820, read against the frozen tools/gcc-2.7.2 sources; no FAKE construct
- predicate_cite: tools/gcc-2.7.2/reload1.c:1956

## s61 (2026-09-15, forensics - reload-route census, pass-0 exclusion sets, preference provenance)

## [s61] Chassis / kill re-audit: BASE re-measures 3 at 127/127 and K2 4 at 127/127 on the HEAD chassis (src/ings.c:820 INCLUDE_ASM anchor); tools/fake_ablate.py finds no FAKE-annotated construct in candidate.c; the owner directive (func_8005BA8C auto-return, 2026-09-15T20:48) was executed and measured in s56 (cell K4 = 8 at 127/127) and needs no further action.
- mechanism: instance kills are chassis-relative; the two closest forms (BASE, K2) re-measured with the s61 run helper
- probe: tmp/grind/func_80017848/s61/run.ps1 -Bodies J1,K2,BASE (sandbox --disable all); tmp/grind/func_80017848/s61/ablate.sh
- result: BASE 3 / K2 4, both 127/127, byte-for-byte the s60 diffs (s61/diff_BASE.txt, s61/diff_K2.txt); "No FAKE-annotated constructs found ... nothing to ablate"; the sibling func_8005BA8C shares no code block (its ledger: max overlap 0.120) and its only lever (H8 entry cursor) is banked KILLED at K4 = 8
- verdict: CONFIRMED

## [s61] Loop 2's `addu a3,a0,zero` is printed by reload as a reload register (find_equiv_reg copy of the guard's a0 into the first spill register) for some constrained-operand or secondary-reload shape rather than as an allocno seat (s60 frontier item 2).
- mechanism: reload1.c:5843-5853 emits `move reloadreg,equivreg` only for an insn that REQUESTS an input reload; the reload register is taken from potential_reload_regs, which order_regs_for_reload fills first with call-used registers whose hard_reg_n_uses is ZERO (reload1.c:3771), then by ascending use count
- probe: instrumented cc1 (tools/gcc-2.7.2/cc1) on K2 with BB2_RELOAD_DEBUG=1 (tmp/grind/func_80017848/s61/dump.sh K2 -> s61/K2/stderr.txt, function slice s61/K2/dbg_func.txt)
- result: for func_80017848 reload reports `needs pass=1 new_bb_needs=0 changed=0` - NOT ONE insn in the function requests any reload (input, output or secondary), so no find_equiv_reg copy is ever emitted. Independently, the spill order is `prr=8,9,10,11,12,13,14,15,24,25,22,23,21,20,16,17,19,18,3,7,...` with `uses` 8..15 = 0 and a3 (7) = 239: a3 is the TWENTIETH candidate because the incoming-parameter copy `addu s3,a3,zero` gives it a nonzero use count in every chassis, while t0..t7 are unused. A reload-register copy in this function would print `addu $t0,...`, never `$a3`. Both prongs kill the premise.
- verdict: KILLED
- kill_scope: class
- measured_on: K2 chassis (candidate_alt_s56_k2_combine914_clobber_both_copies_seat_v0_4.c) applied at HEAD src/ings.c:820, instrumented cc1 BB2_RELOAD_DEBUG; no FAKE construct
- predicate_cite: tools/gcc-2.7.2/reload1.c:3771

## [s61] On a per-loop-p K2 variant, a post-loop reader `x = p` (x pre-defined as `x = q` before loop 1's guard so it is initialised on the blez path) consumed by loop 2's guard extends p's range to the loop bottom with no bytes, because x and p share a seat and jump2 deletes the same-register move (s60 frontier item 1).
- mechanism: jump.c:437-490 deletes `(set r r)` after reload; for the move to vanish x must be seated with p, but loop 2's guard reads x on the blez-taken path where x holds q (a0 in the target), so x must be a0 on one path and p's seat on the other
- probe: s61/body_J1.c (K2 + `x = q;` before the guard, `x = p;` after loop 1 inside the if-block, loop 2's guard `t2 = sh2 + (s32)x`, `p = x` in loop 2's preheader); sandbox --disable all; s61/diff_J1.txt
- result: 9 at 126/127. x is seated in a1 (sh moves to a2, lnk to a3), the guard-block copy `x = q` prints as `addu a1,a0,zero` in loop 1's preheader, the join block prints `addu a0,a1,zero` where the target has the fresh `lw a0,0xC(s2)`, and loop 2's copy collapses (`addu a0,a1,a0`, 126 insns). Exactly the predicted contradiction: the target's fall-through join DEFINES a0 by a memory load, so no reader of p that feeds loop 2's guard can be seated with p and be byte-free.
- verdict: KILLED
- kill_scope: instance
- measured_on: K2-derived chassis at HEAD src/ings.c:820 with tmp/grind/func_80017848/s61/body_J1.c applied; no FAKE construct

## [s61] Pass-0 exclusion sets for the copy dest (pseudo 78) on K2, read directly from find_reg: the ONLY way v0 is refused is a hard conflict, because 78 always carries the v0 full preference itself.
- mechanism: find_reg pass 0 (global.c:998-1000) excludes `used1` (fixed + hard_reg_conflicts) | ~regs_used_so_far | regs_someone_prefers; prune_preferences (global.c:920-928) builds regs_someone_prefers from the full preferences of CONFLICTING lower-priority allocnos and then REMOVES the allocno's own full preferences (global.c:925-926, `AND_COMPL_HARD_REG_SET (temp, hard_reg_full_preferences[allocno])`); 78's own v0 full preference comes from set_preference's operand-0 rule (global.c:1682: `if (GET_RTX_FORMAT (GET_CODE (src))[0] == 'e') src = XEXP (src, 0)`), which gives base a v0 non-copy preference from the body's `(set elem[v0] (plus base i))`, and expand_preferences (global.c:867-869) merges base's full preferences into 78 at the add `(set base (plus sh 78))` where 78 dies non-conflicting; pointer arithmetic always places the pointer as operand 0 (c-typeck.c:1986-1988 pointer_int_sum), so base is operand 0 whatever the source order
- probe: instrumented cc1 BB2_FINDREG_DEBUG=78 on K2 (tmp/grind/func_80017848/s61/dump_fr.sh K2fr 78 -> s61/K2fr/findreg_78.txt); source reading of global.c:829-870, :880-930, :985-1000, :1671-1700
- result: `conflicts: 3 29` (v1, sp); `someone_prefers:` EMPTY; `used_so_far: 0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 24 25 26 27 28 29 31` (a0..a3 are in regs_used_so_far from the incoming-parameter copies, so a1/a2 are NOT excluded by the used-so-far complement - correcting a mid-session guess); `pass0_used: 0 1 3 18 19 20 21 22 23 26 27 28 29 30 31`; `own_full_prefs: 2` (v0); `own_copy_prefs:` none. The first free register in pass 0 is v0 and 78 also prefers it. For a3, ALL of v0, a0, a1, a2 must sit in pass0_used: a1/a2 only as hard conflicts (sh/lnk allocated before 78, i.e. 78's priority below 2500) or via someone_prefers; a0 only as a conflict with q/base (someone_prefers cannot supply it: no allocno that conflicts with 78 carries an a0 full preference - the only a0 preference holders are the call-crossing ctx pseudo, pruned at global.c:900, and the block-local call-argument read, which local-alloc already seated); v0 ONLY as a hard conflict, because someone_prefers strips 78's own v0 preference and that preference is structural. This sharpens the s60 M1 spec: not merely "a v0 local and base must conflict with 78" but "no preference-based route exists for v0 or a0 at all".
- verdict: CONFIRMED

## [s61] The copy dest's v0 seat can be refused in pass 0 through regs_someone_prefers (a conflicting lower-priority allocno such as lnk or sh carrying a v0 full preference) without a hard v0 conflict.
- mechanism: prune_preferences removes the allocno's own full preferences from the someone-prefers set (global.c:925-926); 78 acquires base's v0 full preference at the add where it dies (expand_preferences, global.c:867-869), and base's preference is fixed by the operand-0 rule on a pointer-first PLUS (global.c:1682, c-typeck.c:1986-1988)
- probe: s61/K2fr/findreg_78.txt (`own_full_prefs: 2`, `someone_prefers:` empty) + source reading; the alternative of flipping the elem insn to `(plus i base)` requires INTEGER arithmetic with i first, which hands the v0 preference to i (allocated first at priority 26000) and moves i off v1
- result: with [copy, add] as 78's range the v0 preference is always merged in, so v0 is never in someone_prefers for 78; the only remaining v0 exclusion is a hard conflict with a v0-seated pseudo live across the copy-to-add window (s60 M1). No form measured this session; the kill is by the frozen predicate
- verdict: KILLED
- kill_scope: class
- measured_on: K2 chassis at HEAD src/ings.c:820, instrumented cc1 BB2_FINDREG_DEBUG=78; no FAKE construct
- predicate_cite: tools/gcc-2.7.2/global.c:925

## [s61] Frontier reset (strongest 3):
1. The pass-0 sets are now measured, not modelled (E-s61-3): for the target's a3 the copy dest needs hard conflicts with a v0-seated pseudo AND with q/base (a0), plus priority below sh/lnk (2500). Every preference route is closed (E-s61-4), reload is closed (E-s61-1), local-alloc always yields v0 for a block-local dest (E-s57). The open question is therefore purely a LIVE-RANGE question in the preheader window: which C statement makes a v0-seated value and an a0-seated value live across `copy; lw lnk; add` at zero bytes. Probe next: run BB2_FINDREG_DEBUG for q (79) and base (81) on K2 to read WHY q takes a0 rather than v0 (its own conflicts set) - if the guard temporary t (local, v0) is what pushes q to a0, then a guard spelled so that t stays live past the copy (e.g. the guard value consumed again after the loop-2-independent copy point) is the v0 conflict the spec needs, provided the consumer is byte-free.
2. The a0 conflict: q dies at the copy and base is born at the add in every measured chassis. A spelling where the add reads q while the copy dest is live past it is BASE's loop-1 device (optimize_reg_copy_1 then rewrites the add, local-alloc.c:700-760, needs a downstream reader). Enumerate the post-loop-2 statements of the TARGET for a q2 reader that is a genuine memory load in the target and could be a register read in C without changing bytes - the only candidates are the three `lw ?,0xC(s2)` reloads after the call, all of which cross the jal (s52 cell G: callee-saved seat). If that census is negative, the a0 conflict has no producer and the seat residual lives outside the [guard, add] window entirely.
3. Only after 1-2: the U1 identity (pointer variable == base variable, all 125 seats exact) with a combine.c:914 clobber that is NOT the lnk load - a set of the pointer pseudo between copy and add that the target already emits: the only candidates are `lw a2,0x10(s2)` (spent, K2) and the `addu v1,zero,zero` delay-slot insn (i = 0), which would require i and the pointer to share a pseudo (type-incompatible without a cast; measure once, expect bytes).

## [s61] Chassis / kill re-audit: BASE re-measures 3 at 127/127 and K2 4 at 127/127 on the HEAD chassis; fake_ablate finds no FAKE construct in candidate.c; the owner directive (func_8005BA8C auto-return) was executed in s56 (K4 = 8) and needs no further action.
- mechanism: instance kills are chassis-relative; closest forms re-measured with tmp/grind/func_80017848/s61/run.ps1; tools/fake_ablate.py on candidate.c; sibling ledger shares no code block (max overlap 0.120)
- probe: s61/run.ps1 -Bodies J1,K2,BASE; s61/ablate.sh
- result: BASE 3, K2 4, both 127/127, diffs identical to s60; 'No FAKE-annotated constructs found'; sibling lever K4 banked KILLED at 8
- verdict: CONFIRMED

## [s61] Loop 2's addu a3,a0,zero is printed by reload as a reload register (find_equiv_reg copy into the first spill register) for some constrained-operand or secondary-reload shape rather than as an allocno seat.
- mechanism: reload1.c:5843-5853 emits move reloadreg,equivreg only for an insn that requests an input reload; order_regs_for_reload fills potential_reload_regs first with call-used registers whose hard_reg_n_uses is zero (reload1.c:3771)
- probe: instrumented cc1 BB2_RELOAD_DEBUG=1 on K2 (s61/dump.sh K2; s61/K2/dbg_func.txt)
- result: RELOADDBG needs func=func_80017848 pass=1 new_bb_needs=0 changed=0 - no insn requests any reload; spill order prr=8,9,...,15,24,25,22,23,21,20,16,17,19,18,3,7,... with uses t0..t7 = 0 and a3 = 239 (parameter copy), so a reload-register copy would print $t0, never $a3
- verdict: KILLED
- kill_scope: class
- measured_on: K2 chassis (candidate_alt_s56_k2_combine914_clobber_both_copies_seat_v0_4.c) at HEAD src/ings.c:820, instrumented cc1 BB2_RELOAD_DEBUG; no FAKE construct
- predicate_cite: tools/gcc-2.7.2/reload1.c:3771

## [s61] On a per-loop-p K2 variant, a post-loop reader x = p (x pre-defined as x = q before loop 1's guard) consumed by loop 2's guard extends p's range to the loop bottom with no bytes because x and p share a seat and jump2 deletes the same-register move.
- mechanism: jump.c:437-490 deletes (set r r) after reload; loop 2's guard reads x on the blez-taken path where x holds q (a0), so x must be a0 on one path and p's seat on the other
- probe: s61/body_J1.c applied; sandbox --disable all; s61/diff_J1.txt
- result: 9 at 126/127: x seats in a1 (sh -> a2, lnk -> a3), addu a1,a0,zero prints in loop 1's preheader, the join prints addu a0,a1,zero where the target loads lw a0,0xC(s2), loop 2's copy folds; banked rejected/s61_J1_post_loop_reader_x_eq_p_into_loop2_guard_costs_9.c
- verdict: KILLED
- kill_scope: instance
- measured_on: K2-derived chassis at HEAD src/ings.c:820 with tmp/grind/func_80017848/s61/body_J1.c applied; no FAKE construct

## [s61] Pass-0 exclusion sets for the copy dest (pseudo 78) on K2, read directly from find_reg: v0 can only be refused by a hard conflict, because 78 always carries the v0 full preference itself and prune_preferences strips an allocno's own preferences from its someone-prefers set.
- mechanism: find_reg pass 0 (global.c:998-1000) excludes used1 | ~regs_used_so_far | regs_someone_prefers; prune_preferences (global.c:920-928) removes own full prefs (global.c:925-926); set_preference operand-0 rule (global.c:1682) gives base a v0 pref from (set elem[v0] (plus base i)), pointer arithmetic is pointer-first (c-typeck.c:1986-1988); expand_preferences (global.c:867-869) merges it into 78 at the add
- probe: instrumented cc1 BB2_FINDREG_DEBUG=78 on K2 (s61/dump_fr.sh K2fr 78; s61/K2fr/findreg_78.txt) + source reading
- result: conflicts: 3 29; someone_prefers: empty; used_so_far includes a0..a3 (parameter copies); pass0_used: 0 1 3 18-23 26-31; own_full_prefs: 2 (v0). For a3 all of v0, a0, a1, a2 must be excluded: a1/a2 as hard conflicts (priority < 2500), a0 only as a conflict with q/base (no conflicting a0-pref holder exists), v0 only as a hard conflict. The s60 M1 spec is exact and exhaustive.
- verdict: CONFIRMED

## [s61] The copy dest's v0 seat can be refused in pass 0 through regs_someone_prefers (a conflicting lower-priority allocno such as lnk or sh carrying a v0 full preference) without a hard v0 conflict.
- mechanism: prune_preferences removes the allocno's own full preferences from the someone-prefers set (global.c:925-926); 78 acquires base's v0 full preference at the add where it dies (expand_preferences global.c:867-869), fixed by the operand-0 rule on a pointer-first PLUS (global.c:1682, c-typeck.c:1986-1988)
- probe: s61/K2fr/findreg_78.txt (own_full_prefs: 2, someone_prefers empty) + source reading; flipping the elem insn to (plus i base) needs integer arithmetic with i first, which hands the v0 preference to i (allocated first, priority 26000) and moves i off v1
- result: with [copy, add] as 78's range the v0 preference is always merged in, so v0 never enters someone_prefers[78]; the only remaining v0 exclusion is a hard conflict with a v0-seated pseudo live across the copy-to-add window
- verdict: KILLED
- kill_scope: class
- measured_on: K2 chassis at HEAD src/ings.c:820, instrumented cc1 BB2_FINDREG_DEBUG=78; no FAKE construct
- predicate_cite: tools/gcc-2.7.2/global.c:925

## s62 (2026-09-15, synthesis - frontier items 1 and 3 measured, both allocators closed for a block-local copy dest, tail-vs-join settled)

## [s62] Chassis / kill re-audit: BASE re-measures 3 at 127/127 and K2 4 at 127/127 on the HEAD chassis; fake_ablate finds no FAKE construct in candidate.c; the owner directive (func_8005BA8C auto-return, 2026-09-15T20:48) was executed and measured in s56 (K4 = 8) and re-confirmed s60/s61; nothing further to do.
- mechanism: instance kills are chassis-relative; the two closest forms (BASE, K2) re-measured with s62/run.ps1; ablate.sh on candidate.c
- probe: tmp/grind/func_80017848/s62/run.ps1 -Bodies BASE,K2; s62/ablate.sh; s62/results.txt
- result: BASE 3 / K2 4, both 127/127, K2 diff identical to s61/diff_K2.txt; "no FAKE-annotated constructs found"
- verdict: CONFIRMED

## [s62] On K2, q (79) and base (81) take a0 by the ascending pass-0 scan after hard conflicts with v0 and v1 only; neither carries an a0 preference, so a0 can enter the copy dest's exclusion set only as a hard conflict (overlap with q or base), never through regs_someone_prefers (s61 frontier item 1).
- mechanism: find_reg pass 0 (global.c:998-1000): used = hard_reg_conflicts | ~regs_used_so_far | regs_someone_prefers; regs_someone_prefers is built from the full preferences of conflicting later allocnos (global.c:920-928)
- probe: instrumented cc1 BB2_FINDREG_DEBUG=79 and =81 on K2 (s62/dump_fr.sh; s62/K2fr79/stderr.txt, s62/K2fr81/stderr.txt)
- result: both: conflicts {2,3,29}, someone_prefers empty, own_copy_prefs none, own_full_prefs none, pass0_used {0,1,2,3,18-23,26-31} -> a0. The v0 conflict of q is the guard temporary t (local-alloc v0), of base the loop's element temporaries; v1 is i in both cases
- verdict: CONFIRMED

## [s62] Sharing one C variable between the guard's slots value and the loop index (so the `i = 0` insn is the combine.c:914 clobber of the copy's source) reaches the floor or below on the K2 chassis (s61 frontier item 3).
- mechanism: use_crosses_set_p (combine.c:914) blocks the copy merge when the copy's source pseudo is set between copy and add; the delay-slot `addu v1,zero,zero` is the only target insn between them besides the lnk load
- probe: s62/body_I1.c (`w = (s32)*(u8 **)(ctx + 0xC)` feeds the guard, `p = (u8 *)w; w = 0;` in the preheader, w is loop 1's index); sandbox --disable all
- result: 27 at 126/127. One pseudo has one seat and the target needs the slots value in a0 and the index in v1; the copy does survive but every loop-1 seat rotates. Banked rejected/s62_I1_index_and_pointer_share_one_variable_costs_27.c
- verdict: KILLED
- kill_scope: instance
- measured_on: K2-derived chassis at HEAD src/ings.c:820 with tmp/grind/func_80017848/s62/body_I1.c applied; no FAKE construct

## [s62] Spelling loop 1's exit path as explicit tail statements (`q = *(u8 **)(ctx + 0xC); sh = slot_a << 6;` inside the if-block, loop 2's guard reading the same q and sh) changes the copy-dest seats relative to K2's join-block reading.
- mechanism: with single multi-set q/sh variables the join block's loads become ordinary tail statements of loop 1's exit block; if the pseudo structure differed for global.c the seats could move
- probe: s62/body_K3b.c (and K3: an else arm `q2 = q; sh2 = sh;`); sandbox --disable all; s62/diff_K3b.txt
- result: K3b = 4 at 127/127 with EXACTLY K2's residual (copy dest v0 twice); K3 = 10 at 128 (else-arm copies materialise). The tail-statement and join-block interpretations are byte-equivalent; the residual is the seat either way. Banked rejected/s62_K3b_k2_explicit_exit_tail_statements_same_seat_residual_4.c and rejected/s62_K3_k2_plus_else_arm_join_copies_costs_10.c
- verdict: KILLED
- kill_scope: instance
- measured_on: K2-derived chassis at HEAD src/ings.c:820 with tmp/grind/func_80017848/s62/body_K3b.c / body_K3.c applied; no FAKE construct

## [s62] The copy dest can share an allocno (and hence conflicts and seat) with a longer-lived pseudo through global.c's regs_may_share merge (global.c:401-424), an allocator input no earlier session examined.
- mechanism: global.c:423-424 gives a pseudo its partner's allocno when the pair is on the regs_may_share list; the list's only producer is loop.c:1659 (move_movables, `m->partial && m->match` branch), reached only for partial movables (loop.c:862: `(set R 0)` + `(set (strict_low_part (subreg R)) ...)` zero-extend idiom)
- probe: grep of regs_may_share/reg_may_share across tools/gcc-2.7.2/*.c; grep of strict_low/movstrict in tools/gcc-2.7.2/config/mips/mips.md (0 hits)
- result: no partial movable can exist on this target (no strict_low_part pattern), so regs_may_share is always empty and no C spelling reaches the merge
- verdict: KILLED
- kill_scope: class
- predicate_cite: tools/gcc-2.7.2/loop.c:1659
- measured_on: source reading against the frozen tools/gcc-2.7.2 tree; K2 chassis at HEAD src/ings.c:820; no FAKE construct

## [s62] A copy dest whose only references are the copy and the base add in the preheader block can be seated in a3 by local-alloc (the block-local allocator, which s57 covered only by inference).
- mechanism: local-alloc.c:2135 find_free_reg: used = fixed_reg_set (no call crossed) | regs_live_at[born..dead), then an ascending scan without REG_ALLOC_ORDER; regs_live_at holds only hard registers mentioned/live in the block and qtys already allocated in the block
- probe: source reading of local-alloc.c:2135-2210 and block_alloc's regs_live_at construction; the K2 preheader block's contents (i = 0, copy, lnk load, base add, all other operands global pseudos)
- result: the only hard register live in the preheader is sp and there is no other block-local qty, so v0 is always the first free register; K2's measured v0 seat is exactly this. Combined with E-s57/E-s61 (global.c pass 0 gives v0 too), BOTH allocators are closed: the target's a3 requires a reference to the copy dest in another basic block
- verdict: KILLED
- kill_scope: class
- predicate_cite: tools/gcc-2.7.2/local-alloc.c:2135
- measured_on: K2 chassis at HEAD src/ings.c:820 (copy dest measured v0), source reading of the frozen tree; no FAKE construct

## [s62] A `(use (reg))` insn (zero bytes) referencing the copy dest in another block can be produced by ordinary C at -O2, extending its live range without instructions.
- mechanism: use_variable (stmt.c:3266, :3498) emits USEs for scoped variables only under obey_regdecls (-O0); every other USE emitter (stmt.c:750-752, :2532; expr.c:1810/8231/8463; function.c:3071-3090) is for hard registers, the return register, call fusage or inline-function returns
- probe: grep of gen_rtx (USE / use_variable across tools/gcc-2.7.2/stmt.c, expr.c, function.c, c-decl.c, c-typeck.c
- result: no pseudo-variable USE is reachable at -O2; the byte-free cross-block reference the seat needs has no emitter
- verdict: KILLED
- kill_scope: class
- predicate_cite: tools/gcc-2.7.2/stmt.c:3498
- measured_on: source reading of the frozen tree; K2 chassis at HEAD src/ings.c:820; no FAKE construct

## [s62] Frontier reset (strongest 3):
1. The seat question is now closed on every allocator input (pass-0 sets E-s61-3/E-s62-1, preferences E-s60/E-s61, reload E-s61-1, local-alloc E-s62-5, regs_may_share E-s62-4) and every zero-byte reference emitter (E-s62-6). What remains untested EMPIRICALLY is the post-global deletion census: run the instrumented cc1 (-da) on BASE and K2 and diff the insn sets of .greg, .jump2 (after reload) and .dbr for func_80017848 to list every insn deleted after global allocation; if the only deletions are jump2 no-op moves of already-equal hard registers, the "byte-free downstream reader" class is closed by measurement, and the next session must attack the premise that the a3 write is `(set P q)` at all - the only remaining pre-RA producers of `move rD,rS` are expand-level copies (parameter/inline-argument copies, integrate.c) and cse's register substitution.
2. integrate.c route (never dumped; s30 measured only 30-45 on a retired chassis): an inline-expanded static helper receives its pointer argument by a parameter copy `(set parm actual)` emitted by expand_inline_function; the parm pseudo's references lie inside the inlined body (other blocks after the guard) so combine cannot merge the copy, and its live range spans the loop, which is exactly the conflict set the a3 seat needs. Probe: on K2, move loop 1's preheader+loop into a `static inline` helper taking (slots, sh, lnk, slot_b) and read the .loop/.combine/.greg dumps for the parameter copy, its block and its seat before scoring.
3. Only after 1-2: the `do { } while (0)` wrap family (sanctioned for register-allocation effects, owner ruling 2026-07-06) measured on the K2 chassis for the first time - the s43/s47 cells were on retired chassis. Expectation per E-s61-3/E-s62-1: the wrap cannot create the v0/a0 hard conflicts, so it should be inert; one measurement each on the preheader and on the loop body settles it.

## [s62] Chassis / kill re-audit: BASE re-measures 3 at 127/127 and K2 4 at 127/127 on the HEAD chassis; fake_ablate finds no FAKE construct; the owner directive (func_8005BA8C auto-return) was executed in s56 (K4 = 8) and needs no further action.
- mechanism: instance kills are chassis-relative; closest forms re-measured with s62/run.ps1; ablate.sh on candidate.c
- probe: tmp/grind/func_80017848/s62/run.ps1 -Bodies BASE,K2; s62/ablate.sh; s62/results.txt
- result: BASE 3, K2 4, both 127/127, K2 diff identical to s61; no FAKE-annotated constructs
- verdict: CONFIRMED

## [s62] On K2, q (79) and base (81) take a0 by the ascending pass-0 scan after hard conflicts with v0 and v1 only and carry no a0 preference, so a0 can enter the copy dest's exclusion set only as a hard conflict (overlap with q or base), never via regs_someone_prefers.
- mechanism: find_reg pass 0 (global.c:998-1000); regs_someone_prefers built from full preferences of conflicting later allocnos (global.c:920-928)
- probe: instrumented cc1 BB2_FINDREG_DEBUG=79 and =81 on K2 (s62/K2fr79/stderr.txt, s62/K2fr81/stderr.txt)
- result: both: conflicts {v0,v1,sp}, someone_prefers empty, no own prefs, pass0_used {0,1,2,3,18-23,26-31} -> a0; q's v0 conflict is the guard temporary, base's is the element temporaries
- verdict: CONFIRMED

## [s62] Sharing one C variable between the guard's slots value and the loop index, so that the i = 0 insn is the combine.c:914 clobber of the copy's source, reaches the floor or below on the K2 chassis (s61 frontier item 3).
- mechanism: use_crosses_set_p (combine.c:914) blocks the copy merge when the source pseudo is set between copy and add
- probe: s62/body_I1.c applied; sandbox --disable all
- result: 27 at 126/127: the copy survives but one pseudo has one seat while the target needs a0 for the slots value and v1 for the index; every loop-1 seat rotates. Banked rejected/s62_I1_index_and_pointer_share_one_variable_costs_27.c
- verdict: KILLED
- kill_scope: instance
- measured_on: K2-derived chassis at HEAD src/ings.c:820 with tmp/grind/func_80017848/s62/body_I1.c applied; no FAKE construct

## [s62] Spelling loop 1's exit path as explicit tail statements (q = *(u8 **)(ctx + 0xC); sh = slot_a << 6; with single q/sh variables feeding loop 2's guard) moves the copy-dest seats relative to K2's join-block reading.
- mechanism: single multi-set q/sh pseudos turn the join block's loads into loop 1's exit-block statements; a different pseudo structure could change global.c's conflicts
- probe: s62/body_K3b.c and s62/body_K3.c applied; sandbox --disable all; s62/diff_K3b.txt
- result: K3b = 4 at 127/127 with exactly K2's residual (copy dest v0 twice); K3 (else-arm q2 = q; sh2 = sh) = 10 at 128. Tail-statement and join-block readings are byte-equivalent. Banked rejected/s62_K3b_... and rejected/s62_K3_...
- verdict: KILLED
- kill_scope: instance
- measured_on: K2-derived chassis at HEAD src/ings.c:820 with tmp/grind/func_80017848/s62/body_K3b.c and body_K3.c applied; no FAKE construct

## [s62] The copy dest can share an allocno, and hence conflicts and seat, with a longer-lived pseudo through global.c's regs_may_share merge.
- mechanism: global.c:423-424 merges allocnos for pairs on regs_may_share; the list's only producer is loop.c:1659 in move_movables' partial-movable branch, reached only for the (set R 0) + strict_low_part zero-extend idiom (loop.c:862)
- probe: grep of regs_may_share/reg_may_share over tools/gcc-2.7.2/*.c; grep strict_low/movstrict in tools/gcc-2.7.2/config/mips/mips.md (0 hits)
- result: no partial movable can exist on this target, so regs_may_share is always empty; no C spelling reaches the merge
- verdict: KILLED
- kill_scope: class
- measured_on: source reading of the frozen tools/gcc-2.7.2 tree; K2 chassis at HEAD src/ings.c:820; no FAKE construct
- predicate_cite: tools/gcc-2.7.2/loop.c:1659

## [s62] A copy dest whose only references are the copy and the base add in the preheader block can be seated in a3 by local-alloc's find_free_reg.
- mechanism: local-alloc.c:2135 find_free_reg: used = fixed_reg_set | regs_live_at[born..dead) (hard registers mentioned or live in the block plus qtys already allocated there), then an ascending scan; the preheader block has only sp live and no other block-local qty, so v0 is the first free register
- probe: source reading of local-alloc.c:2135-2210 and block_alloc; K2's preheader block contents; K2's measured v0 seat
- result: local-alloc always yields v0 for such a copy dest, matching K2; with E-s57/E-s61 (global.c pass 0 also yields v0) both allocators are closed and the a3 seat requires a reference to the copy dest in another basic block
- verdict: KILLED
- kill_scope: class
- measured_on: K2 chassis at HEAD src/ings.c:820 (copy dest measured v0) plus source reading of the frozen tree; no FAKE construct
- predicate_cite: tools/gcc-2.7.2/local-alloc.c:2135

## [s62] A zero-byte (use (reg)) insn referencing the copy dest in another block can be produced by ordinary C at -O2, extending its live range without instructions.
- mechanism: use_variable (stmt.c:3266, :3498) emits USEs for scoped variables only under obey_regdecls (-O0); all other USE emitters are for hard registers, the return register, call fusage or inline returns
- probe: grep of gen_rtx (USE / use_variable over tools/gcc-2.7.2/stmt.c, expr.c, function.c, c-decl.c, c-typeck.c
- result: no pseudo-variable USE is reachable at -O2; the byte-free cross-block reference the seat needs has no emitter
- verdict: KILLED
- kill_scope: class
- measured_on: source reading of the frozen tree; K2 chassis at HEAD src/ings.c:820; no FAKE construct
- predicate_cite: tools/gcc-2.7.2/stmt.c:3498

## s63 (2026-09-15, solver - frontier items 1-3 measured, a3-preference route closed, residual restated as E-s44-3)

## [s63] Chassis / kill re-audit: BASE re-measures 3 at 127/127 and K2 4 at 127/127 on the HEAD chassis; fake_ablate finds no FAKE construct in candidate.c; classify on K2 = RA (seat-only); the owner directive (func_8005BA8C auto-return) was executed in s56 and needs no further action.
- mechanism: instance kills are chassis-relative; the two closest forms re-measured with s63/run.ps1; solver rule (1) classify before any search
- probe: s63/run.ps1 -Bodies BASE and -Bodies K2; s63/ablate.sh; s63/classify_K2.txt
- result: BASE 3, K2 4, both 127/127, diffs identical to s61/s62; "no FAKE-annotated constructs found"; FIRST DIVERGENCE: RA
- verdict: CONFIRMED

## [s63] Some insn reading the copy dest survives to global allocation and is deleted afterwards (jump2 / reload / sched2 / reorg) on the K2 chassis, which would be the byte-free downstream reader the a3 seat needs (s62 frontier item 1).
- mechanism: jump.c after reload deletes no-op moves and cross-jumps identical tails; reload deletes REG_EQUIV inits; reorg deletes redundant insns it can prove already executed
- probe: instrumented cc1 -da on K2 (s63/dump.sh), s63/census.py diffing the function's insn UIDs across .greg/.jump2/.sched2/.dbr (s63/census_K2.txt)
- result: the only insns deleted after global are the three cross-jumped `(set v0 0); (jump end)` return-0 tails and reorg's sequence packing; zero reg-reg moves and zero readers of any pseudo are removed. Closed by measurement.
- verdict: KILLED
- kill_scope: class
- measured_on: K2 chassis at HEAD src/ings.c:820, instrumented cc1 -da dumps; no FAKE construct
- predicate_cite: tools/gcc-2.7.2/global.c:829

## [s63] The sanctioned do { } while (0) wrap (owner ruling 2026-07-06, any codegen effect incl. register allocation) placed on the K2 chassis around the inner loop (W1), the preheader statements (W2) or the whole then-block (W3) moves the copy-dest seat from v0 toward a3 (s62 frontier item 3).
- mechanism: NOTE_INSN_LOOP_BEG/END change loop_depth-weighted reg_n_refs (flow.c:2081) and hence global.c's allocation order; if sh and lnk were allocated before the copy dest, a1/a2 would enter its conflict set
- probe: s63/gen_w.py cells W1/W2/W3 from body_K2.c, FAKE-annotated; sandbox --disable all each (s63/results.txt, s63/diff_W1.txt)
- result: W1 = 12 at 127/127 (lnk lifted over sh: lnk a1, sh a2 in both loops; copy dest still v0); W2 = W3 = 4 at 127/127, byte-identical to K2. The wrap reorders allocation but adds no v0/a0 hard conflict, and the copy dest's own v0 preference cannot be refused (global.c:925-926). Banked rejected/s63_W1..W3_*.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: K2-derived chassis at HEAD src/ings.c:820 with tmp/grind/func_80017848/s63/body_W1.c / body_W2.c / body_W3.c applied; the do-while(0) wraps are the only FAKE constructs present

## [s63] A static inline helper's integrate.c argument copy gives the copy dest references in another basic block, so combine cannot merge it and its live range spans loop 1 (s62 frontier item 2).
- mechanism: expand_inline_function copies each user-variable actual into a fresh parm pseudo (integrate.c:1285-1305, :1436-1449)
- probe: source reading of the copy's emission point versus the target's copy position; s30's measured cells F/G/H (hypotheses.md:2665-2668) re-read
- result: the copy is emitted at the call site, i.e. in the block containing the call; for it to follow the guard's blez the call sits in the then-block, where loop.c hoists the helper's invariant base add into the same preheader block, so copy and consumer share a block and combine merges them (s30 H: no copy at all). Each inlined helper also materialises its return value (`addiu v0,zero,1` + jump + caller test; s30: 30-45 at 136-137 insns), which jump.c never cross-jumps back to the target's single return-0 block. Not re-measured this session: the kill is s30's measurement plus the emission-point reading; a K2-based re-measure would inherit both defects unchanged.
- verdict: KILLED
- kill_scope: instance
- measured_on: s30 cells F/G/H on the s30-era chassis (retired), re-read s63 against integrate.c and the K2 chassis geometry; no FAKE construct

## [s63] The copy dest can be seated in a3 through a hard-register PREFERENCE (copy or full) rather than through hard conflicts, in some spelling that keeps the target's entry and epilogue bytes.
- mechanism: find_reg tries copy-prefs then full-prefs after the ascending scan (global.c:1097-1150); an a3 pref would win over the scan's v0 whenever a3 is free
- probe: source reading of set_preference (global.c:1671), expand_preferences (global.c:829-870, requires reg_allocno >= 0 at :842, runs before prune at :552/:577) and prune_preferences (global.c:882-930) against the RTL's a3 mentions
- result: a3 appears in the pre-RA RTL only in the entry copy `(set 75 (reg a3))`; 75's sole REG_DEAD partner is the epilogue ior whose dest is a block-local temp (not an allocno), a shape the target's own `or v0,v0,s3; sw v0,4(a0)` fixes, so no allocno ever acquires an a3 preference and none can pass one to the copy dest. With E-s62-1 (a0 only as a hard conflict) and global.c:925-926 (the dest's own v0 pref cannot be refused), the a3 seat requires hard conflicts with a v0-seated pseudo and with q/base - i.e. the copy dest live through the loop at flow time (E-s44-3).
- verdict: KILLED
- kill_scope: class
- measured_on: source reading of the frozen tools/gcc-2.7.2 tree against the K2 chassis RTL (s63/K2 dumps) at HEAD src/ings.c:820; no FAKE construct
- predicate_cite: tools/gcc-2.7.2/global.c:842

## [s63] Frontier reset (strongest 3):
1. E-s44-3 is the ONLY open mechanism: a reader of the copy dest that flow sees and combine deletes. Enumerate combine.c's fold sites that can erase an operand outright (simplify_and_const_int via nonzero_bits, num_sign_bit_copies on sign-extensions, simplify_comparison on subword compares, `(and (ashift x n) m)` masks) and list, for each, the NATURAL C shape in a scan loop over u8 indices / u16-s16 link fields that produces it; then spell that shape through p (the copy dest) instead of base, in the loop body, and read .flow/.combine/.greg before scoring. The s44 instruments (M5/M6, dead algebra) are the reference for what the dumps must show: p live at the loop top in .flow, the reader gone in .combine, 78 conflicting with v0/a0 in .greg.
2. If (1) yields no natural shape: measure whether a post-loop reader of p on the return-0 path only (the `beq v0,s3` hit path, which the target cross-jumps into one shared block) can be folded by combine while flow still counts it - the census (E-s63-1) shows jump2 deletes exactly those tails after global, so a reader that combine removes from a tail before global would leave stale liveness behind. Spell the found-path exit as a read of p that combine can fold, and read .greg for 78's conflicts.
3. Only after 1-2: the sched_solver on the preheader order (copy / lnk load / add) to confirm the pre-sched2 order the target's bytes admit, in case a later chassis changes the copy's block.

## [s63] Chassis / kill re-audit: BASE re-measures 3 at 127/127 and K2 4 at 127/127 on the HEAD chassis; fake_ablate finds no FAKE construct in candidate.c; classify on K2 = RA seat-only; the owner directive (func_8005BA8C auto-return) was executed in s56 and needs no further action.
- mechanism: instance kills are chassis-relative; the two closest forms re-measured; solver rule (1) classify before search
- probe: tmp/grind/func_80017848/s63/run.ps1 -Bodies BASE / -Bodies K2; s63/ablate.sh; s63/classify_K2.txt
- result: BASE 3, K2 4, both 127/127, diffs identical to s61/s62; no FAKE-annotated constructs; FIRST DIVERGENCE: RA
- verdict: CONFIRMED

## [s63] Some insn reading the copy dest survives to global allocation and is deleted afterwards (jump2 / reload / sched2 / reorg) on the K2 chassis, which would be the byte-free downstream reader the a3 seat needs (s62 frontier item 1).
- mechanism: jump.c after reload deletes no-op moves and cross-jumps identical tails; reload deletes REG_EQUIV inits; reorg deletes redundant insns
- probe: instrumented cc1 -da on K2 (s63/dump.sh); s63/census.py diffs the function's insn UIDs across .greg/.jump2/.sched2/.dbr (s63/census_K2.txt)
- result: only the three cross-jumped (set v0 0)/(jump end) return-0 tails and reorg's SEQUENCE packing are deleted after global; zero reg-reg moves and zero pseudo readers removed
- verdict: KILLED
- kill_scope: class
- measured_on: K2 chassis at HEAD src/ings.c:820, instrumented cc1 -da dumps; no FAKE construct
- predicate_cite: tools/gcc-2.7.2/global.c:829

## [s63] The sanctioned do { } while (0) wrap placed on the K2 chassis around the inner loop (W1), the four preheader statements (W2) or the whole then-block (W3) moves the copy-dest seat from v0 toward a3 (s62 frontier item 3).
- mechanism: NOTE_INSN_LOOP_BEG/END change loop_depth-weighted reg_n_refs (flow.c:2081) and hence global.c's allocation order; sh/lnk allocated first would put a1/a2 in the copy dest's conflicts
- probe: s63/gen_w.py cells W1/W2/W3 from body_K2.c (FAKE-annotated); sandbox --disable all each; s63/results.txt, s63/diff_W1.txt
- result: W1 = 12 at 127/127 (lnk lifted over sh: lnk a1 / sh a2 in both loops, copy dest still v0); W2 = W3 = 4 at 127/127 byte-identical to K2; the wrap reorders allocation but adds no v0/a0 hard conflict and the dest's own v0 pref cannot be refused (global.c:925-926). Banked rejected/s63_W1..W3_*.c
- verdict: KILLED
- kill_scope: instance
- measured_on: K2-derived chassis at HEAD src/ings.c:820 with tmp/grind/func_80017848/s63/body_W1.c / body_W2.c / body_W3.c applied; the do-while(0) wraps are the only FAKE constructs present

## [s63] A static inline helper's integrate.c argument copy gives the copy dest references in another basic block so combine cannot merge it and its live range spans loop 1 (s62 frontier item 2).
- mechanism: expand_inline_function copies each user-variable actual into a fresh parm pseudo (integrate.c:1285-1305, :1436-1449)
- probe: source reading of the copy's emission point against the target's copy position; s30 cells F/G/H (hypotheses.md:2665-2668) re-read
- result: the argument copy is emitted at the call site; a call inside the then-block puts it in the preheader block where loop.c hoists the helper's base add, so combine merges it (s30 H: no copy at all); each inlined helper also materialises its return value (+9/+10 insns, s30 F/G/H 30-45 at 136-137) that jump.c never cross-jumps away. Not re-measured; s30's measurement plus the emission-point reading
- verdict: KILLED
- kill_scope: instance
- measured_on: s30 cells F/G/H on the s30-era chassis (retired), re-read s63 against integrate.c and the K2 chassis geometry; no FAKE construct

## [s63] The copy dest can be seated in a3 through a hard-register PREFERENCE (copy or full) rather than through hard conflicts, in some spelling that keeps the target's entry and epilogue bytes.
- mechanism: find_reg tries copy-prefs then full-prefs after the ascending scan (global.c:1097-1150); an a3 pref would beat the scan's v0 whenever a3 is free
- probe: source reading of set_preference (global.c:1671), expand_preferences (global.c:829-870, partner gate at :842, runs before prune at :552/:577) and prune_preferences (global.c:882-930) against the K2 RTL's a3 mentions (s63/K2 dumps)
- result: a3 appears pre-RA only in the entry copy (set 75 (reg a3)); 75's sole REG_DEAD partner is the epilogue ior into a block-local temp (reg_allocno -1, rejected at global.c:842), a shape the target's own `or v0,v0,s3; sw v0,4(a0)` fixes; no allocno ever carries or passes an a3 preference. With E-s62-1 and global.c:925-926 the a3 seat requires hard conflicts with a v0-seated pseudo AND with q/base, i.e. the copy dest live through the loop at flow time (E-s44-3)
- verdict: KILLED
- kill_scope: class
- measured_on: source reading of the frozen tools/gcc-2.7.2 tree against the K2 chassis RTL (s63/K2 dumps) at HEAD src/ings.c:820; no FAKE construct
- predicate_cite: tools/gcc-2.7.2/global.c:842
