# Evidence bank — single_game_SetStatusUpData

- [s1] [fable-blitz 2026-07-07] Rule inventory: ONE rule -- asmfix.txt:76 replace_with_asmfile; stub src/text1b.c:11565 `void single_game_SetStatusUpData(s32 arg0, s32 arg1, s32 arg2)`. Distance 514; floor 514. Park = rejected distance>500 canonical misroute.

- [s1] [fable-blitz 2026-07-07] Base pointer: p = D_80101EC8 + arg0*0x44C, computed via the exact multiplier chain (((a*17)*4+... = 275a*4); the two IMMEDIATELY ADJACENT completed functions func_8005509C (src/text1b.c:11540) and func_800550E8 (:11555) use the committed spelling `u8 *p = (u8 *)&D_80101EC8 + arg0 * 0x44C;` -- reuse it. arg1 (s2) = own movelist base (stored at p+0x3A4), arg2 (s4) = opponent movelist base.

- [s1] [fable-blitz 2026-07-07] Entry copy: p->0x443 (u8 char id) = lhu p+0xA; p->0x438 (s16 XP/score) = lhu p+8. Mode dispatch on s16 D_800A38DC is a compare CHAIN, not a jtbl (beq 1 / slti 2 / beqz 0 / beq 2 / beq 3, default falls through) -- source-level switch(mode) with cases 0..3; check branch ordering against switch-vs-ifchain-branch-sense when drafting (the slti-2 split suggests GCC's balanced case tree, i.e. a real switch).

- [s1] [fable-blitz 2026-07-07] case 1 (story?): p->0x438 = ((D_800A3783 - 1)/5)*3*256 + 0x400 (signed magic 0x66666667 /5), clamped to 0xD00 via s16 re-read (sll16/sra16 then slti -- the sh THEN re-extend pattern: write, re-read as s16, compare, conditional overwrite). case 0: if (D_800A3680 == D_800A3671) func_8005509C(lh p+4); if (statTbl[char].flags & 0x300) { src = D_8009A8C4 + (lh p+0x86)*32 + D_800A37A0*4; goto common-2-byte-copy; }. case 2 (survival?): if (D_800A389A) { p->0x438 = clamp(((D_800A37D2/10)&0xFF)*3*128 + 0x280, 0x1000) (UNSIGNED magic 0xCCCCCCCD u8 divides); if (D_800A37D2 % 5 == 0) func_8005509C(lh p+4); } else { n = (D_800A37D2/6)&0xFF (magic 0xAAAAAAAB); if (n >= 3) { D_800A37D2 = 0; n = 0; } p->0x443 = 0x19; p->0x1C = (n+2)<<10; p->0x438 = 0; p->0x424 = 0; p->0x3F6 = 0x3C - n*15; DONE (skips common copy); }. case 3 (practice): char = cpu_practice_honmokuroku_data_tbl[D_800A38E2 - 1] + 0x1B (u8 table, include/code6cac.h:10); base = ((D_800A38E2/10)<<4)&0xFF0; p->0x438 = base+0x80, or base+0x180 if flags&0x3000; += 0x200 if flags&0x4000; if ((D_800A38E2-1) % 5 == 0) func_8005509C(lh p+4) (signed 0x66666667 div-5 on E2-1); idx = ((D_800A38E2/10)&0xFF)*2, minus 1 if D_800A38E2%10==0; src = D_8009A9B4 + idx*2; common copy: p->0x424 = src[0]; p->0x3F6 = src[1].

- [s1] [fable-blitz 2026-07-07] Char stat table: D_80099D88, 24-byte records, lhu flags halfword at +0 indexed char*24 (spelled `*(&D_80099D88 + char * 12)` with u16 decl in the adjacent completed code at src/text1b.c:11594 -- 12 u16s), byte member at +3 = D_80099D8B (separate symbol; written `(func_80079154()&3)+1` when flags&0x100). Masks used: 0x300, 0x3000, 0x4000, 0xFF00, 0x100.

- [s1] [fable-blitz 2026-07-07] Post-switch common: if (file_GetFlag1() && mode != 3) p->0x438 = p->0x438*11 >> 4 (s16, (v*3*4-v)>>4); if (!(flags & 0xFF00)) p->0x424 = 0x11 - (s8-ish)(p->0x438 s16 <<16>>24 = high byte with sign); p->0x3BD = 0x10 - (u16 p->0x438 <<16>>24); p->0x39A = 0x8000 / p->0x1C (RUNTIME s32 div with break-7/break-6 guards = plain C division); zero p->0x444[0..7] (u8 loop, unsigned sltiu bound).

- [s1] [fable-blitz 2026-07-07] The big scan (:366-503): for (pl=0; pl<2; pl++) with per-player bindings -- pl==0: rec=p, list=arg1, char=p->0x443; pl==1: rec=*(p+0) (word at struct start = OPPONENT struct ptr), list=arg2, char=lh rec+0xA. mask = 1 << char. rec->0x40A = ((lh rec+0x1A) - 0x1000)*225 >> 11 ((v*8-v)*32+v = 225v). Then for (sec=0; sec<3; sec++): min=0xFFFF(t0), max1=0(t1), max2=0(t2); pl==0 stores current list cursor at p->0x3A8 + sec*4 (sw a3 gated by beqz t4; t6 walks +4); inner while (*(u16*)cursor != 0): entry = listBase + *cursor (halfword offsets); if (entry[4] == 0x40) { u32 m = entry[8]<<24|entry[7]<<16|entry[6]<<8|entry[5] (BYTE-ASSEMBLED little-to-big word -- unaligned u32 read spelled as 4 lbu|or, keep that spelling); if (!(m & mask)) skip; } stat1=entry[1]: if (!=0 && !=0xFF && < min) min=stat1; stat2=entry[2]: if (!=0 && !=0xFF) { if (> max1) max1=stat2; cat=entry[0]&7; if (stat2 > max2 && (entry[3]&0xF)*4 < 0x10 && (cat<2 || cat==7)) max2=stat2; } cursor += 2. After scan: if (lh rec+0xE < 6) triple = {0x7530,0x7530,0x7530}; else base = lh rec+0x40A + 100, triple = {base+min*40, base+max1*40, base+max2*40}; store sh at rec->(0x3F8+sec*2), (0x3FE+sec*2), (0x404+sec*2) (t3 walks +2) -- three s16[3] arrays at 0x3F8/0x3FE/0x404.

- [s1] [fable-blitz 2026-07-07] REGISTER PRESSURE SIGNATURE of the scan: inner temporaries live in t-regs (t0-t2 stats, t3/t6 walking stores, t4/t7 loop counters, t5 rec, t8 listBase, t9 mask) with NO spills and NO calls inside -- all locals; the 0x40A recompute (:383-391) happens ONCE per pl iteration (before the sec loop) but is INSIDE the .L800556BC join -- confirm placement when drafting (it sits between the pl-binding if/else and the sec loop).

- [s1] [fable-blitz 2026-07-07] Zero-init tail (:504-542): opp = *(s32*)p; p->0x40D=p->0x40C=0xFF (sb of -1); p->0x428=-1 (sh); p->0x425=p->0x426=0; words 0x3B4/0x3E0/0x3DC/0x3D8=0; opp->0x440/0x441=0 AND p->0x440/0x441=0 (interleaved sb pairs -- source order alternates opp/self); opp->0x43C/0x43A=0, p->0x43C/0x43A=0 (sh); p->0x362/0x39D/0x3F5/0x3F4/0x3F3/0x3F2=0; sh p->0x3EE/0x3F0/0x3E8=0; p->0x39C=0; sw p->0x394=0; sh p->0x398=0; sw p->0x3C4=0; sh p->0x3C2=0; p->0x3C1/0x3C0=0; p->0x440=0 AGAIN (:540 -- duplicated store in source, keep it); sw p->0x430=0; sw p->0x3E4=-1. The -1 value is rematerialized 3 times (addiu v0,-1 at :503,:507,:509) -- separate constants per store group, natural from distinct statements.

- [s1] [fable-blitz 2026-07-07] Divide/magic inventory for the draft: /5 signed x2 (0x66666667 sra1... case1 uses sra 1: /5; case3's func_8005509C gate uses sra 2 on (E2-1): also written %5? verify: :228-236 computes (E2-1) - ((E2-1)/5)*10?? -- sll2+add,sll1 = *10 -- so it is (E2-1)%10==0, NOT %5; recheck when drafting), /10 unsigned x3 (0xCCCCCCCD srl 3; one spelled srl 2 = /5 unsigned at :113 -- case2's first div is srl 2: D_800A37D2/5 not /10! verify each shift), /6 unsigned (0xAAAAAAB srl 1... 0xAAAAAAAB srl 1 = /3? unsigned /6 uses srl 2 -- :151 srl s5,1 with 0xAAAAAAAB = x/3; then slti <3... so n = D_800A37D2/3?? RECHECK all magic shifts against a reference table when drafting -- getting each divisor right is the main transcription risk), *11>>4, *15, *40, *225>>11, 0x8000/x runtime.

- [s1] [fable-blitz 2026-07-07] m2c reference: tmp/blitz/m2c_single_game_SetStatusUpData.c (257 lines, clean, no jtbl needed) -- use it to settle every divisor/shift question flagged above; mfhi results consistently land in s5 (an UNUSED callee-saved reg claimed by maspsx multu expansion -- s5 is saved in the prologue solely for this, plus s1/s3 for constants 0xFF/1 in the scan: expect GCC to allocate the 0xCCCCCCCD constant to s1 in case 3 since it is reused across the two multu sites).

## [s2] manual lane slotB3 2026-09-26 (function renamed single_game_SetStatusUpData -> func_80055138 by the naming sweep)

Floor table (all measured, sandbox `--disable all`; "wf" = whole-file scratch build
`integration/wf.py`, which splices the body into a COPY of src/text1b.c with the
D_80099D88 record-table declaration applied and func_80055948's use respelled
`D_80099D88[idx].flags`, then scores against build/src/text1b.o):
- first transcription (per-role locals): 156 -> 134 (bit test hoisted) -> 130
- honest per-role locals + every source-level fix below: sandbox 114, **wf 102**
  (candidate.c). Every remaining scored hunk is register seats + the frame
  (one callee-saved register fewer than target: s5).
- ONE function-scope scratch `v` for five roles (case-2 level `D_800A37D2/5`,
  case-2 else level `D_800A37D2/3` with its reset, case-3 table index, the loop's
  byte-assembled mask word, the loop's e[1] and e[2] stat values): **wf 0/516**
  (probe-shared-scratch-v-0.c), func_80055948 stays 0/127. Sandbox shows 12 only
  because the asm-label scaffold `g_sfr asm("D_80099D88")` leaves unresolved
  relocs; wf with the real declaration is 0.

Source-level facts proven by measurement (keep these in any spelling):
1. `switch (D_800A38DC)` cases 0-3, GCC's beq/slti tree; default falls through.
2. case 3 table: `cpu_practice_honmokuroku_data_tbl` is `u8 [][4]` (the target
   computes `(E2-1)*4` then indexes; the pointer-pun `&tbl + (E2-1)*4` folds the -1
   into the offset `-4(at)` — wrong). Needs the header retype (code6cac.h:10) and
   code6cac_b.c:5026 respelled `tbl[tableIndex]`; byte-neutrality of that consumer
   NOT yet measured.
3. case 0 table D_8009A8C4 is `u8 [][8][4]`; the target adds row*32+base BEFORE
   col*4: spell with a row pointer `row = D_8009A8C4[(s16)p->0x86]; src = row[col];`
   (every single-expression form, 8 measured, reassociates to row*32+(col*4+base)).
4. case 3 pair D_8009A9B4 is `u8 [][2]`; use a SEPARATE pointer local from case 0's
   (`pair = D_8009A9B4[idx]`), shared `src` costs 14.
5. mask bit: `bit = 1 << chr;` at the top of the section loop body (loop.c hoists
   it to the player loop and the constant 1 out of both, into s3 as in target);
   inside the while it folds to `srav/andi 1`.
6. per-player binding: `list = (u8 *)cursor;` inside each if/else arm (at the join
   costs 4).
7. zero loop `(p + i)[0x444] = 0` with an unsigned compare (`i < 8U`) gives
   `addu v0,s0,t4` operand order.
8. stat tests: `if (e[1] != 0 && e[1] != 0xFF) { s = e[1]; if (s < lo) lo = s; }`
   (assignment inside the test body -> the target's `move a2,v0` copy).
   `cat = e[0] & 7` computed after the hi1 update, before the hi2 test, u32.
9. tail: `c = rec->0x40A + 100; a = c + lo*40; b = c + hi1*40; c += hi2*40;`
   (a separate `bonus` local ties the lh into v1 = 6 pts); >=6 arm order
   `a = 0; c = 0x7530; b = 0x7530;`.
10. zero-init tail: `other = *(u8 **)p` read once; 0x40D/0x40C are s8 (-1 via
   `li -1`), stored 0x40D first.

Ablation from the 0 form (one role at a time moved to its own local; wf-equivalent
sandbox): zero-loop counter separate 37; m separate 14; case-3 index separate 45;
case-2 first level separate 16; stat values separate 29; case-2 else level separate
45; all five separate 114. Every role contributes.

Mechanism (instrumented cc1, BB2_ALLOC_DEBUG, idump.sh): the stat/mask pseudo gets
a2 in the target because the single pseudo is also live across case 2, where
local-alloc put the 0xCCCCCCCD constant in a0 and the `n*15` temp in v1; those
hard-reg conflicts push it off v1/a0 (e takes a1). Separate pseudos carry no such
conflicts and take v1, which shifts every loop register one seat (cursor a2, lo a3,
..., 0xFF into t9 instead of s1), drops s5 from the frame and moves mfhi to s4.
GCC 2.7.2 does not coalesce distinct user variables, so no per-role spelling can
inherit those conflicts.

Ledger files (s2): candidate.c = honest per-role body with the asm-label scaffold
(`g_sfr asm("D_80099D88")`, `g_cpt asm("cpu_practice_honmokuroku_data_tbl")`) so a bare
`sandbox --candidate` builds against today's text1b.c (sandbox 114; the unresolved g_sfr relocs
inflate it -- wf 102). integration/candidate-wf.c = the same body with the real
`extern StatusFlagRec D_80099D88[];` for `python3 memory/grind/func_80055138/integration/wf.py <file>`.
probe-shared-scratch-v.c (sandbox 12, all from the g_sfr scaffold) / integration/probe-shared-scratch-v-wf0.c
(wf 0/516) = the policy-blocked single-scratch body. NOT landable as is.

### [s2, later] permuter + full-build proof
- Permuter campaign 1 (per-job body, 1600+ iters, -j2): best finds put a single-write
  intermediate into the hi2 condition. Adopted the only clean one as a named intermediate:
  `low_cat = cat < 2; if (hi2 < s2 && (e[3] & 0xF) * 4 < 0x10 && (low_cat || cat == 7))`
  -> per-job floor **35/516** (header model, integration/wf2.py `--only`), sandbox 47
  (+12 from the g_sfr scaffold). candidate.c = that body; integration/candidate-hdr.c = the same
  body for the header model. Its extra pseudo lands in a2 and realigns the loop registers.
- Campaign 2 (from the 35 body, 2900 iters): only constant carriers (`s1 = 0; p[..] = s1`) and
  offset-in-variable forms (29-32). Rejected as banned shapes; stopped.
- Header model (integration/wf2.py, integration/apply_model.py): code6cac.h gains
  `StatusFlagRec D_80099D88[]` and `cpu_practice_honmokuroku_data_tbl[][4]`; text1b.c drops
  its local `extern u16 D_80099D88;` and respells func_80055948's read; code6cac_b.c
  func_80033DF4 reads `cpu_practice_honmokuroku_data_tbl[tableIndex]`. Scratch compare: all
  408 text1b functions and 88 code6cac_b functions instruction-identical with the one-scratch
  body. **Full-build SHA1 == oracle** (62efab4f...) 2026-09-26 with the one-scratch body
  (integration/probe-shared-scratch-v-hdr0.c) spliced + the header model applied under the
  landing lock, then reverted and rebuilt (oracle again).
- Why no per-job spelling reaches 0 (mechanism, checked against our builds): the target's
  case-2 level `andi a2,v0,0xff` and the loop's flag word `or a2,v0,v1` write $a2 while their
  other input dies in the same insn. A variable used inside one basic block is tied to that
  dying input by local-alloc (ours: `andi v1,v1,0xff`, `or v0,v0,v1`). So in the target these
  values sit in a variable that is live in other blocks too -- the shared scratch. Any
  per-job floor keeps at least those hunks plus the stat copies' seats.

## [s3] manual lane slotF 2026-09-26 — Ruling 11 landing preparation

Owner Ruling 11 (commit 262db111c; .claude/rules/ordinary-c-judge-decidable.md) admits a
reused local on allocator-dump necessity. Full (A)-(H) record: **ruling11.md** (this dir).
- Body: candidate.c (= r11/cand.c) is the s2 one-scratch body with `v` renamed `temp`
  (E)(i), the two-loop counter `i` renamed `idx` (also under Ruling 11, (E)(ii)), `mask`
  renamed `chr` (it is the character id; `bit = 1 << chr`), a/b/c renamed
  lo_val/hi1_val/hi2_val, `list = (u8 *)arg1/arg2` (was `(u8 *)cursor`, 0 either way), the
  four unused s2 declarations (lv, idx, m, bonus) dropped (0 either way), and declaration
  comments (F) plus the FAKE staged-value annotation on hi2_val.
- Scores (r11/model.py, header model, scratch TU vs build/src/text1b.o): reuse 0/516;
  per-value twin 102; ablations 4/33/33/2/6/11; counter split 25; unstaged base 6/6/8;
  two-variable partitions 98/2/11; structural twins 102/105/102/102. Whole-file: text1b 408
  functions and code6cac_b all instruction-identical with the model applied (r11/all.log).
- Mechanisms (dumps in ruling11.md): values 1 and 4 need a multi-block pseudo
  (local-alloc.c:472 + combine_regs tie in any per-value spelling); the $a2 seat is global.c
  find_reg over the union of the six values' conflicts; values 5/6 keep their copies only
  while temp is cse's canonical register (cse.c make_regs_eqv); idx's $t4 needs the pseudo
  live across the scan loop; hi2_val staging: a separate base local is single-block and
  tied to the dying lh result.

### Data model shipped with the body (r11/model.py `apply`)
- include/code6cac.h: `StatusFlagRec` (u16 flags; u8 unk2; u8 unk3; u8 unk4[0x14]) and
  `extern StatusFlagRec D_80099D88[];` — aggregate merge, prong (a) by base+offset
  addressing in the original binary: stride 0x18 (`sll 1; addu; sll 3` of the character
  id p[0x443]) in func_80055138 (+0 lhu x5, +3 sb), func_80055948 (+0 lhu), func_80055B60
  (+0 lhu x6, +5 lbu, +7 lbu), func_80058580 (+0 lhu x20, +3/+4/+6/+7/+0xC/+0x14/+0x15 lbu,
  +8/+0xF addiu bases); committed census name g_status_flag_record_table_80099D88
  (named_syms.txt row since 2026-05-17, 75434af56: "12-byte stride; u16 record" = 12 u16 = 0x18 bytes). (b) a record
  table of the evidenced size; (c) no C names D_80099D8B..D_80099D9D; their
  undefined_syms_auto.txt rows get the `alias of D_80099D88+0xN; retire with <sibling>`
  suffix (func_80055B60 / func_80058580 still INCLUDE_ASM); D_80099D90 (+8) has no config
  row (asm/data dlabel only); (d) canonical in the shared header; the TU-local
  `extern u16 D_80099D88;` in text1b.c is removed and func_80055948's per-use pun
  `*(&D_80099D88 + idx * 12)` becomes `D_80099D88[idx].flags`; (e) byte-neutral (text1b
  whole-file identical; full-build SHA1 at landing).
- `cpu_practice_honmokuroku_data_tbl` retyped `u8 [][4]` (named_syms.txt comment: 4-byte
  entries); code6cac_b.c func_80033DF4 `&tbl + (tableIndex * 4)` -> `tbl[tableIndex]`
  (code6cac_b whole-file identical).
- `extern u8 D_8009A8C4[][8][4];` (rows of eight 4-byte entries: 0x8009A8C4..0x8009A927,
  the target adds row*0x20 before col*4) and `extern u8 D_8009A9B4[][2];` in the same
  header. D_8009A8CA (+6, u16 read by still-INCLUDE_ASM func_80058580) lies inside the
  first; it HAS a config row (undefined_syms_auto.txt `D_8009A8CA = 0x8009A8CA;`, corrected
  after the layer-2 FAIL below — the earlier "no config row" was wrong), which the landing
  suffixes `alias of D_8009A8C4+0x6 (entry [0][1], byte 2); retire with func_80058580`; no C
  name. Prong (a) for the [][8][4] shape, independent of this function: func_80058580 reads
  the halfword at byte 2 of entry [row][col-1] with row<<5 + (col-1)*4 (0x8005A854-78:
  `lbu v0,D_800A37A0; lbu v1,0x440(s0); addiu v0,-1; sll v0,2; sll v1,5; addu; lhu
  s3,%lo(D_8009A8CA)(at)`), the same 0x20-byte rows of 4-byte entries func_80055138 indexes.

## [s3] layer-2 FAIL 2026-09-26 (body: rejected/r11-six-value-temp-l2fail.c)
Grounds (orchestrator relay): (1) (D)(3) did not exclude sanctioned-family per-value
spellings (dead store, self-assign, chain-extender, live-use detour, pointer alias,
do-while(0)); the permuter's do-while(0) find (102 -> 33) was dismissed as "adds a statement"
without a mechanism; model: memory/grind/func_8002DE20/ruling11.md (~272 on). (2) temp's values
2, 3, 5, 6 were admitted on measured effect (ablations) only: each needs a universal mechanism
argument or must come out of temp. (3) record error: D_8009A8CA HAS a config row
(undefined_syms_auto.txt:64) and func_80058580 uses it inside the D_8009A8C4 span: add
`/* alias of D_8009A8C4+6; retire with func_80058580 */` and cite func_80058580's
row<<5 + (col-1)*4 indexing (0x8005A868-78) as prong (a) evidence. (The s3 evidence line
"D_8009A8CA ... has no config row" is WRONG.) Passed, not to be relitigated: D_80099D88
stride evidence, the nine alias suffixes, the func_80055948/func_80033DF4 rewrites, `idx`,
the hi2_val staging, the staged scope. Tree reverted, rebuilt to the oracle, lock released.
