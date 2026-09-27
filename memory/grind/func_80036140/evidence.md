# func_80036140 — evidence

## LAYER-2 FAIL #1 (2026-09-26 evening) and the restructure — READ FIRST

The single-landing package (gate commit + one Match commit carrying the split, the merges and the
body) failed layer-2 on two hard objections, both taken to the owner and answered favourably:
1. comm_syms row func_80036140, prong (a)(2): cc1psx+ASPSX differs from the shipped words (81/512;
   our cc1+ASPSX 68). Owner Q15: the row stands when EVERY gp/sym+N decision matches AND every
   other differing word is classified by address. Done: landing/q15/func_80036140.cc1psx-G8.txt
   (66 li-expansion, 10 cc1psx-scheduled instructions incl. 2 moved li's, 4 hazard nops,
   1 branch displacement; 0 unclassified; 25/25 gp/sym+N OK) and .ourcc1-G8.txt (68 li only).
2. -G8 (iv): the CdlATV merge rode in the split commit. Owner Q16: reverse order allowed as
   SEPARATE commits. Restructured into five commits, each built to the oracle from clean
   (landing/tools/apply2.py steps, t_steps.sh; per-commit diffs landing/steps/*.diff):
   A maspsx gate + registration | B split (cdrom_SetMix + func_80035F78 -> -G8 code6cac_b4.c,
   rest -> code6cac_b4_post.c; verbatim) | C merges (CdlATV + CdState through 0x80101EA7;
   func_80036140 still INCLUDE_ASM, its per-word rows kept as aliases) | D Match (func_80036140 +
   func_80036940 -> -G8 code6cac_b5.c, tail -> code6cac_b5_post.c, verbatim; the C body;
   alias rows retire) | E g_cd_result as libcd's u8[8] (see 3).
3. Soft objection: the body used per-byte handles g_cd_result_plus_0x3/4/5 inside the 8-byte
   result buffer while treating the CdlATV records as COMMON objects. Now merged (commit E) and
   the buffer joins the func_80036140 comm row. It cannot precede D: with an array,
   func_80036940 at -G0 keeps &g_cd_result in $s0 (score 5); byte-neutral once it is -G8.
Rejected body (single-commit landing): rejected/joint-g8-single-commit-2026-09-26.c (the D body).
Measured at rev da540504a: all six states (stock, A..E) SHA1 == oracle; A vs stock 36/36 objects
identical; commit-B TU through cc1psx+ASPSX (bases tentative, per-byte handles extern) reproduces
cdrom_SetMix / func_80035F78 exactly (0/18, 0/12), -G0 none; screening at every state identical
(landing/steps/screen_*); movecheck B and D byte-identical (landing/steps/movecheck.txt);
Q14 per-accessor identity on the final state (landing/q14_accessors.txt, 40/40 objects).

## CURRENT STATE (slotI, 2026-09-26, measured on scratch trees built from HEAD 97d8c71a7)

The whole landing is built and verified in scratch full builds (make run INSIDE a scratch copy of
the tracked build inputs, `landing/tools/mktree.sh` + `fullbuild.sh`; never the real tree). It is
NOT landed: one owner question is open (docs/grind/borderline.md 2026-09-26 "the CD state record
must run to 0x80101EA7; two of its new fields are used only by other functions").

| step (scripts in landing/tools/) | full-build SHA1 |
|---|---|
| gate only: `gate.py` + `register.py` on HEAD (func_80036140 still INCLUDE_ASM) | == oracle; all 36 `build/src/*.o` byte-identical to the stock tree of the same rev |
| ip_rec: record-extension respellings in place, no split, no -G8 (`apply_model.py --split none --ext rec`) | == oracle |
| P1 split A only, textual move (`--split A --ext split`): cdrom_SetMix + func_80035F78 -> -G8 `code6cac_b4.c`, the rest -> `code6cac_b4_post.c` | == oracle |
| P2 = P1 + CdlATV merge + record extension (`--split A --merge --ext rec`) | == oracle |
| P3 final (`--split final --merge --ext rec`): + func_80036140 C body; func_80036140 + func_80036940 -> -G8 `code6cac_b5.c`, cdrom_IsIdle.. -> `code6cac_b5_post.c`; transcribed jtbl_80010938 deleted | == oracle; per-function scores all 0 except func_80036140 = 2 (the jump-table `lw %lo(jtbl)` operand: section-relative reloc vs the reference's jtbl symbol, settled by the link) |
| engine test in a scratch tree with gate + registration | 805 passed / 0 failed (stock tree of the same rev: 801 / 0) |
| maspsx unit tests (gated copy) | 142 run, the same 2 pre-existing failures as stock (test_div_expand_li_nop, test_expand_li_0x1; 138 run) + the 4 new test_comm_syms tests pass |

### Layout (five TUs from code6cac_b2_post.c, original address order)
- `code6cac_b2_post.c` (-G0, unchanged flags; stays on RODATA_ALIGN2_FILES — func_80035828's table at 0x800108EC is 4 mod 8): func_80035828 .. bits_DepositMask3F83F8.
- `code6cac_b4.c` (-G8): cdrom_SetMix, func_80035F78.
- `code6cac_b4_post.c` (-G0): snd_SerialMixOn, cdrom_Init, cdrom_FlushInit, cdrom_ReadyCallback.
- `code6cac_b5.c` (-G8): func_80036140, func_80036940.
- `code6cac_b5_post.c` (-G0): cdrom_IsIdle .. func_80037540.
Why TWO -G8 files: with g_cd_atv a CdlATV (needed by func_80036140's struct copy, merge prong (d) puts it in the header), cdrom_SetMix scores 5 at -G0 (-G0 prices a symbol address 2 > a register 1, so the val0 store's forced address register is shared with `CdMix(&g_cd_atv)`: `la v0; sb a0,0(v0) .. move a0,v0` vs the target's `sb a0,%gp_rel(g_cd_atv)($gp); la a0`); at -G8 it is 0. The ORIGINAL toolchain agrees (calib below: COMMON model -G8 0 words differ, -G0 6). func_80035F78 goes with it (prong (i): it gp-accesses D_800A3854, also in cdrom_SetMix's set). snd_SerialMixOn, cdrom_FlushInit, cdrom_ReadyCallback have no gp access and cannot join a -G8 file (prong (iii)), so the two pairs sit in separate -G8 files.

### -G8 prong (i): gp-relative accesses in the ORIGINAL bytes (asm/funcs/*.s)
| function | gp accesses (address  insn  symbol) |
|---|---|
| cdrom_SetMix | 80035F34 sb a0 g_cd_atv; 80035F64 sh zero D_800A3854 |
| func_80035F78 | 80035F7C sb a1 D_800A36B8; 80035F90 sh a0 D_800A3854; 80035F94 sh zero D_800A3840 |
| func_80036140 | 80036140 lh t0 D_800A3854; 80036154 lbu v0 D_800A36B8; 80036158 lh a0 D_800A3840; 80036168 lbu v0 g_cd_atv; 800362D8 lhu v0 D_800A3840; 800362DC lh v1 D_800A3854; 800362E4 sh v0 D_800A3840; 800362FC sh zero D_800A3854; 80036850 lbu v0 g_cd_result; 800368A0 lbu v0 g_cd_result |
| func_80036940 | 80036C24 lbu v0 g_cd_result; 80036C74 lbu v0 g_cd_result |
Adjacent functions outside the new TUs, gp accesses in their original bytes: bits_DepositMask3F83F8 none; snd_SerialMixOn none; cdrom_ReadyCallback none; cdrom_IsIdle none. (Non-adjacent: func_80035828 gp-reads D_800A31D8/D9/DA, D_800A3740; cdrom_Init D_800A31E4 — none in either set.)
Best -G0 score of each body under the existing build, and why -G0 cannot produce the listed accesses:
- func_80036140: 22 (`vbmany.sh p3 both rec`). -G0 prices every symbol address 2, so cse keeps the ATV base addresses in registers across CdMix (s0/s1) instead of the target's gp-relative val0 reads + fresh `la` at the copy; at -G8 SYMBOL_REF_FLAG (mips.h:2556-2561 ENCODE_SECTION_INFO, size <= 8) prices them 1 (mips.c:1631-1632) and the sharing disappears.
- cdrom_SetMix: 5 with the merged declaration (reason above); 0 with today's per-byte splat externs (the merge is what forces -G8).
- func_80035F78, func_80036940: 0 at -G0 in our build — but only because maspsx's sdata_syms/sdata_funcs lists turn cc1's direct `sb/lbu sym` into gp form; cc1 itself emits no gp access at -G0 or -G8, and the ORIGINAL compiler+assembler at -G0 emits none (calib: b4/b5 COMMON -G0: 0 gp accesses, 6 / 62 words differ). They are in the -G8 files because prong (i) forbids leaving an adjacent function outside that gp-accesses a symbol in the moved function's set (func_80035F78 <- D_800A3854 with cdrom_SetMix; func_80036940 <- g_cd_result with func_80036140, owner Q10). Flagged for the reviewer: (i)'s "reason -G0 cannot produce" is met for them only in this sense.

### -G8 prong (ii) + maspsx_comm_syms prong (a): the ORIGINAL toolchain (calibration only)
`landing/tools/calib.py`: preprocess the new TU (scratch tree p3), re-declare the gp-accessed symbols with a chosen storage class, run PsyQ cc1psx (`tools/cc1psx_wrapper.sh -O2 -G8|-G0 -funsigned-char -quiet -mcpu=3000 -mips1 -msoft-float -w`), assemble with Sony ASPSX 2.34 (psyq3.5 archive, dosemu2, `-G8`), read the whole .text (`lnk.py`, PsyQ LNK reader), align per function with the shipped words (relocation immediates masked). Summary `landing/calib/ALL.txt`, outputs `landing/calib/`.
| TU / storage class of g_cd_atv, D_800A36B8, D_800A3840, D_800A3854 (+ g_cd_result in b5) | cc1psx -G8 | cc1psx -G0 |
|---|---|---|
| b4, tentative definition (COMMON) | cdrom_SetMix 0/18 differ, gp 2 = target; func_80035F78 0/12, gp 3 = target | 6/18, 6/12 differ; gp 0 |
| b4, extern | 4/18, 6/12; gp 0 | gp 0 |
| b4, static | 6/18, 6/12; gp 5 / 6 (the +1..+3 offsets gp'd) | gp 0 |
| b4, initialized | 6/18, 6/12; gp 5 / 6 | gp 0 |
| b5, COMMON | gp lists identical to the target in symbol and order (func_80036140 10, func_80036940 2) | gp 0 |
| b5, extern | gp 0 | gp 0 |
| b5, static / initialized | gp 16 / 2 (offsets gp'd) | gp 0 |
b5 word differences with the COMMON model: cc1psx -G8 func_80036140 81 words differ after alignment (64 of them `li` expansions, the rest cc1psx scheduling: mflo placement, the val0 `lbu` position), func_80036940 47 (all `li`). The same TU compiled by OUR cc1 at -G8 (tentative definitions) and assembled by ASPSX 2.34 differs from the shipped words ONLY in `li` expansions (func_80036140 68, func_80036940 47; `landing/calib/li_classify.txt`): this ASPSX expands `li rt,imm` as `ori` (upstream maspsx aspsx test EXPAND_LI_RESULT_1 for 2.34), the shipped bytes (and our build) have `addiu` — unrelated to gp/COMMON. Every gp / non-gp decision for every listed symbol matches the shipped bytes in all -G8 COMMON runs.

### maspsx_comm_syms.txt rows (prong (a) 1-4)
- `cdrom_SetMix: g_cd_atv` — signature: 80035F34 `sb a0,%gp_rel(g_cd_atv)`; 80035F44-58 `lui at` + `sb a1/a2/a3,%lo(g_cd_atv+1/+2/+3)(at)`. Reproduced exactly (0 words differ) by cc1psx -G8 + ASPSX from `CdlATV g_cd_atv;`; extern / static / initialized / -G0 all differ (table).
- `func_80035F78: D_800A36B8` — signature: 80035F7C `sb a1,%gp_rel(D_800A36B8)`; 80035F80-9C `lui at` + `sb a2/a3/v0,%lo(D_800A36B8+1/+2/+3)(at)`. Reproduced exactly (0 differ); alternatives differ.
- `func_80036140: g_cd_atv, D_800A36B8` — signature: 80036154 `lbu %gp_rel(D_800A36B8)`, 80036168 `lbu %gp_rel(g_cd_atv)`; 800361AC/C0, 80036208/1C, 80036264/78 `lui v0` + `lbu %lo(sym+1/+2/+3)(v0)`. cc1psx+ASPSX COMMON -G8: all 10 gp accesses identical and every +1..+3 read non-gp, but 89 other words differ (cc1psx codegen + li); our cc1 -G8 + ASPSX: only li words differ. Alternatives: extern -> 0 gp, static/initialized -> 16 gp (+k gp'd), -G0 -> 0 gp.
- Gate behaviour ((b)): `tools/maspsx/tests/test_comm_syms.py`; both-ways ((c)): 36/36 objects identical on HEAD (gate + rows, func_80036140 INCLUDE_ASM), and in P3 the gate is what keeps +1..+3 non-gp.

### -G8 prong (iii) screening (compiler-flags-canonical.md § Screening scope), `landing/screen_b4.txt`, `landing/screen_b5.txt`
cc1 -G8's own `.extern name,size` list. b4: 4 externs (g_cd_atv 4, D_800A36B8 4, D_800A3854 2, D_800A3840 2), all in sdata_syms.txt. b5: 9 externs; not in sdata_syms and <= 8 bytes: g_cd_result_plus_0x3 / _0x4 / _0x5 (1 byte each: splat per-byte labels of the 8-byte CdlResult g_cd_result; not in sdata_syms because the shipped func_80036140 reads/forms them with lui/%lo, listing them would make maspsx gp them). Built both ways through the full per-file pipeline (recipes in the files): every access identical in bytes and relocation (2 each; offsets inside func_80036140 shift by 4 because -G0 changes the ATV head). No INCLUDE_ASM / INCLUDE_RODATA / file-scope `__asm__` in b4 or b5 (they do not include include_asm.h). Memberships: code6cac_b2_post is on no list but RODATA_ALIGN2 (EXPAND_LB/LH and NO_SR: none); b4 / b5 join GP_FILES only; b4_post / b5_post join nothing.

### -G8 prong (iv-a): jump tables (scratch p3 build/bb2.map)
b2_post .rodata 0x800108EC (func_80035828's tables; 0x800108EC is 4 mod 8 -> listed, unchanged); b4, b4_post, b5_post emit no .rodata; b5 .rodata 0x80010938 size 0x78: func_80036140's 15-entry table at 0x80010938 and func_80036940's 14-entry table at 0x80010978, both 0 mod 8 -> b5 NOT on RODATA_ALIGN2_FILES. With `.align 3` the second table's alignment pad is the zero word at 0x80010974 that the transcribed jtbl_80010938[16] carried as element 15 (origin now explained; the transcribed array is deleted).

### Record extension 0x80101E9C..0x80101EA7 (aggregate-merge compiler necessity) — the open owner question
(a1) Mechanism, dumps `landing/dumps/` (cc1 -dr -ds -dt -dc -dl -dg, command line in each file):
- At -G8 every variable of <= 8 bytes is small data: `symbol_ref/v` (mips.h:2556-2561). ADDRESS_COST (mips.h:2897) of a small-data symbol is 1 (mips.c:1631-1632), equal to a register, and cse.c find_best_addr (2710-2724) replaces an address register by an equal-cost equivalent with the higher rtx cost, i.e. by the symbol. So `s16 *p = &D_80101E9C; (*p)++` becomes `(mem (symbol_ref/v D_80101E9C))` in .cse (ptrrmw_G8: insns 711/715), where -G0 keeps `(mem (reg 282))` (ptrrmw_G0: 722-729). A record member's address `(const (plus D_80101E58 68))` is not small data (the record is > 8 bytes), costs 2, and stays in a register through cse (rec_G8: insns 713-719) — the target's `la; lhu 0(v0); ...; sh 0(v0)`.
- Measured at -G8 (func_80036140 score): separate variables 18, pointer read-modify-write locals at the two sites 18, function-scope pointer aliases 42, record members 2 (jtbl operand only). At -G0 the pointer RMW gives 22 — it only works where -G8 is impossible.
- cc1psx agrees ((a2), `landing/psx/`): separate variables -> `lhu $2,D_80101E9C` / `lw $2,D_80101EA4` direct; record members -> `la $2,D_80101E58+68; lhu $3,0($2)` and `la $3,D_80101E58+76; lw $2,0($3)`, the target's registers.
- So E9C's and EA4's object is over 8 bytes. It cannot overlap CdState (0x80101E58..0x80101E9B, one object on main), so it either is CdState or starts at 0x80101E9C and, being over 8 bytes with 4-byte members, covers 0x80101E9C..0x80101EA7 (E9E, EA0 inside).
- A separate 12-byte record at 0x80101E9C (`--ext sep`) scores cdrom_ReadyCallback 12 and func_80036140 8 (SHA1 != oracle): cdrom_ReadyCallback's `expected_pos` address `(const (plus D_80101E9C 4))` gets its own pseudo (sep12_G0 .cse insn 52: `(set (reg 80) ...)`, then `(mem (reg 80))`), allocated to $s0 across the CdGetSector calls; as a CdState member cse relates it to the register already holding &rec.unk38 (rec_G0 .cse insn 54: `(mem (plus (reg 76) 8))`, cse.c use_related_value, same symbol base only), and it ends up direct like the target. Hence 0x80101E9C..0x80101EA7 is part of CdState. Sanctioned split families (dead store, self-assign, chain-extender, cancelling use, do{}while(0), duplicated-into-arms + jump2 cross-jump, hoist/sink) do not change an address's small-data flag, its cost or its symbol base, which is what decides here; the only family that re-spells the address, the pointer alias, is measured above (18 / 42).
(a4′) Members (full table: landing/member_table.md, generated from asm/funcs by landing/tools/memtable.py): span = 0x80101E62..0x80101EA7, the lowest to highest byte func_80036140 accesses. func_80036140's own new members: unk3C s16 0x80101E9C (800364B4 sh, 800365A4 sh, 80036604/08 la + 8003660C lhu + 80036628 sh; signed: the lhu value is sign-extended sll/sra 16 and compared slti 0x3D at 80036618-20), unk44 s32 0x80101EA4 (80036398 sw, 80036634/38 la + 8003663C lw / 80036650 sw, 80036738 lw, 8003674C sw; signed: `bgtz` after `-4` at 8003664C). Forced-in bytes (owner ruling 2026-09-26 Q13, 61f37ea5b): 0x80101E9E..9F -> `u16 unk3E`: other functions' original accesses all 2 bytes at that offset — cdrom_StartRead 80036E0C sh, game_FrameLoop 80036F54 la (s0) + 80036F9C lhu + 80036FAC sh; signedness evidence: the `lhu` at 80036F9C (unsigned). 0x80101EA0..A3 -> expected_pos: all accesses 4 bytes at that offset — cdrom_ReadyCallback 800360A4 lw (then bne: equality), 800360E4 lw (then addiu +1), 800360FC sw; func_80036940 80036AAC sw — NO admissible signedness evidence under Q13's text (no ordered compare, no plain shift, no divide on the value). Owner ruling Q14 (keep the existing type when nothing reveals signedness and both are byte-identical): every accessor instruction — cdrom_ReadyCallback 800360A4 lw -> only 800360AC bne (equality); 800360E4 lw -> 800360F4 addiu +1 -> 800360FC sw; func_80036940 80036AAC sw of CdPosToInt's v0 — reveals no signedness; the full landing built with `u32 expected_pos` vs `s32` (landing/tools/t_u32.sh, rev 09659d0cb): 40/40 C objects byte-identical, both SHA1 == oracle (landing/q14_s32_vs_u32.txt). Kept type s32 = main's `extern s32 g_cdread_expected_pos;` (include/code6cac.h before this landing) = CdPosToInt's return type (src/system.c `s32 CdPosToInt(u8 *)`; libcd `int`).
Byte-neutral in place: the record-extension respellings alone on the unsplit HEAD tree (func_80036140 still INCLUDE_ASM) give SHA1 == oracle (step ip_rec).

### CdlATV merge (g_cd_atv 0x800A3718, D_800A36B8 0x800A36B8) — prong (a) by independent evidence
(1) base+offset addressing in the original binary: in cdrom_SetMix / func_80035F78 / func_80036140 byte 0 is gp-relative and bytes 1..3 are lui/%lo; with Sony's ASPSX 2.34 that shape arises only from ONE COMMON object accessed at +1..+3 (research-common-gp.md §0-§1; calib table above: separate variables all-gp or none-gp). (2) The 4-byte unaligned `lwl/lwr/swl/swr` copy at 80036310-20 is a BLKmode struct assignment of a 4-byte, 1-aligned aggregate between the two objects (move_by_pieces). (3) Committed naming census (apiscan 9a7da73f5 / 2026-09-24 data manifest): g_cd_atv is libcd's 4-byte CdlATV, CdMix(CdlATV *). Prongs (b) struct of four u8 (libcd layout), (c) D_800A36B9..BB and g_cd_atv_plus_0x1..3 removed from C, undefined_syms_auto.txt and named_syms.txt (no INCLUDE_ASM referrer left), (d) canonical declaration in include/code6cac.h, cdrom_SetMix / func_80035F78 respelled `.valN`, CdMix's in-file prototype takes CdlATV *, (e) scratch full build == oracle. NOT byte-neutral on the unsplit tree (cdrom_SetMix 5 at -G0): it is byte-neutral only once cdrom_SetMix is -G8, so the proof order is split-first (P1: textual move, == oracle) then merge (P2, == oracle).

### Body (landing/tools/body_tpl.c instantiated for `--ext rec`)
Local `CdlATV atv` built field by field for `CdMix(&atv)`; `g_cd_atv = D_800A36B8;` struct copy; one block-scoped `s32 ret` per case (written once, read by `== 2` / `== 5` / `== 1`) — no multiply-written local; one block-local `s32 pos` (CdPosToInt result) in case 0x16; `CdControlF(D_80101E58.rec.unk34 ? 0x1B : 3, 0)`; `D_80101E58.rec.unk08 ? 0 : 0x10`; `if (D_80101E58.rec.unk3C++ > 0x3C)`; `unk44 -= 4; if (unk44 <= 0)`. Case order in source 10..16, 1C, 1D, 1E, 17..1B (target block order). Externs added in b5 before the body: CdMix, D_800A3854, D_800A3840, cdrom_SetMix, CdSync, CdReady, g_cd_result, g_cd_result_plus_0x3, g_cd_result_plus_0x5 (g_cd_result_plus_0x4 is in the header).

## History (before 2026-09-26 evening; superseded where the current state says so)
CD audio/XA playback state machine in src/code6cac_b2_post.c (same original module as
cdrom_SetMix / cdrom_ReadyCallback / func_80036940). Head: while a CD-mix fade is active
(D_800A3854 > 0) it interpolates each of the four CdlATV bytes between g_cd_atv (current) and
D_800A36B8 (target) by D_800A3840/D_800A3854, calls CdMix(&local), steps the counter and, at the
end, copies the target ATV over the current one. Body: `switch (D_80101E60.unk02)` over states
0x10..0x1E (15-entry ADDR_VEC, jtbl_80010938; case source order 10..16, 1C, 1D, 1E, 17..1B
follows the target block order). Every CdSync/CdReady case shares the tail
`else if (ret == 5 /*CdlDiskError*/) state = 0x17;` (cross-jumped in the target).

## Floors (all measured this session)
| body | model | score |
|---|---|---|
| candidate.c (split symbols, byte-wise ATV copy) | current headers (sandbox) | **50/512** |
| rejected/atv-cast-pun-36.c (`*(CdlATV_T *)&g_cd_atv = *(CdlATV_T *)&D_800A36B8`) | current headers (sandbox) | 36 — BANNED construct (per-use pointer pun) |
| integration/record-merge-only-body-16.c | ReplayCamRec extended to 0x80101EA7, split ATV bytes | 16 (scratch whole-file build, all 32 other fns 0) |
| integration/full-model-body.c | record merge + CdlATV merge, -G0 | 34 (siblings cdrom_SetMix 10, func_80035F78 6) |
| same | record merge + CdlATV merge, **-G8** | 14 (siblings 6/6) — only the +1/+2/+3 ATV bytes gp-rel'd |
| same | record merge + CdlATV merge, -G0 + `.comm` knowledge | 22 (cdrom_SetMix 5) |
| same | record merge + CdlATV merge, **-G8 + `.comm` knowledge for g_cd_atv / D_800A36B8** | **2** — the ONLY diff is the jtbl `lw %lo(jtbl)` operand (rodata placement artifact); all 32 other functions in the file 0 |

Scratch tools (integration/): xb.py = whole-file build with an include-override dir, optional
`--g8` (CC_FLAGS_GP) and `--comm a,b` (prepends `.comm sym,4` lines before maspsx = what cc1 emits
for a TU-defined tentative definition), scores EVERY function in the file vs build/src; diff.py =
scored hunks; gp_off_census.py = census below. Headers/src used: integration/*.patch.

## Proven facts
1. **The CD-read globals 0x80101E78..0x80101EA7 are aggregate members.** Target accesses to
   E88/E8C/E9C/EA4 are `la reg,sym; lw 0(reg); ...; sw 0(reg)` (and s0 = &E88 kept across a call in
   case 0x15). GCC 2.7.2 only forms that when the MEM comes from a COMPONENT_REF/ARRAY_REF
   (explow.c memory_address force_reg; DECL_RTL of a plain scalar is used directly) — tmp test
   t1/t3: every scalar spelling (++x, x++, x+=1, x=x+1) is direct, struct member / array element is
   `la`+0(reg). Independent (census, 2026-09-24, pre-dates this session) base+offset evidence in the
   ORIGINAL binary: func_80036940 80036A74 `s0 = &D_80101E8C; s0 -= 0x20` (-> g_cd_loc 0x80101E6C) and
   80036B08 `a1 = &D_80101E98 - 0x2C` (-> 0x80101E6C). cse can only relate two constant addresses
   (use_related_value) when they share one symbol base, so 0x80101E6C..0x80101E99 is ONE object.
   E9A/E9C/E9E/EA0/EA4 (past E99): only this function's member-form codegen (+ game_FrameLoop takes
   &E9E) — no independent base+offset evidence found (scan of every addiu %lo formation into
   0x80101E58..0x80101EA7 across asm/funcs). Same class as the func_800620B8 open question.
   Extending ReplayCamRec through 0x80101EA7 (integration/code6cac.h.patch) keeps all 32 other
   functions in the file at score 0 (only consumers: code6cac_b2_post.c; grep src/ include/).
2. **The ATV copy needs a CdlATV aggregate.** `lwl/lwr/swl/swr` at 80036308 is move_by_pieces of an
   unaligned 4-byte BLKmode object = struct assignment `g_cd_atv = D_800A36B8`. With the splat
   per-byte symbols the only C for it is a per-use pointer pun (auto-FAIL, dossier). CdMix takes
   CdlATV* (libcd.h), cdrom_SetMix stores all 4 bytes then CdMix(&g_cd_atv) (naming manifest
   2026-09-24 CONFIRM rows 0x800A3718/19).
3. **The CdlATV merge cannot match in the current build model — two independent gaps:**
   a. *Cost model (-G0).* In the target val0 of both objects is a gp-rel direct access and the copy
      re-materializes both addresses (`la a1/a0`). Under -G0 cc1 prices symbol_ref at 2 vs reg 1
      (mips_address_cost), so cse keeps the forced address pseudo and shares it between the val0
      read and the copy across the CdMix call (s1/s0). Under -G8 the objects are small data
      (SYMBOL_REF_FLAG, cost 1) and the sharing disappears — measured.
   b. *Assembler rule for `sym+N`.* Target: base byte gp-rel, bytes +1..+3 `lui/%lo` (in
      cdrom_SetMix, func_80035F78 and here). maspsx gp-rel's `sym+N` for every injected sdata extern
      unless the symbol is `.comm` in the TU (`gp_allowed = gp_allow_offset or sym not in
      comm_symbols`, maspsx/__init__.py:786) — i.e. ASPSX never gp-rel'd an offset into a COMMON
      symbol. So in the original TU these two objects were tentative definitions (`CdlATV x;`).
      Global "no gp for extern+offset" is NOT an option: census (gp_off_census.py) finds 9 matched
      sites that need it (text1a_post func_80041E10/80041EB0 g_anim_select+2/+4; text1b
      func_80061C00 D_800A34F0+2, func_8006E534 D_800A3588+2/D_800A358C+2).
   Defining the objects in C (real `.comm`) is not viable: cc1 emits `.comm sym,4,1` which maspsx
   cannot parse (same as CD_cw), and maspsx would then emit .bss storage in b2_post.o, conflicting
   with the undefined_syms_auto.txt address.
4. **-G8 on this file is byte-neutral for every other function's instructions** (all 32 score 0),
   BUT -G8 makes cc1 buffer function bodies (TARGET_FILE_SWITCHING), so the file-scope INCLUDE_ASM of
   func_80036940 floats to the top of the object (measured: func_80036940 at .text+0). A -G8 landing
   needs func_80036940 out of the TU (LINKED_ASM_FUNCS like save_vc_ctrl) or a TU split around it.
5. **Rodata placement.** GCC emits the 15-entry table into code6cac_b2_post.o(.rodata) after the two
   func_80035828 tables (0x4C bytes at 0x800108EC). With .align 3 it lands at +0x50 = 0x8001093C; the
   target is 0x80010938. The original placement (0x938 and func_80036940's table at 0x978 after a
   zero pad word) says the CD module is its own original TU whose rodata starts 8-aligned at
   0x80010938. Landing options: (i) TU split so func_80036140's TU rodata starts at 0x938 (faithful;
   rodata_post keeps a leading zero word until func_80036940 lands), or (ii) add code6cac_b2_post to
   RODATA_ALIGN2_FILES (precedent func_800747D8 / text1b) + leading zero word in rodata_post. Either
   way delete jtbl_80010938 from src/code6cac_b_rodata_post.c.

## Constructs in the full-model body (for the eventual reviewer)
- local `CdlATV atv` (sp+0x10) built field by field, `CdMix(&atv)`; `g_cd_atv = D_800A36B8;`
- `ret = CdSync(1, &g_cd_result)` then `if (ret == 2) ... else if (ret == 5) state = 0x17;`
- state 0x13: `CdControlF(D_80101E60.unk34 ? 0x1B : 3, 0)`; 0x1B: `unk02 = unk08 ? 0 : 0x10`
- `if (D_80101E60.unk3C++ > 0x3C)`; `D_80101E60.unk44 -= 4; if (<= 0)`; one block-local `s32 pos`
  in case 0x16 (single role: CdPosToInt result). No multiply-written locals besides `ret` (one
  role: CdSync/CdReady status, rewritten per case) — check Ruling 5/6 if a reviewer objects.
- externs added locally: CdSync, CdReady, g_cd_result (+_plus_0x3, _plus_0x5) as u8 (8-byte
  CdlResult split by splat; base gp-rel, +3/+4/+5 lui — same .comm story; the split spelling matches
  and no aggregate is needed for it).

## Layer-2 PASS follow-ups (2026-09-26 night)
- (a) Full Q15 (i) pairing for every reordered block, incl. the companion lui/lbu words in blocks 6/11/16
  and the lui s0/addiu s0 pairs in blocks 60/89: landing/q15/func_80036140.pairs_full.txt (tools/pairs.py).
- (b) Q16 failing pre-split builds re-banked at the landing rev 27ac1a933 on the stock cc1 ac80146b:
  landing/steps/q16_failing_builds.txt (same SHA1s as on the previous cc1).
- (c) g_cd_result's 8-byte shape: psyz (Xeeynamo/psyz a438bda) include/libcd.h:344/347 `@param result
  Pointer to result buffer (8 bytes)` / `int CdSync(int mode, u_char* result);`; decomp/src/libcd/type.c
  CdDiskReady `u_char result[8]; ... if (result[0] & CdlStatShellOpen)` (CdlStatShellOpen 0x10,
  libcd.h:127) — the same result[0] & 0x10 test as func_80036140 / func_80036940.

## LANDED 2026-09-26 — COMPLETED-C (layer-2 PASS)
Five commits, each byte-equal to its oracle-proven scratch state (tmp verify_commits.sh):
014401021 maspsx gate + registration [infra-rule: maspsx-comm] | 90e325bba split -> -G8 code6cac_b4.c |
cbc69187b CdlATV + CdState through 0x80101EA7 | a6daf00b6 Match: func_80036140 (-G8 code6cac_b5.c) |
f601fc2ae g_cd_result u8[8]; queue 82243f747. Landed tree: lock.ps1 rebuild SHA1 == oracle, sandbox 0
(512/512), engine test 805/0, check_completion_integrity OK (cdrom_SetMix, func_80035F78, func_80036140
listed as maspsx_comm_syms.txt fidelity-gate completions). Ledger closed.
