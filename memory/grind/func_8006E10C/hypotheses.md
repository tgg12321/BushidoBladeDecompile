# Hypothesis ledger — func_8006E10C

## s1 (2026-07-27, recon)
- H1: "The 13-diff residual is caused by sched1 sinking the dependence-free ff0=240 constant set past 4 calls to its first use, shrinking its live range so it steals $s0 from base." — mechanism: sched.c block-local list scheduling, constant set with LOG_LINKS(nil) gets placed at consumer; probe: -da per-pass dumps, trace insn 48 across passes; result: combine dump has insn 48 before call 55, sched dump has it after insn 93 (just before call 95); verdict: CONFIRMED.
- H2: "Fencing the sink with the sanctioned single-level do{}while(0) wrap anchors the li before the call, flipping the whole allocation to target and letting reorg fill the jal delay slot." — mechanism: loop notes around the set prevent sched1 from moving it out; ff0 then conflicts with v0 -> $s1; probe: `do { ff0 = 0xF0; } while (0);` + sandbox; result: score 13 -> 0, li s1,240 in jal delay slot, 103/103; verdict: CONFIRMED.
