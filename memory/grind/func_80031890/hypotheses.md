# Hypothesis ledger — func_80031890

## s1 (2026-09-02, recon)

- **H1 — CONFIRMED.** The pure-C head/tail of the retired-chassis body, with
  the three per-instruction pinned islands replaced by the four SDK-macro
  islands shipped in func_8002E838 (gte_SetRotMatrix / gte_ldv0 / MVMVA .word /
  gte_stlvnl, "r"-bound addresses, $12-$15 clobbers), reaches sandbox 0.
  Measured: sandbox --disable all = 0 (163/163), verify-oracle build_sha1 ==
  oracle. No FAKE constructs present. Mechanism: the islands are the literal
  PsyQ libgte macro bodies (no C form; GCC 2.7.2 MIPS backend has no cop2
  mnemonics), and every non-island instruction is ordinary GCC 2.7.2 output
  from plain C.
- **H2 — CONFIRMED (folded into H1).** `vec = (s32 *)(ent + 0x44)` as a plain
  local bound "r" in both the ldv0 and stlvnl islands seats in $v0 with no
  coercion, reproducing the target's single `addiu $v0,$s3,0x44` shared by
  both `addu $t4,$v0,$zero` preambles.
- **H3 — CONFIRMED (folded into H1).** `adj = sum_sq / 64` / `adj = -sum_sq /
  64` in the two rng arms + av read into a local before the call reproduces
  the head's cross-jumped `sra 6` join and the pre-call `lh $s1,0x5E`.

Frontier: empty — candidate-ready. Remaining work is the driver's grant
routing (COMPLETED-INLINE-ASM-CANONICAL bucket; inline_asm_canonical.txt line
via the owner-cluster door or a scope grant, per the func_8002E838 /
func_8002EA24 / func_8002FF20 precedents).
