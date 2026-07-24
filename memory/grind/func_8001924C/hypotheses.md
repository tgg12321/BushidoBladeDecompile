# Hypothesis ledger — func_8001924C

Species: 8-byte zero-store phantom frame slot (frame 0x30 target vs 0x28 honest).
Body byte-identical at floor 12; the locals-region SIZE is the ENTIRE gap.
Structural twin func_80049A2C is OWNER-GATED (endgame-lock-disposition).

## Frontier after s1 recon (for next GRIND session)

### H1 (frontier, HIGHEST VALUE) — own cc1 -da greg mechanism check
- **Mechanism:** phantom-frame-slots-gcc272 — GCC 2.7.2 reserves get_frame_size()
  bytes for a LIVE local it register-allocates away (no store emitted) ONLY when a
  pseudo is denied a hard reg and alter_reg reserves a stack slot. The twin's greg
  proof showed ZERO such pseudos (mechanism does not fire; +8 is aggregate-only).
  func_8001924C has NOT yet had its OWN greg dump — this is genuinely un-searched.
- **Next probe:** `cc1 -O2 -G0 ... -da minrepro.c` (pad-in AND pad-out), slice
  func_8001924C from .greg, diff the two Register-dispositions. If any pseudo has
  reg_renumber<0 / no-hard-reg with reg_n_refs>0, the phantom fires on live code and
  can be replicated without pad. If ZERO delta (twin's result), phantom is
  aggregate-only here too -> H3.

### H2 (frontier) — witness-form phantom (real emitted store gated by HImode bitwise)
- **Mechanism:** tslLineG5Init (COMPLETED-C, cheat-free) reaches vars=8 via
  `s16 v1=global; s16 mask=global; if((v1 & ~mask)&1){ REAL STORES; }` — the bitwise
  gates a surviving store so flow's stale reg_n_refs makes alter_reg reserve the slot.
  Naive injection (fb=1 constant) KILLED s1 (folds -> vars=0). Open question: is there
  a spelling using func_8001924C's actual values (the s0+2 flag byte + a second
  non-constant HImode operand) whose emitted instructions ALREADY exist in the 49-insn
  target? The target flag test is `lbu; nop; andi 0x1; beqz` (QImode) — no HImode
  bitwise present, and both arms make calls with no store to gate.
- **Next probe:** enumerate whether ANY target instruction pair can be reframed as a
  gated HImode store without changing bytes. Strong prior: dead (no store in body),
  mirrors twin s2. Measure to convert prior into a KILL.

### H3 (frontier, TERMINAL) — if H1+H2 dead -> endgame-lock species -> owner-gated
- **Mechanism:** identical to owner-gated twin func_80049A2C: aggregate-only +8 slot,
  no phantom firing, scan_hand_coded LOW 0/8 (already confirmed s1), no SOTN precedent
  for a fully-dead 8-byte pad. Per [[endgame-lock-disposition]] this is
  INCOMPLETE-owner-accepted OR canonical-asm (unavailable). NOT owner-gated yet — H1
  (own greg dump) must be run first to certify the phantom mechanism doesn't fire
  HERE. Do not transfer the twin's proof by analogy; measure this function.
- **Next action (only after H1+H2 KILLED):** file OWNER-ESCALATION in
  docs/grind/decisions.md naming func_8001924C, cite twin func_80049A2C precedent,
  return owner-gated.

## Killed this session (s1)
- Naive HImode-bitwise phantom injection (fb=1 constant) -> vars=0, folds, changes
  lbu->lh. KILLED. rejected/himode-bitwise-phantom-inject-vars0.c
- `s32 pad[2]` -> vars=8 = target frame but FORBIDDEN frame-coercion cheat (policy,
  not measurement). rejected/pad2-unused-local-array-frame-coercion.c

## [s1] The honest floor-12 gap is entirely a +8-byte frame-size delta (an 8-byte phantom locals slot at sp+0x10..0x17 that the target never writes or reads); the loop body is byte-identical.
- mechanism: mips.c compute_frame_size: 0x30 = ALIGN8(vars) + ALIGN8(16 args) + ALIGN8(20 gp_regs=5*4) -> vars in [1..8]. Target's 5 saves sit at 0x18..0x28; the 8 bytes below them at 0x10..0x17 are reserved untouched.
- probe: sandbox --disable all (12, 49==49 insns, all diffs sp-relative offset shifts); minrepro cc1 .frame with pad -> vars=8/frame48, without pad -> vars=0/frame40.
- result: sandbox 12; pad-in vars=8 (== target 0x30), pad-out vars=0 (0x28). pad[2] is the sole slot producer.
- verdict: CONFIRMED

## [s1] Naive HImode-bitwise phantom-slot injection does not fire for this function.
- mechanism: phantom-frame-slots-gcc272: two HImode locals feeding an HImode bitwise expr can reserve an untouched stack temp. Reformulated the flag test as s16 fa=*(s16*)(s0+2); s16 fb=1; if((fa&fb)&1).
- probe: minrepro himode variant, cc1 .frame dump.
- result: vars=0 (no phantom slot): fb=1 constant folds (fa&1)&1 -> fa&1, sign-ext combined away before any temp reserved; also changes lbu->lh (diverges). Matches twin func_80049A2C s2 finding.
- verdict: KILLED

## [s1] The canonical-asm (hand-coded) route is unavailable for func_8001924C.
- mechanism: scan_hand_coded S1..S8 signal framework; STRONG tier needs >=1 of S1/S2/S6/S7/S8.
- probe: python3 tools/scan_hand_coded.py --single func_8001924C.
- result: LOW tier, all S1..S8 negative (0 multu pairs, no empty branches, 49 insns/5 spills/7 regs, load burst 2, no sibling cluster, no BIOS jtbl, all $sN saved, no redundant mask). Same as owner-gated twin (LOW 0/8).
- verdict: CONFIRMED
