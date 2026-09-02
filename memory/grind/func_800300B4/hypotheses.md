# Hypothesis ledger — func_800300B4

## s1 (2026-09-02, recon)

- H1 CONFIRMED — the residual of the plain three-island form (13) is a single local-alloc seat
  swap arg0/&mac (s2<->s3), pass = local-alloc.c qty_compare_1 priority (function is one basic
  block). Measured: sandbox 13, BB2_QTY_DEBUG trace.
- H2 KILLED (instance) — a pointer copy `pv = mac` after the island raises &mac's ref count.
  Measured 13, refs unchanged at 4: cse.c canon_reg folds the copy to the class's first pseudo.
  Same mechanism covers any in-block pseudo copy of &mac (second asm operand, alias local).
- H3 CONFIRMED — `do { gte_stlvnl asm } while (0);` doubles the &mac def+asm refs via flow.c
  loop_depth weighting (4 -> 6, priority 1290 -> 1935 > arg0's 1627). Measured sandbox 0 and
  verify-oracle ok. Lever ladder for the seat: natural placement has no free variable (births and
  deaths pinned by target order; every natural spelling gives &mac 4 refs and arg0 7) — the
  do-while(0) family is the single-level, owner-sanctioned device for exactly this effect
  (.claude/rules/do-while-zero-exception.md:29, prerequisite 2: single-level wraps need no
  exhaustion gate).

Frontier: none — candidate at 0. If the Judge rejects the wrap, the only remaining axis is a
natural spelling that lowers arg0 to <=5 refs, which the byte-pinned six s3 uses do not allow;
that would be a ruling-request, not a re-grind.

## s1b (2026-09-02, recon)

- H4 KILLED (class) - island 2 with the VXY0 pack computed in ordinary C (layer-1 prescribed form). Measured
  sandbox 19 in two spellings (lv[] index form; explicit u16 reads), with the E3 do-while(0) wrap present.
  Predicate: the target pack reads through $t4 (asm-internal `move $12,%0` copy, no C handle) into $t5/$t6;
  local-alloc.c:2249 find_free_reg allocates hard regs in numeric order (no REG_ALLOC_ORDER in
  config/mips/mips.h), so with $v0/$v1 free no C temp reaches $13/$14. Both spellings landed in v0/v1 off $s2.
- H5 CONFIRMED (documentary) - the island is on main character-identical in func_800203B4 (owner grant
  2026-09-01, inline_asm_canonical.txt:367), func_8002E838 (Judge PASS decisions.md:20262) and func_80031890
  (Judge PASS decisions.md:20268), all with the pack inside the asm block.

Frontier: ruling-request only. If the ruling admits the gte_ldv0 macro body under condition 3 (as the
func_800203B4 grant text and the two sibling PASSes did), candidate.c is a 0-form ready for re-submission with
the self-vet's precedent citations replaced by func_800203B4 / func_8002E838. If it does not, the function has
no pure-C or sanctioned-asm form for those 7 bytes and the correct disposition is the cluster-door foreclosure.

## s1c (2026-09-02, recon)

- H6 CONFIRMED - the 0-form candidate is admissible under the lifted ban: ruling 2026-09-02 07:30
  (decisions.md:20474) places the gte_ldv0 pack inside the island unit; chassis re-measured at sandbox 0 (83/83).
  Self-vet rewritten without the banned func_8002FC80 / LoadAverageShort12 anchors.

Frontier: none - candidate-ready. If the Judge FAILs the do-while(0) wrap on prerequisites, the only alternative
seat lever is a natural spelling lowering arg0 to <=5 refs, which the six byte-pinned s3 uses forbid (H1); that
outcome would be a ruling-request on the wrap, not a re-grind.

## s1d (2026-09-02, recon)

- H7 CONFIRMED - the banked candidate still measures 0 on HEAD 924b9410 (canonical ASM-PARTIAL 11/83; sandbox 0, 83/83,
  rules_dropped 0); fake_ablate drop-1 = 13 (the E1 seat swap and nothing else), so the do-while(0) wrap is the single
  load-bearing FAKE and hides no lever.
- H8 CONFIRMED (census, no new measurement needed) - under the four mechanical bans in force there is no declarable
  form: the pack is class-dead in C (H4), banned in asm (ban 1), and has no third place to live; every respelling of
  the asm pack is the banned construct by the ban's own any-spelling clause. The residual is a policy variable
  (cluster condition 3 vs SDK macro bodies), owner-only under judge-sole-gate rule 4.

Frontier: none grindable. Re-activation triggers (any one re-opens the item with candidate.c unchanged):
  (1) an owner ruling that condition 3 admits SDK macro-body GPR instructions reading through the macro's $12 copy
      (which is what the func_800203B4 grant text already describes), lifting ban 1;
  (2) an owner ruling that the func_800203B4 / func_8002E838 / func_80031890 islands are citable precedent for
      cluster siblings, lifting ban 3;
  (3) a toolchain finding that lets GCC 2.7.2 base a C-side halfword pair on the asm-internal $t4 copy and seat its
      temps in $13/$14 with $2/$3 free (would void H4's predicate at local-alloc.c:2249).

## s1e (2026-09-02, recon, HEAD 074548cc) - island 2 identified as Sony's gte_ldlv0 header body; provenance measured

- H9 CONFIRMED (documentary, new provenance) - island 2 is NOT `gte_ldv0` (the SVECTOR loader, which in every SDK version is a
  pure `lwc2 $0/$1` pair with no GPR work) but Sony's `gte_ldlv0` = "load a 32-bit VECTOR (s32 vx,vy,vz) into V0", whose body in
  PsyQ Run-time Library Release 4.5 inline_c.h:101-110 (copy on disk: tmp/grind/motion_SetMotion/s7/repos/rood-reverse/include/psx/
  inline_c.h; excerpt banked at tmp/grind/func_800300B4/s1/psyq45_inline_c_excerpt.txt) is VERBATIM
  `lhu $13,4(%0); lhu $12,0(%0); sll $13,$13,16; or $12,$12,$13; mtc2 $12,$0; lwc2 $1,8(%0)` : "r"(r0) : "$12","$13".
  The lhu/lhu/sll/or is Sony's own header text - the s32->s16 pack the GTE requires for VXY0 - not programmer C and not compiler
  output. Every prior record on this function (self_vet s1/s1c, both layer-1 FAILs, the 07:30 ruling, the borderline entry) named
  the island "gte_ldv0 SVECTOR s16-unpack", which is what made the pack read as C-expressible unpacking.
- H10 KILLED (instance) - island 2 spelled as the 4.5 header body verbatim (no `move $12,%0` prefix; temps $12/$13; loads off %0)
  measures sandbox 7 on this chassis (HEAD 074548cc, E3 do-while(0) wrap present, islands 1 and 3 unchanged). The diff is exactly
  the missing `move t4,v0` plus the one-register temp shift ($12->$13, $13->$14) on the six macro lines; opcode order (lhu 4 / lhu 0
  / sll / or / mtc2 / lwc2 8) is identical to the header (tmp/grind/func_800300B4/s1/diff_h10.txt; form in
  rejected/ldlv0-psyq45-verbatim-no-move-prefix-7.c). The identical prefix+shift maps the 4.5 gte_SetRotMatrix (inline_c.h:297-307)
  and gte_stlvnl (:1111-1117) bodies onto islands 1 and 3 - i.e. the target's three islands are the older-SDK spellings of three
  named Sony macros with the cluster's own materialize-then-copy preamble.

Frontier: ruling-request THIS session on provenance grounds (not GCC-internals, not sibling precedent - both are banned bases):
does cluster condition 3 treat the verbatim body of a named Sony GTE macro as THE template, so gte_ldlv0's own GPR pack is inside
the island unit? If admitted (ban 1 lifted): re-apply candidate.c unchanged, rewrite self_vet.md naming the island gte_ldlv0 with
the SDK header as provenance and NO citation of func_8002E838 / func_80031890 / func_8002FC80 / LoadAverageShort12 (bans 3/4),
then candidate-ready. If refused as owner-only (rule 4): progress with these kills banked; the borderline entry
(docs/grind/borderline.md:370) should be amended to name gte_ldlv0 and cite inline_c.h:101-110.

## s1f (2026-09-02, recon, HEAD 22a0ab87) - ban 1 lifted; candidate re-verified and conformed to the func_800203B4 spelling

- H11 CONFIRMED - the banked candidate measures sandbox 0 (83/83, rules_dropped 0) on HEAD 22a0ab87 with the island-2 ban
  lifted (ruling 2026-09-02 07:59, decisions.md:20575). Measured in BOTH island-2 spellings: joined (.word 0x4A486012 inside
  the gte_ldlv0 asm block, as banked by s1e) and split (MVMVA as its own `__asm__ volatile(".word 0x4A486012")` after the
  two nops, exactly as the owner-granted func_800203B4 island at src/code6cac.c:1860-1872). Both 0; the split spelling is
  now the candidate because judge constraint 4 requires character identity with the granted island, and the diff
  (tmp/grind/func_800300B4/s1/island2_203B4.txt vs island2_300B4.txt) shows the only difference is the operand expression
  (`arg0 + 0x2C` vs `vec`). verify-oracle ok with the split candidate in place (build_sha1 == oracle).
- Frontier: none - candidate-ready under the 07:59 ruling. self_vet.md rewritten per constraint 4: island 2 named gte_ldlv0,
  provenance inline_c.h:101-110, authorization inline_asm_canonical.txt:367, no citation of the struck 07:30 entry or of
  func_8002E838 / func_80031890 / func_8002FC80 / LoadAverageShort12 / inline_asm_canonical.txt:174.

## s1g (2026-09-02, recon, HEAD 22a0ab87) - discard root-caused; candidate re-verified

- H12 CONFIRMED - the s1f discard was a validator keyword collision: grindlib._ban_trips matched 9+ of ban 2's 17
  significant terms inside s1f's prose-heavy CONSTRUCTS block (it cited "docs/grind/decisions.md:20575", "2026", "lifted",
  "island", "pack"...), although the vet relied only on the 07:59 ruling. A two-sentence structural CONSTRUCTS inventory
  with all citations moved to T1/T5 passes check_banned_constructs and validate_self_vet (both measured (True, '')).
- H13 CONFIRMED - the banked candidate measures sandbox 0 (83/83, rules_dropped 0) and verify-oracle ok on HEAD 22a0ab87
  this session (Measurement 8). No C change since s1f.

Frontier: none - candidate-ready. Remaining risk is the layer-1/Judge reading of block 2 (gte_ldlv0) under cluster
condition 3, already ruled PASS at decisions.md:20575; if a later ruling reverses it, the only C-side alternative
(pack in C) is the H4 class kill and the correct outcome would be a ruling-request, not a re-grind.

## s1h (2026-09-02, recon, HEAD db16e520; driver reset the ledger state to session 0)

- H14 KILLED (instance) - pack-in-C form A with a NESTED do-while(0) around the gte_stlvnl island (loop_depth 2, &mac refs
  6 -> 8) to recover the 12-insn arg0/&mac seat swap and bound the ban-compliant floor at 7. Measured sandbox 21 (worse than
  the single-wrap 19): arg0 reaches s3 but &mac (pri .387) overshoots lookup (.375) into s0, cascading three seats. Trace
  qtydbg_h14.txt:619-623. Integer-window arithmetic (evidence.md s1h): with arg0 at 9 refs no ref count r gives
  log2(r)*r in (17.8, 19.1) at lifetime 62, so no loop-depth weighting seats &mac in s2 in this chassis. Measured on HEAD
  db16e520, pack-in-C island 2 (no banned construct), islands 1/3 unchanged.
- H15 CONFIRMED - chassis re-measure on HEAD db16e520: candidate.c (banned island-2 block present) sandbox 0, 83/83;
  pack-in-C form A (no banned construct) sandbox 19. Both identical to s1b/s1g; the chassis has not moved.

Frontier (all C-side axes for the island-2 residual are measured dead; the residual is the policy question already logged at
docs/grind/borderline.md:370 and cannot be answered in-pipeline per the 08:12 layer-1 FAIL):
  (1) ban-compliant floor is 19, not 7 - H14 closes loop-depth weighting for the seat swap in the pack-in-C chassis; the only
      remaining seat lever there would be an arg0 ref count of exactly 8 AND a &mac count whose log2(r)*r lands in
      (15.8, 19.1), which no integer satisfies - do not re-probe wrap depth/placement.
  (2) the 0-form needs the island-2 gte_ldlv0 block, which is banned; a ruling-request already produced a Judge PASS (07:59)
      that layer-1 then refused as owner-only (08:12). Re-requesting is a loop; the disposition path is the driver ladder
      reaching `escalation` with the borderline entry as the record.
  (3) re-activation triggers unchanged from s1d (owner ruling on condition 3 / on sibling-precedent citability / a toolchain
      finding voiding H4's local-alloc.c:2249 predicate).

## [s1] H14: pack-in-C form A plus a nested do-while(0) around the gte_stlvnl island (loop_depth 2, &mac refs 6->8) recovers the 12-insn arg0/&mac seat swap and bounds the ban-compliant floor at 7.
- mechanism: flow.c loop_depth ref weighting -> local-alloc.c qty_compare_1 priority floor_log2(refs)*refs/(death-birth); raise &mac above arg0 (9 refs in the pack-in-C chassis) without passing &dir/lookup.
- probe: sandbox --disable all on tmp/grind/func_800300B4/s1/v_h14_packC_nested.c; BB2_QTY_DEBUG traces qtydbg_h14.txt vs qtydbg_packA.txt; diff_h14.txt.
- result: sandbox 21 (single wrap: 19). arg0 reaches s3 but &mac priority .387 > lookup .375 -> s0, cascading lookup/&dir/&mtx one seat each (14 seat diffs); island-2 residual 7 unchanged. Window arithmetic: &mac needs log2(r)*r in (17.8,19.1) at lifetime 62; r=6->12, 7->14, 8->24 - no integer hit; arg0 at 8 refs opens (15.8,19.1), also no hit; arg0 at 10 passes &dir. Loop-depth weighting cannot seat &mac in s2 in this chassis.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD db16e520, pack-in-C island 2 (no banned construct), islands 1/3 as banked, FAKE = nested do-while(0) on island 3 only

## [s1] H15: the chassis has not moved since s1g - candidate.c measures 0 and the ban-compliant pack-in-C form A measures 19 on HEAD db16e520.
- mechanism: chassis re-measure required by the dispatch CHASSIS CHECK (measurement unavailable, ledger state reset to session 0).
- probe: apply candidate.c / v_packC_A.c over the INCLUDE_ASM line; canonical + sandbox --disable all (canonical_s1h.txt, sandbox_s1h_cand.txt, sandbox_s1h_packA.txt).
- result: canonical ASM-PARTIAL 11/83 cop2; candidate sandbox 0 (83/83, rules_dropped 0); pack-in-C A sandbox 19 = 7 island-2 insns + 12 seat-swap insns, identical to s1b/s1g.
- verdict: CONFIRMED

## s2 (2026-09-02, structural, HEAD ca605c18) — ban-compliant floor 19 -> 11

- H16 KILLED (instance) — a C-level pointer to the `mac` frame array (`s32 *m = mac;` driving the
  island-3 asm operand, the three translation adds and both tail calls) raises the &mac quantity's
  ref count. Measured sandbox 31 with refs unchanged at 4 (qty_v_ptrmac_nowrap.txt:604); cse
  propagates the pointer and every `mac[i]` stays a sp+16 frame MEM. Generalizes H2 from pointer
  COPIES to pointer-BASED USES.
  measured_on: HEAD ca605c18, pack-in-C island 2, no do-while wrap.
- H17 KILLED (instance) — addressing island 2 off `arg0` (operand `"r"(arg0)`, `lwc2 $1,0x34($12)`)
  lowers arg0's ref count by deleting the `lv` addiu. Measured sandbox 20 with arg0 refs still 9
  (qty_v_arg0direct_wrap.txt:621): the asm operand is itself an arg0 reference, and the form also
  loses the target's `addiu v0,s3,44`.
  measured_on: HEAD ca605c18, pack-in-C island 2 + single do-while wrap on island 3.
- H18 CONFIRMED — the 12-insn seat residual is reachable in the ban-compliant chassis by raising
  &dir instead of squeezing &mac. `do { func_8002F2D0(mtx, dir); } while (0);` (dir refs 4 -> 6,
  priority .307 -> .462) plus the nested island-3 wrap (&mac .387) orders the four call-crossing
  quantities lookup(.375) > dir(.462)... i.e. dir $s1, lookup $s0, mac $s2, arg0 $s3 — all on
  target. sandbox 19 -> 11 (sb_v_model_macdir.txt). Best ban-compliant form banked at
  memory/grind/func_800300B4/best_ban_compliant.c. NOT a submission: three do-while(0) FAKEs.
- H19 KILLED (instance) — the 4-insn tail-order cost of the dir wrap can be removed by moving the
  wrap boundaries. Five placements measured: call only = 11, call+lookup = 16, MulMatrix0+call = 27,
  call+lookup+func_80049718 = 11, lookup hoisted out = 36. The &dir def is emitted immediately
  before its first use, so every placement that doubles its refs also puts the def inside the loop
  region, and the loop notes hoist `addiu s1,sp,32` past `move a0,s0`.
  measured_on: HEAD ca605c18, pack-in-C island 2 + nested island-3 wrap.
- H20 CONFIRMED — computing `packed` before defining `lv` seats `lv` in $v0 (it no longer overlaps
  the pack temps), so the target's `addiu v0,s3,44` + `move t4,v0` pair matches. Metric unchanged
  at 11 but the island residual is now 5 substitutions + 2 additions instead of 7 substitutions;
  banked as the best ban-compliant spelling.

Frontier:
  (1) Ban-compliant floor is 11 (was 19). Residual = 7 island-2 pack insns (H4 class kill,
      local-alloc.c:2249) + 4 tail-order insns owned by the dir wrap (H19). Next probe for the 4:
      run `pwsh tools/grinder/dump.ps1 func_800300B4` on best_ban_compliant.c and read the .sched
      dump to confirm the loop-note block split, then look for a device that gives &dir 6 refs
      without a loop note (none known — H16 says no C construct adds refs).
  (2) The 11-form carries three do-while(0) FAKEs and is a floor/mechanism bound, not a candidate.
      The 0-form (candidate.c) is still 0 and still blocked by the island-2 ban.
  (3) Do NOT re-probe: pointer aliases of mac (H16), arg0-ref reduction by island-2 spelling (H17),
      wrap placement for the dir wrap (H19), &mac-only ref weighting at any depth (H14).

## [s2] A C-level pointer to the mac frame array (s32 *m = mac; driving the island-3 asm operand, the three translation adds and both tail calls) raises the &mac quantity's ref count in the pack-in-C chassis.
- mechanism: local-alloc.c:1670 qty_compare_1 priority = floor_log2(refs)*refs*size/(death-birth); the lever would be extra references to the &mac pseudo.
- probe: tmp/grind/func_800300B4/s2/v_ptrmac_nowrap.c + v_ptrmac_wrap.c; sandbox --disable all; BB2_QTY_DEBUG trace qty_v_ptrmac_nowrap.txt.
- result: sandbox 31 (= the no-wrap pack-in-C baseline) and 26 with the wrap; the &mac quantity is unchanged at birth=32 death=94 refs=4 (qty_v_ptrmac_nowrap.txt:604). cse propagates the pointer, so every mac[i] stays an sp+16 frame MEM and never references the pseudo. Generalizes s1's H2 from pointer copies to pointer-based uses.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD ca605c18, pack-in-C island 2 (no banned construct), islands 1/3 as banked, FAKE = none / single do-while(0) on island 3

## [s2] Addressing island 2 off arg0 itself (operand "r"(arg0), lwc2 $1,0x34($12)) lowers arg0's ref count from 9 by deleting the lv addiu, moving arg0 below &mac in the local-alloc order.
- mechanism: local-alloc.c:1670 qty_compare_1; arg0 at 9 refs sits in floor_log2 bucket 3 (27/94 = .287), at 7 refs it would fall to bucket 2 (14/94 = .149).
- probe: tmp/grind/func_800300B4/s2/v_arg0direct_wrap.c; sandbox --disable all; BB2_QTY_DEBUG trace qty_v_arg0direct_wrap.txt.
- result: sandbox 20 (worse than 19). arg0 stays at refs=9 (qty_v_arg0direct_wrap.txt:621) because the asm operand is itself an arg0 reference, and the form additionally loses the target's addiu v0,s3,44. arg0's 9 refs are invariant in this chassis: 7 byte-pinned refs (def + six uses) plus exactly 2 from the C-side pack however spelled.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD ca605c18, pack-in-C island 2 (no banned construct), single do-while(0) wrap on island 3

## [s2] The 12-insn arg0/&mac seat residual of the pack-in-C chassis can be closed by raising &dir's local-alloc priority (widening the window from above) instead of squeezing &mac into the (.287, .307) gap.
- mechanism: local-alloc.c:1670 qty_compare_1 + flow.c loop_depth ref weighting: a do-while(0) around func_8002F2D0(mtx, dir) puts &dir's def and first use at loop_depth 2 (refs 4 -> 6, priority 8/26 = .307 -> 12/26 = .462), so the nested-wrap &mac (24/62 = .387) sits below &dir and above arg0 (27/94 = .287).
- probe: tmp/grind/func_800300B4/s2/v_model_macdir.c and v_macdir2.c; sandbox --disable all; BB2_QTY_DEBUG trace qty_v_model_macdir.txt; diff_v_model_macdir.txt.
- result: sandbox 11 (from 19). All four call-crossing quantities land on the target seats: lookup $s0, &dir $s1, &mac $s2, arg0 $s3 (qty_v_model_macdir.txt:617-623); every one of the 12 seat-swap insns is gone. Residual 11 = 7 island-2 pack insns + 4 tail-order insns. Form banked at memory/grind/func_800300B4/best_ban_compliant.c; it carries three do-while(0) FAKEs and is a floor/mechanism bound, not a submission.
- verdict: CONFIRMED

## [s2] The 4-insn tail-order cost the dir wrap introduces can be removed by moving the wrap's boundaries.
- mechanism: NOTE_INSN_LOOP_BEG/END from the do-while(0) split the region around the &dir def, and the scheduler hoists addiu s1,sp,32 past move a0,s0 (and lh v0,2(s3) past li a1,1).
- probe: five placements measured with the nested island-3 wrap held fixed: call only (v_model_macdir), call+lookup (v_macdir_lookup), MulMatrix0+call (v_macdir_wide), call+lookup+func_80049718 (v_macdir2), lookup hoisted out (v_macdir3).
- result: 11 / 16 / 27 / 11 / 36. The two 11s carry different but equally-sized 4-insn tails. GCC emits the &dir def immediately before its first use, so any placement that doubles its refs also puts the def inside the loop region; no placement measured here avoids the hoist.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD ca605c18, pack-in-C island 2 (no banned construct), nested do-while(0) on island 3 plus one do-while(0) on the func_8002F2D0 call

## [s2] Computing packed before defining lv seats lv in $v0 so the target's addiu v0,s3,44 + move t4,v0 pair matches.
- mechanism: local-alloc find_free_reg: with lv defined after the pack, lv's live range no longer overlaps the pack temps, so it can reuse $2 instead of being pushed out to $a1.
- probe: tmp/grind/func_800300B4/s2/v_macdir_lvlate.c and v_macdir_lvlate_u16.c; sandbox + diff_v_macdir_lvlate_u16.txt.
- result: both 11; the diff shows addiu v0,s3,44 and move t4,v0 as matching context for the first time, so the island-2 residual is now 5 substitutions + 2 additions instead of 7 substitutions. Banked as the best ban-compliant spelling.
- verdict: CONFIRMED

## s3 (2026-09-02, structural, HEAD 9c1533fc) — ban-compliant floor 11 -> 7

## [s3] H21: the 4-insn tail-order cost of the &dir do-while(0) wrap is removed by widening the wrap UPWARD (starting it at the mac translation adds and running it to the end of the function) rather than by moving its narrow boundaries.
- mechanism: NOTE_INSN_LOOP_BEG/END are scheduling-region boundaries. In the s2 (narrow) placement the note lands between `move a0,s0` and the &dir def `addiu s1,sp,32`, and between `li a1,1` and `lh v0,2(s3)`, so sched hoists both later insns one slot. Starting the wrap above every call-argument sequence in the tail keeps the identical flow.c loop_depth ref weighting on &dir while putting no note inside a call setup.
- probe: tmp/grind/func_800300B4/s3/{v_addall,v_dirall,v_mulall,v_dir3stmt,v_dirsplit2,v_dirlast,v_dirmid,v_dirtail2,v_dirdeep,v_lookupearly,v_lookupin,v_idxsplit}.c via probe.sh; sandbox --disable all; diff_v_addall_mac1.txt.
- result: adds..end = **7** (new floor, from 11); call..end = 9; MulMatrix0..end = 14; s2 narrow forms 11/11; the other eight placements 16-29. At 7 the tail is fully matching: the entire remaining diff is one hunk containing only the island-2 pack.
- verdict: CONFIRMED

## [s3] H22: the island-3 do-while(0) can be dropped on the wide-wrap chassis because &mac now gains loop weighting from the two trailing calls inside the wide wrap.
- mechanism: flow.c loop_depth ref weighting; the wide wrap already contains both `mac` call references, so the dedicated island-3 wrap might be redundant.
- probe: tmp/grind/func_800300B4/s3/v_addall_mac0.c (no island-3 wrap), v_addall_mac1.c (one level), v_addall.c (nested two levels); sandbox --disable all.
- result: 0 levels = 19 (the four call-crossing seats break again), 1 level = 7, 2 levels = 9. The island-3 wrap is required and is pinned at exactly one level, so the banked s3 form carries two FAKE do-while(0) wraps instead of s2's three.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 9c1533fc, pack-in-C island 2 (no banned construct), wide dir wrap (mac adds through end of function)

## [s3] H23: some island-2 pack spelling reduces the 7-insn residual on the fully-closed (tail-matching) chassis.
- mechanism: the residual is the target's `lhu t6,4(t4); lhu t5,0(t4); sll t6,t6,0x10; or t5,t5,t6; mtc2 t5,$0` — base `$t4` is the asm block's own `move $12,%0` preamble register and the temps are `$t5`/`$t6`; local-alloc.c:2249 find_free_reg hands out hard regs in numeric order and `$v0`/`$v1` are free at that point.
- probe: six spellings on the 7-chassis (tmp/grind/func_800300B4/s3/v_i2_{lvfirst,lvidx,lvlate2,inline,orswap}.c plus the banked arg0-based lv-late form); sandbox --disable all; diff_v_i2_*.txt.
- result: all six score exactly 7. The `lv`-based spellings additionally regress composition (they seat `lv` in `$a1`, losing the matching `addiu v0,s3,44` + `move t4,v0` context pair). Re-confirms s1's H4 class kill on the s3 chassis.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 9c1533fc, wide dir wrap + single island-3 wrap, pack-in-C island 2 (no banned construct)

Frontier after s3:
  (1) Ban-compliant floor is 7 and the residual is ONE diff hunk = the island-2 gte_ldlv0 SVECTOR
      pack. Every other instruction of func_800300B4 byte-matches in ban-compliant pure C.
      The remaining distance to COMPLETED-C is entirely the policy question at
      docs/grind/borderline.md:370 (does cop2-addressing-preamble-cluster.md condition 3 admit a
      verbatim PsyQ gte_ldlv0 SDK macro body, whose GPR pack is not an addressing preamble?).
      Do NOT re-request that ruling; it is filed.
  (2) The 7-form carries two do-while(0) FAKEs and is a floor/mechanism bound, not a submission.
      The 0-form (candidate.c) is still 0 and still blocked by the island-2 ban.
  (3) Do NOT re-probe: dir-wrap placement (12 spans measured, H19+H21), island-3 wrap depth (H22),
      island-2 pack spelling (H23 + H4 + H17), pointer aliases of mac (H16), &mac-only ref
      weighting at any depth (H14), lookup statement position (H21 table).

## [s3] The 4-insn tail-order cost of the &dir do-while(0) wrap is removed by widening the wrap UPWARD - starting it at the mac translation adds and running it to the end of the function - rather than by moving its narrow boundaries as s2's H19 sampled.
- mechanism: NOTE_INSN_LOOP_BEG/END are scheduling-region boundaries. In the s2 narrow placement the note lands between 'move a0,s0' and the &dir def 'addiu s1,sp,32', and between 'li a1,1' and 'lh v0,2(s3)', so sched hoists both later insns one slot. Starting the wrap above every call-argument sequence in the tail preserves the identical flow.c loop_depth ref weighting on &dir while putting no loop note inside a call setup.
- probe: Twelve wrap spans measured with tmp/grind/func_800300B4/s3/probe.sh (apply the form over the INCLUDE_ASM line, 'sandbox func_800300B4 --disable all', normalized-insn diff, BB2_QTY_DEBUG trace): v_addall, v_dirall, v_mulall, v_dir3stmt, v_dirsplit2, v_dirlast, v_dirmid, v_dirtail2, v_dirdeep, v_lookupearly, v_lookupin, v_idxsplit.
- result: adds-through-end = 7 (new floor, down from 11); call-through-end = 9; MulMatrix0-through-end = 14; the two s2 narrow forms 11/11; the remaining eight placements 16-29. At 7 the tail matches completely - the whole remaining diff is one hunk containing only the island-2 pack. Best form banked at memory/grind/func_800300B4/best_ban_compliant.c.
- verdict: CONFIRMED

## [s3] The island-3 do-while(0) wrap can be dropped on the wide-wrap chassis because &mac already gains loop weighting from the two trailing calls that now sit inside the wide wrap.
- mechanism: flow.c loop_depth ref weighting feeding local-alloc.c:1670 qty_compare_1 priority = floor_log2(refs)*refs*size/(death-birth); the wide wrap already contains both 'mac' call references, so a dedicated island-3 wrap might be redundant.
- probe: tmp/grind/func_800300B4/s3/v_addall_mac0.c (no island-3 wrap), v_addall_mac1.c (one level), v_addall.c (nested two levels); sandbox --disable all.
- result: 0 levels = 19 (the four call-crossing seats break again), 1 level = 7, 2 levels = 9. The island-3 wrap is required and is pinned at exactly one level, so the banked s3 form carries two FAKE do-while(0) wraps instead of s2's three.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 9c1533fc, pack-in-C island 2 (no banned construct), wide dir wrap spanning the mac translation adds through the end of the function

## [s3] Six island-2 pack spellings measured on the fully-closed (tail-matching) chassis reduce the 7-insn residual below 7.
- mechanism: The residual is the target's 'lhu t6,4(t4); lhu t5,0(t4); sll t6,t6,0x10; or t5,t5,t6; mtc2 t5,$0'. Base $t4 is the asm block's own 'move $12,%0' preamble register and the temps are $t5/$t6; local-alloc.c:2249 find_free_reg hands out hard regs in numeric order and $v0/$v1 are free at that point.
- probe: tmp/grind/func_800300B4/s3/v_i2_lvfirst.c (pack read through the lv pointer), v_i2_lvidx.c (((u16*)lv)[0]/[2]), v_i2_lvlate2.c (lv defined first, arg0-based reads), v_i2_inline.c (pack expression written inline as the asm operand, no named local), v_i2_orswap.c (or operands swapped high|low), plus the banked arg0-based lv-late spelling; sandbox --disable all + diff_v_i2_*.txt.
- result: All six score exactly 7 and the diff hunk is unchanged in size. The lv-based spellings additionally regress composition: they seat lv in $a1, losing the matching 'addiu v0,s3,44' + 'move t4,v0' context pair. Re-confirms s1's H4 class kill on the s3 chassis.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 9c1533fc, wide dir wrap + single island-3 wrap, pack-in-C island 2 (no banned construct)

## s4 (2026-09-02, permuter, HEAD 81558ab7) � floor stays 7, FAKE count 2 -> 1

Three decomp-permuter campaigns, all launched and stopped in-session via
tools/permuter_campaign.py (workspaces tmp/grind/func_800300B4/s4/perm{1,2,3}; hand-built
minimal-TU workspace validated against the full-TU chassis: the FAKE-free base compiles to the
same 84-instruction body with exactly the known arg0/&mac seat swap + island-2 pack diff).

## [s4] H24: the two do-while(0) FAKE wraps of the s3 7-form can be replaced by a SINGLE wrap whose span opens inside the island block.
- mechanism: flow.c loop_depth ref weighting into local-alloc.c:1670 qty_compare_1. s3's H21/H22
  sweep only tested spans that START at or after the `mac[i] += mat[i+5]` translation adds, so
  every span it measured excluded the island-3 asm and therefore needed a second, dedicated wrap
  to weight &mac. A span that opens at the island-2 or island-3 asm covers the &mac def AND its
  asm use AND both trailing calls in one loop region, weighting &mac and &dir together, while
  still keeping every NOTE_INSN_LOOP_BEG out of a call-argument sequence.
- probe: permuter campaign perm1 (FAKE-free base, weighted 388) proposed the island-2..end span at
  iteration 1606 (weighted 180, tmp/grind/func_800300B4/s4/perm1/output-180-1/source.c). Five spans
  then measured by hand with `sandbox func_800300B4 --disable all`
  (tmp/grind/func_800300B4/s4/probe.sh, sb_v_*.txt).
- result: island-2..end = **7**, island-3..end = **7**, island-1..end = 19, pack..end = 20,
  adds..end (no island-3 wrap) = 19. The 7-forms carry ONE do-while(0) instead of s3's two, at the
  identical floor and with the identical single-hunk residual (the island-2 gte_ldlv0 pack).
  New best ban-compliant form banked at memory/grind/func_800300B4/best_ban_compliant.c
  (island-3..end spelling, clean 
-escaped asm, re-measured 7).
- verdict: CONFIRMED

## [s4] H25: an ordinary-C form (no do-while(0), no coercion construct) reaches the four call-crossing seats on the ban-compliant chassis.
- mechanism: if any ordinary-C spelling can move &mac/&dir above arg0 in local-alloc.c:1670
  qty_compare_1 order without a loop note, the 7-form needs no FAKE at all. s1's H2 and s2's H16
  killed pointer copies and pointer-based uses individually; this probes the space mechanically.
- probe: permuter campaign perm2 on the FAKE-FREE ban-compliant base (weighted 388) with
  perm_ins_block, perm_add_mask, perm_xor_zero, perm_mult_zero, perm_add_self_assignment,
  perm_pad_var_decl, perm_dummy_comma_expr, perm_empty_stmt, perm_condition and perm_inline all
  set to weight 0.0 � i.e. the randomizer may only reorder/rename/retype/split ordinary C.
  24,590 iterations, 6 jobs, 745 s (campaign.log + campaign_meta.json in
  tmp/grind/func_800300B4/s4/perm2/).
- result: best weighted score 338 (base 388); 19 outputs, none below 338. Nothing in the sampled
  ordinary-C neighbourhood reaches the 180 that the single do-while(0) form reaches, i.e. the
  seat swap is not closed by any ordinary-C spelling the permuter sampled. The two best proposals
  are dead-local junk (`unsigned long new_var; ... func_800393C8(new_var = arg0[10], ...)`) and
  would be cheats regardless; banked at rejected/permuter-ordinary-c-best-24k-iters-s4-338w.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 81558ab7, pack-in-C island 2 (no banned construct), islands 1/3 as banked,
  FAKE = none (perm_ins_block disabled); 24,590 permuter iterations

## [s4] H26: some C spelling on the closed single-wrap chassis reduces the 7-insn island-2 residual.
- mechanism: same as H23/H4 � the target pack reads through the asm-internal $t4 copy into
  $t5/$t6 and local-alloc.c:2249 find_free_reg hands out hard regs in numeric order with $v0/$v1
  free, so no C temp can land in $13/$14.
- probe: permuter campaign perm3 seeded on the s4 single-wrap 7-form (base weighted 180),
  all randomizers enabled, 7,212 iterations / 236 s
  (tmp/grind/func_800300B4/s4/perm3/). Its one improvement, weighted 175
  (output-175-1: a named `u16 *` intermediate for the pack's low half), plus a clean hand-spelled
  version of it and an `lv`-based variant, were then measured in the sandbox.
- result: sandbox 7 for both hand-spelled forms (sb_v_i3end_hwptr.txt, sb_v_i3end_lvhw.txt); the
  weighted 175 is a register-class delta the objdump metric does not see. No proposal below 7 in
  7,212 iterations. Re-confirms H4/H23 on the s4 chassis.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 81558ab7, s4 single-wrap 7-form (one do-while(0) FAKE spanning island 3
  through end of function), pack-in-C island 2 (no banned construct); 7,212 permuter iterations

Frontier after s4:
  (1) Unchanged and dominant: the 7-insn residual is the island-2 gte_ldlv0 GPR pack and is a
      policy question, filed at docs/grind/borderline.md:370. Do NOT re-request that ruling and do
      NOT re-derive the pack in C (H4 class kill, H17, H23, H26).
  (2) The best ban-compliant form now carries ONE FAKE do-while(0) instead of two. Removing that
      last wrap is measured dead for the ordinary-C neighbourhood the permuter samples (H25); the
      untested remainder is a structural chassis change the permuter cannot express (N-way
      statement duplication into arms, goto-into-body), which is a `structural`/`synthesis`
      modality probe, not a permuter one.
  (3) Do NOT re-probe: wrap span placement (17 spans now measured across H19/H21/H24), wrap depth
      (H22), island-2 pack spelling (H4/H17/H23/H26), pointer aliases of mac (H2/H16), and
      ordinary-C randomization of the FAKE-free chassis (H25).

## [s4] The two do-while(0) FAKE wraps of the s3 7-form can be replaced by a SINGLE wrap whose span opens inside the island block (at or after the island-2 asm) and contains the island-3 asm.
- mechanism: flow.c loop_depth ref weighting feeding local-alloc.c:1670 qty_compare_1. s3's H21/H22 span sweep only sampled spans opening at or after the mac translation adds, so every span it measured excluded the island-3 asm and needed a second dedicated wrap to weight &mac. One loop region covering the &mac def, its island-3 asm use and both trailing calls weights &mac and &dir together, and opening it after the pack keeps every NOTE_INSN_LOOP_BEG out of a call-argument sequence (the 4-insn tail cost s2 paid).
- probe: permuter campaign tmp/grind/func_800300B4/s4/perm1 (FAKE-free ban-compliant base, weighted 388) proposed the island-2..end span at iteration 1606 (weighted 180, output-180-1); five spans then measured by hand with `sandbox func_800300B4 --disable all` via tmp/grind/func_800300B4/s4/probe.sh
- result: island-2..end = 7, island-3..end = 7, island-1..end = 19, pack..end = 20, adds..end without an island-3 wrap = 19. The 7-forms carry ONE do-while(0) at the identical floor with the identical single-hunk residual. New best ban-compliant form banked at memory/grind/func_800300B4/best_ban_compliant.c (island-3..end spelling, re-measured 7 after being rewritten with clean \n-escaped asm).
- verdict: CONFIRMED

## [s4] An ordinary-C form carrying no do-while(0) wrap and no coercion construct reaches the four call-crossing seats on the ban-compliant chassis, in the neighbourhood the permuter samples.
- mechanism: local-alloc.c:1670 qty_compare_1 priority = floor_log2(refs)*refs*size/(death-birth); the lever would be an ordinary-C spelling that moves &mac/&dir above arg0 without a loop note. s1's H2 and s2's H16 killed pointer copies and pointer-based uses one spelling at a time; this probes the space mechanically.
- probe: permuter campaign tmp/grind/func_800300B4/s4/perm2 on the FAKE-free base (weighted 388) with perm_ins_block, perm_add_mask, perm_xor_zero, perm_mult_zero, perm_add_self_assignment, perm_pad_var_decl, perm_dummy_comma_expr, perm_empty_stmt, perm_condition and perm_inline all at weight 0.0; 6 jobs, 24,590 iterations, 745 s, harvested with --stop.
- result: Best weighted score 338 (base 388) over the whole campaign; 19 outputs, none lower. The single do-while(0) form reaches 180 on the same metric, so nothing sampled closes the arg0/&mac seat swap. The two best proposals are dead-local junk (an `unsigned long new_var;` assigned inside a call argument) and are cheats by family regardless; banked at rejected/permuter-ordinary-c-best-24k-iters-s4-338w.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 81558ab7, pack-in-C island 2 (no banned construct), islands 1/3 as banked, FAKE = none (perm_ins_block disabled); 24,590 permuter iterations

## [s4] Some C spelling on the closed single-wrap chassis reduces the 7-insn island-2 residual below 7.
- mechanism: Same predicate as H4/H23: the target pack reads through the asm-internal $t4 copy into $t5/$t6, and local-alloc.c:2249 find_free_reg hands out hard regs in numeric order with $v0/$v1 free, so no C temp lands in $13/$14.
- probe: permuter campaign tmp/grind/func_800300B4/s4/perm3 seeded on the s4 single-wrap 7-form (base weighted 180, independently confirming 180 <-> sandbox 7), all randomizers enabled; 6 jobs, 7,212 iterations, 236 s, harvested with --stop. Its one improvement (weighted 175: a named u16* intermediate for the pack's low half) was hand-spelled two ways and sandboxed.
- result: Both hand-spelled forms measure sandbox 7 (sb_v_i3end_hwptr.txt, sb_v_i3end_lvhw.txt); the weighted 175 is a register-class delta the objdump insn metric does not see. No proposal below 7 in 7,212 iterations. Banked at rejected/named-hw-pointer-for-pack-low-half-s4-7.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 81558ab7, s4 single-wrap 7-form (one do-while(0) FAKE spanning island 3 through end of function), pack-in-C island 2 (no banned construct); 7,212 permuter iterations
