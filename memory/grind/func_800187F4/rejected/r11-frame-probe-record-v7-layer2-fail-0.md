# func_800187F4 — v7 landing, seventh layer-2 FAIL (records only), 2026-09-29

Body: the same bytes (fast3 c7 0/662; sandbox 0 on 644/644; the reviewer's own verify-oracle --rebuild
--allow-dirty gave the oracle SHA1). The reviewer passed the bytes, the 85 islands character for character
against inline_o.h, the four Q29 words, the 85 hashes, the gate regions, all seven Ruling 11 locals (prongs
A–H, Q28 (a)-(e), Q30/Q32, mechanism figures, cited GCC lines, abl_d.log), the lz[6] frame text, every Match
and auth figure, and the v5/v6 fixes. Its 7 new FAKE-free probes missed (best 4).

FAIL grounds:
1. proof §5: "The frame-probe bodies (lzc5_*, fr_*: lz[2]-lz[8] or scalars) lack lz[6] and also count: none
   reaches 0" was false: lzc5_5 / rv6_lz5 (lz[5], the Q32 twin) and rv6_lz6 score 0.
2. probes.md closing line counted "four zeros"; the table has eight (three controls, three dead-store closers,
   two frame probes).
3. The v7 ledger was uncommitted while the Match message cited proof v7.

Fixed in v8: §5 rewritten as a record of the frame probes; probes.md closing line lists the eight zero rows
(checked mechanically); rv7_* banked (measured by the author, equal to the reviewer's); proof §3A/§7 rows;
Match message v8 (seven reviewers, 46 more probes); the ledger is committed before auth:/Match:.
