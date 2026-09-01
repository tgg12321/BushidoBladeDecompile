# Hypothesis ledger — func_800203B4

## s1 (2026-09-01, recon)

- **H1 — CONFIRMED (measured, closes the function):** "The func_8002FDB0-authorized
  single-block island spelling (hardcoded $12-$15 inside the asm template, operand as `"r"`,
  explicit clobbers) transplanted onto the pre-migration pure-C head reproduces all 65
  instructions." Mechanism: $t4..$t7 ARE $12..$15, so the SDK-macro `move $12,%0` emits the
  target's `addu $t4,$rN,$zero`; GCC materializes the operand addresses (`addiu $v0,$sp,0x10`
  / `addiu $v0,$sp,0x30` / `addiu $s0,$s0,0x354` via `arg0 += 0x354`) exactly as the target.
  Probe: applied body, `sandbox func_800203B4 --disable all`. Result: **0, 65==65,
  rules_dropped 0, first measurement.** No permuter, no dumps needed.
- H2 (never needed): dead locals m0/m1/m2 from the historical body required for frame size —
  KILLED implicitly: frame is 0x50 without them (score 0 includes prologue/epilogue).

Frontier: empty — bytes proven; residual is the driver-side canonical-asm grant per the
2026-08-17 cluster ruling (see evidence.md fact 7).

## s1 retry (2026-09-01, recon)

- **H1 re-CONFIRMED this session:** candidate.c applied to src/code6cac.c scores
  sandbox --disable all == 0 (65/65, rules_dropped 0). Artifact:
  tmp/grind/func_800203B4/s1/code6cac_sandbox0_retry.o.
- **H3 — KILLED (measured):** "scan_hand_coded gives STRONG tier, opening the
  [[judge-sole-gate]] rule-3 grant path without cluster membership." Result: tier LOW,
  score 1 (s4 only). The scanner path is closed; cluster inheritance is the only route.
- **H4 — OPEN (the ruling question):** "The 2026-08-17 FDB0 cluster ruling covers the
  $v0/$s0-source spelling of the materialize-then-copy cop2 idiom." Cannot be resolved
  agent-side: the enumeration (.claude/rules/cop2-addressing-preamble-cluster.md) is
  count-anchored at 28 and omits this function (evidence.md fact 9). Emitted as
  ruling-request 2026-09-01.

Frontier: the H4 ruling. No codegen work remains — do NOT spend sessions re-measuring;
the C body is final in candidate.c. Next session: check whether the ruling landed
(docs/grind/decisions.md / an updated cluster-membership file / a grant line in
inline_asm_canonical.txt); if YES follow evidence.md fact 11's integration steps via
candidate-ready; if NO, foreclosure per fact 11.
