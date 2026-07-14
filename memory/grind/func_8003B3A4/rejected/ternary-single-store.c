/* REJECTED s2 (2026-07-13): sandbox distance 6 (55 vs 56 insns).
   Identical failure mode to the direct conditional write: expand_assignment
   keeps the SYMBOL_REF MEM symbolic -> $at sb macro after the diamond. */
    D_8010277D = (a1 != 0) ? 0xE : 0x1D;
