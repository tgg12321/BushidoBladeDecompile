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
