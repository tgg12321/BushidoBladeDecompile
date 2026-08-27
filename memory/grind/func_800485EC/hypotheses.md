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

## [s2] 2026-08-27 (recon)

4. **H4 — full-tail duplication into arms reaches target's cross-jump-shaped
   join.** Statement: duplicating the whole pixel parse into both arms lets
   jump2 re-merge and place the label at `lh a0`. Probe: measured. Result: 16
   (72 insns) — copies not re-merged AND per-copy reads still fold onto tim.
   **KILLED** (two independent grounds; rejected/tail-dup-full-pixel-parse.c).
5. **H5 — the read-fold is unconditional on any valid tim+K equivalence.**
   Statement: cse.c's find_best_addr applies fold_rtx uncosted (2663) and
   prefers higher-rtx_cost equal-ADDRESS_COST addresses (2719-2721), so both
   `(plus p 2)` and bare `(reg p)` rewrite onto tim unless `(plus tim 8)`
   fails exp_equiv_p's reg_tick check. Probe: source read of cse.c + s2 dumps.
   Result: confirmed at RTL level (insn 123 def survives, reads stay
   p-relative only when tim is reassigned in between). **CONFIRMED.**
6. **H6 — exact F6 cancellation pair `tim++; tim--;` after `p = tim + 2;`
   closes the function byte-free.** Mechanism: reg_tick bump invalidates the
   equivalence at cse1/cse2; flow.c deletes the dead pair pre-RA. Probe:
   sandbox --disable all. Result: **0 at 68/68**; caller func_80048530 also 0.
   **CONFIRMED — closes the function** (pending layer-1/Judge on the F6
   family claim; annotation + exhaustion prerequisites in place).
