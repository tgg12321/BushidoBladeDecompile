# Q56 step 1 — sdata_exclude.txt liveness audit (+ step-2 evidence prep)

Scratch commit: `a739dbd204308581879038127c303a5c6ceb883e` (main HEAD at start). Scratch tree `/tmp/q56/tree` (WSL-native; `git archive` + symlinked tools/gcc-2.7.2, .venv, disc). No worktree, no main-tree tracked edits, no commit.

## Baseline and method

- **Scratch baseline:** clean `make build/bb2.exe` → exe SHA1 `62efab4f73f992798c43e8c730aa43baa10bb4fa` (== oracle), bin SHA1 `42fce5aff1490a579e919b68af56ebc5b0dc657f` (`/tmp/q56/baseline.sha1`).
- **Per-row test (incremental):** for each row, a copy of sdata_exclude.txt without that line; the owning TU (the C object defining the function, via `nm` of the reference objects) is rebuilt with the Makefile's own recipe (`make BUILD_DIR=<scratch> MASPSX_FLAGS=… MASPSX_FLAGS_GP=…` with only `--sdata-exclude=` repointed) and the .o compared byte-for-byte with the reference .o. Differing objects were relinked (reference build dir, object swapped, the Makefile's ld/objcopy) and the .bin compared: every differing object also changed the linked bin.
- **Method controls:** (1) all 20 affected TUs rebuilt with the UNMODIFIED list → 20/20 objects identical; (2) full clean build without row 5 (live) → exe `637575f6…`, bin identical to the incremental relink bin; (3) full clean build without row 18 (dead) → oracle SHA1.
- **Sanity bound (all 105 rows removed, full clean build):** exe `c1819c29…` (≠ oracle); 18 objects differ; exactly **74 functions** change (branch-target shifts normalised) — identical to the set of live-row functions (72 single-row live + func_8006EACC + func_80070C70). No function outside the list changes.
- **Proposed deletion verified by FULL clean build:** the 31 lines in `dead_rows.txt` deleted → exe SHA1 `62efab4f73f992798c43e8c730aa43baa10bb4fa` (oracle). Additionally deleting the 11 dead symbol entries inside live rows (section 'Dead symbols inside live rows') → also oracle SHA1.
- **Duplicates:** lines 56/73 (`func_8006EACC: D_800A36AC`) and 80/104 (`func_80070C70: g_gpu_ot_ptr`) are exact duplicates: each alone tests dead, removing both is live (bin changes). Keep 56 and 80, delete 73 and 104.
- **Step-2 census (`census.json`):** shipped bytes = `asm/funcs/*.s`; symbols compared by resolved address. ASPSX model used for the flag (memory/grind/func_80036140/research-common-gp.md §0, Sony ASPSX 2.34 run under dosemu2): a file gets gp for a symbol only if it DEFINES it (`.comm`/`.lcomm`/`.sdata`); an `extern` symbol is never gp. So within one original file, the base access of a symbol is gp in every function or in none.

## Counts

- Rows: **105** (103 functions; 160 symbol entries).
- **LIVE: 74** (72 live individually + lines 56 and 80, live once their duplicate is gone).
- **DEAD (delete): 31** — exact lines in `dead_rows.txt`.
- Dead symbol entries inside live rows: 11 (optional trim, oracle-verified).
- Rows whose removal breaks the build: **none** (every variant compiled and linked).

## DEAD rows (31)

| line | row | TU | function status | why inert |
|---|---|---|---|---|
| 18 | `comb_Write8: g_comb_send_buf` | code6cac_c_mid | C | inert for our C: without the row maspsx gp-converts nothing here (no direct access under that name — la/indexed only, or a merged/aliased spelling). Shipped refs: g_comb_send_buf: shipped F refs lo-access=0 la=1 gp=0 |
| 19 | `comb_Read8: g_comb_recv_buf` | code6cac_c_mid | C | inert for our C: without the row maspsx gp-converts nothing here (no direct access under that name — la/indexed only, or a merged/aliased spelling). Shipped refs: g_comb_recv_buf: shipped F refs lo-access=0 la=1 gp=0 |
| 20 | `comb_WaitRead8: g_comb_recv_buf_plus_0x4, D_800A36C4` | code6cac_c_mid | C | symbol(s) not in sdata_syms.txt, never gp-eligible |
| 34 | `func_800494D4: D_800A33E8` | text1b | C | inert for our C: without the row maspsx gp-converts nothing here (no direct access under that name — la/indexed only, or a merged/aliased spelling). Shipped refs: D_800A33E8: shipped F refs lo-access=1 la=0 gp=0 |
| 40 | `func_80063BD0: D_800A344C, D_800A3454` | text1b_tu1c | C | inert for our C: without the row maspsx gp-converts nothing here (no direct access under that name — la/indexed only, or a merged/aliased spelling). Shipped refs: D_800A344C: shipped F refs lo-access=0 la=1 gp=0; D_800A3454: shipped F refs lo-access=0 la=1 gp=0 |
| 43 | `func_80065800: g_gpu_ot_ptr, D_800A3834` | text1b_tu1c | INCLUDE_ASM (queue: active) | body is INCLUDE_ASM — maspsx never rewrites it; **shipped bytes access g_gpu_ot_ptr, D_800A3834 non-gp while F is in sdata_funcs: the row is likely needed again if this function lands as C (it would then have to meet (a)-(d))** |
| 44 | `func_800678A8: D_800A34E8` | text1b_tu1c | C | inert for our C: without the row maspsx gp-converts nothing here (no direct access under that name — la/indexed only, or a merged/aliased spelling). Shipped refs: D_800A34E8: shipped F refs lo-access=1 la=0 gp=0 |
| 45 | `func_80067D14: D_800A34E8` | text1b_tu1c | C | inert for our C: without the row maspsx gp-converts nothing here (no direct access under that name — la/indexed only, or a merged/aliased spelling). Shipped refs: D_800A34E8: shipped F refs lo-access=0 la=1 gp=0 |
| 47 | `func_80069250: D_800A3518` | text1b_tu1c | C | inert for our C: without the row maspsx gp-converts nothing here (no direct access under that name — la/indexed only, or a merged/aliased spelling). Shipped refs: D_800A3518: shipped F refs lo-access=0 la=1 gp=0 |
| 48 | `func_800693CC: D_800A350C, D_800A3518, D_800A36AC` | text1b_tu1c | INCLUDE_ASM (queue: rotated) | body is INCLUDE_ASM — maspsx never rewrites it; **shipped bytes access D_800A36AC non-gp while F is in sdata_funcs: the row is likely needed again if this function lands as C (it would then have to meet (a)-(d))** |
| 54 | `func_8006B578: D_800A350C` | text1b_tu1c | C | inert for our C: without the row maspsx gp-converts nothing here (no direct access under that name — la/indexed only, or a merged/aliased spelling). Shipped refs: D_800A350C: shipped F refs lo-access=0 la=1 gp=0 |
| 57 | `func_8006B92C: D_800A350C` | text1b_tu1c | C | inert for our C: without the row maspsx gp-converts nothing here (no direct access under that name — la/indexed only, or a merged/aliased spelling). Shipped refs: D_800A350C: shipped F refs lo-access=0 la=1 gp=0 |
| 59 | `func_8006BD28: g_gpu_ot_ptr` | text1b_tu1c | INCLUDE_ASM (canonical) | body is INCLUDE_ASM — maspsx never rewrites it; canonical-asm (COMPLETED-INLINE-ASM-CANONICAL): the body stays asm, the row can never apply |
| 66 | `func_8006D5D4: D_800A350C` | text1b_tu1c | C | inert for our C: without the row maspsx gp-converts nothing here (no direct access under that name — la/indexed only, or a merged/aliased spelling). Shipped refs: D_800A350C: shipped F refs lo-access=0 la=1 gp=0 |
| 70 | `func_8006DF68: D_800A350C` | text1b_tu1c | C | inert for our C: without the row maspsx gp-converts nothing here (no direct access under that name — la/indexed only, or a merged/aliased spelling). Shipped refs: D_800A350C: shipped F refs lo-access=0 la=1 gp=0 |
| 73 | `func_8006EACC: D_800A36AC` | text1b_tu1d | C | exact duplicate of line 56 |
| 74 | `func_8006ECF4: D_800A3588, D_800A358C` | text1b_tu1d | C | inert for our C: without the row maspsx gp-converts nothing here (no direct access under that name — la/indexed only, or a merged/aliased spelling). Shipped refs: D_800A3588: shipped F refs lo-access=1 la=0 gp=0; D_800A358C: shipped F refs lo-access=1 la=0 gp=0 |
| 76 | `func_8006F100: D_800A35C8` | text1b_tu1d | C | inert for our C: without the row maspsx gp-converts nothing here (no direct access under that name — la/indexed only, or a merged/aliased spelling). Shipped refs: D_800A35C8: shipped F refs lo-access=0 la=1 gp=0 |
| 87 | `func_80074220: g_gpu_ot_ptr` | text1b_tu1e | C | function not in sdata_funcs.txt (exclusion only consulted for sdata_funcs members) |
| 90 | `func_80074E08: g_gpu_ot_ptr` | text1b_tu2 | INCLUDE_ASM (queue: active) | body is INCLUDE_ASM — maspsx never rewrites it; **shipped bytes access g_gpu_ot_ptr non-gp while F is in sdata_funcs: the row is likely needed again if this function lands as C (it would then have to meet (a)-(d))** |
| 92 | `func_800759D0: g_gpu_ot_ptr` | text1b_tu2 | INCLUDE_ASM (queue: active) | body is INCLUDE_ASM — maspsx never rewrites it; **shipped bytes access g_gpu_ot_ptr non-gp while F is in sdata_funcs: the row is likely needed again if this function lands as C (it would then have to meet (a)-(d))** |
| 95 | `func_800770B8: g_gpu_ot_ptr` | text1b_tu2 | INCLUDE_ASM (queue: active) | body is INCLUDE_ASM — maspsx never rewrites it; **shipped bytes access g_gpu_ot_ptr non-gp while F is in sdata_funcs: the row is likely needed again if this function lands as C (it would then have to meet (a)-(d))** |
| 101 | `func_80069AE4: g_gpu_ot_ptr` | text1b_tu1c | C | function not in sdata_funcs.txt (exclusion only consulted for sdata_funcs members) |
| 102 | `SetDrawEnv2: D_8009BE78, D_8009BE7A` | display | C | function not in sdata_funcs.txt (exclusion only consulted for sdata_funcs members) |
| 103 | `_addque2: _qin, _qout, D_8009BE75, D_8009BF54, D_8009BE80, D_8009BF48, D_8009BF80, _qlog, D_8009BF6C, D_8009BF70, D_8009BE7C` | display | C | function not in sdata_funcs.txt (exclusion only consulted for sdata_funcs members) |
| 104 | `func_80070C70: g_gpu_ot_ptr` | text1b_tu1d | C | exact duplicate of line 80 |
| 105 | `func_80033D38: D_800A3858, D_80101ED2, D_80101ED6, D_800A38E9` | code6cac_b_tu2 | C | function not in sdata_funcs.txt (exclusion only consulted for sdata_funcs members) |
| 106 | `_spu_gcSPU: _spu_AllocLastNum, _spu_memList` | main | C | function not in sdata_funcs.txt (exclusion only consulted for sdata_funcs members) |
| 107 | `func_80068F70: D_8009BC04` | text1b_tu1c | C | symbol(s) not in sdata_syms.txt, never gp-eligible |
| 108 | `func_80077B30: D_8009BD41, D_8009BD42, D_8009BD43, D_8009BD3B, D_8009BD3C, D_8009BD3D, D_8009BD38` | text1b_b | C | symbol(s) not in sdata_syms.txt, never gp-eligible |
| 109 | `_SpuSetAnyVoice: _spu_env, _spu_RXX, _spu_RQmask` | main | C | function not in sdata_funcs.txt (exclusion only consulted for sdata_funcs members) |

## LIVE rows (74) — bytes that change + step-2 evidence

Removing a live symbol turns each listed direct access `lui rX,%hi(S); l/s rY,%lo(S)(rX)` into one `l/s rY,%gp_rel(S)($gp)` (function shrinks one word per access; later branches shift). Columns: **chg** = accesses that flip (HI16 relocs removed when that symbol alone is dropped); **shipped F** = F's own refs to S's address in the original bytes (lo-access / la / gp); **gp users** = functions anywhere that access S's address gp-relative (count, their TUs); **same-TU gp** = gp users built into F's own TU.

| line | function | TU | status | symbol | sym live? | chg | shipped F (lo/la/gp) | F gp refs (any sym) | gp users (n; TUs) | same-TU gp | flag |
|---|---|---|---|---|---|---|---|---|---|---|---|
| 5 | func_80016E60 | ings | C | D_800A31DA | LIVE | 1 | 1/0/0 | 19 | 3; code6cac_b2_post, code6cac_b2_pre, replay_camera_rob_back_loose2 | — | NEEDS JUDGMENT (TU-consistent) |
| 5 | func_80016E60 | ings | C | D_800A3770 | dead | 0 | 1/0/0 | 19 | 1; ings | sys_GameInit | — (dead symbol; trim) |
| 6 | main | ings | C | D_800A31DA | LIVE | 1 | 1/0/0 | 24 | 3; code6cac_b2_post, code6cac_b2_pre, replay_camera_rob_back_loose2 | — | NEEDS JUDGMENT (TU-consistent) |
| 6 | main | ings | C | D_800A3770 | dead | 0 | 0/1/0 | 24 | 1; ings | sys_GameInit | — (dead symbol; trim) |
| 7 | func_80030D7C | code6cac_b_tu2 | C | D_800A38DC | LIVE | 1 | 1/0/0 | 3 | 2; ings | — | NEEDS JUDGMENT (TU-consistent) |
| 8 | func_80031B24 | code6cac_b_tu2 | C | g_disp_fade | LIVE | 1 | 1/0/0 | 3 | 2; ings | — | NEEDS JUDGMENT (TU-consistent) |
| 9 | func_80034708 | code6cac_b3 | C | D_800A3690 | LIVE | 6 | 6/0/0 | 16 | 1; ings | — | NEEDS JUDGMENT (TU-consistent) |
| 9 | func_80034708 | code6cac_b3 | C | D_800A36F9 | LIVE | 7 | 7/0/0 | 16 | 1; ings | — | NEEDS JUDGMENT (TU-consistent) |
| 10 | func_80035480 | code6cac_b2_pre | C | D_800A3834 | LIVE | 1 | 1/0/0 | 3 | 2; ings | — | NEEDS JUDGMENT (TU-consistent) |
| 11 | func_80035618 | replay_camera_rob_back_loose2 | C | D_800A3834 | LIVE | 1 | 1/0/0 | 7 | 2; ings | — | NEEDS JUDGMENT (TU-consistent) |
| 12 | func_80035828 | code6cac_b2_post | C | D_800A36F1 | LIVE | 2 | 2/0/0 | 18 | 2; ings | — | NEEDS JUDGMENT (TU-consistent) |
| 12 | func_80035828 | code6cac_b2_post | C | D_800A3834 | LIVE | 5 | 5/0/0 | 18 | 2; ings | — | NEEDS JUDGMENT (TU-consistent) |
| 12 | func_80035828 | code6cac_b2_post | C | D_800A38DC | LIVE | 1 | 1/0/0 | 18 | 2; ings | — | NEEDS JUDGMENT (TU-consistent) |
| 13 | func_800383A4 | code6cac_c_mid | C | g_memcard_fd | LIVE | 2 | 2/0/0 | 23 | 2; code6cac_c | — | NEEDS JUDGMENT (TU-consistent) |
| 14 | func_80038658 | code6cac_c_mid | C | g_memcard_fd | LIVE | 2 | 2/0/0 | 5 | 2; code6cac_c | — | NEEDS JUDGMENT (TU-consistent) |
| 15 | func_800397D4 | code6cac_c_mid | C | D_800A3834 | LIVE | 1 | 1/0/0 | 1 | 2; ings | — | NEEDS JUDGMENT (TU-consistent) |
| 16 | func_8003993C | code6cac_c_mid | C | D_800A3834 | LIVE | 1 | 1/0/0 | 18 | 2; ings | — | NEEDS JUDGMENT (TU-consistent) |
| 16 | func_8003993C | code6cac_c_mid | C | D_800A38DC | LIVE | 1 | 1/0/0 | 18 | 2; ings | — | NEEDS JUDGMENT (TU-consistent) |
| 17 | comb_ResetClose | code6cac_c_mid | C | D_800A3834 | LIVE | 1 | 1/0/0 | 2 | 2; ings | — | NEEDS JUDGMENT (TU-consistent) |
| 21 | func_8003CE18 | code6cac_c2 | C | D_800A3834 | LIVE | 1 | 1/0/0 | 1 | 2; ings | — | NEEDS JUDGMENT (TU-consistent) |
| 22 | func_8003CF84 | code6cac_c2 | C | D_800A3834 | LIVE | 1 | 1/0/0 | 1 | 2; ings | — | NEEDS JUDGMENT (TU-consistent) |
| 22 | func_8003CF84 | code6cac_c2 | C | D_800A38DC | LIVE | 2 | 2/0/0 | 1 | 2; ings | — | NEEDS JUDGMENT (TU-consistent) |
| 23 | func_8003D330 | code6cac_c2 | C | g_gpu_ot_ptr | LIVE | 1 | 1/0/0 | 1 | 3; ings | — | NEEDS JUDGMENT (TU-consistent) |
| 24 | func_8003D39C | code6cac_c2 | C | g_gpu_ot_ptr | LIVE | 1 | 1/0/0 | 3 | 3; ings | — | NEEDS JUDGMENT (TU-consistent) |
| 24 | func_8003D39C | code6cac_c2 | C | D_800A3930 | dead | 0 | 0/1/0 | 3 | 0;  | — | — (dead symbol; trim) |
| 25 | func_8003E6D8 | code6cac_c2 | C | D_800A3708 | LIVE | 2 | 2/0/0 | 4 | 1; text1a_c | — | NEEDS JUDGMENT (TU-consistent) |
| 25 | func_8003E6D8 | code6cac_c2 | C | D_800A3820 | LIVE | 6 | 6/0/0 | 4 | 1; text1a_c | — | NEEDS JUDGMENT (TU-consistent) |
| 26 | func_8003EB84 | code6cac_c2 | C | D_800A3820 | LIVE | 2 | 2/0/0 | 1 | 1; text1a_c | — | NEEDS JUDGMENT (TU-consistent) |
| 27 | gpu_AddDrawMove | text1a_pre | C | D_800A36AC | LIVE | 1 | 1/0/0 | 7 | 3; ings | — | NEEDS JUDGMENT (TU-consistent) |
| 27 | gpu_AddDrawMove | text1a_pre | C | D_800A378C | LIVE | 1 | 1/0/0 | 7 | 2; text1a_c | — | NEEDS JUDGMENT (TU-consistent) |
| 28 | func_80044504 | text1a_c | C | D_800A36AC | LIVE | 1 | 1/0/0 | 4 | 3; ings | — | NEEDS JUDGMENT (TU-consistent) |
| 29 | func_800460E4 | text1a_c2 | C | D_800A38DC | LIVE | 1 | 1/0/0 | 2 | 2; ings | — | NEEDS JUDGMENT (TU-consistent) |
| 30 | game_Init | sound | C | D_800A3790 | LIVE | 1 | 1/0/0 | 1 | 1; text1a_c | — | NEEDS JUDGMENT (TU-consistent) |
| 31 | func_80046BF4 | sound | C | g_gpu_ot_ptr | LIVE | 1 | 1/0/0 | 1 | 3; ings | — | NEEDS JUDGMENT (TU-consistent) |
| 31 | func_80046BF4 | sound | C | D_800A378C | LIVE | 1 | 1/0/0 | 1 | 2; text1a_c | — | NEEDS JUDGMENT (TU-consistent) |
| 31 | func_80046BF4 | sound | C | D_800A3820 | LIVE | 1 | 1/0/0 | 1 | 1; text1a_c | — | NEEDS JUDGMENT (TU-consistent) |
| 32 | camera_CalcAngles | sound | C | D_800A3708 | LIVE | 1 | 1/0/0 | 2 | 1; text1a_c | — | NEEDS JUDGMENT (TU-consistent) |
| 33 | func_80048BA4 | text1b | C | D_800A36AC | LIVE | 1 | 1/0/0 | 5 | 3; ings | — | NEEDS JUDGMENT (TU-consistent) |
| 33 | func_80048BA4 | text1b | C | D_800A378C | LIVE | 1 | 1/0/0 | 5 | 2; text1a_c | — | NEEDS JUDGMENT (TU-consistent) |
| 33 | func_80048BA4 | text1b | C | D_800A3820 | LIVE | 6 | 6/0/0 | 5 | 1; text1a_c | — | NEEDS JUDGMENT (TU-consistent) |
| 35 | func_8005FC9C | text1b_tu1c | C | D_800A36AC | LIVE | 1 | 1/0/0 | 5 | 3; ings | — | NEEDS JUDGMENT (TU-consistent) |
| 35 | func_8005FC9C | text1b_tu1c | C | g_gpu_ot_ptr | LIVE | 4 | 4/0/0 | 5 | 3; ings | — | NEEDS JUDGMENT (TU-consistent) |
| 36 | func_80060768 | text1b_tu1c | C | g_gpu_ot_ptr | LIVE | 5 | 5/0/0 | 8 | 3; ings | — | NEEDS JUDGMENT (TU-consistent) |
| 37 | func_800620B8 | text1b_tu1c | C | g_gpu_ot_ptr | LIVE | 1 | 1/0/0 | 70 | 3; ings | — | NEEDS JUDGMENT (TU-consistent) |
| 38 | func_8006295C | text1b_tu1c | C | g_gpu_ot_ptr | LIVE | 1 | 1/0/0 | 50 | 3; ings | — | NEEDS JUDGMENT (TU-consistent) |
| 39 | func_80063084 | text1b_tu1c | C | g_gpu_ot_ptr | LIVE | 1 | 1/0/0 | 79 | 3; ings | — | NEEDS JUDGMENT (TU-consistent) |
| 41 | func_80063E10 | text1b_tu1c | C | D_800A344C | dead | 0 | 1/0/0 | 51 | 1; text1b_tu1c | func_80060C60 | — (dead symbol; trim) |
| 41 | func_80063E10 | text1b_tu1c | C | D_800A3454 | dead | 0 | 0/1/0 | 51 | 1; text1b_tu1c | func_80060C60 | — (dead symbol; trim) |
| 41 | func_80063E10 | text1b_tu1c | C | g_gpu_ot_ptr | LIVE | 1 | 1/0/0 | 51 | 3; ings | — | NEEDS JUDGMENT (TU-consistent) |
| 42 | func_800646E8 | text1b_tu1c | C | g_gpu_ot_ptr | LIVE | 1 | 1/0/0 | 51 | 3; ings | — | NEEDS JUDGMENT (TU-consistent) |
| 46 | func_80068D88 | text1b_tu1c | C | g_gpu_ot_ptr | LIVE | 1 | 1/0/0 | 8 | 3; ings | — | NEEDS JUDGMENT (TU-consistent) |
| 49 | func_80069F80 | text1b_tu1c | C | g_gpu_ot_ptr | LIVE | 1 | 1/0/0 | 3 | 3; ings | — | NEEDS JUDGMENT (TU-consistent) |
| 50 | func_8006A1A0 | text1b_tu1c | C | g_gpu_ot_ptr | LIVE | 1 | 1/0/0 | 3 | 3; ings | — | NEEDS JUDGMENT (TU-consistent) |
| 51 | func_8006A564 | text1b_tu1c | C | g_gpu_ot_ptr | LIVE | 4 | 4/0/0 | 4 | 3; ings | — | NEEDS JUDGMENT (TU-consistent) |
| 52 | func_8006A880 | text1b_tu1c | C | g_gpu_ot_ptr | LIVE | 13 | 13/0/0 | 17 | 3; ings | — | NEEDS JUDGMENT (TU-consistent) |
| 53 | func_8006B120 | text1b_tu1c | C | g_gpu_ot_ptr | LIVE | 2 | 2/0/0 | 15 | 3; ings | — | NEEDS JUDGMENT (TU-consistent) |
| 55 | func_8006B898 | text1b_tu1c | C | D_800A3518 | dead | 0 | 0/1/0 | 2 | 3; text1b_tu1c | func_80068F70, func_8006C21C, func_8006E2A8 | — (dead symbol; trim) |
| 55 | func_8006B898 | text1b_tu1c | C | D_800A36AC | LIVE | 1 | 1/0/0 | 2 | 3; ings | — | NEEDS JUDGMENT (TU-consistent) |
| 56 | func_8006EACC | text1b_tu1d | C | D_800A36AC | LIVE | 1 | 1/0/0 | 11 | 3; ings | — | NEEDS JUDGMENT (TU-consistent) |
| 58 | func_8006BB68 | text1b_tu1c | C | g_gpu_ot_ptr | LIVE | 2 | 2/0/0 | 2 | 3; ings | — | NEEDS JUDGMENT (TU-consistent) |
| 60 | func_8006BEC4 | text1b_tu1c | C | D_800A36AC | LIVE | 1 | 1/0/0 | 24 | 3; ings | — | NEEDS JUDGMENT (TU-consistent) |
| 60 | func_8006BEC4 | text1b_tu1c | C | g_gpu_ot_ptr | LIVE | 2 | 2/0/0 | 24 | 3; ings | — | NEEDS JUDGMENT (TU-consistent) |
| 61 | func_8006C168 | text1b_tu1c | C | D_800A3518 | dead | 0 | 0/1/0 | 2 | 3; text1b_tu1c | func_80068F70, func_8006C21C, func_8006E2A8 | — (dead symbol; trim) |
| 61 | func_8006C168 | text1b_tu1c | C | D_800A36AC | LIVE | 1 | 1/0/0 | 2 | 3; ings | — | NEEDS JUDGMENT (TU-consistent) |
| 62 | func_8006C21C | text1b_tu1c | C | g_gpu_ot_ptr | LIVE | 7 | 7/0/0 | 6 | 3; ings | — | NEEDS JUDGMENT (TU-consistent) |
| 63 | func_8006CFBC | text1b_tu1c | C | g_gpu_ot_ptr | LIVE | 1 | 1/0/0 | 2 | 3; ings | — | NEEDS JUDGMENT (TU-consistent) |
| 64 | func_8006D338 | text1b_tu1c | C | D_800A3518 | dead | 0 | 0/1/0 | 2 | 3; text1b_tu1c | func_80068F70, func_8006C21C, func_8006E2A8 | — (dead symbol; trim) |
| 64 | func_8006D338 | text1b_tu1c | C | D_800A36AC | LIVE | 1 | 1/0/0 | 2 | 3; ings | — | NEEDS JUDGMENT (TU-consistent) |
| 65 | func_8006D3DC | text1b_tu1c | C | g_gpu_ot_ptr | LIVE | 1 | 1/0/0 | 3 | 3; ings | — | NEEDS JUDGMENT (TU-consistent) |
| 67 | func_8006D74C | text1b_tu1c | C | D_800A3518 | dead | 0 | 0/1/0 | 2 | 3; text1b_tu1c | func_80068F70, func_8006C21C, func_8006E2A8 | — (dead symbol; trim) |
| 67 | func_8006D74C | text1b_tu1c | C | D_800A36AC | LIVE | 1 | 1/0/0 | 2 | 3; ings | — | NEEDS JUDGMENT (TU-consistent) |
| 68 | func_8006D808 | text1b_tu1c | C | D_800A36AC | LIVE | 1 | 1/0/0 | 4 | 3; ings | — | NEEDS JUDGMENT (TU-consistent) |
| 68 | func_8006D808 | text1b_tu1c | C | g_gpu_ot_ptr | LIVE | 3 | 3/0/0 | 4 | 3; ings | — | NEEDS JUDGMENT (TU-consistent) |
| 69 | func_8006DD94 | text1b_tu1c | C | g_gpu_ot_ptr | LIVE | 1 | 1/0/0 | 3 | 3; ings | — | NEEDS JUDGMENT (TU-consistent) |
| 71 | func_8006E068 | text1b_tu1c | C | D_800A3518 | dead | 0 | 0/1/0 | 2 | 3; text1b_tu1c | func_80068F70, func_8006C21C, func_8006E2A8 | — (dead symbol; trim) |
| 71 | func_8006E068 | text1b_tu1c | C | D_800A36AC | LIVE | 1 | 1/0/0 | 2 | 3; ings | — | NEEDS JUDGMENT (TU-consistent) |
| 72 | func_8006E534 | text1b_tu1d | C | g_gpu_ot_ptr | LIVE | 1 | 1/0/0 | 45 | 3; ings | — | NEEDS JUDGMENT (TU-consistent) |
| 75 | func_8006F038 | text1b_tu1d | C | g_gpu_ot_ptr | LIVE | 2 | 2/0/0 | 3 | 3; ings | — | NEEDS JUDGMENT (TU-consistent) |
| 77 | func_8006F528 | text1b_tu1d | C | g_gpu_ot_ptr | LIVE | 5 | 5/0/0 | 17 | 3; ings | — | NEEDS JUDGMENT (TU-consistent) |
| 78 | func_8006F97C | text1b_tu1d | C | D_800A3588 | dead | 0 | 3/0/0 | 25 | 1; text1b_tu1d | func_8006E534 | — (dead symbol; trim) |
| 78 | func_8006F97C | text1b_tu1d | C | D_800A358C | dead | 0 | 0/1/0 | 25 | 1; text1b_tu1d | func_8006E534 | — (dead symbol; trim) |
| 78 | func_8006F97C | text1b_tu1d | C | g_gpu_ot_ptr | LIVE | 3 | 3/0/0 | 25 | 3; ings | — | NEEDS JUDGMENT (TU-consistent) |
| 79 | func_80070188 | text1b_tu1d | C | g_gpu_ot_ptr | LIVE | 1 | 1/0/0 | 53 | 3; ings | — | NEEDS JUDGMENT (TU-consistent) |
| 80 | func_80070C70 | text1b_tu1d | C | g_gpu_ot_ptr | LIVE | 3 | 3/0/0 | 8 | 3; ings | — | NEEDS JUDGMENT (TU-consistent) |
| 81 | func_80070F78 | text1b_tu1d | C | g_gpu_ot_ptr | LIVE | 1 | 1/0/0 | 81 | 3; ings | — | NEEDS JUDGMENT (TU-consistent) |
| 82 | func_800720FC | text1b_tu1d | C | g_gpu_ot_ptr | LIVE | 6 | 6/0/0 | 62 | 3; ings | — | NEEDS JUDGMENT (TU-consistent) |
| 83 | func_80072BC4 | text1b_tu1d | C | g_gpu_ot_ptr | LIVE | 1 | 1/0/0 | 1 | 3; ings | — | NEEDS JUDGMENT (TU-consistent) |
| 84 | func_80072CD4 | text1b_tu1d | C | g_gpu_ot_ptr | LIVE | 1 | 1/0/0 | 1 | 3; ings | — | NEEDS JUDGMENT (TU-consistent) |
| 85 | func_80072FCC | text1b_tu1d | C | g_gpu_ot_ptr | LIVE | 1 | 1/0/0 | 1 | 3; ings | — | NEEDS JUDGMENT (TU-consistent) |
| 86 | func_80073200 | text1b_tu1d | C | g_gpu_ot_ptr | LIVE | 4 | 4/0/0 | 5 | 3; ings | — | NEEDS JUDGMENT (TU-consistent) |
| 88 | func_80074488 | text1b_tu1e | C | g_gpu_ot_ptr | LIVE | 1 | 1/0/0 | 8 | 3; ings | — | NEEDS JUDGMENT (TU-consistent) |
| 89 | func_80074B18 | text1b_tu2 | C | g_gpu_ot_ptr | LIVE | 1 | 1/0/0 | 2 | 3; ings | — | NEEDS JUDGMENT (TU-consistent) |
| 91 | func_800753D8 | text1b_tu2 | C | g_gpu_ot_ptr | LIVE | 2 | 2/0/0 | 1 | 3; ings | — | NEEDS JUDGMENT (TU-consistent) |
| 93 | func_8007636C | text1b_tu2 | C | g_gpu_ot_ptr | LIVE | 2 | 2/0/0 | 9 | 3; ings | — | NEEDS JUDGMENT (TU-consistent) |
| 94 | func_80076D74 | text1b_tu2 | C | g_gpu_ot_ptr | LIVE | 2 | 2/0/0 | 5 | 3; ings | — | NEEDS JUDGMENT (TU-consistent) |
| 96 | func_80077724 | text1b_tu2 | C | D_800A36AC | LIVE | 1 | 1/0/0 | 2 | 3; ings | — | NEEDS JUDGMENT (TU-consistent) |
| 97 | func_80077D94 | text1b_b | C | g_gpu_ot_ptr | LIVE | 5 | 5/0/0 | 29 | 3; ings | — | NEEDS JUDGMENT (TU-consistent) |
| 98 | func_800784E4 | text1b_b | C | g_gpu_ot_ptr | LIVE | 1 | 1/0/0 | 6 | 3; ings | — | NEEDS JUDGMENT (TU-consistent) |
| 99 | func_80078654 | text1b_b | C | g_gpu_ot_ptr | LIVE | 2 | 2/0/0 | 3 | 3; ings | — | NEEDS JUDGMENT (TU-consistent) |
| 100 | func_80078824 | text1b_b | C | g_gpu_ot_ptr | LIVE | 1 | 1/0/0 | 6 | 3; ings | — | NEEDS JUDGMENT (TU-consistent) |

## Per-symbol view (distinct live symbols)

gp users = every function whose SHIPPED bytes reach the symbol's address gp-relative (under the ASPSX model its original file defined the symbol). The last column lists functions in those same TUs whose shipped bytes make a direct non-gp (lui/%lo load/store) access — a per-file model cannot put them in the defining file unless the access is indexed.

| symbol | addr | live exclusions | gp users (function (TU)) | non-gp direct refs in the gp users' TUs |
|---|---|---|---|---|
| D_800A31DA | 0x800a31da | 2 | func_80035480 (code6cac_b2_pre), func_80035618 (replay_camera_rob_back_loose2), func_80035828 (code6cac_b2_post) | — |
| D_800A3690 | 0x800a3690 | 1 | sys_GameInit (ings) | — |
| D_800A36AC | 0x800a36ac | 13 | func_80016E60 (ings), func_800174F4 (ings), main (ings) | — |
| D_800A36F1 | 0x800a36f1 | 1 | main (ings), sys_GameInit (ings) | — |
| D_800A36F9 | 0x800a36f9 | 1 | sys_GameInit (ings) | — |
| D_800A3708 | 0x800a3708 | 2 | func_80044504 (text1a_c) | — |
| D_800A378C | 0x800a378c | 3 | func_800444BC (text1a_c), func_800444E0 (text1a_c) | — |
| D_800A3790 | 0x800a3790 | 1 | func_80044504 (text1a_c) | — |
| D_800A3820 | 0x800a3820 | 4 | func_80044504 (text1a_c) | func_80044800 |
| D_800A3834 | 0x800a3834 | 8 | func_80016E60 (ings), main (ings) | — |
| D_800A38DC | 0x800a38dc | 5 | func_80016E60 (ings), main (ings) | — |
| g_disp_fade | 0x800a36a8 | 1 | func_800174F4 (ings), sys_Init (ings) | — |
| g_gpu_ot_ptr | 0x800a374c | 44 | func_80016E60 (ings), func_800174F4 (ings), main (ings) | — |
| g_memcard_fd | 0x800a3794 | 2 | memcard_ReadFile (code6cac_c), memcard_WriteFile (code6cac_c) | — |

Note D_800A3820: func_80044800 (text1a_c, not in sdata_funcs) accesses it directly non-gp (0x80044AC4/0x80044AD4, `lw`/`sw` via lui) while func_80044504 in the same object uses gp (0x80044634) — under the per-file model these two cannot share an original file; that is a text1a_c TU-boundary fact, independent of the exclusion rows (whose functions are in other TUs).


## Dead symbols inside live rows (11 entries; optional trim, oracle-verified jointly)

line 5 func_80016E60: D_800A3770 · line 6 main: D_800A3770 · line 24 func_8003D39C: D_800A3930 · line 41 func_80063E10: D_800A344C, D_800A3454 · lines 55/61/64/67/71 (func_8006B898, func_8006C168, func_8006D338, func_8006D74C, func_8006E068): D_800A3518 · line 78 func_8006F97C: D_800A3588, D_800A358C. Variant file: `/tmp/q56/variants/cleaned_trim.txt`.

## Step-2 flags — summary and reading

- NEEDS JUDGMENT (TU-consistent): **88** live (function, symbol) pairs

- **(a) evidence status:** for NO live row is Sony-ASPSX evidence banked (no ledger runs cc1psx+ASPSX with the
  row's behaviour; the only ASPSX runs, research-common-gp.md, concern the COMMON `sym+N` rule). So as of today
  no row *clearly meets* (a). (a)(1)'s shipped signature holds for every live pair (F's own shipped refs to S are
  all non-gp — `gp` column is 0 throughout — while other functions reach S gp-relative).
- **What ASPSX would do:** per file, gp iff the file defines S. A live row therefore asserts "F's original file
  declared S `extern`". *TU-consistent* pairs (no gp user of S in F's current TU) are reproducible in principle
  by a per-file model and need the (a)(2)/(a)(3) cc1psx+ASPSX runs on the TU. *TU-inconsistent* pairs (a
  same-TU function reaches S gp-relative in the shipped bytes) cannot be reproduced by one ASPSX run over our
  current file; they hold only if the original file boundary lies between F and those gp users — a TU-split
  question, not an assembler-fidelity one. Neither is decided here.
- **(b)** the gate's single transform is structural: `_sdata_allowed_for_current_func` returns False for listed
  (F,S) — only removes gp, base and offset alike, keyed on `.ent` (INCLUDE_ASM bodies never touched). It is
  broader than the COMMON gate's (b) (which spared the base access).
- **(c)/(d) registration gap:** sdata_exclude.txt is in the Makefile `PIPELINE_DEPS` and mirrored in
  `engine/buildconfig.py`, but it is NOT in `engine/cheats.py MASPSX_GATE_LISTS`, not in grindlib `GATE_FILES`,
  and has no layer-2 PASS on record — (c)'s registration and (d)'s review are unmet for every row.
- **Aggregate pattern (all 88 live pairs are TU-consistent):** every gp user of every live symbol sits in a
  DIFFERENT TU from all the functions excluded for it — mostly `ings` (g_gpu_ot_ptr, D_800A36AC, D_800A3834,
  D_800A38DC, D_800A3690/36F1/36F9, g_disp_fade), `text1a_c` (D_800A3708/378C/3790/3820), `code6cac_c`
  (g_memcard_fd) and the 0x80035480-0x80035828 TUs (D_800A31DA). In aggregate the list is exactly what the
  ASPSX per-file rule ("gp only in the file that defines the symbol") predicts, i.e. it patches the global
  sdata_syms x sdata_funcs over-grant. That suggests a global per-file model could replace the list (an owner
  question under the Q62 pattern; not evaluated here). The only per-file contradiction found is outside the
  rows: D_800A3820 in text1a_c (func_80044504 gp vs func_80044800 non-gp).
- **Surprises:** (1) two exact duplicate rows; (2) six dead rows name INCLUDE_ASM functions — func_8006BD28 is
  canonical-asm (safe to drop); func_80065800, func_800693CC, func_80074E08, func_800759D0, func_800770B8 are
  queued, and their shipped bytes access the listed symbols non-gp, so the row is likely needed again on
  landing (func_80065800 has a ready landing package whose memory/grind/func_80065800/candidate.c names
  g_gpu_ot_ptr directly — deleting line 43 before it lands would very likely make that package miss the
  oracle; hold line 43, or the landing must re-add it under (a)-(d)); (3) 7 rows name functions not in sdata_funcs.txt and are structurally inert; (4) no row's removal
  breaks the build.


## Follow-up (B): the 5 dead rows naming still-queued INCLUDE_ASM functions — do their banked candidates need the row?

Measured by applying each banked landing package to fresh scratch copies of the pinned commit and building the
WHOLE EXE (`tmp/q56/candtest.py` -> `/tmp/q56/candtest.json`): (1) with the row, (2) with the row deleted,
(3) on the per-file-model POC tree (`MODEL.md`: no sdata lists at all).

| line | row | function | banked candidate | with row | row deleted | per-file model (no lists) | needs the row today? |
|---|---|---|---|---|---|---|---|
| 43 | `func_80065800: g_gpu_ot_ptr, D_800A3834` | func_80065800 (queue: active) | `candidate.c` + ready landing package `tools/land.py` (body `tmp/f65800/final.c`) | oracle | NOT oracle (32c91286) | oracle | **YES** — deleting line 43 breaks the package |
| 92 | `func_800759D0: g_gpu_ot_ptr` | func_800759D0 (queue: active) | `candidate.c` + `landing.patch` (its 2 text1b_tu2.c hunks respelling D_8009BCE4 are already on main) | oracle | NOT oracle (b85c0efd) | oracle | **YES** — deleting line 92 breaks the package |
| 48 | `func_800693CC: D_800A350C, D_800A3518, D_800A36AC` | func_800693CC (queue: rotated) | none (evidence.md, hypotheses.md, pre-include-asm-body.c only) | — | — | — | no candidate; the shipped bytes access D_800A36AC non-gp, so a future C body naming it directly would need the D_800A36AC part (the other two symbols: la only) |
| 90 | `func_80074E08: g_gpu_ot_ptr` | func_80074E08 (queue: active) | none at `candidate.c` (spellings under `ff-c-2026-09-30/`, not tested) | — | — | — | no banked candidate.c; shipped bytes access g_gpu_ot_ptr non-gp -> a C body naming it directly would need the row |
| 95 | `func_800770B8: g_gpu_ot_ptr` | func_800770B8 (queue: active) | none (`ff-c-2026-09-30/f0_control.diff` only) | — | — | — | as line 90 |

**Consequence for step 1's deletion list:** lines 43 and 92 are dead on today's tree but are LIVE for the ready
landing packages (both packages reach the oracle only with their row). Deleting them would force those landings to
re-add the rows under (a)-(d). Lines 48/90/95 are likely in the same position once their functions become C. Under the
per-file model (MODEL.md) none of the five rows is needed: both packages land at the oracle SHA1 with no list at
all, because their files (text1b_tu1c, text1b_tu2) do not define g_gpu_ot_ptr / D_800A36AC / D_800A3834 (those are
defined, i.e. gp-accessed, only in `ings`).

