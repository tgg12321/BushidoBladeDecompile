# gte vein - examined and dropped

- **Command-level contradiction sweep (all 61)**: every `gte_<op>` name's op is present in its body (raw-word decode,
  scan.txt). No RESET on that ground.
- **nclip family RESET (38 rows)**: dropped - NCLIP (0x4B400006) is present in every one and is the cull gate of each
  face loop; the names under-describe polygon emitters but do not contradict them.
- **0x80063E10 / 0x8004DDB4 / 0x8004C404 / 0x80019310 RESET**: dropped - GTE transform work is the substance of each
  body and the named op is the dominant (or only) one; not an incidental macro.
- **0x80054440 RESET of "wrapper"**: dropped - a batch NCT loop, not thin, but NCT is the core op; "wrapper" is
  imprecise, not contradicted.
- **0x80052B7C / 0x80052C4C precise RENAME**: dropped - `gte_mvmva` is accurate; a full restatement (RT from a0, TR from a
  separate s16 triple; or RT rows 1-2 only with row 3 inherited) gives no clean short name.
- **0x8004A808 UPGRADE/RENAME to a batch name**: dropped - accurate as is and RTPT is not emulated.
- **0x8004A348 as Sony RotMatrixZYX (library identity)**: dropped - executed both from the EXE: same product but
  19688/20000 outputs differ (max 13 LSB); named as restatement math_RotMatrixZYX instead.
- **0x80052788 as Sony LoadAverageShort12 (identity)**: dropped - different signature (t vs p0/p1) and not a libscan match;
  equivalence used only as emulation evidence.
- **0x800525D8 as Sony VectorNormal-family**: dropped - LIBGTE MSC02 is a libscan no-match and no Sony copy is in the EXE to
  emulate; restatement name used instead.
- **libscan "verbatim" LIBSND VM_VIB @0x8004A930**: spurious - it is the 2-word `jr ra; nop` tail of func_8004A808; no identity.
- **UPGRADE of the generic `gte_mvmva`/`gte_nclip`/`gte_rtpt` names**: dropped - a name shared by 13/38 functions that states
  one op is not a computation restatement of any of them.
