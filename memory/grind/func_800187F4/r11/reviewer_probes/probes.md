# Reviewer-proposed spellings banked from tmp/ (measured 2026-09-28/29 with r11/tools/fast3.sh)

lines = differing objdump lines vs the target on the real per-file recipe; "insns" = objdump instruction lines
of the function (target 662; the engine sandbox counts 644). Bodies: this directory, kept exactly as measured,
so they carry the source comments of their time (older lz[6] wording, or none); comments do not affect bytes.
Q30 status is by construct family (see proof §3A); the v2 reviewer's bodies carry the v2 chassis's two FAKE
dead stores.

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
| rv_nf0 | v2 layer-2 | nforce split, on the v2 chassis c6 | 21 | SET ASIDE: carries c6's two FAKE dead stores | dead-store-fake-exception.md prerequisite 3: "**Mandatory annotation:** `/* FAKE: <one-line reason> */` or `// FAKE: <reason>` ON the statement." |
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

| rv5_base | v5 layer-2 | landing template control (the reuse spelling) | 0 | control |  |
| rv5_dl_cmp_direct | v5 layer-2 | delta split, `if (SCR->pos[1] > SCR->ground) { dg = ...` (663 insns) | 57 | COUNTS (FAKE-free) |  |
| rv5_dl_coll_dgfirst | v5 layer-2 | delta split, the same with dg declared first | 4 | COUNTS (FAKE-free) |  |
| rv5_dl_coll_dy0first | v5 layer-2 | delta split, dy0 and dg both at collision-block scope, dy0 declared first | 4 | COUNTS (FAKE-free) |  |
| rv5_dl_dy0_in_list | v5 layer-2 | delta split, dy0 first in the `dx0, dz0, dy1, dx1, dz1` declaration list | 4 | COUNTS (FAKE-free) |  |
| rv5_nb_ellscope | v5 layer-2 | nbits split, lzcount declared at ellipsoid-body scope, shift in the arm | 2 | COUNTS (FAKE-free) |  |
| rv5_nb_inline | v5 layer-2 | nbits with no local, (0x16 - (lz[0] & ~1)) written inline twice | 2 | COUNTS (FAKE-free) |  |
| rv5_nb_onedecl | v5 layer-2 | nbits split, `s32 lzcount, shift;` one declaration in the arm | 2 | COUNTS (FAKE-free) |  |
| rv5_nf_blocks | v5 layer-2 | nforce split, each count a block-local initializer in a compound block around its force loop | 21 | COUNTS (FAKE-free) |  |
| rv5_nf_blocks_const | v5 layer-2 | nforce split, the same with const s32 counts | 21 | COUNTS (FAKE-free) |  |
| rv5_wk_allcopy | v5 layer-2 | work split, all three sq1 reads redirected to the copy | 26 | COUNTS (FAKE-free) |  |
| rv5_wk_cmp_copy | v5 layer-2 | work split, compare on the copy (temp < 0x400) | 26 | COUNTS (FAKE-free) |  |
| rv5_wk_idx_copy | v5 layer-2 | work split, LZC-arm LUT index temp >> nbits | 26 | COUNTS (FAKE-free) |  |
| rv5_wk_pv | v5 layer-2 | work split into sq1/dist1 (per-value) | 26 | COUNTS (FAKE-free) |  |
| rv5_wk_small_copy | v5 layer-2 | work split, small-arm LUT indexed by the copy | 26 | COUNTS (FAKE-free) |  |

rv5_* generated by rv5_mk.py (the v5 reviewer's generator); measured by the author with
r11/tools/fast3.sh, scores equal to the reviewer's.

| rv6_base | v6 layer-2 | landing template control (the reuse spelling) (frame 0x78, 662 insns) | 0 | control |  |
| rv6_dl_condassign | v6 layer-2 | delta split, if ((dg = pos.y - ground) > 0), dy0 first in the sphere list (frame 0x78, 662 insns) | 4 | COUNTS (FAKE-free) |  |
| rv6_lz2 | v6 layer-2 | frame probe: the reuse body with lz[2] (frame 0x68, 662 insns) | 24 | frame probe |  |
| rv6_lz3 | v6 layer-2 | frame probe: the reuse body with lz[3] (frame 0x70, 662 insns) | 24 | frame probe |  |
| rv6_lz4 | v6 layer-2 | frame probe: the reuse body with lz[4] (frame 0x70, 662 insns) | 24 | frame probe |  |
| rv6_lz5 | v6 layer-2 | frame probe: the reuse body with lz[5] (frame 0x78, 662 insns) | 0 | frame probe (Q32 twin) |  |
| rv6_lz6 | v6 layer-2 | frame probe: the reuse body with lz[6] (frame 0x78, 662 insns) | 0 | frame probe |  |
| rv6_lz7 | v6 layer-2 | frame probe: the reuse body with lz[7] (frame 0x80, 662 insns) | 24 | frame probe |  |
| rv6_nf_add_intest | v6 layer-2 | nforce single-valued, add loop tests idx < node[7] (frame 0x78, 663 insns) | 67 | COUNTS (FAKE-free) |  |
| rv6_nf_sub_intest | v6 layer-2 | nforce single-valued, subtract loop tests idx < node[8] (frame 0x78, 663 insns) | 65 | COUNTS (FAKE-free) |  |
| rv6_tp_s32_ell_onedecl | v6 layer-2 | temp split, s32 bytes as one declaration at ellipsoid-body scope (frame 0x78, 662 insns) | 42 | COUNTS (FAKE-free) |  |
| rv6_tp_u16 | v6 layer-2 | temp split, the same with u16 bytes (frame 0x78, 662 insns) | 42 | COUNTS (FAKE-free) |  |
| rv6_tp_u8 | v6 layer-2 | temp split (copy lzc_in; table bytes u8, at arm scope) (frame 0x78, 662 insns) | 42 | COUNTS (FAKE-free) |  |
| rv6_tp_u8_ell | v6 layer-2 | temp split, u8 bytes declared at ellipsoid-body scope (frame 0x78, 662 insns) | 42 | COUNTS (FAKE-free) |  |

rv6_* generated by rv6_mk.py (the v6 reviewer's generator); measured by the author with
r11/tools/fast3.sh (frames from each build's `addiu sp,sp,-N`), scores equal to the reviewer's.

| rv7_dl_splitinit | v7 layer-2 | delta per-value (dg/dy0), `dg = pos.y; dg -= ground;` (frame 0x78, 662 insns) | 4 | COUNTS (FAKE-free) |  |
| rv7_inl_root | v7 layer-2 | static inline LUT-root helper called twice; per-value sq/root/byte/lzcount/shift, no Q28 copy (frame 0x78, 662 insns) | 45 | COUNTS (FAKE-free) |  |
| rv7_inl_root_copy | v7 layer-2 | the same helper, first call fed from a block-local `lzc_in = sq1` copy (frame 0x78, 663 insns) | 51 | COUNTS (FAKE-free) |  |
| rv7_lz2 | v7 layer-2 | frame probe: the reuse body with lz[2] (frame 0x68, 662 insns) | 24 | frame probe |  |
| rv7_lz3 | v7 layer-2 | frame probe: the reuse body with lz[3] (frame 0x70, 662 insns) | 24 | frame probe |  |
| rv7_lz8 | v7 layer-2 | frame probe: the reuse body with lz[8] (frame 0x80, 662 insns) | 24 | frame probe |  |
| rv7_nf_forinit | v7 layer-2 | nforce per-value, counts loaded in the for-init comma expression (frame 0x78, 662 insns) | 21 | COUNTS (FAKE-free) |  |
| rv7_tp_nobyte_copy | v7 layer-2 | temp per-value, copy as its own local `lzc_in`, table bytes read inline in the shift (frame 0x78, 662 insns) | 42 | COUNTS (FAKE-free) |  |
| rv7_wk_splitinit | v7 layer-2 | work per-value (sq1/dist1), sq1 summed by split-init (frame 0x78, 662 insns) | 26 | COUNTS (FAKE-free) |  |
| rv7_wkdl_splitinit | v7 layer-2 | both of the above (frame 0x78, 662 insns) | 30 | COUNTS (FAKE-free) |  |

rv7_* generated by rv7_mk.py (the v7 reviewer's generator); measured by the author with
r11/tools/fast3.sh, scores and frames equal to the reviewer's.

Banked counting spellings reaching 0: none (best 2). The zero rows are the reuse controls (rv4_base,
rv5_base, rv6_base), the set-aside dead-store closers (rv_both, rv_wd_after2, rv_wd_end) and the lz[5]/lz[6]
frame probes (rv6_lz5, rv6_lz6).
