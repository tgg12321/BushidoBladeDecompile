# Hypothesis ledger — func_80087CAC

## s2 (structural, 2026-07-14)

- H1 (judge probe a): direct `D_80102806 = a0; ... return D_80102806;` byte-matches because GCC CSEs the symbol address into a shared register. **KILLED** — sandbox 12; $at-macro store + independent lui/lh reload + store hoisted above base lw. GCC 2.7.2 has no cross-access HIGH unification for symbol store/load pairs (confirms s1's func_800892F8 target evidence).
- H2: a half-form (one access direct, one via pointer) produces an intermediate shape identifying which access drives the $a3 allocation. **KILLED** — both half-forms compile byte-identical to the full-direct form (single-use pointer CSE-folded away); there is no intermediate shape. Corollary CONFIRMED: both accesses through the pointer are necessary AND sufficient (committed form = 0).
- H3: `*(&D_80102806)` spelling (no named pointer local) keeps an address pseudo. **KILLED** — folds at tree level, sandbox 12, same bytes.
- Disposition (s1 frontier item 2, now evidence-complete): the pointer IS the minimal C spelling; correct path is a ruling under pointer-alias-fake-exception (owner ruling family 2026-07-01), with the note that a read+write pointer to a status global is also plausibly the ORIGINAL idiom (cf. func_8008BEA4's unflagged `flag` pointer) — arguably not FAKE at all. Ruling requested in s2 outcome.
