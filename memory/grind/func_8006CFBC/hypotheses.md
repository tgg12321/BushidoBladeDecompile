# func_8006CFBC -- hypotheses (ruled in / out)

2026-09-30 laneC (all sandbox --disable all against src/text1b_tu1c.c, 218 target insns):

- RULED OUT: any carrier-free spelling of `s.table = s.header + 0xC` closes the
  function. Every one costs 8 (sites 1-2 lose the `addiu` above the header store,
  site 3 gets `$v1` for `$v0`). Mechanism in r11/proof.md (sched1 birth boost +
  local-alloc tie). Spellings: r11/spellings/*.c.
- RULED OUT: reading the table element twice so the header load stays live past the
  add (vA/vB): the second read is not CSE'd, 16-17.
- RULED OUT: Ruling 4 compound splits (`s.table = hdr; s.table += 0xC;`, t1 8;
  per-value `cells = hdr; cells += 0xC;`, t2 9): CSE folds the split, the dest is
  still one single-set pseudo.
- RULED OUT: Ruling 9 for the carrier -- prong (e) fails at site 1 (no read of `s`
  between two column iterations whose row bits are both clear).
- CONFIRMED: the union word view is needed for the one `sw $zero,0x48($sp)`; plain
  element stores cost one extra store (q33/u_*.c).
