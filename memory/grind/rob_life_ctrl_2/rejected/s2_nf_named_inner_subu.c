/* REJECTED s2 — named `s32 nf = 0x1000 - f;` for inner subu of t.
 * Form: { s32 nf = 0x1000 - f; q = ...; t = (v*(0x1000-((s*nf)>>12)))>>12; }
 * Score: 2 (unchanged). objdump bytes IDENTICAL to HEAD (probe2_nf.txt). */
