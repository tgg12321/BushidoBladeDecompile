# Evidence bank — calc_loc_mat_fw_80055B60

- [s1] [fable-blitz 2026-07-07] Queue state: distance 1108, verdict ASM-STRUCTURAL, status PARKED with 2026-06-09 audit reason 'REJECTED - reads as standard GCC-compiled C, do NOT auto-authorize'. Exactly 1 rule: asmfix.txt:211 replace_with_asmfile - the src body (src/text1b.c:11655) is an empty stub, so the 1108 distance IS the whole unwritten function, not a plateau. This is a from-scratch decomp, not a wall-grind.

- [s1] [fable-blitz 2026-07-07] DUPLICATE-LEAD DISPELLED: calc_loc_mat_fw (code6cac_b, 1202 asm lines) vs calc_loc_mat_fw_80055B60 (1225 lines) - difflib SequenceMatcher on label/address-normalized instruction streams gives ratio 0.0117 (12 diff-ops covering essentially the whole body). Same splat first-pass naming tag only (named_syms.txt:3553-3555 'first-pass subsystem tag (calc)'); one solution will NOT close both. Do not re-run this comparison.

- [s1] [fable-blitz 2026-07-07] Semantic identity: per-player CPU input decision function. s2 = D_80101EC8 + arg0*0x44C (marionation player struct, stride 0x44C; prologue L4-14) held in s2 for the whole body; arg0 spilled to sp+0x30 (reloaded once at L1170 for the block[arg0]=4 sh), arg1 (output ptr) spilled to sp+0x38. Epilogue writes s2->0x3D0..0x3E4 (6 words: key, trig-ish derived masks) then copies them to arg1[0..0x14] (L1191-1211) - the shape of PSX pad key/now/trig processing (nor = ~key at L1171, and-not chains L1176-1181).

- [s1] [fable-blitz 2026-07-07] Self/opponent select (L25-36): if (s2->0x3E8 & 1) {a1=s2->0x0; s0=s2;} else {s0=s2->0x0; a1=s2;} with sw zero,0x3CC(s2) unconditionally in the delay slot (L29). 0x3E8 is a frame counter (incremented at L294-295), so self/opp view alternates per frame parity.

- [s1] [fable-blitz 2026-07-07] R3 rand-flag ladder (L74-261, the RA-sensitive core): 4x jal func_80079154 with threshold tests against (s16)(s2->0x438) >> {19,16,17,20} sll/sra shifts; each boolean lands in a distinct callee-save via a LAZY shift in the NEXT region's branch delay slot: fp=s1<<3 (L117 delay), s7=s0<<4 (L132 delay), s6=a1<<7 (L159 delay), s5=a0<<8 (L174 delay), s4=a1<<9 (L187 delay), s3=a0<<10 (L202 delay), s1=a1<<11 (L222 delay); then one 6-term or-reduction at L256-261. The a0/a1 temporaries ping-pong through a chain of move-ID compare ladders (0x2/0x1B/0x28/0x26/0x11 then 0x13/0x1B/0x30 then 0x6/0x4/0x14 twice). Natural C: flag_bN locals each assigned (cond)<<K where the ASSIGNMENT statement sits between compare ladders (GCC schedules the sll into the next ladder's first delay slot).

- [s1] [fable-blitz 2026-07-07] Region map (13 regions): R1 prologue+self/opp select L1-36; R2 atan2 angle single_game_getEnemyCharId(dF4-delta, dFC-delta) -> 0x43A masked to +-0x800, abs -> 0x43C (L37-65); R3 rand-flag ladder (above); R4 move-ID ori bits 0x1000/0x2000/0x4000/0x8000/0x10000 for IDs 0x15/0x19/0x1A self+opp (L262-289); R5 sw 0x430, 0x3E8++ counter, 0x3F5 saturating counter with 8-way move-ID reset ladder (L291-319); R6 D_8009A088[moveId] srav bit-test -> conditional 0x20000 or clear 0x...FDFFFF (L320-372); R7 0x43E angle-approach mult with D_80099D8D/8F[idx*0x18] bytes + 0x3F0 hysteresis inc/dec (L373-434); R8 dist=opp->0x148 - self->0x148 -> 0x442 in {1,2,0} (L436-454); R9 if (0x430 & 0x15500) jal func_80056CB8(s2) (L456-463); R10 0x428 target-slot management: file_GetFlag1 gate, 8-iter min-scan loop over u8 pairs at s2+0x414 stride 2 (L577-637), slot write-back, else path with ang_hosei_80056FE8 -> s4, 0x426 in {1,2,3} vs D_800A387C, 0x42A/0x42C/0x42E stores, second 8-iter scan loop setting 0x20/0x60 bits (L645-769); R11 0x425 via 12-iter loop over D_80106A78 stride 0x64: dx^2+dz^2 -> jal func_8007E11C (sqrt) -> two atan2 calls -> angle-diff/dist gates (L771-881); R12 giant bit-0x1 gate chain setting/clearing bit 0x2 with ~15 move-ID compares + D_80099D88[idx] mask tests (L883-1067); R13 0x8000-table bit -> and -0x79 + D_80102790&0x100 -> 0x40000 (L1068-1095), 0xE>=6 -> ori 0x78 (L1096-1105), retry loop (<=4 iters): jal (s2->0x3B4 ? func_80055948 : func_80058580)(s2) while ret==-1 (L1106-1133), then the sp-block key/trig build + lwl/lwr copy + output (L1134-1211).

- [s1] [fable-blitz 2026-07-07] R13 packed copy: lwl/lwr from D_800A3258+3/+0 -> swl/swr to sp+0x2B/0x28 (L1142-1146) = assignment of a 4-byte align-1 aggregate (u8[4] wrapped in a struct, GCC 2.7.2 emits lwl/lwr pairs for align<4 struct assign); the copy target is then indexed by s2->0x441 as u8 array (L1150-1154). The 6-word output block at sp+0x10..0x24 is built as: sp18=result-of-retry-loop (0 if -1), key bit-tweaks for move 0x19 (sllv by table byte) / 0x13 (ori 4), sp18 truncated to u16 (lhu at L1169 then re-sw), sp24=~key&0xFFFF, sp1C=key&~old(0x3D8), sp20=~key&old; ((s16*)block)[arg0]=4 at L1182-1186 - a struct { } local written field-wise then block-copied twice (to s2->0x3D0 and to arg1).

- [s1] [fable-blitz 2026-07-07] Callee inventory + status: func_80079154 (rand, 4 calls), single_game_getEnemyCharId (misnamed atan2, 3 calls: L47, L831, L839), func_8007E11C (isqrt, L810), file_GetFlag1 (L568), ang_hosei_80056FE8 (L673 - own grind ledger exists, INCOMPLETE), func_80056CB8 (L462 - next function in src/text1b.c:11660, INCOMPLETE with pins), func_80055948 + func_80058580 (retry-loop pair L1113/L1118; func_80058580 is the 2991-line slog-kengo-dead-end function). All calls are plain jals - callee completion status does not block this function.

- [s1] [fable-blitz 2026-07-07] No jump tables (zero jtbl refs), no GTE/cop2 ops, no hi/lo tricks beyond two plain mult (L386 signed mult by table byte, L802/L808 squared-distance mults feeding func_8007E11C). Four bounded loops, all counter-style (8-iter x2 stride 2, 12-iter stride 0x64, 4-iter retry). Park audit's 'standard GCC-compiled C' reading is confirmed by this pass: prologue is textbook (ra/fp/s7..s0 descending at L15-23), all addressing is %hi/%lo or GP-free struct offsets.

- [s1] [fable-blitz 2026-07-07] RA observation for R3: the or-reduction at L256-261 consumes t0(sp+0x50 reload of s3-saved early flags), fp, s7, s6, s5, s4, s3, s1 in EXACTLY ascending shift order (2,3,4,7,8,9,10,11 after the <<2 from s0). The early flags (0x40060-masked 0x430 | rand-bit<<0? at L74-92) are computed BEFORE the ladder and spilled to sp+0x50 (L93) because all 8 callee-saves + fp are exhausted - the C likely had ~9 live flag locals; a first draft should declare them in target consumption order and let priority allocation assign s-regs by first-def order.

- [s1] [fable-blitz 2026-07-07] R7 detail worth pre-encoding: 0x43E = 0x3F8[idx2] + ((0x404[idx2] - 0x3F8[idx2]) * D_80099D8D[0x443*0x18]) >> 8 where idx2 = s2->0x86<<1 (s16 index, L373-398); then a1 = D_800A387C - (s16)0x43E; s4 = (D_80099D8F[0x443*0x18]*25)>>3 (the sll/addu x3 chain L403-407 = v*3*8+v = v*25 then srl 3); hysteresis: if (s4 < abs(a1)) { sign-mismatch check vs 0x3F0>>15 resets 0x3F0; then 0x3F0 += (a1>=0 ? +1 : -1) } else 0x3F0 = 0 (L408-434). Encode this arithmetic once, from this note - it is the only nontrivial math in the function.

- [s1] [fable-blitz 2026-07-07] m2c reference decomp captured at tmp/blitz/m2c_calc_loc_mat_fw_80055B60.c (633 lines, --valid-syntax, clean run, 2-arg signature void (s32 arg0, void *arg1)). NOTE: current src stub declares 4 args (s32 x4) - target only homes/uses a0/a1; keep whatever signature the caller requires but only 2 are read.

- [s1] [fable-blitz 2026-07-07] Template siblings: no COMPLETED near-duplicate exists (not in tmp/duplicates_leads.txt). Closest structural relatives for R13's key/trig block: the pad-processing shape (key, ~key, key&~old, ~key&old written as consecutive words then double block-copy) - search src/ for '0x3D0' '0x3D8' writers among completed functions in text1b/code6cac_c when drafting; the marionation player-struct offsets (0x430 flags, 0x438, 0x43A/0x43C/0x43E angles, 0x414 slot pairs) already have precedent in completed marionation-family matches.

## s3 (2026-10-01, laneA) — landing body and its Ruling 11 (D) record

Landing body = probes/r11/landing_body-0.c (= candidate.c; sandbox --disable all 0, 0 source-level / 0
operand-only, with the PracticeMenuRec members of mkhdr.py). Scoring harness: probes/r11/sbx.py + sweep.sh (engine
sandbox with the mkhdr.py header ahead of include/). Simplifications measured byte-neutral and adopted: the flag word
is one expression (no flags / b3..b11 locals; probes/receipts); `rec->unk_428 = cond ? opp->unk_5C : 0xFE` without a
local; `!= 1 && != 2` and `== 0x16 || == 0x17` spellings; D_80106A78 walked by byte offset as its other consumers do.
Receipts kept: `* 25` (signed) 1; slot-loop `for (i = 0; ...)` init 2; me/opp as two ternaries 3 (probes/receipts/).

Reused locals (Ruling 11; i on Q51, below). Values (target registers in brackets):
- work [a1]: D_800A387C - unk_43E; its sign (work >> 31, read by the unk_3F0 reset test and the +-1 step).
- temp [s0]: the unk_148 distance (unk_442 band); slot count - 1 clamped at 0; the slot increment (4 or 8 by
  unk_6A == 0x11, then + old count, clamped at 0xFF); the 0x20/0x60 flag; the wrapped ratan2() difference; the
  poll result of func_80055948/func_80058580.
- temp2 [s4]: (unk7 * 25) >> 3; func_80056FE8() result; SquareRoot0() distance.
- temp3 [a1]: the least slot count (from 0x100); func_80056FE8() + 800.
(A) all four are declared at function scope, the innermost scope enclosing their writes (top-level statements for
work/temp/temp2; both arms of the unk_430 & 0x80 if/else for temp3). (B)(2) path records (Ruling 5 2(c)): `temp = 4`
can follow a slot count of 4 left by the scan, but with every scanned count 0 temp holds 0 there; `temp = 0x20` can
follow a distance of 0x20, but with equal unk_148 positions temp holds 0; `temp3 = 0x100` follows an indeterminate
value (temp3 is only written in the exclusive other arm); clamps write only when the value is outside range.
(C)(1) one-variable-per-value spelling: probes/r11/onevar_PV-91.c (mkpv.py; only declarations and identifiers
differ, each value's local at its innermost scope) = 91. (C)(3): every value is a load, arithmetic or call result in
the target, except the per-branch constants 4/8 and 0x20/0x60 (Q20) and temp3's 0x100 start (the `li a1,256` of the
scan value, whose other writes are loads).
(D)(1) probes/r11/d_proof.txt (dproof.sh: build cc1 -da dumps, then the instrumented cc1 with BB2_ALLOC_DEBUG /
BB2_FINDREG_DEBUG on the same tu.i, for landing body pseudos 79-83 and every per-value pseudo).
(D)(2) mechanism (tools/gcc-2.7.2/global.c find_reg 940-1150, set_preference 1671-1754):
- temp: the wrapped ratan2() difference is computed from the first ratan2() result, which local-alloc keeps in s0
  across the second call; set_preference gives the destination a full preference for s0. Per value only `da`
  carries it (pseudo 787 -> s0); dist -> v1, n -> a0, add -> a0, mask -> a1, ret -> v1 (pseudos 81/602/592/722/82).
  As one pseudo (80, priority 40566, allocated first) all six values take s0, the target's register in every region.
- temp2: the SquareRoot0() value is live across the two ratan2() calls, so the shared pseudo (81) crosses 2 calls:
  call-used registers are excluded (find_reg 970-975) and s4 is the lowest callee-saved one free of its conflicts
  {s0-s3}. Per value: lim is block-local (local-alloc, v0), near -> a0, len -> s5.
- temp3: shared pseudo 82 conflicts {v0,v1,a0,s0,s2-s4} -> a1 for both values; per value least (591) has a1 in its
  conflict set -> a2, far (656) -> a1.
- work: pseudo 79 carries a full preference for a1 (own_full_prefs 5) -> a1 for both values; per value
  diff (79) -> a0, sign (484, preference v1) -> v1.
(D)(3)/(4) measured, sandbox --disable all on the full TU (probes/r11/):
- full split 91; structural respellings: all per-value locals at function scope 91, declaration order reversed 91.
- per-value ablations (one value split, the rest shared): dist 3, n 48, add 81, mask 3, da 74, ret 6, lim 4, near 6,
  len 20, least 4, far 4, sign 14; per variable fully split: temp 67, temp2 16, temp3 4.
- permuter campaigns from the full split (probes/r11/permuter_harvest.txt; mini-TU workspace with the full-TU
  function asm, perm_compile.sh / perm_mk.sh): 2652 + 9478 iterations (-j 2, about 35 min), base 1165, best 490,
  no 0; every find re-uses a per-value local for a further value, adds a dummy constant or splits a statement.
i (Q51): one counter for the four loops (slot scan, flag scan, D_80106A78 scan, poll retry), as SOTN's
AddToInventory (src/dra/5D5BC.c:141-198 @aa53500, splat.us.dra.yaml `[0x5D5BC, c, 5D5BC]`) reuses `i` for its two
loops (:173, :183). Per-loop counters measured: slot 43, flag 5, objects 31, retry 5 (probes/r11/ablate_i_*).

## s3b (2026-10-01) — layer-2 round 1: split verdict (= FAIL), body a6f34d363c20ad3f
- rev-55B60-r11 PASS: Ruling 11 (A)-(F) for temp/temp2/temp3/work; ablations re-measured; all 31 partitions of temp
  nonzero (probes/rev55b60r11/partitions_results.txt); Q51 for i holds. Reviewer respellings banked in
  probes/rev55b60r11/ (resp_results.txt: mask ternary 6, far inline 10, sign inline 14, lim inline 10, add ternary 0;
  resp2: add split + ternary 81, all-temp split + ternary 67, PV + ternary 91). `temp = unk_6A == 0x11 ? 8 : 4` (0)
  is the simpler spelling of the increment and is adopted.
- rev-55B60-dm FAIL (data model / casts), objections = next frontier:
  (1) `extern u8 D_80106A78;` scalar walked by byte offset (14 puns): type the 0x64-byte records in code6cac.h,
      `extern Rec D_80106A78[12]`, respell every C consumer (func_80030580, func_80030D7C,
      code6cac_b_tu2.c:4231/4364/4921, code6cac_b.c:133) as its own preparatory landing.
  (2) retire named_syms rows g_status_flag_record_table_80099D88_plus_5 and g_practice_lesson_init_done_plus_4.
  (3) no (u16) casts on unk_6A: correct the header field to u16 (header-type-correction-from-use-sites; check
      96471164a, which flipped it to s16), byte-neutral for every consumer.
  (4) disclose/respell the second handle D_80101F4E (code6cac.h:724) over unk_86 (func_800218C8/func_80021904).
  (5) type unk_3B4 as a pointer (func_80055948 loads/stores one) or disclose.
  Rejected body: rejected/layer2-r1-datamodel-fail-0.c. Tree reverted; rebuild == oracle.

## s4 (2026-10-01, laneA) — preparatory landing for dm item 1: D_80106A78 / D_800A36F2 typed
Obj80106A78 (0x64-byte record) + `extern Obj80106A78 D_80106A78[12]` and `extern u8 D_800A36F2[2]` in
include/code6cac.h; 12 consumer bodies in src/code6cac_b_tu2.c respelled (func_80030208, func_8003043C,
func_8003047C, func_80030524, func_80030580, func_800307D0, func_80030900, cpu_set_move_command_and_dir,
func_80030BA8, func_80030D04, func_80030D7C, func_80031B24); per-word labels D_80106A7A/80/82 retired from
undefined_syms_auto.txt and named_syms.txt. Harness: probes/prep_d6a78/ (conv.py automatic obj-walk respelling,
manual.py hand bodies f_*.c, hdr.py, land_p1.py; tucheck.py scores every function of a modified TU against
build/src). Measured: func_80030208 goto+members 33, index `for` 51, pointer `for` 4, init order swapped 0;
func_80030BA8 goto+members 8 (f_80030BA8_goto.c), index `for` 34, pointer `for` 0; func_80030D7C members with
scalar D_800A36F2 20 (sched.c true_dependence 834-836 escape lets the D_800A36F2 load pass an in-struct velocity
store), D_800A36F2[2] array read 0. Whole TU: 81/81 functions 0; full build == oracle.

## s4b (2026-10-01) — preparatory landing FAILED; reverted; cluster plan written
rev-d6a78-a FAIL (data model: old externs D_80106A7A/80/82 still declared; +0x0C..0x2B is a MATRIX; record-only
callees func_800300B4 / func_8002FF20 / func_80031890 still pun via `(u8 *)obj`; per-body debt in func_80030208,
func_8003043C, func_8003047C, func_80030524, func_80030580, func_800307D0). rev-d6a78-b: FAIL func_80030D04
(`neg` constant holder; literal 0/17); PASS func_80030900, cpu_set_move_command_and_dir, func_80030BA8,
func_80030D7C (D_800A36F2[2] honest; sched.c 834-836 verified: scalar-style read 20, [0] 0; Ruling 11 on the new
body: PV2 21, temp-only 4, work-only 17 — probes/rev_d6a78b/), func_80031B24. Verdicts in each function's
memory/grind/<func>/layer2.jsonl. Staged hunks reverted; rebuild == oracle. Next: cluster-plan-2026-10-01.md.

## s5 (2026-10-01, laneC) — re-baselined on main d688fc1e5 (PracticeMenuRec typed by b3843cc02 / cb42a7dea)
- Body (candidate.c = probes/r11b/landing.c): the s3 body with the (u16) casts on unk_6A and (s16) on unk_438
  dropped (both members are now u16 / s16 on main), unk_A1[0]/[1] and unk_A3[0]/[1] (main's arrays), `temp =
  rec->unk_6A == 0x11 ? 8 : 4`, func_80056FE8(rec) / func_80058580(rec) without casts (PracticeMenuRec * prototypes),
  and the D_80106A78 scan through `Obj80106A78 *obj = &D_80106A78[i]` members. Header on top of main (probes/d6a2/
  hdr.py): u16 unk_5C, s16 unk_6C, PadState unk_3D0 (func_80055138's four word stores respelled .held / .pressed /
  .released / .unheld, mksrc.py), u8 unk_414[8][2], s16 unk_43E. Every text1b function 0 (tucheck).
  Remaining interface casts at calls, as func_8002AB08's `(u8 *)self` (cb42a7dea): func_80056CB8((s32)rec),
  func_80055948((u8 *)rec) (their completed bodies keep their own parameter types).
- D_80106A78 cluster (rev-55B60-dm (1), cluster-plan steps 1-2, redone; probes/d6a2/): rejected-p1 patch +
  MATRIX member (local tag Obj80106A78Mat), extern s8 D_8008E338[27][5], D_80106A7A/80/82 externs deleted,
  D_8008EBA0[22] (dlabel = 22 shorts; func_80031890 indexes it by limb), PracticeMenuRec s16 unk_332[12]
  (0x332..0x349, func_80030B10 shifts up to 12; func_80022580's three scalar uses -> [0]). Bodies f/*.c:
  func_800300B4 / func_8002FF20 / func_80031890 (ent) on Obj80106A78 members; func_8003047C / func_80030580 /
  func_800307D0 / func_80030900 / cpu_set_move_command_and_dir / func_80030BA8 on PracticeMenuRec members;
  literals instead of neg/val holders; range tests `k >= 0x12 && k < 0x1E` and `-15 <= vy <= 15` style instead
  of the unsigned casts (all byte-neutral); func_80031B24's `ch` = &g_practice_menu_table[other]. Interface
  casts kept at calls into unchanged prototypes: func_80032854 (u8 *), func_80030B10 (u8 *), func_80027AD8
  (u8 *), func_800274BC / func_8005344C (s32 *), RotMatrixX/Y/Z (TU prototype s32 *). code6cac_b_tu2 81/81,
  code6cac_b, code6cac_tu2 94/94 all 0 (chk.sh). func_800300B4 island 2 operand `&arg0->unk_2C` changes its
  region hash [1] (rh.py); the other two canonical bodies keep theirs.
- Ruling 11 (D) re-measured on this exact body (probes/r11b/scores.txt; mkpv.py / abl2.py / abl_i.py adapted):
  landing 0, PV 91; single-value splits dist 3, n 48, add 81, mask 3, da 74, ret 6, lim 4, near 6, len 20,
  least 4, far 4, sign 14; per variable temp 67, temp2 16, temp3 4; Q51 per-loop counters slot 43, flag 5,
  objects 31, retry 5 — identical to s3. (D)(1) dumps
  probes/r11b/d_proof.txt: pseudos 79-83 and every per-value pseudo get the same hard registers, priorities
  and conflict sets as s3 (renumbered: mask 723, da 788, near 656, len 761, far 657); the s3 mechanism text
  stands.
