# func_800187F4 — v5 landing, fifth layer-2 FAIL (text only), 2026-09-28

Body: identical bytes to candidate.c (c7); fast3 control rv5_base = 0/662; sandbox 0 (644/644); oracle green.
The reviewer passed the code, the staged diff, the 85 hashes, both rows, the detectors, and all seven
Ruling 11 locals under Q28/Q30/Q31/Q32 (14 new FAKE-free probes, all missing, best 2).

FAIL grounds (all text):
1. The lz[6] OVERSIZED-LOCALS frame derivation was arithmetically false: "Of the 0x40, 8 are the spill slot
   and 32 ... are the four 8-byte phantom slots ...; the 16 bytes left are this object's." 8 + 32 + 16 = 56,
   not 0x40. The object occupies sp+0x10-0x27 = 24 bytes (lz[0]/lz[1] written by gte_stlzc, then a 16-byte
   unwritten tail). The same error was in the Match message and proof §7.
2. proof §4 (D)(1) cited the v2 (c6) dumps table instead of the landing-chassis dumps; §3 nbits cited the
   v2 chassis instead of dump_v1pv_nbits / dump_v1pv_nbits2 in r11/dumps_table_landing.txt.
3. proof §3A said "No counting spelling reaches the target." (an unqualified universal); Q31 requires
   "banked".
4. Bank the 14 new probes (rv5_*) and update the reviewer counts (18 + 14 = 32).

Fixed in v6: comment in candidate.c / r11/variants_v3/c7.c (bytes unchanged, re-measured 0, oracle green),
proof v6 (§3, §3A, §4, §7, v6 note), r11/reviewer_probes/rv5_* + probes.md (measured by the author with
fast3, scores equal to the reviewer's), Match message v6.
