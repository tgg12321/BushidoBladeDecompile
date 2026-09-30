# SelWork cluster — frontier (2026-09-30, ff-c; layer-2 rev-45)

One data-model task covering the select work area at `D_800A36A0` and its per-player
side tables, too big for a fix-forward. On 2026-09-30 all four completed consumers on the
reviewed diff were reopened under owner Q37: func_800747D8, func_80075670, func_800768DC,
func_800770B8. func_800759D0 and func_80075F80 were already reopened (their ledger:
memory/grind/func_800759D0/evidence.md). Land the next attempt as ONE change covering
every item below, with a fresh layer-2 on the exact bytes.

## What was submitted (rev-45 reviewed this)

`selwork-cluster-2026-09-30.patch` (this directory) = the reviewed src/text1b_tu2.c diff
against HEAD 1c47528d6 (exact bytes were tmp/ffc/backup_45/text1b_tu2.c): rebuild
build_sha1 62efab4f73f992798c43e8c730aa43baa10bb4fa, integrity OK, sandbox 0 for all four.
- `extern s16 D_800A35D0[2][2];` (per-player {s16, s16}: player*4 index at 0x80076948/5C,
  parallel to SelWork f40[player]; two data words 0x800A35D0..D7); consumers respelled
  `D_800A35D0[0]` / `D_800A35D0[player]`; func_800770B8 via a typed row pointer
  `s16 (*sym)[2]` (FAKE pointer alias; without it 19/175, without the re-set 18/175).
- `extern u8 D_8009BCE4[20];` with `D_8009BCE4[x]` subscripts.
- `(&D_8009BD21)[f67 * 2]` -> `D_8009BD20[f67][1]`; `extern u8 D_8009BD21;` removed.
- SelWork_800768DC: `pad1C[0x18]` -> `union { s16 half[2]; s32 word; } f1C, f20;` +
  `pad24[0x10]`; `.word` named only at func_800770B8's two word clears
  (0x800772BC `sw $zero,0x20($a0)`, 0x800772C0 `sw $zero,0x1C($a0)`).
Scores of every spelling: memory/grind/func_800770B8/evidence.md and the ff-c-2026-09-30/
diffs in func_800768DC/ and func_800770B8/.

## rev-45 verdicts (as relayed by the orchestrator, 2026-09-30)

- func_800768DC: PASS on that diff (hash 0d456e1392bb3646), but it goes stale as soon as the
  f10/f3C unions land and it cannot land alone on this diff.
- func_800747D8, func_80075670: FAIL — a cast word read at SelWork +0x10, where S_800747D8
  (the file's second view of the same work area, `union { s32 word10; s16 half10[2]; }
  field10` at 0x10) and SelWork_800768DC (`s16 f10[2]`) disagree about 0x10.
- func_800770B8: FAIL — an unadmitted `p_old` reuse, and mixed raw-cast / member handles to the
  same work area (`*(s16 *)(ap + 0x10)` etc. beside `SELWORK_800768DC->f20.word`).
- Cluster-wide: the D_8009BD20 shared-header / linker-row cleanup is owed; and Q33 prong (5)
  — the f1C/f20 unions grow sizeof(SelWork_800768DC) from 0x92 to 0x94 (the s32 member makes
  the struct 4-aligned), which prong (5) ("the struct's size ... unchanged") forbids: this
  needs an owner ruling or a layout that keeps the size.
(The reviewer's full text is appended at the end of this file.)

## Frontier — the one landing the next worker should build

1. **One SelWork type.** Merge S_800747D8 into SelWork_800768DC (one typedef, one macro, every
   consumer through it: func_800747D8, func_80075670, func_8007636C, func_800768DC,
   func_800770B8, and the reopened func_800759D0 / func_80075F80). Q46 unions over every
   member the target touches with one word access: f10 (func_800747D8 / func_80075670's word
   read at +0x10 — cite the lw addresses), f1C, f20 (func_800770B8's two sw), f3C (func_80075F80's
   single word test at 0x80076060, per the laneC frontier). Every other access through the
   half[] members. Re-review func_800768DC (its PASS is stale once f10/f3C change).
2. **Q33(5) size question.** With s32 members the struct becomes 4-aligned: sizeof 0x92 -> 0x94.
   Either show nothing depends on sizeof/array-of-SelWork (it is a single object reached through
   D_800A36A0) and ask the owner for a ruling that prong (5)'s "size unchanged" allows tail
   padding, or find a layout that keeps 0x92 (not possible with an s32 member unless the struct
   already ends 4-aligned). File the question; do not land around it.
3. **func_800770B8 `p_old`.** It holds `arg0 + 0x58` (passed to func_8006E950 / func_80076FF8),
   then func_8006E49C's result (the new work area), then the FAKE same-value restore. Try a
   separate `work` local for the func_8006E49C result (store to D_800A36A0 and `work->+4 = prev`),
   with the dead-store FAKE moved to whichever pseudo the combine_regs mechanism actually needs;
   measure; bank. The raw `*(s16 *)(ap + off)` / `*(s32 *)(p + off)` writes into the work area
   should all become SelWork members (f10/f14/f08/f0C/f3C/f40/f5C/f60/f64/f65/f66/f67/f68/f6A/f7E).
4. **D_8009BD20.** Declare `extern u8 D_8009BD20[2][2];` once in include/game.h (data: 0x01 0x02
   0x03 0x04, asm/data/7D920.data.s:23805-23817; users text1b_tu2.c and text1b_tu1e.c), fold the
   D_8009BD21 dlabel into D_8009BD20 (4 bytes) in asm/data/7D920.data.s, respell the asm users
   (`D_8009BD20 + 1`), and remove undefined_syms_auto.txt row 71 (`D_8009BD21 = 0x8009BD21;`) and
   the named_syms.txt `g_text1b_pair_lookup_BD20_hi` row (aggregate-merge prong c).
5. **GaugeWork 0x6A mismatch.** func_80077374's local `typedef struct { u8 pad[0x6A]; u8
   rows[2][10]; } GaugeWork;` types 0x6A..0x7D as u8[2][10], while func_800770B8 writes
   `s16` at 0x6A + player*10 + 2*k (k < 5) and SelWork has pad66[0x18] there. One type for those
   bytes (the s16[2][5] the stores show, or whatever func_800759D0's `rows[i]` argument really
   is), and GaugeWork retired into SelWork.
6. **func_800759D0 / func_80075F80 respell notes** (both reopened; frontier in
   memory/grind/func_800759D0/evidence.md, "FRONTIER (the reviewer's fix list)"): their PASSed
   parts ([2][10] D_8009BCF8, D_8009BCE4[20], the SELWORK f48 view) re-land unchanged with the
   cluster; func_80075F80 needs `D_800A35D0[arg3]` and the f3C union word member at 0x80076060.

## Reopen record (2026-09-30)

All four back to INCLUDE_ASM in one revert commit; func_800747D8's jump table re-emitted as
asm/rodata/jtbl_80015A0C.s (5 words, read from the oracle EXE: .L800748F0 .L80074984 .L800749D8
.L80074A58 .L80074AB0) via INCLUDE_RODATA immediately before its INCLUDE_ASM, so text1b_tu2.o's
.rodata keeps jtbl (0x0) then D_80015A20 (0x14). Rebuild == oracle. Landed bodies in each
function's rejected/selwork-cluster-2026-09-30.c (func_800747D8's with its doc comment).

## rev-45 full review text (appended by the orchestrator from the reviewer's messages, 2026-09-30)

Verdicts (all re-scored sandbox 0; full-build SHA1 not verified by the reviewer):
func_800747D8 d8db9fc860829384 FAIL · func_80075670 ae836e096f26313c FAIL ·
func_800768DC 0d456e1392bb3646 PASS · func_800770B8 120523d39b6ddfad FAIL.

**func_800747D8 / func_80075670 (FAIL).** `src/text1b_tu2.c:211` / `:578` read `*(s32 *)(base + 0x10)`, a
cast word view over the two per-player s16 at 0x10. The target has the same single read (0x800747F4
`lw $v0,0x10($a2)`, 0x80075684 `lw $v0,0x10($v1)`), and the halves are per-player s16 (func_800768DC
0x8007690C `lh 0x10` at base + player*2). Q33 (4) refuses the cast where a union would do. `S_800747D8`
already declares a `field10 {word10, half10}` union that :211 does not use, while `SelWork_800768DC`
declares the same bytes as plain `s16 f10[2]`; Q46 (5) wants one definition. Fix: merge the two structs
into one type with a Q46 union at 0x10, name `.word` only at those two reads, and send every other f10
access through `.half[]` (:310, :591, :817, :856, :1070).

**func_800768DC (PASS).** All member access through SelWork. `D_800A35D0[arg3]` matches the target
(0x80076948 `sll 2`, base add at 0x8007695C) and is the `s16 *` pair func_800692C0 walks
(text1b_tu1c.c:6251). `D_8009BCE4[...]` matches the `lbu`/`sb` at 0x80076B88–A4. `(u32 *)&arg0` is only a
prototype conversion. Caveat: the f10/f3C unions will change this body, so its PASS goes stale then.

**func_800770B8 (FAIL).**
1. D_8009BD21 merge incomplete. Prongs (a)/(b) hold (func_800747D8 builds the D_8009BD20 base at
   0x800749B4 and adds the column; 2-byte stride at 0x80074694–D4 and 0x80077338–44). Prong (c):
   `undefined_syms_auto.txt:71 D_8009BD21` stays although its only asm users (func_80074488.s,
   func_800770B8.s) are compiled from C and asm/text1b.s is not linked. Prong (d): D_8009BD20 is declared
   TU-local at text1b_tu2.c:106, :170 and text1b_tu1e.c:520 (identical, so one handle), but a merged
   cross-TU table belongs in the shared header. Fix: `extern u8 D_8009BD20[2][2];` in include/game.h with
   an object-model comment (7D920.data.s:23805–23817), delete the three TU-local externs and row 71.
2. Q46 unions f1C/f20 against Q33: (1) holds (0x800772B4 `lw $a0,D_800A36A0`; 0x800772BC `sw $zero,0x20($a0)`;
   0x800772C0 `sw $zero,0x1C($a0)`). (2) holds except size: `s16 half[2]` is the real member (func_80075F80
   `lhu`/`sh` 0x1C at 0x80076114/24, `lh` 0x20 at 0x80076158/88, `lh` 0x1C/0x20 at 0x800761B4/B8, `sh` 0x20 at
   0x8007616C/78/0x800761A4; func_800759D0 `lh` 0x1C/0x20 at 0x80075BC4/C8). (3) `.word` only at :1128-1129.
   (4) no cast through the union. **(5) FAILS on size**: offsets unchanged, but the s32 raises alignment
   2→4 and sizeof 0x92→0x94; (2) forbids filler and nothing accesses 0x92+. Needs an owner ruling on
   whether trailing alignment padding counts as "size" (docs/audits/RETRO-AUDIT-2026-09-29.md, open Q2).
3. `p_old` (:1007 decl, :1031 `= arg0 + 0x58`, :1038 `= func_8006E49C(...)`, :1052 FAKE restore) holds two
   unrelated values; the FAKE at :1041-1051 covers only the dead store, and its mechanism
   (combine_regs reg_n_deaths == 1) depends on the second role. No ruling admits it. Try
   `s32 *work = (s32 *)func_8006E49C(...); D_800A36A0 = (u8 *)work; work[1] = (s32)prev;`; else a Ruling 11
   package with an honest generic name.
4. Mixed handles: raw `*(T *)(base + off)` writes to SelWork members (f10/f14/f3C :1070-1074, f40 :1080-1081,
   f7E via p_7e :1086-1087, f5C/f60 :1094-1095, f64 :1131/:1133/:1136-1137, f65 :1142) beside the new member
   stores; MEM_IN_STRUCT_P differs. The do-while FAKE records a struct-typed rederive at 178 insns (miss).
   Measure the member spelling on today's chassis before claiming the casts are required.
5. FAKE comments at :1012-1027 and :1041-1051 cite evidence.md [s9]/[s11] and hypotheses.md deleted in
   bf8588dd8; repoint to `bf8588dd8^:memory/grind/func_800770B8/{evidence,hypotheses}.md`. The new `sym` row
   pointer FAKE (:1060-1066) meets pointer-alias-fake-exception (f3 19/175, f3b 18/175).

Declarations: `s16 D_800A35D0[2][2]` real (8 bytes at 91C98.data.s:4279-4282; one symbol row, :905); TU-local
acceptable, but declared 3× (:171, :567, :998) — collapse. `u8 D_8009BCE4[20]` real (7D920.data.s:23732-23753).

Concerns: (1) func_80075F80's banked body `(&D_800A35D0) + (arg3 * 2)` silently becomes +arg3*16 under
`[2][2]` — respell `D_800A35D0[arg3]`. (2) With f1C/f20 unions, func_800759D0 (`state[0x1C / 2]`) and
func_80075F80 must go through `.half[arg3]`; neither re-lands unchanged. (3) Do every SelWork union (f10, f1C,
f20, f3C) plus the S_800747D8 merge in ONE landing and settle Q33(5) once. (4) The D_800A35D0 type change
forces 747D8/75670 respells, which can't PASS without the f10 fix, so 768DC can't land alone. (5) GaugeWork
(text1b_tu2.c:1154-1157) declares 0x6A as `u8 rows[2][10]` and passes `rows[i]` as `s16 *`, while
func_800770B8 / func_80076D74 treat those bytes as s16 — another mismatched view; queue it.

## Note after the reopen (ff-c, 2026-09-30)

rev-45 item 1 says D_8009BD21 has no built asm referrer. That was true of the reviewed tree only:
since 5fbcca017 func_800770B8 is INCLUDE_ASM again, and asm/funcs/func_800770B8.s references
`%hi/%lo(D_8009BD21)` (0x80077338-44). So undefined_syms_auto.txt row 71 must stay until the
cluster landing either re-lands func_800770B8 as C or respells that asm as `D_8009BD20 + 1`. My
func_800770B8 ledger line "the dlabel / linker row stay for the INCLUDE_ASM users" was wrong for
the reviewed tree (both referrers were C then).
