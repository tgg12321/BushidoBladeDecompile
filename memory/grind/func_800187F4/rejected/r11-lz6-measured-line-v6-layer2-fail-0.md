# func_800187F4 — v6 landing, sixth layer-2 FAIL (records only), 2026-09-29

Body: the same bytes as v5 (fast3 control rv6_base 0/662; sandbox 0 on 644/644; the reviewer's own
verify-oracle --rebuild --allow-dirty gave the oracle SHA1). The reviewer passed the bytes, the 85 header-exact
statements and their 85 region hashes, the Q29 words (decoded field by field against the seven target
addresses), the canonical gate regions, the cited commits, all seven Ruling 11 locals under prongs A–H and
Q28/Q30/Q31/Q32, the lz[6] 0x40 derivation, and the probe totals. Its 7 new FAKE-free probes missed (best 4).

FAIL grounds:
1. The lz[6] comment's "Measured:" line ("lz[2]..lz[4] give frame 0x68") was false: lz[3] and lz[4] give 0x70
   (rv6_lz3/rv6_lz4 on c7; the author's own tmp/func_800187F4/fast_lzc5_4 on c5, `addiu sp,sp,-112`). A frame
   shift of any size costs the same 24 lines, which the older records read as "same frame". The same error was
   in proof §7 and evidence.md:152.
2. template.c still carried the v5 "16 bytes left" wording, so proof §0's "template.c (= r11/variants_v3/c7.c)"
   was false.

Fixed in v7: the "Measured:" line (lz[2] 0x68, lz[3]/lz[4] 0x70, lz[5]/lz[6] 0x78, lz[7]/lz[8] 0x80, every size
measured) in c7.c / template.c / candidate.c; template.c byte-identical to c7.c, which regenerates candidate.c
exactly (fast3 c7 = 0); proof v7 (§7, §3A row, v7 note); evidence.md:152; rv6_* banked (measured by the author,
scores and frames equal to the reviewer's); Match message v7 (six reviewers, 39 more probes).
