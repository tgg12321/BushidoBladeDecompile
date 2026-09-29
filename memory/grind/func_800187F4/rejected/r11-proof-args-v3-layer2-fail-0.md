# rejected: r11-proof-args-v3-layer2-fail-0 (2026-09-28, third fresh layer-2 FAIL)

Body: v3 (c7 = the v1 code; ledger a8dbc2f57), unchanged and not at fault: sandbox 0 (644/644), oracle-green
spliced; 85 region hashes, rows (canonical row cites Q30), islands, lz[6] carve-out + census, Q28 (a)-(d),
(A)(B)(C)(E)(F) all PASSED. The reviewer found NO FAKE-free one-variable-per-value spelling reaching 0
(~2,200 probes: 814 loop forms incl. goto/while/do-while over 11 bases, 96 statement orders, 461 early-init
variants, 799 split-partition combinations, best 2) — v3_review/*_res.txt.

FAILED Ruling 11 (D)(3) on the ARGUMENTS:
- idx: the stated property (counter priority below the count's) appears in a FAKE-free per-value spelling
  with the init hoisted (`idx_add = 0;` before `if (node[6] >= -0xFF)`; pri 9552 < 9767, $t2) that misses
  for another reason (guard no longer folded: slt/beqz vs blez + delay-slot init; one phantom slot lost):
  27 lines, 3 with lz[8] (v3_review/bodies/rv3I_*). The real property was not stated.
- work: the head argument assumed temp shared; with the Q28 copy its own local, sq1 stays the head yet the
  spelling misses (rv3x_0011000 27, rv3x_0041001 13).
- nforce / idx: "loop nesting is fixed for FAKE-free spellings" is false (goto-form loops drop the loop
  notes; rv3L_c7_fgf register-only diff 56); idx's priority moves with its init placement.
- Q30 literal reading: the v1/v2 closers rewritten with lz[5] (byte-identical to lz[6]) still reach 0 and,
  read literally, "lack a construct the reuse body carries", so would count (bodies/rv3_*_lz5).
- Wording: Match message "lz[6] carried unchanged by every compared spelling" (frame probes lack it);
  proof §5 "same annotation" (it is the same declaration).
