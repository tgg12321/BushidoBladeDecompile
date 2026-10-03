# X2 10: the libcd cdread block as ONE CdlREAD object (libcd.h) — OWNER QUESTION for cb_read's 3 reads
Patch: tmp/laneX2/10-cdlread.patch (stacks on 09 for system.c). Files: include/libcd.h, src/system.c, src/ings2.c,
undefined_syms_auto.txt, named_syms.txt, symbol_addrs.txt. Needs HEAD >= a3ec056e2.
Finding CONFIRMED: system.c held 12 file-scope per-member externs (D_800A14D4..FC, g_CdReadMode_value), a
block-scope scalar view `extern volatile s32 D_800A14D0` (cd_read_retry, CdRead) and a block-scope struct view
(CdReadSync); ings2.c a second CdlREAD typedef + extern. Q21 cond. 3 (one handle per location per file) is not met.
Investigation (measured, harness vs build/src/system.o; dumps tmp/laneX2/dmp_cr3):
- ONE file-scope `extern volatile CdlREAD D_800A14D0`, every access through members, no pointer locals:
  all 44 system.c functions 0 EXCEPT cb_read 2 (one extra addiu). The la-form reads the old code forced with
  pointer locals (cd_read_retry tsl/md [FAKE], CdReadBreak tsl, CdRead ps, cb_read pp/tsl [unlabelled]) all
  come out of the struct naturally — cse's related-value addressing of one symbol (.cse: insn 34
  `mem (plus r77 -4)`), which is evidence the block is one object. The 4 pointer locals + scalar view go.
- cb_read's first block reads cnt, size, tslmode as %hi/%lo of their own addresses (0x80081FEC/2000/2014).
  Through the struct, expand emits `la D_800A14D0+20` (insn 24) and cse bases the block's later members on that
  register, so the first read keeps the la. Per-member handles needed, measured: none 2, cnt 3, cnt+size 2,
  cnt+tslmode 3, size+tslmode 2, all three 0. No single-view spelling found.
Landed form (this patch): CdlREAD + `extern volatile CdlREAD D_800A14D0;` in include/libcd.h (system.c and ings2.c
share it; ings2's copy goes); cb_read keeps ONE block-scope `extern volatile s32 D_800A14E4, D_800A14E0,
g_CdReadMode_value;` with a `/* FAKE: */` comment carrying the measurement above. Every other access is a member.
- Bodies (all 0): cb_read, cb_data, cd_read_retry, CdReadBreak, CdRead, CdReadSync, CdReadMode (ings2.c).
- Symbol rows: D_800A14D4/D8/DC/E8/EC/F0/F4/F8/FC retire (undefined_syms_auto, named_syms D_800A14FC); the
  91C98.data.s dlabels stay. False registry comments (D_800A14D0, D_800A14E4, g_CdReadMode_value) corrected.
FAKEs: +1 (cb_read's 3 reads), -2 (cd_read_retry tsl/md); 4 unlabelled aliases gone (CdReadBreak tsl, CdRead ps,
cb_read pp/tsl). volatile_extern_allowlist.txt (guarded) not edited: rows D4..FC except E0/E4 now stale.
OWNER QUESTION (Q21 grant and the 2026-07-09/10 allowlist grant predate Q91): item 4 refuses a second extern for
the same bytes. Admit cb_read's three FAKE per-member reads (Q73-style, this function only), or keep HEAD's form?
Strictly fewer handles than HEAD either way; if refused, no byte-exact single-view form exists (above).

Draft commit message:
cheat-cleanup: libcd cdread state is one CdlREAD object (libcd.h) - views and pointer aliases retired

system.c reached the cdread module block through 12 per-member externs, a scalar and a struct view of
D_800A14D0, and pointer locals; ings2.c repeated the type. One `extern volatile CdlREAD D_800A14D0` in
libcd.h now serves both files: CdReadSync, CdRead, CdReadBreak, cd_read_retry, cb_data and CdReadMode read
members only, and their la-form reads need no pointer aliases. cb_read keeps three FAKE-labelled per-member
reads (measured; owner ruling: <verbatim here>).

Verification:
- harness: system.c (44) and ings2.c every function 0, sections equal; every TU including libcd.h/code6cac.h 0
- stacked 01..11 link SHA1 62efab4f73f992798c43e8c730aa43baa10bb4fa; verify-oracle --rebuild + layer-2 at landing
