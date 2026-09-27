# Hypothesis ledger — func_80027AD8 (was calc_teasi_loc_fw)

## [s2] manual lane slotK 2026-09-26

- CONFIRMED (s1 frontier 1): the full-body draft collapses 572 -> 97 (candidate.c).
- CONFIRMED (s1 frontier 2): `u8 D_8008EB74[3][2][2]` indexed [cat][sign][same] matches the lookup.
- CONFIRMED (s1 frontier 3): diff and sign share s0 by allocation (separate variables suffice).
- KILLED: opp ternary respellings `(s16)(...)`, `!= NULL`, `== NULL ? 0xB : 0x19`, `u16` lvalue (all 97,
  two stores remain); staging the ternary through `code` (o5, 99: one store, opp refs 6, opp still s5).
- KILLED: `same` respellings: `same = 0; if (==) same = 1;`, `same = 1; if (!=) same = 0;`, if/else,
  `(a ^ b) == 0` (all 99: jump.c converts to scc, single set, still birthing); chained/ordered stores
  q1-q5 (99-100); u8 same (106, andi); split-init `same = a ^ b; same = same == 0;` and kin
  (t1-t4, 137-162: the extra refs push same's priority above limb/player).
- KILLED: code dispatch as switch(code) (110, balanced case tree); `else if (code != 2) return 0; else`
  (97, same as ours).
- KILLED: copy-direction probes c1 (local typed copy for the fields, param for func_800278C0: the param
  keeps its REG_EQUIV and is never allocated) 104; c2 (param for the fields, local copy for
  func_800278C0: param pri 526, never allocated) 111; m5 (param only for func_800278C0). None
  reproduces the target's allocation.
- OPEN (frontier): what makes the R6a s5 holder conflict with opp. By cse.c make_regs_eqv the holder
  cannot be live from the entry (else it becomes canonical and the sltiu targets it directly, unlike the
  target's `sltiu v1; addu a0,v1; addu s5,v1`), and a single-set fresh `same` is birthing-boosted below
  the stores. Needed: a fresh R6a variable with >= 2 live sets inside the CSE block and priority between
  opp and sign, OR a store operand that depends on same's def.
- OPEN: the fp copy of rec (a 10th long-lived pseudo). No ordinary construct found; m2 shows the
  param-reuse + copy family reaches the shape but is banned (Ruling 11 (A), (C)(3)).
- OPEN: jump.c:1827 swap on `else if (code == 2) {..} else return 0;`: need a spelling where the else
  label has two uses or the then-range does not end in a jump to the label after the else.
- OPEN: permuter campaign from candidate.c (not yet run).
- KILLED: code dispatch guards `if (code != 0 && code != 1 && code != 2) return 0;` (h1, 105) and
  `if (code > 2) return 0;` (h2, 102) ahead of the chain.
- PARTIAL (banned family, measurement only): m10 param-reuse + local copy = 55; see evidence.md s2.
  Its open contradiction (allocation wants an entry-live or non-birthing s5 holder, the R6a byte shape
  wants a block-local one) is the sharpest frontier. If the next worker proves the m10 family is the
  ONLY reachable form, it needs a borderline.md policy-question (Ruling 11 (A) bars parameters,
  (C)(3) bars bare copies) - not written yet because m10 does not reach 0.
