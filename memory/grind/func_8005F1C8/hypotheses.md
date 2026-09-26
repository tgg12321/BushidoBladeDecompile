# Hypothesis ledger — func_8005F1C8

## s2 (manual lane, slotJ, 2026-09-26)
- KILLED: nibble-count hoisting controlled by the compare form (cmp_rev, <<2, u8 cast):
  all 109-230; the lever was wins's TYPE (s16/u16 = 0).
- KILLED: count as s32/u8/u16 ternary; s16 if/else is the only 0 form measured.
- KILLED: separate counters per phase (38-73); one shared `i` for P1..P4 (spilled, 146+).
- KILLED: `j*(k*16)`, `(k*16)*j`, `j*(k*16) + j*431` for x0 (55-64); both-`<<4` = 19.
- OPEN (not needed for 0): named intermediate `inset = k*16` inside the tile loop also
  scores 0 but is a named-intermediate family construct (FAKE + prerequisites); the
  plain `(k << 4)` spelling needs no family.
