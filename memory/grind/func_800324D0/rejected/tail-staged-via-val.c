/* REJECTED (s5 ledger entry, 2026-08-20): staging the loop-tail read through
 * u8 `val` instead of u32 `cmd` (`val = *ptr; c = val;`) measures 27, 68/68 —
 * the entire 12-point gain of the cmd staging vanishes. The SImode (u32)
 * carrier is load-bearing: only the u32 borrow splits cmd's webs and re-seeds
 * the find_reg census; a QImode borrow leaves the original 27 rotation.
 * Placement variant `cmd = *ptr; ptr++; c = cmd;` is also dead: 28, build 69
 * (shape breaks). The exact working spelling is
 * `cmd = *ptr; c = cmd; ptr++;` — see candidate.c. */
void func_800324D0_rejected_tail_staged_via_val(u8 *pad);
