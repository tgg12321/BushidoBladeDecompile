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
