# Evidence bank — func_80023F08

- [s1] Current state: whole-body cheat = asmfix.txt:103 'replace_with_asmfile asm/funcs/func_80023F08.s'; src stub in code6cac.c. Queue distance 2981 ~= full 2983-insn body. ASM-STRUCTURAL verdict is the distance>500 auto-route; 2026-06-09 canonical audit REJECTED hand-coded - compiled C awaiting decomp.

- [s1] Scale/shape: 2983 insns (0x80023F08-0x80026DA0), 226 labels, 0x250-byte frame, 15 backward branches, SINGLE exit (jr $ra @ 0x26D9C), 65 calls to 38 unique callees. Crucially NO jr $v0 switch dispatch -> NO jtbl-infra wall (unlike sibling monster func_80058580). Structurally the EASIER of the two 2900+ monsters.

- [s1] Function role: per-player CPU move-command executor. Entry (asm lines 2-84): s1 = &D_80101EC8[arg0*0x44C] (17x->69x->276x->275x->1100x=0x44C strength-reduced stride, lines 5-11); +0x3C call counter++; copies 6 words arg1[0x0..0x14] -> s1+0x24..0x38; then 4x inline bit-repack of +0x2C/30/34/38: (v&0xFFF)|((v&0x8000)>>3)|((v&0x7000)<<1) (input-encoding conversion), gated by D_800A38BA!=0 and arg0 in {0,1} && s16 +0x6 == 0.

- [s1] SHARED-STRUCT payoff: D_80101EC8 stride 0x44C is the SAME robot-AI struct func_80058580 receives by pointer (both use u16 state +0x6A, opponent ptr +0x0; func_80058580 uses +0x425..0x449, this uses +0x24..0x82). code6cac_b.c:893 (COMPLETED) already spells 'var_s0 = &D_80101EC8 + idx * 0x44C' - raw-offset convention, no struct header; one offset map serves both monsters.

- [s1] Six GCC-2.7.2 movstrsi inline block-move expansions = the 12 paired backward branches: (0x24EF8/0x24F4C), (0x24FA4/0x24FF8), (0x252FC/0x25350), (0x253B0/0x25408), (0x26294/0x262E8), (0x26390/0x263E8). Each = runtime '(src|dst)&3' test dispatching lwl/lwr unaligned loop vs lw/sw aligned loop + 4-byte tail (asm lines 1100-1143). Copy size 0x84 = one move record; e.g. src = D_800A3888[arg0] + rec*0x84 (33x<<2 stride, lines 1091-1099) -> sp+0x9C, second copy -> sp+0x18.

- [s1] Block-move mechanism lead: the RUNTIME alignment dispatch means cc1 could not prove either pointer's alignment -> source was almost certainly builtin memcpy(dst_ptr, src_ptr, 0x84) on plain pointers, NOT a struct assignment (struct assign of a 4-aligned 0x84 type would emit the aligned loop only). Note bb2_memcpy in matched siblings (code6cac.c:1520) is an extern CALLED function - different idiom; no matched-src precedent found yet for the INLINE expansion.

- [s1] Real (non-blockmove) loops: 0x24448/0x24460 exit stubs + 0x244C4 move-record list walker (record ptr +0x50: level byte +0x8 vs +0x40, count u16 +0xA, data +0xC; input-bit test (1 << s16 +0xA) against record halfwords, lines 397-409) and 0x24E40 region state 0x15/0x33 handling reading byte table D_8008D9EC[+0xA].

- [s1] Direction-select if-chain at 0x244F4 (lines 410-445): (rec & 0x30) in {0,0x10,0x20,0x30} selects one of the four repacked words +0x2C/+0x30/+0x38/+0x34 into s2; (rec & 0x2000) zeroes it; state +0x6A==0x11 && D_800A38AE==arg0 also zeroes. m2c reconstructs this as if-chain (2 'switch' in m2c output are if-trees; no real jtbl).

- [s1] Command-echo global cluster written on several exit paths (lines 340-346, 370-381): D_800A376E (u16 flag=1), D_800A36D8 (u32), D_800A36CA (u16), D_800A381C (u16) - plus per-struct +0x7C (u32 cmd), +0x80/+0x82 (u16), +0x4C, +0x5E. Heavy callees: func_80021424 x10, func_80021A98(idx, +0x7C, s16 +0x80) x7, func_8001F860 x4 - all with existing C bodies.

- [s1] Init/side-effect singleton callees confirm role and give semantic anchors: md_game_rob_data_init, efc_rob_Init, single_game_SetAbilityData, camera_set_zoom, game_GetPlayerData, cpu_set_move_command_and_dir, cpu_check_tubazeri_2, cpu_check_same_dir_timer, scratchpad_Save/Restore, hirahira_w_ctrl, coli_check_circle_hit_line. 13 refs to Judge (sine table) = trig/angle math sections.

- [s1] m2c reference decompile SUCCEEDED clean first try (no jtbl needed): tmp/blitz/m2c_80023F08.c (1337 lines, 0 errors, 12 do + 12 while loops, 21 gotos, 47 stack locals). The 0x250 frame layout (record staging buffers sp+0x18 and sp+0x9C, 0x84 bytes each) is recoverable from m2c's sp-offset locals.

- [s1] No transplant shortcut: slog-kengo-dead-end memory records func_80023F08 (2983) has NO Kengo equivalent at function, sub-region, or callee-signature level. First-principles only.

- [s1] Unread bulk regions for later sessions: 0x240A0-0x24414 and ~0x25430-0x26290 (the big state-machine middle with most singleton calls); everything read so far is ordinary field-compare/branch code in the style of matched cpu_* siblings.

## [s2] 2026-10-01 laneC manual session — scaffold to sandbox 0 (local data model, not landable yet)
- m2c regenerated: s2/m2c.c (`python3 tools/m2c/m2c.py --valid-syntax asm/funcs/func_80023F08.s`, s2/m2c.sh).
- Floor history (sandbox --disable all --candidate, each banked as candidate.c at the time):
  v1 508 (first typed draft) -> v4 357 (array/block-scoped stack locals) -> v8 268 -> v12 227 -> v15 52 -> v16 17 -> v17 3 -> v18 **0**
  (2983/2983 insns, 0 source-level / 0 operand-only hunks). v18 == candidate.c at this commit.
- v18 still uses a candidate-local mirror struct (R23 / Pose23 / Move23 / ScrPad23) and several casts; it is a
  byte-exact SCAFFOLD, not a landable body. Data-model + policy cleanup is the next step (see hypotheses.md [s2]).
- Mechanisms found (each measured in the full function):
  * Frame: GCC 2.7.2 gives every BLKmode local 8-byte alignment and allocates address-taken scalars lazily, so the
    target frame implies `Pose pose[2]` (0x18/0x9C), `Vec3i32 v[2]` (0x120/0x12C), `s32 pos[3]`, `s16 ang[4]`
    (0x148; ang[0]/ang[1] = pose[0]/pose[1] facing), then &dir (0x150) / &sid (0x154) at first address-take, then
    block-scoped dv[3]/tgt[3] (0x28 arm), MATRIX m1/m2 (rot arm), vc (31A arm).
  * 0x84-byte Pose copies have the runtime (src|dst)&3 dual loop because Pose holds only 16-bit members
    (alignment 2): plain struct assignment, not memcpy. `rec->unk_1F8 = v[1]`, LeafPos scratchpad copies,
    `rec->unk_C8 = rec->unk_B8`, `rec->unk_24C = rec->unk_104`, `rec->unk_24 = *pad` are struct assignments too.
  * Pointer-select loads (`D_8008E0BC` row, dispatch[c ? 0x17 : 0x18]): fold-const distributes `p + (c ? i : k)`
    into `c ? p+i : p+k` with p SAVE_EXPR'd; `x < 4 ? x : 3` folds to MIN_EXPR instead (branchy index), so the
    clamp must be spelled `x >= 4 ? 3 : x`.
  * `obj = rec->unk_5E == 0 ? .. + 1 : 0` (ternary) keeps cse from carrying unk_58[1] into the add arms; the
    if-form lets jump.c turn `add = unk_58[1]` into a store-flag AND mask.
  * State local: two sites load unk_6A into an int local before the test (0x86=0x84 test, 8/0x22 test): the
    target loads 6A before the 7A branch / rematerialises li 8 — only reproduced with an int local.
  * 0x8C block: if/goto shape (`beqz h -> clear; j set`); COND-in-if gives bne/j.
  * The two `40 >= A5 && 40 <= A6` range tests must not be textually identical (fold factors
    `(A && R) || (B && R)` into `(A||B) && R`): second spelled `A6 >= 40`.
  * 0x62|=8 block: gate1 then `if (DC==3 && arg0==1 && 384C==4) {opp checks} else if (gate2) {self checks}`,
    0x10/0x20 parts written in both arms (cross-jump merges).
  * Mask test `(ent[2] | ((u32)ent[3] << 16)) & (1 << cls)`: with an int OR combine rewrites to srav/andi; the
    u32 OR keeps li 1 / sllv / and.
  * Store-order (sched2) fixes: 0x23 arm B8,D8,1F8; 0x28 arm 104x,104z,72,74; D8 += 104 in x,y,z; 168 x,y,z;
    `+= 1` (not ++) on unk_288; 15E/160/162 written per if/else arm (cross-jumped).

## [s3] 2026-10-01 laneC — landing body (header data model, policy pass)
- Landing body: memory/grind/func_80023F08/candidate.c (== tmp body5.c; needs the header/consumer edits of
  s3/hdr_edit.py + s3/tu_edit.py). Built with those edits (s3/xbuild.py over every TU that includes
  code6cac.h): func_80023F08 0/2983; every other function in every such TU byte-identical, except
  func_80020D70 (score 0; relocation addend only: D_800A388C -> D_800A3888+4).
- Data model (include/code6cac.h): MotionFrame (0x84, 16-bit channels), MoveScript (u16 ids, u8 frame bounds /
  flags at +7..+9, command list at +0xA), PracticeMenuRec gains PadState unk_24 (replaces unk_22[]/unk_2C/
  unk_30/unk_34[]; code6cac_b.c func_80026DA4's 7 reads become unk_24.held/.pressed), unk_42/44/46, unk_4A/4C,
  MoveScript *unk_50 (text1b.c: 2 reads each in func_80055B60 / func_80058580 become unk_50->unk_08),
  u16 *unk_54, unk_62..unk_68, unk_70, unk_74/78/7A, MoveScript *unk_7C, unk_80/82, unk_94, SVec4i16 unk_98,
  unk_A5..AC, unk_B3/B4, unk_154, unk_1DA, LeafPos unk_25C, u16 unk_288[2], MotionFrame unk_290, unk_314..31C,
  Vec3i32 unk_320; ScrPad + SPAD move from code6cac_b_tu2.c to the header; D_8008DA50/94/D8 become s16[];
  D_800A36D8 MoveScript *; D_800A3888 MotionFrame *[2] (func_80020D70 respelled; D_800A388C extern retired);
  func_80023F08(s32, PadState *) and its three callers drop `(s32)&buf` / `(s32)&sp10`.
- Every member width agrees with the original accesses (lhu on s16 members only where the body casts
  (u16) for a range test or does a read-modify-write / plain copy).
- Locals (the scaffold's d/a/b/state/t were split or renamed and each split measured):
  * temp: four values (unk_14C limit, folded angle gap, turn step, stick side) — Ruling 11 package below.
  * face (if/else, one value) and face90 = face + 0x400 (once-written, 4 reads): splitting them out of the
    scaffold's shared temp is byte-identical (r11/split_face measured 0), so they are not in temp.
  * frac, twist, perp: once-written, read 3/2/2 times. state x2: block-scoped once-written named
    intermediates, FAKE family 6 (fake/ below). table x4: block-scoped u16 * holding func_80021424's
    dispatch record (types the void * result; the cast spelling is byte-identical).
  * r (old heading) removed: `twist = -ratan2(old) + ratan2(new)` evaluates the old heading first, 0.
  * 0x62 |= 8 block: the 0x10/0x20 parts are written once after `else goto skip_62` (no duplication; 0).
- Ruling 11 (temp), record in r11/: one-variable-per-value split_all 25 (2982 insns); per-value ablations
  lim 6, gap 7, turn 3, side 25; all 14 non-trivial set partitions of {lim,gap,turn,side} nonzero
  (r11/partitions.txt: 3..25); structural respelling split_block (each value block-scoped) 25; permuter
  campaign from split_all (r11/permuter_harvest.txt: 3032 iterations, every find borrows another local or changes the arithmetic, none 0). Mechanism (r11/d_proof.txt, instrumented cc1
  BB2_FINDREG_DEBUG / BB2_ALLOC_DEBUG on the same tu.i): global.c find_reg's pass loop (global.c:1052)
  takes the first allowed hard reg outside the allocno's conflicts. Shared, temp is one allocno with the
  union conflict set {2,3,4} -> $a1 for all four values, as the target. Split: lim conflicts {2} with
  own_full_prefs {3} -> $v1; gap {2,3} -> $a0; side {2,3,4,5} -> $a2; turn is block-local and goes to
  local-alloc.c block_alloc -> $a0.
- FAKE named intermediates (fake/): site 1 `state` before the 0x86 = 0x84 test: direct reads score 3 (+2
  insns): jump.c thread_jumps redirects the 0x23 test's unk_7A == 0 branch past this test and the 6A load
  follows the branch (fake/site1_final_s.txt; target lhu 0x6A at 0x80024C10 before beqz 0x80024C14);
  u16 state 1. Site 2 `state` before the 8/0x22 test: direct reads score 11: the second test compares
  against the first test's constant pseudos in .cse (fake/site2_cse.txt); target li 8 / li 0x22 at
  0x80025780 / 0x80025788.
- [s3b] Cast / goto pass (orchestrator review list): (u16)rec->unk_0E < 2 -> (unk_0E == 0 || unk_0E == 1),
  byte-identical; (u32)ent[3] << 16 kept (no cast 3); the two interface casts and the gotos kept with
  measured alternatives — casts/receipts.txt. candidate.c updated (bytes == the s3 body).
- [s3c] Checklist 1001c pass: the (u32)ent[3] cast is a value-preserving fold-const dodge, so the mask is now a
  `u32 mask` named intermediate (FAKE family 6, fold-const.c:4437 mechanism, fake/mask_jump.txt); bytes
  unchanged. Checklist item 4 scan: ~250 pre-existing raw `*(T *)(p + 0xNN)` sites in src/ reach offsets this
  data model types (many are PracticeMenuRec handles in u8 *-typed bodies: code6cac_b_tu2.c 111,
  code6cac_tu2.c 44, text1b.c 34, text1a_pre_tu2.c 24, text1b_tu1c.c 23, ...) — scope question to orchestrator.
