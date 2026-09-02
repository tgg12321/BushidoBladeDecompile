# Hypothesis ledger — func_800300B4

## s1 (2026-09-02, recon)

- H1 CONFIRMED — the residual of the plain three-island form (13) is a single local-alloc seat
  swap arg0/&mac (s2<->s3), pass = local-alloc.c qty_compare_1 priority (function is one basic
  block). Measured: sandbox 13, BB2_QTY_DEBUG trace.
- H2 KILLED (instance) — a pointer copy `pv = mac` after the island raises &mac's ref count.
  Measured 13, refs unchanged at 4: cse.c canon_reg folds the copy to the class's first pseudo.
  Same mechanism covers any in-block pseudo copy of &mac (second asm operand, alias local).
- H3 CONFIRMED — `do { gte_stlvnl asm } while (0);` doubles the &mac def+asm refs via flow.c
  loop_depth weighting (4 -> 6, priority 1290 -> 1935 > arg0's 1627). Measured sandbox 0 and
  verify-oracle ok. Lever ladder for the seat: natural placement has no free variable (births and
  deaths pinned by target order; every natural spelling gives &mac 4 refs and arg0 7) — the
  do-while(0) family is the single-level, owner-sanctioned device for exactly this effect
  (.claude/rules/do-while-zero-exception.md:29, prerequisite 2: single-level wraps need no
  exhaustion gate).

Frontier: none — candidate at 0. If the Judge rejects the wrap, the only remaining axis is a
natural spelling that lowers arg0 to <=5 refs, which the byte-pinned six s3 uses do not allow;
that would be a ruling-request, not a re-grind.

## s1b (2026-09-02, recon)

- H4 KILLED (class) - island 2 with the VXY0 pack computed in ordinary C (layer-1 prescribed form). Measured
  sandbox 19 in two spellings (lv[] index form; explicit u16 reads), with the E3 do-while(0) wrap present.
  Predicate: the target pack reads through $t4 (asm-internal `move $12,%0` copy, no C handle) into $t5/$t6;
  local-alloc.c:2249 find_free_reg allocates hard regs in numeric order (no REG_ALLOC_ORDER in
  config/mips/mips.h), so with $v0/$v1 free no C temp reaches $13/$14. Both spellings landed in v0/v1 off $s2.
- H5 CONFIRMED (documentary) - the island is on main character-identical in func_800203B4 (owner grant
  2026-09-01, inline_asm_canonical.txt:367), func_8002E838 (Judge PASS decisions.md:20262) and func_80031890
  (Judge PASS decisions.md:20268), all with the pack inside the asm block.

Frontier: ruling-request only. If the ruling admits the gte_ldv0 macro body under condition 3 (as the
func_800203B4 grant text and the two sibling PASSes did), candidate.c is a 0-form ready for re-submission with
the self-vet's precedent citations replaced by func_800203B4 / func_8002E838. If it does not, the function has
no pure-C or sanctioned-asm form for those 7 bytes and the correct disposition is the cluster-door foreclosure.
