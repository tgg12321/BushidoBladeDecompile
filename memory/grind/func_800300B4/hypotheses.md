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

## s5 (2026-09-02, synthesis, HEAD 9b2e1331) — floor stays 7; the last FAKE is now PROVEN NECESSARY

Chassis re-measured at dispatch (the brief reported "measurement unavailable"):
`memory/grind/func_800300B4/best_ban_compliant.c` = sandbox **7**, `candidate.c` (the 0-form
carrying the banned island-2 block) = sandbox **0** (tmp/grind/func_800300B4/s5/sb_v_base.txt,
sb_v_zero.txt). Nothing has moved since s4.

KILL RE-AUDIT (mandated): the two closest-to-target banked instance kills were re-measured on this
chassis — `rejected/named-hw-pointer-for-pack-low-half-s4-7.c` = 7 and
`rejected/island2-pack-through-lv-seats-lv-in-a1-s3-7.c` = 7, both unchanged from s3/s4. Neither
kill was measured with a FAKE construct that is no longer present (both carry the same single
do-while(0) wrap the current best form carries), so no kill needed voiding.

## [s5] H27: no loop-note-free (FAKE-free) C form on the ban-compliant pack-in-C chassis reaches the target's four call-crossing seats.
- mechanism: `qty_compare_1` (tools/gcc-2.7.2/local-alloc.c:1666) ranks quantities by
  `floor_log2(n_refs) * n_refs * qty_size / (qty_death - qty_birth)`. Two invariants pin the
  comparison. (i) arg0's ref count is 9 on this chassis: 7 byte-pinned references plus exactly 2
  from the C-side halfword pack, and cse.c's address propagation defeats every attempt to move
  those 2 onto a different pseudo — a pointer spelling (`lv = (s32 *)(arg0 + 0x2C);` then
  `*(u16 *)lv | (*(u16 *)((u8 *)lv + 4) << 16)`) has the known `plus (reg arg0) (const 44)`
  substituted straight back into both MEMs. (ii) &mac's ref count is 4 with no loop note, and
  s2's E-s2-3 measured that no C construct adds references to it (the same cse propagation folds
  `s32 *m = mac;` back to sp+16). So the ranking is arg0 27/94 = .287 vs &mac 8/62 = .129, and
  arg0 always outranks &mac — the 12-insn seat swap is present in every FAKE-free form.
- probe: four FAKE-free forms measured with `sandbox func_800300B4 --disable all` via
  tmp/grind/func_800300B4/s5/probe.sh — `v_arg0_nowrap` (the banked 7-form with the wrap braces
  removed), `v_lvpack_nowrap` (pack read through `lv`), `v_lvidx_nowrap` (pack as
  `((u16 *)lv)[0] | (((u16 *)lv)[2] << 16)`), `v_struct_nowrap` (mac/dir merged into one frame
  struct). Ref counts read from the instrumented cc1 (BB2_QTY_DEBUG, tmp/grind/func_800300B4/s5/
  qtydbg.py) for the two spellings that were supposed to differ.
- result: **all four score 19** (sb_v_arg0_nowrap.txt, sb_v_lvpack_nowrap.txt,
  sb_v_lvidx_nowrap.txt, sb_v_struct_nowrap.txt). The two qty traces are byte-identical
  (`diff qty_v_lvpack_nowrap.txt qty_v_arg0_nowrap.txt` is empty; both report
  `reg1=72 birth=2 death=96 refs=9` for arg0 and `reg1=115 birth=32 death=94 refs=4` for &mac),
  which closes s2's E-s2-2 "whatever the spelling" claim with a direct trace on the one spelling
  it had never traced. Controls: the same lv-pack form WITH the banked wrap scores 7
  (`v_lvpack_wrap`), and the same struct form with the wrap scores 7 (`v_struct_wrap`) — so the
  wrap, not the spelling, is what carries the seats.
- verdict: KILLED
- kill_scope: class
- predicate_cite: tools/gcc-2.7.2/local-alloc.c:1666
- measured_on: HEAD 9b2e1331, pack-in-C island 2 (no banned construct), islands 1/3 as banked,
  FAKE = none in all four probes
- CONSEQUENCE (this is the session's deliverable): the single `do { ... } while (0);` wrap the
  best ban-compliant form carries is not an incidental convenience — it is the only measured
  device that moves &mac and &dir past arg0 in qty_compare_1 order, and the FAKE-free
  neighbourhood is now excluded arithmetically (s5 H27) as well as empirically (s4 H25, 24,590
  permuter iterations). That is precisely the lever-exhaustion evidence
  `.claude/rules/do-while-zero-exception.md` prerequisite (a) demands, and any future submission
  should cite H27 + H25 for it rather than the older per-spelling instance kills.

## [s5] H28: merging `mac` and `dir` into one frame struct changes the &mac/&dir quantity structure.
- mechanism: if both frame addresses derived from one base pseudo, that base's references would
  sum (4 + 4) and its lifetime would span both, potentially clearing the (.287, .307) window that
  E-s2-3 showed is unreachable for &mac alone at its pinned lifetime 62.
- probe: `struct { s32 mac[3]; s32 _g; s32 dir[2]; } f;` reproducing the target's sp+0x10 /
  sp+0x20 layout, with `#define mac f.mac` / `#define dir f.dir` so the body is otherwise
  character-identical; measured with and without the banked wrap
  (tmp/grind/func_800300B4/s5/v_struct_{wrap,nowrap}.c).
- result: 7 with the wrap and 19 without — identical to the plain-locals chassis in both
  conditions. Each member address is still materialized as its own sp-relative `addiu` pseudo, so
  no quantities merge. Banked at rejected/merged-frame-struct-no-quantity-change-19.c. This was
  the last untried statement-chassis variation for the seat problem.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 9b2e1331, pack-in-C island 2 (no banned construct), islands 1/3 as banked,
  measured both FAKE-free and with the single do-while(0) wrap

Frontier after s5 (RESET — strongest first):
  (1) UNCHANGED AND DOMINANT. The whole 7-insn residual is the island-2 gte_ldlv0 GPR pack and is
      a policy question (does cop2-addressing-preamble-cluster.md condition 3 admit a verbatim
      PsyQ 4.5 SDK macro body whose lhu/lhu/sll/or is template text rather than an addressing
      preamble?), filed at docs/grind/borderline.md:370 with the s3 addendum. Do NOT re-request
      the ruling, do NOT re-derive the pack in C (H4 class kill + H17/H23/H26), and do NOT submit
      candidate.c or cite the struck 07:30 / 07:59 rulings or the banned sibling precedents.
  (2) The single do-while(0) FAKE is now PROVEN NECESSARY (H27 class kill), not merely
      unreplaced. There is no remaining probe here: the axis is closed, and the finding's value is
      as the FAKE prerequisite-(a) citation for whatever submission eventually clears item (1).
  (3) The three re-activation triggers from s1's H8 are unchanged and remain the only routes:
      (a) an owner ruling admitting SDK macro-body GPR instructions under condition 3 (lifts
      ban 1); (b) an owner ruling making the func_800203B4 / func_8002E838 / func_80031890 islands
      citable precedent for cluster siblings (lifts ban 3); (c) a toolchain finding that lets
      GCC 2.7.2 base a C-side halfword pair on the asm-internal $t4 copy and seat its temps in
      $13/$14 with $2/$3 free (would void H4's predicate at local-alloc.c:2249).

## [s5] No loop-note-free (FAKE-free) C form on the ban-compliant pack-in-C chassis reaches the target's four call-crossing seats ($s0 lookup / $s1 &dir / $s2 &mac / $s3 arg0).
- mechanism: qty_compare_1 (tools/gcc-2.7.2/local-alloc.c:1666) ranks quantities by floor_log2(refs)*refs*size/(death-birth). Two measured invariants pin the comparison: arg0 carries 9 refs (7 byte-pinned + exactly 2 from the C-side halfword pack) because cse.c substitutes the known plus(reg arg0)(const 44) address back into both halfword MEMs whatever pointer spelling is used; and &mac carries 4 refs with no loop note because the same propagation folds every pointer copy back to sp+16 (s2 E-s2-3). So arg0 27/94 = .287 always outranks &mac 8/62 = .129 and the 12-insn seat swap is present in every FAKE-free form. Only flow.c loop_depth ref weighting moves the numerators.
- probe: Four FAKE-free forms scored with `sandbox func_800300B4 --disable all` via tmp/grind/func_800300B4/s5/probe.sh: v_arg0_nowrap (banked 7-form minus the wrap braces), v_lvpack_nowrap (pack read through a single lv pointer), v_lvidx_nowrap (index spelling), v_struct_nowrap (mac/dir merged into one frame struct). Ref counts read from the instrumented cc1 with BB2_QTY_DEBUG (tmp/grind/func_800300B4/s5/qtydbg.py) for the two spellings that were expected to differ. Controls with the banked wrap: v_lvpack_wrap, v_struct_wrap.
- result: All four FAKE-free forms score exactly 19. The two qty traces are byte-identical (diff of qty_v_lvpack_nowrap.txt and qty_v_arg0_nowrap.txt is empty): arg0 reg1=72 birth=2 death=96 refs=9, &mac reg1=115 birth=32 death=94 refs=4. Both wrap controls score 7, so the wrap and not the spelling carries the seats. This closes s2's E-s2-2 'whatever the spelling' claim with a direct trace on the one spelling it never traced, and converts s4's empirical 24,590-iteration permuter result (H25) into an arithmetic exclusion.
- verdict: KILLED
- kill_scope: class
- measured_on: HEAD 9b2e1331, pack-in-C island 2 (no banned construct), islands 1/3 as banked, FAKE = none in all four probes
- predicate_cite: tools/gcc-2.7.2/local-alloc.c:1666

## [s5] Merging mac and dir into a single frame struct changes the &mac / &dir quantity structure enough to clear the (.287, .307) seat window.
- mechanism: If both frame addresses derived from one base pseudo, that base's references would sum (4 + 4) and its lifetime would span both, potentially reaching a priority that &mac alone cannot reach at its pinned lifetime 62 (s2 E-s2-3 showed its achievable priorities are .129/.194/.387/.484).
- probe: struct { s32 mac[3]; s32 _g; s32 dir[2]; } f; reproducing the target's sp+0x10 / sp+0x20 layout, with #define mac f.mac / #define dir f.dir so the rest of the body is character-identical; measured both with and without the banked wrap (tmp/grind/func_800300B4/s5/v_struct_wrap.c, v_struct_nowrap.c).
- result: 7 with the wrap and 19 without - identical to the plain-locals chassis in both conditions. Each member address is still materialized as its own sp-relative addiu pseudo, so no quantities merge. This was the last untried statement-chassis variation for the seat problem. Banked at rejected/merged-frame-struct-no-quantity-change-19.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 9b2e1331, pack-in-C island 2 (no banned construct), islands 1/3 as banked, measured both FAKE-free and with the single do-while(0) wrap

## s6 (2026-09-02, synthesis, HEAD 5eac882d) - re-activation trigger (c) closed with a predicate

Chassis re-measured at dispatch (the brief again reported "measurement unavailable"):
`memory/grind/func_800300B4/best_ban_compliant.c` = sandbox **7**, `memory/grind/func_800300B4/
candidate.c` (the 0-form carrying the banned island-2 block) = sandbox **0**
(tmp/grind/func_800300B4/s6/sb_v_base.txt, sb_v_zero.txt). Identical to s3/s4/s5; nothing moved.

KILL RE-AUDIT (mandated). `tools/fake_ablate.py --func func_800300B4 --file code6cac_b --candidate
memory/grind/func_800300B4/best_ban_compliant.c` reports 1 FAKE unit, keep-all = 7, drop-1 = 27 -
the 27 is the s5 tooling artefact (the ablator deletes only the `do {` line and orphans the 5-line
FAKE comment tail), NOT a FAKE-free measurement. The hand-built FAKE-free control
(tmp/grind/func_800300B4/s6/v_nowrap.c, braces removed, body intact) re-measures **19** on this
chassis, agreeing with s4/s5. The two closest-to-target banked instance kills re-measure unchanged:
`rejected/named-hw-pointer-for-pack-low-half-s4-7.c` = 7,
`rejected/island2-pack-through-lv-seats-lv-in-a1-s3-7.c` = 7. No kill needed voiding.

## [s6] H29: GCC 2.7.2's local allocator has a path that seats a C-side pseudo in $13/$14 while $2/$3 are free - i.e. s1's H4 predicate (find_free_reg numeric order, local-alloc.c:2249) has a bypass that would let the island-2 pack be written in C.
- mechanism: This is s1's re-activation trigger (c), the only one of the three that a grind session
  can act on, and it is the frontier's own next_probe: "re-read tools/gcc-2.7.2/local-alloc.c
  find_free_reg and the reload register-class handling for any path that assigns $12-$14 to a pseudo
  while $2/$3 are free". `find_free_reg` has exactly one order-bypass: when `just_try_suggested` is
  set (local-alloc.c:2207-2213) the scan set is restricted to `qty_phys_copy_sugg[qty]` /
  `qty_phys_sugg[qty]`, so a quantity carrying a suggestion of hard reg 13 or 14 would take it
  regardless of $2/$3 being free. Everything else falls through to the ascending numeric scan at
  local-alloc.c:2249-2254, whose `int regno = i;` arm is the live one because MIPS defines no
  REG_ALLOC_ORDER (the guard is regclass.c:112; there is no definition anywhere under config/mips/).
- probe: instrumented cc1 (tools/gcc-2.7.2/cc1) run with BB2_SUGG_DEBUG=1 and BB2_QTY_DEBUG=1 over
  the whole TU with best_ban_compliant.c applied - one `SUGGDBG-QTY` line per quantity (suggestion
  sets, class, birth/death/refs) and one `SUGGDBG-FFR` line per find_free_reg call (the `used` and
  `first_used` hard-reg sets it actually scans). Dump:
  tmp/grind/func_800300B4/s6/suggdbg_all.txt (3,357 lines; func_800300B4 is the span headed
  `func=func_800300B4`). Then two contention probes that make $v0/$v1 busy across the pack by
  hoisting it (v_packhigh.c: pack computed between the `game_GetPlayerData` call and the `mat`
  load; v_packtop.c: pack as the function's first statement), each sandboxed and the closer one
  re-dumped (suggdbg_packhigh.txt).
- result: **KILLED, and the trigger is closed.** (i) The three pack quantities carry NO suggestions
  at all - `qty=4 ... ncopysugg=0 nsugg=0`, `qty=5 ... ncopysugg=0 nsugg=0`, `qty=6 ...
  ncopysugg=0 nsugg=0` - so the `just_try_suggested` bypass never runs for them; only quantities
  0/1/3/7/12/15/17 (the ones copied to/from $2, $4-$7, i.e. return value and argument registers)
  get a suggestion pass at all, and every suggestion recorded anywhere in this function is one of
  {4,5,6,7,30}. Hard regs 13/14 are not reachable as a suggestion in this ABI: a suggestion is only
  created by a copy between a pseudo and a hard register, and nothing but hardcoded-$N asm ever
  copies to $t5/$t6. (ii) The numeric scan is confirmed to have 13/14 FREE and simply pass them
  over: `SUGGDBG-FFR qty=4 class=1 jts=0 born=20 dead=26 used=0,1,4,26..67` (regs 2,3,5-25 all
  free) -> got $2; `qty=5 ... used=0,1,2,4,12,...` -> got $3; `qty=6 ... used=0,1,3,4,12,...` ->
  got $2. Note $12 IS in the `used` sets - the island asm block's `"$12"` clobber marks it - which
  is the only mechanism in this function that ever marks a $t register used. (iii) The contention
  probes show the scan is monotone in the number of conflicting live quantities and moves one
  register per conflict, not more: hoisting the pack above island 1 pushes its value quantity from
  $3 to **$5** (`QTYDBG blk=0 ord=15 qty=3 reg1=77 birth=14 death=30 refs=4 got=5`) and scores 9;
  hoisting it to the top of the function scores 10. So reaching $13 from $2 requires ELEVEN further
  simultaneously-live conflicting quantities across the pack's 6-insn live range - a quantity count
  this function's straight-line body cannot produce without invented dead values (a cheat family).
  Forms banked at rejected/pack-hoisted-above-island1-contention-s6-9.c and
  rejected/pack-hoisted-to-function-top-s6-10.c.
- verdict: KILLED
- kill_scope: class
- predicate_cite: tools/gcc-2.7.2/local-alloc.c:2207
- measured_on: HEAD 5eac882d, pack-in-C island 2 (no banned construct), islands 1/3 as banked,
  FAKE = the single banked do-while(0) wrap (present in the dumped form; the register-order finding
  is wrap-independent - the pack quantities cross no call and take no loop weighting)
- CONSEQUENCE: s1's H8 re-activation trigger (c) is now a measured dead end, not an open question.
  The three triggers reduce to the two owner-ruling ones (a) and (b). Any future session tempted to
  "look for a toolchain path to $t5/$t6" should read this entry and stop.

## [s6] H30: making $v0/$v1 busy across the island-2 pack (the explicit conditional in H4's predicate, never previously tested) moves the pack temps toward the target's $t5/$t6 far enough to reduce the 7-insn residual.
- mechanism: H4/H23's predicate is conditional - "with $v0/$v1 free no C temp reaches $13/$14".
  Statement placement is the one ordinary-C lever that changes which hard regs are live across the
  pack's range, so hoisting the pack into the region where the `game_GetPlayerData` return value and
  the `playerData[arg0[9]]` index chain are live is the direct test of that condition.
- probe: tmp/grind/func_800300B4/s6/v_packhigh.c (pack between the call and the `mat` load) and
  v_packtop.c (pack as the first statement of the function); `sandbox func_800300B4 --disable all`
  via tmp/grind/func_800300B4/s6/probe.sh; register outcomes read from suggdbg_packhigh.txt.
- result: 9 and 10 (baseline 7) - both worse. The mechanism works in the expected direction but is
  far too weak: contention moves the pack value quantity exactly two registers ($3 -> $5), while
  the target needs $13/$14, and the hoist simultaneously costs 2-3 insns of position/scheduling in
  the previously-matching prologue region. Confirms the numeric-order model quantitatively and
  disposes of the "make $v0/$v1 busy" reading of H4's conditional.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 5eac882d, pack-in-C island 2 (no banned construct), islands 1/3 as banked,
  FAKE = the single banked do-while(0) wrap

Frontier after s6 (RESET - strongest first):
  (1) UNCHANGED AND DOMINANT, and now the ONLY item. The whole 7-insn ban-compliant residual is the
      island-2 `gte_ldlv0` GPR pack, and it is a policy question filed at
      docs/grind/borderline.md:370 (with s3 and s6 addenda): does
      .claude/rules/cop2-addressing-preamble-cluster.md condition 3 admit a verbatim PsyQ 4.5 SDK
      macro body whose lhu/lhu/sll/or is template text rather than an addressing preamble? Do NOT
      re-request the ruling, do NOT submit candidate.c, do NOT cite the struck 07:30 / 07:59
      rulings or the banned sibling precedents, and do NOT re-derive the pack in C
      (H4 + H17 + H23 + H26 + H29 + H30).
  (2) The seat axis is closed and the single do-while(0) FAKE is proven necessary (s5 H27 class
      kill at local-alloc.c:1666, backed by s4 H25's 24,590 ordinary-C-only permuter iterations).
      No probe. Cite H27 + H25 as the do-while-zero-exception prerequisite-(a) lever exhaustion.
  (3) Re-activation triggers are now TWO, not three: (a) an owner ruling admitting SDK macro-body
      GPR instructions under condition 3 (lifts ban 1); (b) an owner ruling making the
      func_800203B4 / func_8002E838 / func_80031890 islands citable precedent for cluster siblings
      (lifts ban 3). Trigger (c) - a toolchain path seating a C pseudo in $13/$14 while $2/$3 are
      free - is KILLED by s6 H29 with the suggestion-path predicate at local-alloc.c:2207, and must
      not be re-opened without new compiler evidence.

## [s6] GCC 2.7.2's local allocator has a path that seats a C-side pseudo in $13/$14 while $2/$3 are free - i.e. s1's H4 predicate (find_free_reg ascending numeric order, tools/gcc-2.7.2/local-alloc.c:2249) has a bypass that would let the island-2 gte_ldlv0 GPR pack be written in ordinary C. This is s1's re-activation trigger (c), the only one of the three that a grind session can act on.
- mechanism: find_free_reg has exactly one bypass of ascending hard-reg order: when just_try_suggested is set, the scan set is restricted to qty_phys_copy_sugg[qty] / qty_phys_sugg[qty] (tools/gcc-2.7.2/local-alloc.c:2207-2213), so a quantity carrying a suggestion of hard reg 13 or 14 would take it regardless of $2/$3 being free. Everything else falls through to the ascending scan at :2249-2254, whose 'int regno = i;' arm is live because MIPS defines no REG_ALLOC_ORDER (guard at regclass.c:112; nothing under config/mips/ defines it).
- probe: Instrumented cc1 (tools/gcc-2.7.2/cc1) run with BB2_SUGG_DEBUG=1 and BB2_QTY_DEBUG=1 over the whole TU with best_ban_compliant.c applied, capturing one SUGGDBG-QTY line per quantity (suggestion sets, class, birth/death/refs) and one SUGGDBG-FFR line per find_free_reg call (the exact 'used' and 'first_used' hard-reg sets scanned): tmp/grind/func_800300B4/s6/suggdbg_all.txt, 3357 lines. Then two contention probes making $v0/$v1 busy across the pack by hoisting it (v_packhigh.c, v_packtop.c), sandboxed via tmp/grind/func_800300B4/s6/probe.sh, with the closer one re-dumped (suggdbg_packhigh.txt).
- result: KILLED, and the trigger is closed. (i) The three island-2 pack quantities carry no register suggestions at all - qty=4/5/6 all report ncopysugg=0 nsugg=0 - so the just_try_suggested bypass never runs for them; only quantities 0/1/3/7/12/15/17 (those copied to/from $2 and $4-$7, i.e. return value and argument registers) get a suggestion pass, and every suggestion recorded anywhere in this function is one of {4,5,6,7,30}. A suggestion is created only by a copy between a pseudo and a hard register, and in this ABI nothing but hardcoded-$N asm ever copies to $t5/$t6. (ii) The ascending scan is confirmed to have $13/$14 FREE and simply pass over them: 'SUGGDBG-FFR qty=4 class=1 jts=0 born=20 dead=26 used=0,1,4,26..67' -> got $2; qty=5 used=0,1,2,4,12,... -> got $3; qty=6 used=0,1,3,4,12,... -> got $2. ($12 appears in 'used' only because the island asm block clobbers "$12" - the sole mechanism in this function that ever marks a $t register used.) (iii) The contention probes show the scan is monotone in the number of conflicting live quantities and moves one register per conflict: hoisting the pack above island 1 moves its value quantity from $3 to $5 (QTYDBG blk=0 ord=15 qty=3 reg1=77 birth=14 death=30 refs=4 got=5) for a score of 9. Reaching $13 from $2 therefore requires eleven further simultaneously-live conflicting quantities across the pack's 6-insn live range, which this straight-line body cannot produce without invented dead values (a cheat family). s1's H8 re-activation trigger (c) is now a measured dead end; the three triggers reduce to the two owner-ruling ones (a) and (b).
- verdict: KILLED
- kill_scope: class
- measured_on: HEAD 5eac882d, pack-in-C island 2 (no banned construct), islands 1/3 as banked, FAKE = the single banked do-while(0) wrap present in the dumped form; the register-order finding is wrap-independent (the pack quantities cross no call and take no loop weighting)
- predicate_cite: tools/gcc-2.7.2/local-alloc.c:2207

## [s6] Making $v0/$v1 busy across the island-2 pack - the explicit conditional in H4's predicate, never previously tested - moves this form's pack temps toward the target's $t5/$t6 far enough to reduce the 7-insn residual.
- mechanism: H4/H23 phrased the kill conditionally ('with $v0/$v1 free no C temp reaches $13/$14'). Statement placement is the one ordinary-C lever that changes which hard registers are live across the pack's range, so hoisting the pack into the live range of the game_GetPlayerData return value and the playerData[arg0[9]] index chain is the direct test of that condition.
- probe: tmp/grind/func_800300B4/s6/v_packhigh.c (pack computed between the call and the mat load) and v_packtop.c (pack as the function's first statement); 'sandbox func_800300B4 --disable all' via tmp/grind/func_800300B4/s6/probe.sh; register outcomes read from suggdbg_packhigh.txt.
- result: 9 and 10 against the baseline 7 - both worse. The mechanism works in the expected direction but is far too weak: contention moves the pack value quantity exactly two registers ($3 -> $5) while the target needs $13/$14, and the hoist simultaneously costs 2-3 insns of position/scheduling in the previously-matching prologue region. Banked at rejected/pack-hoisted-above-island1-contention-s6-9.c and rejected/pack-hoisted-to-function-top-s6-10.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 5eac882d, pack-in-C island 2 (no banned construct), islands 1/3 as banked, FAKE = the single banked do-while(0) wrap

## [s6] The chassis is unchanged since s5 and no banked instance kill needs voiding: best_ban_compliant.c still measures 7, candidate.c still measures 0, the FAKE-free control still measures 19, and the two closest-to-target banked kills still measure 7.
- mechanism: Mandated dispatch chassis re-measure (the brief reported 'measurement unavailable') plus the mandated KILL RE-AUDIT of the instance kills whose forms sit closest to the target.
- probe: tmp/grind/func_800300B4/s6/probe.sh over v_base (= best_ban_compliant.c), v_zero (= candidate.c), v_nowrap (hand-built FAKE-free control, braces removed and body intact), v_reaudit_hwptr (= rejected/named-hw-pointer-for-pack-low-half-s4-7.c), v_reaudit_lvseat (= rejected/island2-pack-through-lv-seats-lv-in-a1-s3-7.c); plus 'python3 tools/fake_ablate.py --func func_800300B4 --file code6cac_b --candidate memory/grind/func_800300B4/best_ban_compliant.c'.
- result: v_base 7, v_zero 0, v_nowrap 19, v_reaudit_hwptr 7, v_reaudit_lvseat 7 - all identical to s3/s4/s5. fake_ablate reports 1 FAKE unit, keep-all 7, drop-1 27; the 27 is the s5-documented ablator artefact (it deletes only the 'do {' line and orphans the five-line FAKE comment tail) and is not a FAKE-free datum, so the hand-built v_nowrap = 19 remains the authoritative FAKE-free control. No banked kill names a FAKE construct absent from the current best form, so none needed voiding.
- verdict: CONFIRMED

## [s7] H31: the 7-insn island-2 residual is owned by the register allocator (the standing s1-H4 / s6-H29 attribution), so RA-layer levers are the right place to search.
- mechanism: s1 H4 and s6 H29/H30 read the residual as `find_free_reg`'s ascending numeric scan
  (tools/gcc-2.7.2/local-alloc.c:2249) seating the pack temps in $2/$3 instead of the target's
  $t5/$t6. The solver modality mandates typing the residual BEFORE searching any backend
  (`inverse_compose.py classify`), which is the check that had never been run on this function.
- probe: `python3 tools/ra_solver/inverse_compose.py classify code6cac_b func_800300B4
  --target-object build/src/code6cac_b.o --ours-object tmp/sandbox/func_800300B4/code6cac_b.o`
  (object path, the mandated escape for an INCLUDE_ASM-routed function), run against the banked
  best form and against six alternative spellings plus the FAKE-free control. Then
  `pwsh tools/grinder/dump.ps1 func_800300B4` and a per-pass scan of the two HImode MEMs of the pack
  (tmp/grind/func_800300B4/s7/scanpass.py, seg.py) to READ the owning pass rather than infer it.
- result: **KILLED — the attribution was one layer too low.** The classifier reports
  `FIRST DIVERGENCE: PRE-RA`: the register-blanked instruction MULTISETS differ (ours
  `lhu #,44(#)` + `lhu #,48(#)`, target `lhu #,0(#)` + `lhu #,4(#)`), which RA and the scheduler
  cannot express because they permute and rename a FIXED multiset. The dumps name the pass exactly:
  `.rtl` carries `(mem:HI (reg 76))` and `(mem/s:HI (plus (reg 76) 4))` — the TARGET's addressing
  shape, emitted by the front end — and `.cse` carries
  `(mem:HI (plus (reg 72) (const_int 44)))`. reg 76 is the `lv` pointer, reg 72 is `arg0`. The RA
  measurements of s1/s6 remain valid but describe the SECOND of two locks, not the first; closing
  the RA layer alone was never sufficient, which is why every RA-directed probe since s1 has been
  flat at 7.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 5dc32f12, best_ban_compliant.c (pack-in-C island 2, no banned construct, islands
  1/3 as banked), FAKE = the single banked do-while(0) wrap; re-confirmed identically on the
  FAKE-free control v_nowrap.c (score 19) and on five alternative pack spellings

## [s7] H32: some semantically-correct C spelling of the island-2 halfword pack renders its two loads with displacements 0 and 4 off a materialised pointer (the target's shape) rather than 44 and 48 off arg0.
- mechanism: The PRE-RA divergence found in H31 is created by cse.c. `fold_rtx`'s MEM case calls
  `find_best_addr (insn, &XEXP (x, 0))` (tools/gcc-2.7.2/cse.c:5034); `find_best_addr` (cse.c:2622)
  walks the address's equivalence class, picks the lowest `ADDRESS_COST`, and breaks ties by the
  HIGHEST `rtx_cost` (cse.c:2717-2726, rationale in its header comment at cse.c:2613-2616). On MIPS
  `ADDRESS_COST(ADDR) = REG_P(ADDR) ? 1 : mips_address_cost(ADDR)` (config/mips/mips.h:2897) and
  `mips_address_cost` returns 1 for `(plus reg SMALL_INT)` (config/mips/mips.c:1653-1654), so the
  bare-REG address and the `reg+44` address TIE at cost 1 and the rtx_cost tiebreak at cse.c:2720
  hands the win to the PLUS form. The condition that makes the PLUS entry exist is simply that the
  pointer's equivalence class contains a `(plus reg CONST_INT)` — which it does for any pointer
  computed as `arg0 + constant`, i.e. for every semantically-correct spelling of this pack.
- probe: six spellings measured and re-classified through tmp/grind/func_800300B4/s7/pc.sh —
  `v_base` (banked best), `v_lvcast` (`*(u16*)lv | (((u16*)lv)[2]<<16)`), `v_hwptr` (named
  `u16 *hw = (u16*)lv`, indexed 0/2), `v_amp` (`*(u16*)&lv[0] | (*(u16*)&lv[1]<<16)`), `v_hwarg`
  (named `u16 *hw` straight off arg0), `v_order` (pointer materialised BEFORE the pack, order-only
  control), plus the FAKE-free control `v_nowrap`. Positive control `v_probe_matbase` deliberately
  changes semantics to base the reads on `mat`, a pointer LOADED FROM MEMORY (so cse holds no
  `(plus reg const)` entry for it), isolating the predicate's condition.
- result: **KILLED.** All six spellings score 7 (v_nowrap 19) and all seven classify PRE-RA with the
  identical multiset delta `lhu #,44(#)/lhu #,48(#)` vs `lhu #,0(#)/lhu #,4(#)`. Naming the pointer,
  indexing through it, taking its address, and reordering its definition are all re-folded by
  find_best_addr. The positive control confirms the predicate exactly: with a memory-loaded base the
  multiset MATCHES and the classification flips to `RA` — proving the 44/48 rendering is caused by
  equivalence-class membership in find_best_addr and by nothing else. The residual is therefore
  DOUBLY locked and each lock is independently sufficient: (1) PRE-RA at cse.c:2720 forces the
  44/48 displacements; (2) RA at local-alloc.c:2207/:2249 (s6 H29) means that even with the
  displacements fixed the loads would be based on a C pseudo in $a0/$v0, never on `$t4` — and `$t4`
  is written only by the island's own `move $12, %0`. The RA residual of the positive control states
  the whole remaining problem: target `lhu t5,0(t4) ; lhu t6,4(t4)` against ours
  `lhu v1,0(a0) ; lhu v0,4(a0)`. The only instruction stream in which those loads can be based on
  `$t4` is one where they are emitted INSIDE the asm template — i.e. the `gte_ldlv0` SDK macro body,
  which is exactly the policy question already filed at docs/grind/borderline.md:370.
- verdict: KILLED
- kill_scope: class
- predicate_cite: tools/gcc-2.7.2/cse.c:2720
- measured_on: HEAD 5dc32f12, pack-in-C island 2 (no banned construct), islands 1/3 as banked;
  measured BOTH with the single banked do-while(0) FAKE present (six forms, score 7) and FAKE-free
  (v_nowrap, score 19) — the PRE-RA lock is FAKE-independent

## [s7] H33 (kill re-audit, mandated): the chassis is unchanged since s6 and no banked instance kill needs voiding.
- mechanism: the dispatch brief reported the HEAD honest floor as "measurement unavailable", and the
  floor has now been flat for three sessions, so both the chassis re-measure and the instance-kill
  re-audit are mandatory before any new probe.
- probe: tmp/grind/func_800300B4/s7/probe.sh and pc.sh over v_base (= best_ban_compliant.c), v_zero
  (= candidate.c), v_nowrap (s6's hand-built FAKE-free control) and v_reaudit_hwptr
  (= rejected/named-hw-pointer-for-pack-low-half-s4-7.c, the banked instance kill whose form sits
  closest to the target).
- result: v_base 7, v_zero 0 (classify: IDENTICAL), v_nowrap 19, v_reaudit_hwptr 7 — every number
  identical to s3/s4/s5/s6 on HEAD 5dc32f12. No banked kill rests on a FAKE construct absent from
  the current best form, so none needed voiding. The re-audit did, however, add information the
  earlier re-audits could not: v_nowrap and v_reaudit_hwptr both classify PRE-RA with the same
  multiset delta, so the closest-to-target banked kill was measuring the RA layer beneath an
  unbroken cse lock.
- verdict: CONFIRMED

Frontier after s7 (strongest first):
  (1) UNCHANGED, DOMINANT, still the ONLY item, now with a two-layer mechanical proof. The whole
      7-insn ban-compliant residual is the island-2 `gte_ldlv0` GPR pack, and it is the policy
      question at docs/grind/borderline.md:370 (s3, s6 and now s7 addenda). Do NOT re-request the
      ruling, do NOT submit candidate.c, do NOT cite the struck 07:30 / 07:59 rulings or the banned
      sibling precedents, and do NOT re-derive the pack in C — the C derivation is now closed at
      TWO independent layers (H32 class kill at cse.c:2720, s6 H29 class kill at
      local-alloc.c:2207), on top of H4/H17/H23/H26/H30.
  (2) The seat axis is closed and the single do-while(0) FAKE is proven necessary (s5 H27 class kill
      at local-alloc.c:1666, backed by s4 H25's 24,590 ordinary-C-only permuter iterations). No
      probe. Cite H27 + H25 as the do-while-zero-exception prerequisite-(a) lever exhaustion.
  (3) Solver backends are FORECLOSED as a route for this residual, by the solver's own triage:
      `inverse_compose.py classify` types the real body PRE-RA and reports "no backend — the
      residual is upstream of every model". `inverse.py` / `inverse_sched.py` on this function would
      produce fiction. Do not spend a future solver session on them; the ONLY body that classifies
      RA is the semantics-changed positive control v_probe_matbase.

## [s7] The 7-insn island-2 residual is owned by the register allocator (the standing s1-H4 / s6-H29 attribution), so RA-layer levers are the right place to search.
- mechanism: s1 H4 and s6 H29/H30 read the residual as find_free_reg's ascending numeric scan (tools/gcc-2.7.2/local-alloc.c:2249) seating the pack temps in $2/$3 instead of the target's $t5/$t6. The solver modality mandates typing the residual with inverse_compose.py classify BEFORE searching any backend, which had never been run on this function.
- probe: python3 tools/ra_solver/inverse_compose.py classify code6cac_b func_800300B4 --target-object build/src/code6cac_b.o --ours-object tmp/sandbox/func_800300B4/code6cac_b.o (object path, the mandated escape for an INCLUDE_ASM-routed function whose build object carries the target's own assembled bytes), run on the banked best form; then pwsh tools/grinder/dump.ps1 func_800300B4 and a per-pass scan of the pack's two HImode MEMs across every -da dump (tmp/grind/func_800300B4/s7/scanpass.py, seg.py) to READ the owning pass instead of inferring it.
- result: KILLED - the attribution was one layer too low. The classifier reports FIRST DIVERGENCE: PRE-RA: the register-blanked instruction multisets differ (ours 'lhu #,44(#)' + 'lhu #,48(#)', target 'lhu #,0(#)' + 'lhu #,4(#)'), which RA and the scheduler cannot express because they permute and rename a fixed multiset - the tool's own next-tool field reads 'no backend - the residual is upstream of every model'. The dumps name the pass exactly: .rtl carries (mem:HI (reg 76)) at insn 41 and (mem/s:HI (plus (reg 76) (const_int 4))) at insn 44, i.e. the FRONT END already emits the target's addressing shape through the lv pointer (insn 38 sets reg 76 = reg 72 + 44; reg 72 is arg0), and .cse carries (mem:HI (plus (reg 72) (const_int 44))). Every later dump (.loop/.combine/.flow/.lreg/.sched/.greg) carries the rewritten form. The RA measurements of s1/s6 remain valid but describe the SECOND of two locks; closing the RA layer alone was never sufficient, which is why every RA-directed probe since s1 has measured flat at 7.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 5dc32f12, best_ban_compliant.c (pack-in-C island 2, no banned construct, islands 1/3 as banked), FAKE = the single banked do-while(0) wrap; re-confirmed identically on the FAKE-free control v_nowrap.c (score 19) and on five alternative pack spellings

## [s7] Some semantically-correct C spelling of the island-2 halfword pack renders its two loads with displacements 0 and 4 off a materialised pointer (the target's shape) rather than 44 and 48 off arg0.
- mechanism: cse.c fold_rtx's MEM case calls find_best_addr (tools/gcc-2.7.2/cse.c:5034 -> :2622). find_best_addr walks the address's equivalence class, takes the lowest ADDRESS_COST and breaks ties by the HIGHEST rtx_cost (cse.c:2717-2726; rationale in its header comment at cse.c:2613-2616). On MIPS ADDRESS_COST(ADDR) = REG_P(ADDR) ? 1 : mips_address_cost(ADDR) (config/mips/mips.h:2897) and mips_address_cost returns 1 for (plus reg SMALL_INT) (config/mips/mips.c:1653-1654), so a bare-REG address and a reg+44 address TIE at cost 1 and the rtx_cost tiebreak at cse.c:2720 hands the win to the PLUS form. The condition creating the PLUS entry is simply that the pointer's equivalence class contains a (plus reg CONST_INT) - true for any pointer computed as arg0 + constant.
- probe: Six spellings measured and re-classified via tmp/grind/func_800300B4/s7/pc.sh (sandbox + classify per form): v_base (banked best, pack off arg0), v_lvcast (*(u16*)lv | (((u16*)lv)[2]<<16)), v_hwptr (named u16 *hw = (u16*)lv, indexed 0/2), v_amp (*(u16*)&lv[0] | (*(u16*)&lv[1]<<16)), v_hwarg (named u16 *hw straight off arg0), v_order (pointer materialised BEFORE the pack, order-only control), plus the FAKE-free control v_nowrap. Positive control v_probe_matbase deliberately changes semantics to base the two reads on mat, a pointer LOADED FROM MEMORY, so cse holds no (plus reg const) entry for it - isolating the predicate's condition.
- result: KILLED. All six spellings score 7 (v_nowrap 19) and all seven classify PRE-RA with the identical multiset delta lhu #,44(#)/lhu #,48(#) vs lhu #,0(#)/lhu #,4(#): naming the pointer, indexing through it, taking its address and reordering its definition are all re-folded by find_best_addr. The positive control confirms the predicate exactly - with a memory-loaded base the multiset MATCHES the target and the classification flips to RA. The residual is therefore doubly locked, each lock independently sufficient: (1) PRE-RA at cse.c:2720 forces the 44/48 displacements; (2) RA at local-alloc.c:2207/:2249 (s6 H29) means that even with the displacements fixed the loads would be based on a C pseudo seated in $a0/$v0, never on $t4 - and $t4 is written only by the island's own 'move $12, %0'. The positive control's RA residual states the whole remaining problem: target 'addiu v0,s3,44 ; lhu t5,0(t4) ; lhu t6,4(t4) ; move t4,v0 ; mtc2 t5,$0 ; or t5,t5,t6 ; sll t6,t6,0x10' against ours 'addiu a1,s3,44 ; lhu v0,4(a0) ; lhu v1,0(a0) ; move t4,a1 ; mtc2 v1,$0 ; or v1,v1,v0 ; sll v0,v0,0x10'. The only stream in which those loads can be based on $t4 is one where they are emitted inside the asm template - i.e. the gte_ldlv0 SDK macro body, which is exactly the policy question filed at docs/grind/borderline.md:370.
- verdict: KILLED
- kill_scope: class
- measured_on: HEAD 5dc32f12, pack-in-C island 2 (no banned construct), islands 1/3 as banked; measured BOTH with the single banked do-while(0) FAKE present (six forms, score 7) and FAKE-free (v_nowrap, score 19) - the PRE-RA lock is FAKE-independent
- predicate_cite: tools/gcc-2.7.2/cse.c:2720

## [s7] The chassis is unchanged since s6 and no banked instance kill needs voiding: best_ban_compliant.c still measures 7, candidate.c still 0, the FAKE-free control still 19, and the closest-to-target banked kill still 7.
- mechanism: Mandated dispatch chassis re-measure (the brief reported 'measurement unavailable') plus the mandated KILL RE-AUDIT, the floor having been flat for three sessions.
- probe: tmp/grind/func_800300B4/s7/probe.sh and pc.sh over v_base (= best_ban_compliant.c), v_zero (= candidate.c), v_nowrap (s6's hand-built FAKE-free control) and v_reaudit_hwptr (= rejected/named-hw-pointer-for-pack-low-half-s4-7.c, the banked instance kill whose form sits closest to the target).
- result: v_base 7, v_zero 0 (classify: IDENTICAL), v_nowrap 19, v_reaudit_hwptr 7 - every number identical to s3/s4/s5/s6 on HEAD 5dc32f12. No banked kill rests on a FAKE construct absent from the current best form, so none needed voiding. The re-audit added information earlier re-audits could not: v_nowrap and v_reaudit_hwptr both classify PRE-RA with the same multiset delta, so the closest-to-target banked kill was measuring the RA layer beneath an unbroken cse lock.
- verdict: CONFIRMED
