# Reviewer-proposed spellings banked from tmp/ (measured 2026-09-28 with r11/tools/fast3.sh)

lines = differing objdump lines vs the target on the real per-file recipe. Bodies: this directory.

| tag | proposed by | spelling | lines | status under Q30/Q32 | annotation requirement (set-aside only) |
|---|---|---|---|---|---|
| rv4_base | v4 layer-2 | landing template control (the reuse spelling) | 0 | control |  |
| rv4_dl_init | v4 layer-2 | s32 dg = ... initializer | 4 | COUNTS (FAKE-free) |  |
| rv4_dl_inv | v4 layer-2 | if (dg <= 0x3200) {div} else {0x400} | 11 | COUNTS (FAKE-free) |  |
| rv4_dl_reg | v4 layer-2 | register dg | 4 | COUNTS (FAKE-free) |  |
| rv4_dl_split | v4 layer-2 | vy_new = vy; vy_new -= ... compound split | 52 | COUNTS (FAKE-free) |  |
| rv4_dl_tern | v4 layer-2 | vy_new = vy - (dg > 0x3200 ? 0x400 : dg / 8) | 11 | COUNTS (FAKE-free) |  |
| rv4_ix_reg | v4 layer-2 | register idx_add/idx_sub | 67 | COUNTS (FAKE-free) |  |
| rv4_nb_andvar | v4 layer-2 | lzcount = lz[0] & ~1; shift = 0x16 - lzcount | 2 | COUNTS (FAKE-free) |  |
| rv4_nb_neg | v4 layer-2 | shift = -(lzcount & ~1) + 0x16 | 348 | COUNTS (FAKE-free) |  |
| rv4_nb_reg | v4 layer-2 | register lzcount, shift | 2 | COUNTS (FAKE-free) |  |
| rv4_nb_s16 | v4 layer-2 | s16 shift | 37 | COUNTS (FAKE-free) |  |
| rv4_nb_u32 | v4 layer-2 | r11pv_nbits, u32 lzcount | 2 | COUNTS (FAKE-free) |  |
| rv4_nf_reg | v4 layer-2 | register nforce_add/nforce_sub | 21 | COUNTS (FAKE-free) |  |
| rv4_tq_reg | v4 layer-2 | register lzc_in | 2 | COUNTS (FAKE-free) |  |
| rv4_tq_u32 | v4 layer-2 | u32 lzc_in (the Q28 copy as a fresh local) | 2 | COUNTS (FAKE-free) |  |
| rv4_wk_reg | v4 layer-2 | register sq1 | 26 | COUNTS (FAKE-free) |  |
| rv4_wk_u32 | v4 layer-2 | u32 sq1 | 26 | COUNTS (FAKE-free) |  |
| rv_both | v1 layer-2 | work and delta split + both dead stores | 0 | SET ASIDE: dead stores | dead-store-fake-exception.md prerequisite 3: "**Mandatory annotation:** `/* FAKE: <one-line reason> */` or `// FAKE: <reason>` ON the statement." |
| rv_nf0 | v2 layer-2 | nforce split | 21 | COUNTS (FAKE-free) |  |
| rv_nfA1 | v2 layer-2 | nforce split + do-while(0) wraps | 42 | SET ASIDE: do-while(0) | do-while-zero-exception.md prerequisite 1: "**Inline `/* FAKE: ... */` or `// FAKE` annotation at the construct site** (not file-header prose), naming the observed effect" |
| rv_nfA1b | v2 layer-2 | nforce split + do-while(0) wraps | 42 | SET ASIDE: do-while(0) | do-while-zero-exception.md prerequisite 1: "**Inline `/* FAKE: ... */` or `// FAKE` annotation at the construct site** (not file-header prose), naming the observed effect" |
| rv_nfA1c | v2 layer-2 | nforce split + do-while(0) wraps | 42 | SET ASIDE: do-while(0) | do-while-zero-exception.md prerequisite 1: "**Inline `/* FAKE: ... */` or `// FAKE` annotation at the construct site** (not file-header prose), naming the observed effect" |
| rv_nfA2 | v2 layer-2 | nforce split + do-while(0) wraps | 42 | SET ASIDE: do-while(0) | do-while-zero-exception.md prerequisite 1: "**Inline `/* FAKE: ... */` or `// FAKE` annotation at the construct site** (not file-header prose), naming the observed effect" |
| rv_tp1 | v1 layer-2 | temp split + chain-extender `work + lut1 - lut1` | 132 | SET ASIDE: chain-extender | dead-store-fake-exception.md prerequisite 3: "**Mandatory annotation:** `/* FAKE: <one-line reason> */` or `// FAKE: <reason>` ON the statement." |
| rv_tp2 | v1 layer-2 | temp split + dead stores | 42 | SET ASIDE: dead store | dead-store-fake-exception.md prerequisite 3: "**Mandatory annotation:** `/* FAKE: <one-line reason> */` or `// FAKE: <reason>` ON the statement." |
| rv_tp3 | v1 layer-2 | temp split (bytes at loop-body scope) + chain-extender `work + lut1 - lut1` | 132 | SET ASIDE: chain-extender | dead-store-fake-exception.md prerequisite 3: "**Mandatory annotation:** `/* FAKE: <one-line reason> */` or `// FAKE: <reason>` ON the statement." |
| rv_tp4 | v1 layer-2 | temp split, table-byte locals at loop-body scope | 42 | COUNTS (FAKE-free) |  |
| rv_wd_after2 | v1 layer-2 | work split + `sq1 = 0;` dead store before `tot` | 0 | SET ASIDE: dead store | dead-store-fake-exception.md prerequisite 3: "**Mandatory annotation:** `/* FAKE: <one-line reason> */` or `// FAKE: <reason>` ON the statement." |
| rv_wd_end | v1 layer-2 | work split + `sq1 = 0;` dead store at the end of the ellipsoid body | 0 | SET ASIDE: dead store | dead-store-fake-exception.md prerequisite 3: "**Mandatory annotation:** `/* FAKE: <one-line reason> */` or `// FAKE: <reason>` ON the statement." |

Counting spellings reaching 0: none (best 2). The four zeros are the reuse control and the dead-store closers, set aside.
