> **2026-08-24 MIGRATION NOTE:** HEAD is now `INCLUDE_ASM` (migrated in
> a7892ba2 (2026-08-24 sweep 2)); rules retired, in-source cheat-asm removed. "HEAD"
> claims below describe the pre-migration tree (`retired-chassis-2026-08/body.c`).

# Evidence bank — func_800871D4

- WIP rejected_form: one-expr var_v1 = temp_a0 & 0xFFFF: GCC FOLDS the redundant andi (higher score)

- WIP rejected_form: two-stmt var_v1 = temp_a0; var_v1 &= 0xFFFF: andi appears but with extra move

- WIP rejected_form: declaration-order swap (var_a1 before var_a2): no effect on register assignment (score stays 10)

- == imported from memory/wip notes.md ==
# func_800871D4 — BLOCKED (cc1-vs-cc1psx andi-fold divergence CONFIRMED)

## TL;DR
Bit-flag setter in main.c: reads D_8010280A (u16), builds two shift masks, clears a
table slot, RMWs four flag globals. HEAD has 2 cheat-asm andi blocks + empty scheduling
barrier. cc1psx calibration (2026-06-16) CONFIRMS: cc1psx generates `andi $v1,$a0,0xffff`
from `var_v1 = temp_a0 & 0xFFFF` (no fold); our cc1 folds it. Best honest floor: score=10.

## Confirmed gap (10 Levenshtein diffs, oracle=52 insns, candidate=50 insns)
1. Oracle: `lui a0; lhu a0` — candidate: `lui v1; lhu v1` (2 load-register diffs)
2. Oracle has `andi v1,a0,0xffff` at pos 3 (missing from candidate, 1 delete)
3. Oracle has `andi v1,a0,0xffff` at pos 13 (restore after else, missing, 1 delete)
4. a1/a2 register swap throughout (6 diffs: var_a2 in a1, var_a1 in a2 in candidate vs oracle)

## cc1psx calibration results (2026-06-16)
- func_testA (temp_a0 + `& 0xFFFF`): cc1psx generates `andi $3,$4,0xffff` (NOT folded)
- func_testB (direct `var_v1 = D_8010280A`): cc1psx generates `lhu $3` (no andi, same as cc1)
- Divergence CONFIRMED: cc1psx does not fold andi from u16-bounded source; our cc1 does.

## Rejected forms
- `var_v1 = temp_a0 & 0xFFFF` (one-expr): our cc1 folds the andi completely
- `var_v1 = temp_a0; var_v1 &= 0xFFFF` (two-stmt): generates move+andi, not single andi
- declaration-order swap (var_a1 before var_a2): no effect on register assignment, score=10

## Best candidate (candidate.c, score=10, cheat-reviewer PASS)
Direct form: `var_v1 = D_8010280A` (no temp_a0, no mask). Semantics differ from oracle
in else-path (oracle restores var_v1 to original D_8010280A via second andi; direct form
leaves var_v1 = D_8010280A-16 for the post-if computation). Score is 10 (honest floor).

## Next step (Adjudicator)
Park with confirmed cc1psx divergence. The oracle requires cc1psx-specific behavior
(no andi fold from u16-bounded lhu) that our cc1 cannot reproduce in pure C.


- [s1] canonical func_800871D4 -> verdict=C, distance=10, total=52

- [s1] sandbox --disable all (candidate.c/direct form) -> score=10, target_insns=52, build_insns=50 (missing 2 insns = both andi $v1,$a0,0xFFFF)

- [s1] sandbox --disable all (m2c-shape with explicit &0xFFFF twice) -> score=12 (rejected/m2c-shape-explicit-andi-restore.c)

- [s1] Oracle structure: lhu $a0,%lo(D_8010280A); nop; andi $v1,$a0,0xFFFF; ...; else-arm ends with andi $v1,$a0,0xFFFF (re-materialize / restore). GCC keeps $a0 as master copy of loaded u16, $v1 as the masked working copy destroyed by -=0x10.

- [s1] m2c reconstruction shows explicit `var_v1 = D_8010280A & 0xFFFF` in the else arm (restore) — but this fold-defeats fail in our cc1 because RTL nonzero_bits proves the source is u16-clean.

- [s1] WIP inheritance already killed: (a) one-expr `temp_a0 & 0xFFFF` (folds), (b) two-stmt split (adds extra move), (c) decl-order swap (no effect).

- [s1] WIP cc1psx calibration (2026-06-16): cc1psx does NOT fold `temp_a0 & 0xFFFF` from u16 source — this is a fork-vs-cc1psx divergence. Per [[no-compiler-divergence]] this is informational only; cannot patch cc1.

- [s1] Cross-tree census (5 other BB2 funcs with the same shape, ALL INCOMPLETE) elevates this from single-function difficulty to a shared unresolved family across the codebase.

- [s1] Sibling func_80086BFC (main.c:1143, COMPLETED-C, PsyQ verbatim-linked note2pitch2) uses the same <16 / -=16 split-shift on a *s16* var_a3 with a local shift subexpression — no re-mask needed because s16 semantics differ.

- [s1] Function is a bitmask-flag setter with trivial control flow (single if/else + linear stores). Zero call sites in the function body. NO hand-coded-asm signals (S1/S2/S6 tier per scan_hand_coded); the construct is pure C in intent.

- [s2] [s2] sandbox --disable all with candidate.c (u32 var_v1, in-place -=0x10, zero cheat-asm) = score=10 target_insns=52 build_insns=50 (missing 2 andi insns) — re-confirmed s1's honest floor after applying candidate.c to src/main.c.

- [s2] [s2] sandbox --disable all with u16 var_v1 narrow-type variant = score=11 build_insns=52 (WORSE); banked rejected/u16-var-v1-narrow-type.c.

- [s2] [s2] scan_hand_coded --single func_800871D4 = tier=LOW score=0/8, no S1/S2/S6 strong signals; canonical-asm authorization decisively unavailable (Gate 1 FAIL).

- [s2] [s2] Cross-tree BB2 census (from s1): 5 sibling functions share the exact `lhu $r,GLOBAL; nop; andi $r2,$r,0xFFFF` shape, all INCOMPLETE; zero COMPLETED-C precedent in-tree (Gate 2 FAIL under standing 2026-07-27 ruling).

- [s2] [s2] cc1 combine.c folds `& 0xFFFF` on a u16-typed lhu result because nonzero_bits proves the source is 16-bit-clean; 2026-06-16 WIP calibration recorded cc1psx does NOT fold, but per no-compiler-divergence.md this is informational only (the compiler is frozen).

- [s2] [s2] Every non-cheat structural spelling of the initial read has been measured: direct u32 assign (floor=10, folds both andis), one-expr `temp_a0 & 0xFFFF` (folds), two-stmt split `var_v1=temp_a0; var_v1&=0xFFFF` (extra move, worse), m2c-shape explicit-andi twice (score=12), decl-order swap (no effect), u16 narrow-type (score=11).

- [s2] [s2] The remaining hypothesized levers from s1 frontier (H1 bitfield struct pun of &D_80102808; H2 alternative first-andi source crossed with in-place -=0x10) both reduce to a no-new-park-categories cheat-by-spelling: no semantic purpose in a bit-flag setter, GCC-internals-only justification, would not be written by a human programmer from the function's spec — not surfaced.

- [s2] [s2] Filed docs/grind/decisions.md entry `2026-07-28 — func_800871D4 — OWNER-ESCALATION — RESOLVED BY STANDING RULING (2026-07-27): REFUSED / OWNER-ACCEPTED INCOMPLETE` documenting both-gate failure and terminal disposition.

- [s3] CHASSIS RE-MEASURE: candidate.c (s2 direct-read form) re-applied to src/main.c -> sandbox --disable all score=10, target_insns=52, build_insns=50. The ledger's floor=10 was confirmed live before any probe.

- [s3] **THE ANDI MECHANISM IS SOLVED.** `x & 0xFFFF` on a u16-typed `lhu` result is folded by combine.c ONLY when the pseudo holding the load result has a SINGLE use. can_combine_p refuses to substitute the load insn into the AND insn when the load's destination register is still live afterwards, so simplify_and_const_int / nonzero_bits never runs on the combination and the mask is emitted as a real `andi`. Give the raw load TWO uses and BOTH masks survive. Measured: `temp_a0 = D_8010280A; var_v1 = temp_a0 & 0xFFFF; ... else { ...; var_v1 = temp_a0 & 0xFFFF; }` -> score=6, build_insns=52 == target_insns=52. This kills the s1/s2 conclusion that "our cc1 folds the andi and cc1psx does not" as the operative obstacle: our cc1 emits BOTH andis from pure C. The 2026-06-16 cc1psx-divergence finding was measuring a single-use spelling.

- [s3] The s1 rejected form `m2c-shape-explicit-andi-restore.c` (score 12) failed for a DIFFERENT reason than recorded: it wrote `var_a1 = 1 << (var_v1 - 0x10)` instead of modifying var_v1 in place, which made the else-arm restore a dead store that DCE removed. Combined with the single-use fold, both andis vanished. The in-place `var_v1 -= 0x10` is load-bearing: it is what makes the restore LIVE.

- [s3] At score=6 the instruction stream is STRUCTURALLY EXACT vs target (same opcodes, same order, both andis, correct delay slots). Verified by objdump of tmp/sandbox/func_800871D4/main.o and by tools/ra_solver/inverse_compose.py classify main func_800871D4, which reports "FIRST DIVERGENCE: IDENTICAL - the honest stream already equals target" (its comparison blanks registers). The entire residual is a REGISTER PERMUTATION: the two symmetric mask locals take each other's seats, $a1 <-> $a2.

- [s3] RA model extracted (tools/ra_solver/extract.py func_800871D4 main). At score=6 the four global allocnos are pseudo 73 = raw u16 load (temp_a0), 74 = var_v1, 75 = then-arm mask (paired with D_801078D8), 76 = else-arm mask (paired with D_801078DA). global.c order = `74 73 75 76`, dispositions 73->$a0, 74->$v1, 75->$a1, 76->$a2. Target requires 75->$a2 and 76->$a1, i.e. the order `74 73 76 75`. global.c sorts by floor_log2(n_refs)*n_refs*size/live_length: 75 has 3 refs / 19 insns (pri .158), 76 has 3 refs / 21 insns (pri .143), so 75 sorts first and takes the lower free register. Allocation is strictly ascending from the first free hard reg; there are no preference atoms (inverse.py reports 8 preference atoms FORECLOSED - $a1/$a2 never appear as hard regs in this function's pre-RA RTL, so global.c set_preference can never record a preference for them).

- [s3] `inverse.py global --swap 75,76` verdict: **REACHABLE, minimal solution size 1 atom, 18 distinct vectors.** The distinct atom families are: refs_down pseudo 75 (3->2 or 3->1); live_extend pseudo 75 (19->23 or 19->27); live_shrink pseudo 76 (21->17 or 21->13); refs_up pseudo 76 (3->4 .. 3->15). This is a solved specification, not a search.

- [s3] live_extend of pseudo 75 CONFIRMED as a working atom: hoisting `var_a2 = 0` out of the else arm to the declaration (`s32 var_a2 = 0;`) extends 75's live range across block 0 and flips the order to `74 73 76 75` with dispositions 75->6 ($a2), 76->5 ($a1) - **the target's seats exactly**. sandbox score = 3, build_insns = 53. Re-extraction of the model confirms the flip is the priority sort, not luck.

- [s3] The score-3 residual is ENTIRELY the +1 instruction the hoist costs. GCC's sched2 sinks the block-0 `move a2,zero` to immediately before the `beqz` (it has no successors inside block 0, so it ranks last), and reorg.c then claims it for the branch delay slot. The target's delay slot holds `addiu $v0,$zero,1`, which is what makes the constant 1 available to BOTH arms from one materialisation; with the delay slot occupied, `li v0,1` is emitted separately in each arm. Net: 53 insns, 3 edit-distance (delay-slot content + the duplicate li).

- [s3] Placement of the zero-store WITHIN block 0 is INERT: as a declaration initialiser (`s32 var_a2 = 0;`) and as a plain statement immediately before the `if` both give score=3 / build_insns=53 with byte-identical output. sched2's ranking makes the source position irrelevant. Banked rejected/block0-zero-hoist-steals-delay-slot.c - do not re-measure block-0 positions.

- [s3] Declaration-order swap of the two mask locals (var_a1 before var_a2) re-measured on the NEW 52-insn chassis: still completely inert (score stays 6). This reconfirms s0's finding on a different chassis - global.c priority, not declaration order, owns this seat.

- [s3] Then-arm statement-order swap (`var_a1 = 0;` before the shift) on the new chassis: score=8, build_insns=53 - the `j` delay slot is lost because the shift is no longer the arm's last instruction. Banked rejected/then-arm-stmt-order-swap-53insn.c.

- [s3] The 2026-07-28 docs/grind/decisions.md entry for this function (REFUSED / OWNER-ACCEPTED INCOMPLETE under the standing 2026-07-27 ruling) rested on two premises that are now MEASURED FALSE: (a) that our cc1 cannot emit the redundant andi from pure C, and (b) that the structural axis was exhausted at floor 10. The floor moved 10 -> 6 -> 3 this session with ordinary C and no annotation-bearing construct. A superseding note was appended to docs/grind/decisions.md.

- [s3] Chassis re-measured live before probing: the s2 candidate (direct read) still gives score=10, target_insns=52, build_insns=50.

- [s3] New floor this session: 3 (build_insns=53, target_insns=52), saved as memory/grind/func_800871D4/candidate.c.

- [s3] Intermediate chassis at score=6 has build_insns == target_insns == 52 and a structurally exact instruction stream - same opcodes, same order, both andis, correct delay slots; banked as rejected/dualuse-52insn-a1a2-seats-swapped.c because the two mask pseudos take each other's seats.

- [s3] tools/ra_solver/inverse_compose.py classify main func_800871D4 reports 'FIRST DIVERGENCE: IDENTICAL' on the score-6 chassis (its comparison blanks registers), independently confirming the residual is a pure register permutation.

- [s3] RA model (score-6 chassis): allocnos 73=raw u16 load, 74=var_v1, 75=then-arm mask (pairs with D_801078D8), 76=else-arm mask (pairs with D_801078DA); global.c order 74 73 75 76; dispositions 73->$a0, 74->$v1, 75->$a1, 76->$a2. Target needs order 74 73 76 75.

- [s3] inverse.py --swap 75,76 = REACHABLE, 1 atom, 18 vectors; 8 preference atoms FORECLOSED because $a1/$a2 never appear as hard regs in this function's pre-RA RTL.

- [s3] The score-3 form's RA model re-extracts to order 74 73 76 75 with dispositions 75->6 ($a2), 76->5 ($a1) - the target's seats exactly. The only residual is the +1 instruction: the block-0 zero-store steals the beqz delay slot from `addiu $v0,$zero,1`, so `li v0,1` is emitted in both arms instead of once.

- [s3] The s1 rejected m2c-shape form (score 12) failed because it wrote `1 << (var_v1 - 0x10)` rather than modifying var_v1 in place, making the else-arm restore a dead store DCE removed - not because 'cc1 folds both masks' as recorded.

- [s3] Block-0 POSITION of the zero-store is inert (declaration initialiser vs statement before the if are byte-identical); do not re-measure block-0 placement variants.

- [s3] The 2026-07-28 docs/grind/decisions.md terminal disposition for this function rested on two premises now measured false; a superseding note was appended to docs/grind/decisions.md this session. The function is grindable, not exhausted.

- [s3] src/main.c was restored to its HEAD INCLUDE_ASM state at end of session; all candidate/rejected forms live in memory/grind/func_800871D4/.

- [s4] CHASSIS RE-MEASURE: s3's candidate.c re-applied to src/main.c -> sandbox --disable all score=3, target_insns=52, build_insns=53. The ledger floor was confirmed live before any probe.

- [s4] **FLOOR 3 -> 0. The function is MATCHED in pure C.** sandbox --disable all score=0, build_insns == target_insns == 52, rules_dropped=0, and `verify-oracle` full build SHA1 == 62efab4f73f992798c43e8c730aa43baa10bb4fa (build_matches true) with the C body in src/main.c. Zero regfix/asmfix rules, zero inline asm, zero annotation-bearing construct.

- [s4] THE CLOSING LEVER WAS THE FOURTH inverse.py ATOM FAMILY, not the two the s3 frontier named. s3 recorded `live_shrink pseudo 76: 21->17` as "unreachable within the target's fixed instruction stream" and told future sessions to spend nothing on it. That was wrong, and the reason is instructive: s3's analysis treated the block-3 death point of the else-arm mask as "fixed by the target's ordering of the two RMW groups", but the target's ordering constrains the EMITTED instruction order, not the SOURCE order - sched1/sched2 are free to restore it. Interleaving the two read-modify-write groups at source level (both key-off loads, then both key-on loads, then both ORs, then both stores, then both masked write-backs) moves the else-arm mask's last reference ~5 RTL insns earlier, drops its live_length below the then-arm mask's, and flips global.c's allocno_compare ordering - with NO block-0 store, hence no extra instruction. Measured on the s3 score-6 chassis: score 6 -> 0, build_insns 52.

- [s4] The interleave must be COMPLETE. Two partial interleaves both stay at score 6 / 52 insns: moving only the D_801078DA `or` up next to the D_801078D8 `or` (rejected/s4-partial-interleave-da-or-only.c), and moving the DA `or` above the D_800F1B10 load (rejected/s4-partial-interleave-da-or-before-load.c). The allocno's live_length is set by its LAST reference, so the DA store and the D_800F1B12 write-back have to move up too.

- [s4] The `& 0xFFFF` masks inherited from s3 are load-bearing on the interleaved chassis and no type-level spelling replaces them. Measured: no masks at all, plain `vc = raw;` copies = score 6 / 53 insns (rejected/s4-no-mask-plain-copies.c); `u16 raw` + `u16 vc` narrow locals with no mask = score 14 / 55 insns, GCC 2.7.2's PROMOTE_MODE re-truncates after the in-place `vc -= 0x10` (rejected/s4-u16-locals-promote-mode-truncations.c); `u32 raw` + `u16 vc` type-level narrowing = score 7 / 54 insns (rejected/s4-u32-raw-u16-vc-narrow.c).

- [s4] IN-TREE PRECEDENT for the `u16` load + `u32 x = value & 0xFFFF` shape: `SpuGetVoiceVolume` at src/main.c:2884 and src/main.c:2891 (`temp_a3 = temp_v1 & 0xFFFF;` on a `u16` loaded from `*(u16 *)(temp)`), and `_SsVmVSetUp` at src/main.c:1250 (`if ((a0 & 0xFFFFu) >= 0x10)`). Both carry zero regfix/asmfix rules, are absent from engine/queue.json and inline_asm_canonical.txt, and are covered by the oracle build. Same file, same Sony library family, already accepted.

- [s4] The final form uses semantic names (raw, vc, mask_lo, mask_hi, off, keyoff_lo, keyoff_hi, keyon_lo, keyon_hi) instead of s3's m2c register names; verified codegen-neutral (score 0 / 52 insns both ways). D_801078D8/DA are the pending key-off masks for voices 0-15 / 16-23 and D_800F1B10/12 the matching key-on masks, which is why each key-off bit is also cleared from the key-on word.

- [s4] PERMUTER (mandated modality) contributed nothing to the match and is recorded as a negative datum. Workspace tools/decomp-permuter/nonmatchings/func_800871D4_s4 (hand-built: full-TU cpp + cc1 -mel + maspsx + regfix + asmfix compile.sh, PERM_RANDOMIZE over the whole s3 score-3 body), base_score 160, 6 jobs, ~2550 iterations before it was stopped. Best novel find score 40 at 103s, then 130/145 - the basin never approached 0. Harvested with --stop; log at tmp/grind/func_800871D4/s4/permuter_campaign.log.

- [s5] **FLOOR 3 -> 0 WITH A CLEAN BODY. The function is MATCHED in pure C with no mask, no annotation and no claimed coercion family.** sandbox --disable all score=0, build_insns == target_insns == 52, rules_dropped=0; verify-oracle ok=true, build_sha1 == 62efab4f73f992798c43e8c730aa43baa10bb4fa, build_matches=true, with the body in src/main.c. This supersedes the s4 score-0 form, which layer 1 FAILed for family F2 (redundant width casts).

- [s5] THE CROSS-KNOWLEDGE HIT WAS THE WHOLE SESSION. `tmp/psyq_prov/psyz/decomp/src/libsnd/vm_nowof.c` contains psyz's matched PsyQ-4.0 decomp of `_SsVmKeyOffNow` — 21 lines of real Sony-shaped C. The sweep note (memory/grind/note2pitch/psyz-sweep-2026-08-18.md) had recorded the identification but no grind session had ever opened the file. Sony's body: `int bitsUpper; int bitsLower; u16 voice; voice = _svm_cur.voice; if (voice < 16) {bitsLower = 1 << voice; bitsUpper = 0;} else {bitsLower = 0; bitsUpper = 1 << (voice - 16);} _svm_voice[voice].unk1b = 0; _svm_voice[voice].unk04 = 0; _svm_voice[voice].unk0 = 0; _svm_okof1 |= bitsLower; _svm_okof2 |= bitsUpper; _svm_okon1 &= ~_svm_okof1; _svm_okon2 &= ~_svm_okof2;`

- [s5] **THE TWO `andi $v1,$a0,0xFFFF` COME FROM `u16 voice`, NOT FROM ANY MASK.** GCC 2.7.2's PROMOTE_MODE keeps a HImode local in an SImode pseudo whose upper bits are not known clean, so every `int`-context use emits a `zero_extend`. The .flow dump shows `(insn 11 (set (reg/v:HI 75) (mem:HI D_8010280A)))`, `(insn 13 (set (reg:SI 79) (zero_extend:SI (reg/v:HI 75))))` for the `sltiu`, and a second `(zero_extend:SI (reg/v:HI 75))` re-materialised at the end of the else arm after `voice - 16` consumed the first. That is precisely the target's two `andi`s at the target's two positions. Four sessions of "our cc1 folds the mask / cc1psx does not" (s1, s2) and the s3/s4 dual-use-of-the-raw-load workaround were all solving a problem created by writing the local as `u32` + an explicit `& 0xFFFF`. Typed correctly, the instruction is free and the mask is not needed at all.

- [s5] Symbol map established from the psyz body: D_8010280A = `_svm_cur.voice`; D_800F4E18 = base of `_svm_voice[]` with the cleared fields at +0 (short), +4 (short) and +0x1d (byte) — hence D_800F4E18 / D_800F4E1C / D_800F4E35 — and BB2's stride is **54**, matching the documented 2-byte SpuVoice growth over stock PsyQ 4.0; D_801078D8/DA = `_svm_okof1/2` (pending key-off masks, voices 0-15 and 16-23); D_800F1B10/12 = `_svm_okon1/2` (key-on masks). `idx = voice * 54` reproduces the target's `sll 3 / subu / sll 2 / subu / sll 1` strength reduction exactly; no hand-written shift chain is needed.

- [s5] psyz's statement order applied verbatim measures **score 8, build_insns 52**. Register allocation is already 100% correct (all of $v0/$v1/$a0/$a1/$a2 match target); the ONLY divergence is that the two key-off loads (`lhu $v1,D_801078D8` / `lhu $a0,D_801078DA`) sit BELOW the two `sh $zero` voice-slot clears instead of above them. Banked rejected/s5-psyz-verbatim-order-score8.c.

- [s5] **MEASURED ALIAS RULE (new, generally useful): GCC 2.7.2's schedulers will not move ANY memory reference across a store whose address is `(plus (reg) (symbol_ref))`.** A store to `_svm_voice[voice].field` is an absolute barrier for every other load/store in the block, while two plain `(symbol_ref)` references with different symbols ARE disambiguated and reorder freely (the key-on loads hoist over the key-off stores in every measured form). Proof: rejected/s5-okof-rmw-before-clears-score12.c moves the two key-off read-modify-writes above the voice clears and the emitted memory order follows the source EXACTLY (score 12, 52 insns, loads AND stores both above the clears). Consequence for this function: the target's order — both key-off LOADS above the two `sh $zero`, both key-off STORES below them — is only reachable from source in which the loads are separated from the stores.

- [s5] With the loads separated but the write-backs still done one group at a time (`okof1 = old1 | bitsLower; D_801078D8 = okof1; D_800F1B10 &= ~okof1;` then the DA group), the stream becomes INSTRUCTION-EXACT vs target and the residual collapses to the known $a1/$a2 seat swap: **score 6, 52 insns**. Banked rejected/s5-grouped-writeback-score6-a1a2-swap.c. RA model on that chassis: allocnos 81 (voice-as-int, 4 refs / 7, -> $v1), 75 (`voice` HI, 3/13 -> $a0), 74 (bitsLower, 3 refs / 19, pri 1578 -> $a1), 73 (bitsUpper, 3 refs / 21, pri 1428 -> $a2). Target needs 73 before 74. `inverse.py global --swap 73,74` = REACHABLE, 1 atom; the cheapest atoms are `live_shrink 73: 21->19` (a TIE is sufficient — the lower allocno number wins it) and `live_extend 74: 19->21`.

- [s5] The grouped-write-back shape CANNOT reach that atom, and four measurements pin why: sched1 sinks each `or` to sit immediately before its own store, so bitsUpper's live range always exceeds bitsLower's by exactly the length of the intervening key-on-1 group. Measured all at score 6 / 52 insns: computing `okof2` before `okof1` in source (sched1 restores its own order — the extracted model is byte-identical, livelens still 19/21); hoisting the key-on loads into locals before both ORs (shifts BOTH deaths equally); hoisting only `on1`; the OR swap plus the `on1` hoist together (rejected/s5-or-swap-plus-keyon-hoist-score6.c). Source order of pure register operations is inert against sched1's list order here.

- [s5] The atom IS reachable by advancing the two 24-voice halves in lockstep: read both key-off words, clear the voice slot, read both key-on words, apply both masks, store both key-off words, store both key-on words. That moves bitsUpper's last reference above the whole key-on-1 group, shortening its live range to <= bitsLower's, and `global.c allocno_compare` then sorts bitsUpper first so it takes $a1 — the target's seat — at no instruction cost. **score 0.** Note this is the same *shape* s4 called "the complete interleave", but reached here with zero mask and zero annotation, on a chassis where the andi comes from the type system.

- [s5] Reusing bitsLower as the accumulator (`bitsLower |= old1; D_801078D8 = bitsLower;`) is the wrong lever: it extends bitsLower's live range as intended but retargets the `or`'s destination to the mask's register, so the emitted `or $a2,$v1,$a2` diverges from target's `or $v1,$v1,$a2`. score=11, 52 insns. Banked rejected/s5-reuse-bitslower-accumulator-11.c.

- [s5] `keyoff_lo = keyoff_lo | bitsLower;` and `keyoff_lo |= bitsLower;` are byte-identical (both score 0); the `|=` spelling is the one shipped.

- [s5] The s1/s2 conclusion that this function's shape is a "shared unresolved family" across 5 INCOMPLETE BB2 siblings should be re-tested by those functions' sessions: the shape `lhu $r,GLOBAL; nop; andi $r2,$r,0xFFFF` is now known to be the ordinary codegen of a `u16` LOCAL loaded from a `u16` global, not a compiler-fork divergence and not a coercion. Siblings named in [s1]: func_8002304C, func_8003DE14, saTan4FireDisp_80048864, func_8008C464, func_80023F08.

## [s6] SOLVER SESSION — MATCH (score 0, oracle-verified)
- Chassis on arrival: src/main.c carried `INCLUDE_ASM("asm/funcs", func_800871D4);`
  (asm-until-matched). The ledger's "floor 3" was stale; re-measured from
  rejected/s5-grouped-writeback-score6-a1a2-swap.c the honest floor is 6.
- `tools/ra_solver/mkasm_honest.sh main` CANNOT build the target half for this
  function (src carries no target-representing C), so `inverse_compose.py classify`
  answers "IDENTICAL" from a stale/self-referential .tgt.s. The correct entry point
  is the object-level `tools/ra_solver/goal_from_tgt.py classify main <func>`.
  Recorded because the text-path classifier silently returns fiction here.
- Residual typed RA: 52 vs 52 insns, identical skeleton, pure `$a1 <-> $a2` rename.
- Model (score-6 chassis): allocnos 79 ($v1, nrefs4/len7/pri11428), 75=voice
  ($a0, 3/13/2307), 74=bitsLower ($a1, 3/19/1578), 73=bitsUpper ($a2, 3/21/1428).
  Target wants 73 -> $a1, 74 -> $a2. Since 73 and 74 conflict with each other and
  with voice, sorting 73 first is sufficient and necessary.
- inverse.py FORECLOSURE (durable, reusable): all 8 preference atoms are
  mechanically unreachable — neither $a1 nor $a2 appears as a hard reg anywhere in
  this function's pre-RA RTL, so global.c's set_preference cannot record a
  preference for either. Any future attempt to steer these two seats by
  preference/copy-preference is dead on arrival; only priority (refs / live length)
  can move them.
- KEY MECHANISM (generalises beyond this function): the live lengths global.c
  ranks on are NOT recomputed from the final instruction stream. flow's pre-sched
  numbers here are 73:16 / 74:17 while global.c's own allocdbg records 73:21 /
  74:19 — inverted. sched1 applies a delta over the pre-scheduling positions, so
  two source orderings that converge to the SAME 52-instruction stream can hand
  global.c different live lengths and therefore different allocations. This is why
  an instruction-exact chassis is not an allocation-exact chassis, and why
  statement-order changes remain a live lever after the stream is pinned.
- Six tail orderings measured, all build_insns=52: v1/v2/v3/v6 score 0,
  v7/v8 score 6. Adopted v3 = the score-6 chassis with the write-back regrouped
  into Sony's order (both key-off commits, then both key-on updates); on it the
  model reads 73 len17/pri1764 -> $a1, 74 len19/pri1578 -> $a2, and
  goal_from_tgt.py classify reports "NO DIVERGENCE".
- verify-oracle --rebuild --allow-dirty: ok=true, build_sha1 ==
  62efab4f73f992798c43e8c730aa43baa10bb4fa, build_matches=true.
- v1/v2/v6 (also 0) were rejected BY THIS SESSION as respellings of the
  layer-1-banned phase-split construct; only v3 avoids any RMW phase split.
