# Hypothesis ledger — func_8003D52C

## s1 (recon, 2026-09-02)
- H1 CONFIRMED: address-form va_start (`&(last) + 1`) reproduces the `sw $s5,0x53C($sp)` home-slot
  store that the retired regfix rule used to insert. Probe: v3 body with builtin macro → d1; with
  address macro → d0.
- H2 CONFIRMED: one pointer local for both loops puts the segment pointer in $s1 and restores the
  whole callee-save assignment. Probe: v1 (separate locals) d38 → v2 (merged) d0.
- H3 KILLED (instance): separate `seg_ptr`/`p` locals, measured d38 on this chassis, no FAKE constructs.
- Status: candidate-ready, v3 in src/code6cac_c2.c, sandbox 0.
