# Hypothesis ledger — func_800485EC

## [s1] 2026-08-27 (recon)

1. **H1 — K&R narrow-param linkage.** Statement: the function takes s16/u16
   narrow args under default-promotion (K&R def + unprototyped extern), not
   an ANSI s32 prototype. Mechanism: callee `lhu` at +0x38/+0x3C vs caller
   `sll/sra`+`sw`. Probe: K&R definition + `extern void func_800485EC();`,
   re-measure caller. Result: prologue/arg reads byte-exact; caller
   sandbox 0 preserved. **CONFIRMED.**
2. **H2 — clut-arm order needs a named bnum.** Statement: target reads the
   clut bnum immediately after the p=tim copy; embedding the load in
   `tim = p + (*p >> 2)` lets sched1 reorder. Probe: named `bnum = *p;`
   before the cx/cy stores. Result: 11 → 7; clut arm byte-exact.
   **CONFIRMED.**
3. **H3 — tail fold is cse1 find_best_addr, defeated only by killing tim's
   quantity.** Statement: any fresh `p = tim + 2` in the join folds its
   reads onto tim (dump-proven); the natural escapes are (a) walking tim
   itself (39 — walker must die pre-call to get $v1), (b) live tim set
   routed to pixdata (22 — pixdata must come from p), (c) def-in-arms
   (3 — cross-jump can't reproduce target's join-internal addiu),
   (d) live self-increment + fresh copy `tim += 2; p = tim;` (0).
   **CONFIRMED — (d) closes the function.**
