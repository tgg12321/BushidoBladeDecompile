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
(The reviewer's full text was requested from rev-45; if it arrives it is appended below.)

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
