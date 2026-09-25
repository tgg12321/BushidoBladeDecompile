# func_8007636C hypotheses

- KILLED: m2c's split scalar stack locals reflect the original source. They
  erase descriptor writes and score 248.
- CONFIRMED: offsets 0x18..0x43 are one live descriptor object; nearby matched
  callers use the same layout and the aggregate restores the target operations.
- OPEN: rewrite the three rendering loops with shared source locals rather than
  m2c SSA temporaries to remove the extra spill and recover the 0x70 frame.
- OPEN: use the matched rendering functions around `func_80069F80` as source
  templates for descriptor setup and packet allocation order.
