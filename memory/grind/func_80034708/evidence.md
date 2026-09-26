# Evidence bank — func_80034708

- [s1] [fable-blitz 2026-07-07] Rule inventory: exactly ONE rule -- asmfix.txt:86 `replace_with_asmfile asm/funcs/func_80034708.s`; stub at src/code6cac_b.c:4080 `void func_80034708(void)`. Queue distance 542; floor = 542 until a C draft lands. Park is the rejected distance>500 canonical misroute; pure-C work.

- [s1] [fable-blitz 2026-07-07] Shape: (A) D_800A37B8++ then func_80079154() then 14 DispSleepMenuTex(label, cursorColor, value[, ...]) rows -- cursorColor = (lh gp-rel D_800A3174 or D_800A3176 == rowIdx) ? &D_800A3180 : &D_800A3178, alternating 3174/3176 per row; labels D_800A3188..D_800A31D0 (8-byte stride) plus rodata D_80010834/D_80010840; (B) for (i=0;i<2;i++) pad-input loop on D_80102794 word: bits 0x1000/0x4000/0x8000/0x2000 << (i*16) = up/down/left/right; up/down move cursor (bounds: i==0 max 0xB, i==1 max 3, func_8005C650(0,0x7F,0x7F) click); left/right run func_8005C650(4,0x3F,0x3F) then switch(cursor) via jtbl_8001086C (dec) / jtbl_8001089C (inc), both 12 cases, already migrated to C rodata at src/code6cac_b_rodata_pre.c:5,11; (C) per-iteration wrap tail: *p7C = (*p7C+0x21)%0x21 (magic 0x3E0F83E1), *p7E = (*p7E+8)%8 (signed shift-div), *p80 = (*p80+2)%2; (D) after loop: D_80102784 = (D_80102784+0x26)%0x26 (magic 0x6BCA1AF3), D_80102785 = (D_80102785+7)%7 (magic 0x92492493), D_80102786 &= 1, D_80102787 &= 1, D_800A36F9 = wraps to 0..3 via andi 0x1FC subtract trick ((v+4) - ((v+4>=0?v+4:v+7)&0x1FC) pattern, i.e. %4 signed), D_800A3690 &= 1; then if (D_80102794 & 0x08000800) { func_8005C650(1,0x7F,0x7F); DispSamnailWindow(); }.

- [s1] [fable-blitz 2026-07-07] Jump-table case targets are per-cursor value edits: cases 0-2 dec/inc s8 bytes via walking pointers s1/s2/s3 (= D_8010277C/D_8010277E/D_80102780 + i); case 3 u16 D_80102778[i] -=/+= 0x80 (lhu/sh, i indexed via s6=i*2); case 4 D_80102784--/++ (via s7 reg); case 5 D_80102785 (direct lui lbu/sb); case 6 D_80102786 (via fp=s7+2); case 7 D_80102787 (materialized address, lbu 0(v1)/sb -- asm lines 344-350, 435-441); cases 8-9 SHARED between both tables: D_80106A73 ^= 1 / ^= 2 (via s5); case 10 D_800A36F9--/++; case 11 D_800A3690--/++. Shared cases 8/9 appear in BOTH jtbls at the same addresses -- single source blocks reached from two switches = cross-jump merge of identical case bodies (xori is direction-agnostic), NOT duplicated source.

- [s1] [fable-blitz 2026-07-07] THE TYPING CRUX: phase-A reads D_8010277D/7E/7F/80/81 and D_80102784/85 with plain `lb` (direct lui+%lo) -- bare lb is only reachable from a SIGNED decl (an (s8) value cast on a u8 gives lbu+sll24/sra24 instead, 3 insns; pointer-pun *(s8*)& is the forbidden coercion family). Current include/code6cac.h:401-409 declares 7C..87 all u8. code6cac.c already re-declares 7D/7F/81 as s8 (lines 56,60,61) -- per-TU redeclaration precedent exists on main.

- [s1] [fable-blitz 2026-07-07] CONSISTENCY REQUIREMENT vs func_800343F0 (SAME TU, src/code6cac_b.c:3969 area): its ledger (memory/grind/func_800343F0/evidence.md) plans header retype of D_80102787 u8->s8 and keeps 84/85/86 u8 with (s8) VALUE casts (its target has lbu+sll/sra for 84/85/86). But THIS function's target reads 84/85 with bare lb -> 84/85 must be s8-typed in this TU. Joint resolution: retype 84/85 (and 87) to s8; then func_800343F0's three cast sites become (s8)(u8)D_801027xx (zero-extend narrows the lb to lbu via combine.c, outer (s8) re-emits sll24/sra24) -- byte-equivalent to its current committed form but derived from the s8 decl. 86 can STAY u8: this function's phase-C `D_80102786 &= 1` is lbu (native u8) and its phase-A lb of 86 goes through the s5 pointer (see next fact), not the decl.

- [s1] [fable-blitz 2026-07-07] Phase-A pointer view: s5 = &D_8010277C held across the whole phase; reads value[0] (lb 0(s5), row 1), value[0xA]=D_80102786 (lb 0xA(s5), row 12) and value[0xB]=D_80102787 (lb 0xB(s5), row 13) SIGNED through it, while rows 2-6 use direct named globals. Source shape: a local `s8 *p = &D_8010277C;` (no cast needed once 7C is s8; main already has the cast precedent `s8 *p = (s8 *)&D_8010277C` at src/code6cac.c:1502) with p[0]/p[0xA]/p[0xB] for exactly those three rows. This also satisfies func_800343F0's caveat: 87's phase-A read here is via the s8 pointer, and its three unsigned sites (case-dec :346, case-inc :437, `&= 1` :534) need (u8) VALUE casts under the s8 decl (combine.c zero_extend-of-load -> lbu).

- [s1] [fable-blitz 2026-07-07] Under the s8 retype of 87, this function's unsigned sites spell: `D_80102787 = (u8)D_80102787 - 1` / `+ 1` (lbu+addiu+sb, address materialized once for load-modify-store) and `D_80102787 = (u8)D_80102787 & 1` (lbu+andi+sb, separate lui/%lo pair per the direct-addressing shape at asm :533-538).

- [s1] [fable-blitz 2026-07-07] Phase-B register story: s7=&D_80102784 anchors derived pointers s1=s7-8 (=7C+i), s2=s7-6 (=7E+i), s3=s7-4 (=80+i), fp=s7+2 (=86), s5=&D_80106A73, s6=i*2, s0=&D_800A3174+i*2 (cursor s16 walk); s1/s2/s3/s6/s0 all advance at loop tail (walking pointers -- see walking-pointer-serializes-parallel-loads rule; the addiu-from-s7 anchoring suggests GCC CSE'd nearby constant addresses off one base, which falls out naturally when the pointers are initialized together before the loop).

- [s1] [fable-blitz 2026-07-07] MIXED WIDTH through the SAME registers: dec/inc cases read s1/s2/s3 with lbu (asm :296,:302,:308,:387,:393,:399) but the wrap tail reads the same pointers with lb (:474,:486,:501). One register serving both signedness = two source-level views coalesced (e.g. u8* for the -- / ++, s8 wrap arithmetic via (s8) value cast... but (s8) cast on u8* deref emits sll/sra which the tail does NOT have -- tail is bare lb). Most consistent source: pointers typed s8* (bare lb in tail is native) and the dec/inc cases spelled with (u8) value casts (combine narrows to lbu). Verify in draft.

- [s1] [fable-blitz 2026-07-07] Wrap-tail arithmetic details: *s1 wrap uses +0x21 then %0x21 -> `*p = (s8)((*p + 33) % 33)`; *s2 uses +8 then signed %8 (bgez/addiu 0xF/sra 3/sll 3 sequence = C signed x%8); *s3 +2 then %2 (srl-31 round); D_80102784 +0x26 %0x26; D_80102785 +7 %7; D_800A36F9 +4 then signed %4 via andi 0x1FC (0x1FC mask = ~3 on the 9-bit range; GCC 2.7.2 signed %4 with known-small range). Magic constants 0x3E0F83E1 (/33), 0x6BCA1AF3 (/38... verify: pairs with sra 4), 0x92492493 (/7) all standard expand_divmod signed-magic; write plain % and they fall out.

- [s1] [fable-blitz 2026-07-07] Callee set: func_80079154(void) (extern src/code6cac_b.c:92), DispSleepMenuTex (include/code6cac.h:441, declared (s32,s32,s32,s32) -- NOTE rows pass only 3 args; 4th arg gap to check when drafting), func_8005C650(s32,s32,s32) (extern :2900, used nearby at :3106/:3499 -- same-file sibling func at :3499 also does the func_8005C650(counter,val,val) click idiom), DispSamnailWindow(void). Row 7/8 values: (u16)D_80102778 >> 8 and (u16)D_8010277A >> 8 (lhu+srl -- D_80102778 declared s16 include/code6cac.h:399, needs u16 view or cast for lhu; check srl vs sra).

- [s1] [fable-blitz 2026-07-07] m2c reference generated with jump tables supplied (tmp/blitz/m2c_func_80034708.c, 305 lines, 0 M2C_ERROR; tables at tmp/blitz/jtbl_80034708.s). Cursor state: D_800A3174/D_800A3176 are gp-rel s16 (lh %gp_rel) -- the 2-iteration loop walks them as an s16[2] via s0.

## [s2] manual lane slotB4 2026-09-26 — first C body, floor 542 -> 8 (under -G8), 90 (under -G0)

Renames since s1 (naming sweep 2026-09-25): func_80079154 -> `rand`, DispSleepMenuTex ->
`func_8003D52C` (printf-style debug print, defined src/code6cac_c2.c:1062 as
`(u8 *fmt, s32 first_arg, ...)`), DispSamnailWindow -> `func_800344B4`; D_80102794 is
`D_80102788.pressed` (PadState at 0x80102788, +0xC).

Measured (tmp/func_80034708/gscore.py = engine scorer on a whole-file build of a COPY of
src/code6cac_b.c; reproduces the sandbox number exactly at -G0; `--g8` swaps only -G0->-G8):
- v1 per-symbol transcription: 268. v2 + aggregate 0x78..0x87: 199. v3 + colour locals: 155.
- v5 + flags byte as member +3 of an aggregate at 0x80106A70: -G0 144, **-G8 49**.
- v6 (= candidate.c) + ternary cursor wrap forms: **-G8 8, 544/544 insns, frame 64 matches**;
  -G0 90. The 8 at -G8 are all relocation-form artifacts (see F4).

F1. **The 0x7C..0x87 bytes are ONE aggregate in the original** (hard evidence, compiler-proved):
  the loop preheader derives &7C/&7E/&80/&86 from &84 (`addiu s1,s7,-8` / `-6` / `-4` /
  `fp = s7+2`); cse.c use_related_value only relates constants with the SAME symbol base.
  Rows 11/12 (`lb 0xA(s5)`, `lb 0xB(s5)`) and row 1 (`lb 0(s5)`) come out of the SAME
  mechanism with the aggregate declared: cse1 relates every phase-A byte to row 1's forced
  address reg; cse2 folds rows 2-10 back to constants inside its PATHLENGTH window and cannot
  for rows 11/12 (dump-verified, dump_v4 f.cse vs f.cse2, insn 76). With separate scalars
  none of this appears (v1). Tested with base 0x78 (`g_practice_lesson_size_a`, resolvable
  name) — the function does not discriminate 0x78 vs 0x7C as the base (case 3's
  D_80102778[i] block materializes its own base).
F2. **0x80106A73 (file flags) is a member at +3 of an aggregate at 0x80106A70 (>8 bytes)**:
  target CSEs its address in phase A (s2, rows 13/14) and copies it into the loop's s5;
  a plain scalar is a VAR_DECL mem (never forced into a reg) and stays direct (v3).
  func_80035280's FAKE `f = &D_80106A73; src = f - 3;` is the same base-relative shape.
F3. **The cursor s16[2] at 0x800A3174 is an array AND the TU was compiled -G8.**
  Phase A reads cursor[0]/cursor[1] gp-direct; the loop walks s0 = &cursor[i].
  Under -G0 an ARRAY_REF address is force_reg'd (explow.c memory_address) and a bare (reg)
  address is never folded back (find_best_addr only folds non-REG addresses), so cursor[0]
  stays in a register (v3: `lh 0(s0)` everywhere). Under -G8, ENCODE_SECTION_INFO sets
  SYMBOL_REF_FLAG on the 4-byte extern, mips_address_cost(symbol)=1 == reg, and cse
  substitutes the symbol -> direct gp reads exactly as target. Only other way to get the
  target phase A at -G0 is two scalars + `(&D_800A3174)[i]` in the loop (v4, score 75) —
  a cross-object pun, not admissible. Corroboration that the ORIGINAL TU was -G8: the
  <=8-byte strings of this function ("~c777", "%s%5d", "%sO%4d\n") sit in .sdata at
  0x800A3178.., the 9-byte ones ("%sCR%3d\n") in .rodata — mips_select_section's -G8 rule.
F4. Residual at -G8 (score 8) = relocation spelling only: 4x `%gp_rel(D_800A3174+2)` vs target
  `%gp_rel(D_800A3176)` (same linked bytes) + 2x jtbl `lw %lo(.rodata+0x43C)` vs the external
  jtbl_8001086C/8001089C (rodata placement; certified only by the full build).
F5. Frame: the target reserves vars=8 = ONE combine-orphan reload slot (BB2_FRAME_DEBUG);
  the if/else nest for the down-key wrap made three (sign-extend pairs whose HI value is
  reused). `cursor[i] < ((i != 0) ? 3 : 11)` + `cursor[i] = (i != 0) ? 3 : 11` gives exactly
  the target's two-compare / two-zero-store / shared-lhu-increment shape and vars=8.
F6. Whole-file -G8 on today's src/code6cac_b.c changes 19 functions (objdiff) and file-scope
  INCLUDE_ASM/INCLUDE_RODATA float under -G8 (TARGET_FILE_SWITCHING) — so landing needs a TU
  split: func_80034708 is followed only by func_80034F88/8003504C/80035280/80035430 in the file.

## [s2 cont.] slotB4 2026-09-26 — full integration proven: scratch full link SHA1 == oracle

F7. **The aggregate base is 0x80102778, not 0x8010277C** (compiler-proved): with the byte
  block based at 0x7C, row 1's address is a bare symbol and rows 2/3 stay s5-relative
  (score 12, 542 insns); based at 0x78 (row 1 = S+4) they fold back exactly as target
  (score 8, 544/544). Independent corroboration: func_8001C444 initialises exactly
  0x78..0x87 (named_syms' "parameters at 0x80102778-80102787").
F8. **The file flags byte is +0x23 of the 0x24-byte file record at 0x80106A50**
  (func_80037F40 checksums 0x24 bytes from 0x80106A50 as one block; func_800167EC
  initialises it; func_80035280 walks colour bytes from the flags address). Size > 8 is
  required under -G8 (a 4-byte colour+flags struct measures 46, not 8).
F9. **Every other consumer is byte-neutral** under both merges (objdiff of all 34 TUs rebuilt
  with the new headers: only relocation-addend lines differ), with ONE respelling:
  ings.c func_800167EC takes `p = (u8 *)&D_80106A50;` first (with the record merged, taking
  the flags address first makes cse relate the base to it; base-first folds back to the
  target's direct stores — measured variants a/b/c/d, tmp ings_try.py).
F10. -G0 closing forms: every honest array/struct spelling of the cursor measures 90
  (g0a `*D_800A3174`, g0b struct{c[2]}, g0c struct{p1,p2}); only two scalars + a
  `(&D_800A3174)[i]` cross-object loop reaches the artifact floor (4, rejected/
  g0-scalar-cursor-pun-score4.c) — a per-use pun, not admissible.
F11. Landing package (integration/*.py, applied by integration/apply.py from the live tree):
  include/code6cac.h `PracticeParams D_80102778` (0x78..0x87, 16 B) replacing D_80102778[2],
  D_8010277A, PlayerBytePairs D_8010277C, D_80102782..87; include/system.h `FileRecord
  D_80106A50` (0x24 B) replacing g_file_disc_type (and C uses of g_file_disc_size,
  g_file_flags, D_80106A58/5C/70/73); consumers converted in code6cac, code6cac_b,
  code6cac_b2_pre/post, code6cac_c2, code6cac_c_ab, code6cac_c_mid, replay_camera_rob_back_loose2,
  ings; TU split code6cac_b.c | code6cac_b3.c (func_80034708, -G8) | code6cac_b3_post.c
  (4 tail functions, verbatim); code6cac_b_rodata_pre.c loses the two hand-copied jtbls
  (b3.o emits them at 0x8001086C behind the pre file's lead word; SUBALIGN(2) keeps
  placement); prong-(c) retirements in undefined_syms_auto/named_syms/symbol_addrs/sdata_syms
  (D_8010277C and D_80106A70 rows stay, suffixed, for INCLUDE_ASM func_8003993C /
  func_8001BE20); asm/data/91C98.data.s one dlabel for the s16[2] cursor.
  tmp/func_80034708/fullbuild.sh: every TU rebuilt from the package + relink =
  62efab4f73f992798c43e8c730aa43baa10bb4fa. func_80034708 in code6cac_b3.o scores 8 =
  4x gp_rel(D_800A3174+2) vs (D_800A3176) + 1x jtbl lo16 section addend — relocation
  spelling only.

## [s2 cont.] cc1psx calibration (owner ruling Q2 2026-09-26: compiler-necessity + cc1psx)
F12. The landed src/code6cac_b3.c compiled by the ORIGINAL PsyQ cc1psx (tools/cc1psx_wrapper.sh,
  integration/psx_calib.sh): at -G8 it emits the 16 gp-direct cursor reads (`lh $2,D_800A3174[+2]`)
  exactly as our cc1 and the target; at -G0 it emits ZERO (all 20 cursor reads off a register),
  frame vars=8 at both. With the merged declarations cc1psx also produces the target's
  related addressing: `la $23,D_80102778+12` + `addu $19/$18/$17,$23,-4/-6/-8`, `lb $6,10/11($21)`
  for rows 11/12, `la $18,D_80106A50+35` (flags address CSE'd). Normalized cc1psx-vs-ours diff =
  104 lines, all scheduling of the per-iteration pad-word load (known cc1psx/KMC scheduling skew);
  the -G and object-model effects are identical in both compilers.
  Minimal span: the PracticeParams span 0x78..0x87 is the labels this TU and its consumers
  address (row-1 fold proves base <= 0x78; func_8001C444 initialises exactly 0x78..0x87);
  FileRecord 0x50..0x73 is the checksummed record (flags needs a >8-byte object; the record
  is the documented one).

## [s3] slotB4 2026-09-26 — layer-2 FAIL (s2 landing) and the PracticeParams frontier
Layer-2 FAIL of the s2 landing (integration/landing-s2-g8.patch = the exact staged diff, 22 files;
re-apply with `git apply --index` once -G8 is authorized): (1) new -G8 file unauthorized at the time
(owner Q5 since approved, rules pending); (2) PracticeParams 0x78..0x87 fails prong (a): mixed
layout blocks the compiler-necessity route's (a4), the 0x78 base rested on this function's scores,
and "func_8001C444 initialises exactly 0x78..0x87" was overstated (it never stores 0x82/0x83).
ACCEPTED by layer-2: the body, the FileRecord merge, the verbatim b3_post move, build-file edits.
(The ings.c func_800167EC base-first reorder belongs to the FileRecord merge, not PracticeParams.)

F13. **cc1psx (a2) test for the 0x78 base.** Same preprocessed code6cac_b3.c, -G8, two declarations:
  - base 0x7C (12-byte object over 0x7C..0x87, `u16 D_80102778[2]` separate):
    cc1psx emits rows 2/3 as `lb $6,1($21)` / `lb $6,2($21)` (base-relative) —
    NOT the target (integration/cc1psx/psx-G8-base7C.s); our cc1 identical (score 12, 542 insns).
  - base 0x78 (16-byte object): cc1psx emits `lb $6,D_80102778+5 / +6` exactly as the target
    (integration/cc1psx/psx-G8-base78.s); our cc1 score 8 = relocation spelling only.
F14. (a1) mechanism, dumps banked (tmp i7c/i78 .cse/.cse2 via integration/vdump.sh): row 1's
  address is force_reg'd. With base 0x7C that address is the bare SYMBOL_REF; cse1 relates rows 2/3
  to it and row 11 (S+10) is then related to row 2's pseudo (reg96 = reg85+1, `+9`), which keeps
  reg96's set live, so cse2 cannot fold rows 2/3 back to constants. With base 0x78 row 1 is
  CONST S+4, every later row relates to reg85 directly and cse2 folds rows 2-10 (not 11/12,
  PATHLENGTH window) exactly as the target. The decision depends on 0x7C NOT being the
  object's first byte: no spelling with the object starting at 0x7C reproduces it.
  Measured 7C-base respellings (all 12 or worse): s8-typed fields 12, `u8 unk_0[4][2]` 12,
  rows 1-6 through one block pointer 14.
F15. (a3) minimal span: the function references 0x78 (rows 7/8 lhu, case 3 [i]) through 0x87 —
  the 16-byte span is exactly the minimal span; 0x82/0x83 lie inside (merged per prong (c);
  func_80022F34 indexes 0x82[player], func_8003B3A4/B484 write 0x83).
  (a4) is NOT met literally: the span is a heterogeneous block (u16[2], four u8[2] pairs, four u8),
  not an array of one record. No single-record array spelling is honest (byte fields read as u8,
  0x78/0x7A read as u16). The only other evidence for the grouping is the 2026-05-17 naming census
  ("lesson parameters at 0x80102778-80102787", 60a758c35), which named the words separately.

## [s3 cont.] proof package for the re-landing (owner Q5 per-file -G8 + Q7 mixed-field struct, rules pending)
F16. Three spellings of 0x78..0x87 in the -G8 TU (integration/vscore.py; dumps in integration/dumps/,
  cse + cse2, gzip; cc1psx outputs in integration/cc1psx/):
  - A fully split (today's header: u16 D_80102778[2], 6-byte PlayerBytePairs D_8010277C, u8
    D_80102784..87): our cc1 102; cc1psx reads every row direct (`lb $6,D_8010277C..+5`,
    `D_80102784..87`) with no s7/s5 base registers (psx-G8-split.s). Under -G8 the 6-byte
    PlayerBytePairs is itself small data, so the base registers vanish entirely.
  - B 0x7C..0x87 one object, 0x78 separate: our cc1 12; cc1psx rows 2/3 base-relative
    (psx-G8-base7C.s).
  - C 0x78..0x87 one object (PracticeParams): our cc1 8 = relocation spelling only (0 on the landed
    tree); cc1psx exactly the target (psx-G8-base78.s).
F17. Draft -G8 prong (i): best honest -G0 score = 90 (every array/struct cursor spelling, F10);
  the -G0 floor 4 needs the cross-object `(&D_800A3174)[i]` pun (banned; set aside).
F18. Re-landing memberships (draft prongs iii-v), measured by integration/reland_check.sh
  (RELAND=1 builds every TU, relinks): code6cac_b3 in GP_FILES + EXPAND_LB_FILES +
  RODATA_ALIGN2_FILES; code6cac_b3_post in EXPAND_LB_FILES + RODATA_ALIGN2_FILES (both inherit
  code6cac_b's memberships; expand_lb_funcs.txt names only func_8003047C, so --expand-lb is inert for
  both; ALIGN2 is inert for b3 because its second table starts at 0x30, and b3_post has no .rodata).
  Result: SHA1 == oracle, func_80034708 score 8 (relocation spelling). bb2.ld: only the six
  code6cac_b3/_post section lines; no LINKED_ASM_FUNCS entry. The banked landing-s2-g8.patch lacks
  these four list memberships — add them to Makefile + engine/buildconfig.py when re-applying.

## [s3 cont.] re-landing conformed to the draft -G8 prongs (owner Q5 + Q7 approved; rules pending)
F19. Neighbours: the target's only gp-relative accesses in this region are func_80034708's own 16
  (`%gp_rel(D_800A3174)` x12, `%gp_rel(D_800A3176)` x4). func_800343F0, func_800344B4 (code6cac_b.c
  side) and func_80034F88, func_8003504C, func_80035280, func_80035430 (moved side) have ZERO
  gp-relative accesses, and no asm/funcs file other than func_80034708.s names D_800A3174/D_800A3176.
F20. Respellings proven byte-neutral BEFORE the split (integration/inplace.sh): both merges, every
  consumer conversion and the ings.c func_800167EC base-first reorder applied in place, with
  func_80034708 still INCLUDE_ASM in code6cac_b.c, no -G8, bb2.ld and all symbol files untouched ->
  every TU rebuilt, relinked: SHA1 62efab4f73f992798c43e8c730aa43baa10bb4fa == oracle. objdiff:
  every changed function differs only in relocation addends.
F21. The move is textually identical (integration/movecheck.py): the 302-line / 12600-byte block
  after func_80034708 in the in-place code6cac_b.c is byte-identical to the tail of
  code6cac_b3_post.c; the split code6cac_b.c differs from the in-place file only by the removed
  `/* kengo ... */` + INCLUDE_ASM(func_80034708) lines, the moved block, and the one extern used
  only by the moved block (`extern s32 *func_80077D00(void);`), which moves with it.
F22. Memberships (apply.py): code6cac_b3 -> GP_FILES, EXPAND_LB_FILES (inert: expand_lb_funcs.txt
  names only func_8003047C), RODATA_ALIGN2_FILES (mechanical: it emits jump tables at 0x8001086C and
  0x8001089C, both 4 mod 8; object .rodata now 2**2); code6cac_b3_post -> EXPAND_LB_FILES (inert),
  NOT RODATA_ALIGN2 (it emits no jump table / no .rodata). bb2.ld: six section lines only; no
  LINKED_ASM_FUNCS. integration/reland_check.sh with exactly these memberships: every TU rebuilt,
  relinked, SHA1 == oracle, func_80034708 score 8 (relocation spelling only).
  Best honest -G0 score: 90 (F17). landing-s2-g8.patch is superseded by apply.py (regenerates from
  the live tree, includes the memberships).

## [s4] slotB4 2026-09-26 — ledger for the re-landing under 965b3a001

### -G8 by proof, prong (i): every gp-relative access in func_80034708's original bytes
(asm/funcs/func_80034708.s; the listed set is {D_800A3174, D_800A3176} = the s16[2] cursor array)

| address | instruction |
|---|---|
| 0x80034760 | `lh $v0, %gp_rel(D_800A3174)($gp)` |
| 0x8003478C | `lh $v0, %gp_rel(D_800A3176)($gp)` |
| 0x800347B4 | `lh $v0, %gp_rel(D_800A3174)($gp)` |
| 0x800347D8 | `lh $v0, %gp_rel(D_800A3176)($gp)` |
| 0x800347FC | `lh $v0, %gp_rel(D_800A3174)($gp)` |
| 0x80034828 | `lh $v0, %gp_rel(D_800A3176)($gp)` |
| 0x8003484C | `lh $v0, %gp_rel(D_800A3174)($gp)` |
| 0x80034878 | `lh $v0, %gp_rel(D_800A3176)($gp)` |
| 0x800348AC | `lh $v1, %gp_rel(D_800A3174)($gp)` |
| 0x800348D0 | `lh $v1, %gp_rel(D_800A3174)($gp)` |
| 0x800348F4 | `lh $v1, %gp_rel(D_800A3174)($gp)` |
| 0x8003491C | `lh $v1, %gp_rel(D_800A3174)($gp)` |
| 0x80034944 | `lh $v1, %gp_rel(D_800A3174)($gp)` |
| 0x80034974 | `lh $v1, %gp_rel(D_800A3174)($gp)` |
| 0x800349A0 | `lh $v1, %gp_rel(D_800A3174)($gp)` |
| 0x800349CC | `lh $v1, %gp_rel(D_800A3174)($gp)` |

Adjacent functions that stay outside the new TU: func_800344B4 (before, stays in code6cac_b.c) and
func_80034F88 (after, moves to code6cac_b3_post.c) — ZERO gp-relative accesses each (so none to the
listed set). No asm/funcs file other than func_80034708.s names D_800A3174 or D_800A3176.
Best -G0 score: 90 (every honest array/struct spelling; F10), because under -G0 an ARRAY_REF
address is force_reg'd (explow.c memory_address) and a bare-REG address is never folded back by
cse.c find_best_addr, so cursor[0]/[1] stay register-based instead of the 16 gp reads above.
(ii): integration/cc1psx/psx-G8-base78.s = 16 gp-direct cursor reads (`lh $2,D_800A3174[+2]`),
psx-G0-base78.s = 0 (F12).
(iii) screening: the TU's <=8-byte data externs are D_800A3174 (4 B, sdata_syms), D_800A3690 /
D_800A36F9 (1 B, sdata_syms; sdata_exclude'd for func_80034708 as before) and D_800A37B8 (s32).
D_800A37B8 lies inside gp range (0x800A37B8 - _gp = +0x6EC) but is not in sdata_syms because no
original function addresses it gp-relative (10 referrers, all lui/%lo) — it belongs to another TU's
data; its only access here is the scalar `D_800A37B8++` (lw %lo / sw %lo), identical at -G0 and
-G8 (dumped: plain VAR_DECL MEMs are never force_reg'd). Every other extern is typed > 8 bytes
(PracticeParams 16, FileRecord 36, PadState 24) or is an incomplete string array.

### (a4′)(4) PracticeParams D_80102778 member table (every func_80034708 access, target bytes)
Base registers in the target: s5 = &0x7C (0x80034778), s7 = &0x84 (0x800349F4), s1 = s7-8 (&0x7C,
walks +1), s2 = s7-6 (&0x7E, walks +1), s3 = s7-4 (&0x80, walks +1), fp = s7+2 (&0x86),
s6 = 2*i (case 3 index).

| off | addr | member | width | type | accesses (address opcode) |
|---|---|---|---|---|---|
| 0x0 | 0x80102778 | unk_0[2] | 2x2 | u16 | 0x80034864 lhu (row 7, [0]); 0x80034890 lhu (row 8, [1]); case 3 [i], i=0..1: 0x80034B78 lui / 0x80034B7C addiu / 0x80034B80 addu s6 / 0x80034B84 lhu / 0x80034B94 sh, 0x80034CB8 lui / 0x80034CBC addiu / 0x80034CC0 addu s6 / 0x80034CC4 lhu / 0x80034CD4 sh |
| 0x4 | 0x8010277C | unk_4[2] | 1x2 | u8 | 0x80034778 lui / 0x8003477C addiu s5 / 0x80034780 lb 0(s5) (row 1, [0]); 0x800347A4 lui / 0x800347A8 lb (row 2, [1]); walker s1 [i], i=0..1: 0x80034A0C addiu s1,s7,-8; 0x80034B3C lbu / 0x80034B4C sb (case 0 dec); 0x80034C7C lbu / 0x80034C8C sb (case 0 inc); 0x80034DA0 lb / 0x80034DCC sb (wrap %33); 0x80034DFC addiu s1,+1 |
| 0x6 | 0x8010277E | unk_6[2] | 1x2 | u8 | 0x800347C8 lui / 0x800347CC lb (row 3, [0]); 0x800347EC lui / 0x800347F0 lb (row 4, [1]); walker s2: 0x80034A08 addiu s2,s7,-6; 0x80034B50 lbu / 0x80034B60 sb (case 1 dec); 0x80034C90 lbu / 0x80034CA0 sb (case 1 inc); 0x80034DD0 lb / 0x80034DF4 sb (wrap %8); 0x80034DF8 addiu s2,+1 |
| 0x8 | 0x80102780 | unk_8[2] | 1x2 | u8 | 0x80034810 lui / 0x80034814 lb (row 5, [0]); 0x8003483C lui / 0x80034840 lb (row 6, [1]); walker s3: 0x80034A04 addiu s3,s7,-4; 0x80034B64 lbu / 0x80034B74 sb (case 2 dec); 0x80034CA4 lbu / 0x80034CB4 sb (case 2 inc); 0x80034E08 lb / 0x80034E28 sb (wrap %2); 0x80034E34 addiu s3,+1 |
| 0xA | 0x80102782 | unk_A[2] (offset-named filler) | 2 | u8 | none — gap filler exactly 2 bytes (other consumers: func_80022F34 indexes [player], func_8003B3A4/func_8003B484 write [1]) |
| 0xC | 0x80102784 | unk_C | 1 | u8 | 0x800348C0 lui / 0x800348C4 lb (row 9); 0x800349F4 lui / 0x800349F8 addiu s7 (base); 0x80034B98 lbu / 0x80034BA8 sb (case 4 dec); 0x80034CD8 lbu / 0x80034CE8 sb (case 4 inc); 0x80034E3C lui / 0x80034E40 addiu a2 / 0x80034E44 lb / 0x80034ECC sb (wrap %38) |
| 0xD | 0x80102785 | unk_D | 1 | u8 | 0x800348E4 lui / 0x800348E8 lb (row 10); 0x80034BAC lui / 0x80034BB0 lbu / 0x80034BBC lui / 0x80034BC0 sb (case 5 dec); 0x80034CEC lui / 0x80034CF0 lbu / 0x80034CFC lui / 0x80034D00 sb (case 5 inc); 0x80034E64 lui / 0x80034E68 lb / 0x80034EEC lui / 0x80034EF0 sb (wrap %7) |
| 0xE | 0x80102786 | unk_E | 1 | u8 | 0x80034908 lb 0xA(s5) (row 11); 0x800349FC addiu fp,s7,2; 0x80034BCC lbu / 0x80034BDC sb 0(fp) (case 6 dec); 0x80034D0C lbu / 0x80034D1C sb 0(fp) (case 6 inc); 0x80034E70 lui / 0x80034E74 lbu / 0x80034E80 lui / 0x80034E84 sb (&= 1) |
| 0xF | 0x80102787 | unk_F | 1 | u8 | 0x80034930 lb 0xB(s5) (row 12); 0x80034BE0 lui / 0x80034BE4 addiu / 0x80034BE8 lbu / 0x80034BF8 sb (case 7 dec); 0x80034D20 lui / 0x80034D24 addiu / 0x80034D28 lbu / 0x80034D38 sb (case 7 inc); 0x80034E88 lui / 0x80034E8C lbu / 0x80034E98 lui / 0x80034E9C sb (&= 1) |

(a4′)(1) span: lowest byte the function accesses = 0x80102778 (row 7 lhu, case-3 [0]); highest =
0x80102787 (row 12, case 7, tail) — span 0x78..0x87 exactly; no byte outside it.
(a4′)(2) signedness: every byte member is declared u8; the `lb` reads are `(s8)` value casts
(rows 1-6 / 9-12 and the wrap reads), the `lbu` read-modify-writes and `&= 1` use the u8 member as
declared — no pointer pun. unk_0 is u16 (lhu everywhere). Arrays exactly where the function indexes
or walks (unk_0 [i], unk_4/unk_6/unk_8 walkers), length 2 = the reachable range (i = 0..1).
(a4′)(3) gap: 0x82..0x83 only, one offset-named filler `u8 unk_A[2]`.
(a1)/(a2) against the separate labels: F13/F14/F16 (dumps integration/dumps/, cc1psx
psx-G8-split.s / psx-G8-base7C.s / psx-G8-base78.s).

### (iv-a) RODATA_ALIGN2
code6cac_b3 emits jump tables at 0x8001086C and 0x8001089C in the shipped binary (both 4 mod 8) ->
ON the list. code6cac_b3_post emits no jump table (no .rodata) -> OFF the list.

## [s5] slotB4 2026-09-26 — second layer-2 FAIL (re-land under 965b3a001) and the fixes
FAIL objections: (1) -G8 (iii) screening: D_800A37B8 (s32) referenced, not in sdata_syms, not typed
>8 (reviewer also named D_800A36F9 / D_800A3690); (2) (a4')(5)/(d): cross-member pointer puns in
func_8003504C (`(u8 *)&unk_D - 9`) and func_8003B5A4 (`chardata = &unk_4[1]; chardata[2]` = the
next pair); (3) false header comment ("func_8001C444 initialises exactly this range"). Everything
else (body, FileRecord, move, bb2.ld, buildconfig, gp table, neighbours) passed. Reverted, rebuilt
== oracle, lock released; the staged diff is integration/landing-s4-rejected.patch.

Fixes (all in the generator, applied in place BEFORE the split; integration/inplace.sh: every TU
rebuilt, SHA1 == oracle with func_80034708 still INCLUDE_ASM; every changed function differs only
in relocation addends):
F23. PracticeParams now keeps the three per-player pairs 0x7C..0x81 as ONE member `u8 unk_4[6]`
  (unk_4[2 * k + player]). func_80034708 walks all three pairs with i in 0..1, so the reachable
  range is exactly 0x7C..0x81 ((a4')(2)); 0x82/0x83 stay the offset-named filler unk_A[2].
  Consequences: func_8003B5A4's `chardata = &D_80102778.unk_4[1]; chardata[0] / chardata[2]` now
  stays inside one array member (unchanged text, no cross-member step); func_8003AF40's other-player
  read is `(&D_80102778.unk_4[2])[(u32)arg0 < 1u]` (the flat `unk_4[2 + (cond)]` spelling makes
  fold() turn the index into a branch: 47 -> 52, measured). func_80034708's body uses unk_4[0..5],
  unk_4[i], unk_4[2 + i], unk_4[4 + i]; score unchanged (8, relocation spelling).
F24. func_8003504C (moves to b3_post): the `s8 *b = &unk_D; u8 *w = (u8 *)b - 9;` pair is gone;
  `D_80102778.unk_4[i] = lv; ... if ((s8)D_80102778.unk_D == 0) D_80102778.unk_4[i] = ... - 3;`
  — with one object cse derives the target's `a1 = t0 - 9` itself: byte-neutral.
F25. func_80035280 (moves to b3_post): the FAKE second handle `f = &flags; src = f - 3` (a step
  from the flags member into the colour member) is gone: flag reads name D_80106A50.flags and
  `src = D_80106A50.color;` follows them (order measured: src first = 110 insns, flags via a pointer
  = 104, this order = byte-neutral). Its clock-record byte re-view became `FileTimeRec *base =
  D_80106A50.times; base[i].unk_4 / base[i].unk_0` (byte-neutral; its FAKE comment's first line now
  describes the pointer). code6cac_b.c's two `s32 *flags = &D_80106A50;` became `&D_80106A50.unk_00`.
F26. Header comment corrected: "func_8001C444 sets every byte of it except 0x82/0x83".
F27. Move identity re-checked (291 lines, byte-identical); full split build SHA1 == oracle,
  func_80034708 score 8.
F28. Objection (1) is NOT fixed — no honest declaration exists: D_800A37B8 is a 4-byte word (splat
  dlabel 0x800A37B8..BB, next label 0x800A37BC), D_800A3690 an 8-byte-or-smaller blob, D_800A36F9
  one byte; none is truthfully >8 bytes, and D_800A37B8 cannot join sdata_syms (sdata_funcs members
  func_80035480 / func_80035618 / func_80035828 read it with lui/%lo). D_800A3690 / D_800A36F9 ARE in
  sdata_syms (sdata_exclude'd for this function, as before). Census (tmp ext_b3.sh): the new TU's
  <=8-byte externs outside sdata_syms = 1 (D_800A37B8); the approved -G8 files carry 29
  (text1a_pre 7, text1a_post 22, several outside gp range, e.g. D_800F64BA). Filed as a
  policy-question in docs/grind/borderline.md.
