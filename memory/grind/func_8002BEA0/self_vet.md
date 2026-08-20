# SELF-VET — func_8002BEA0

CONSTRUCTS: canonical GTE LZCS/LZCR __asm__ island with "$12","$13","$14","$15" clobber list; pointer locals t2_base/t3_base into D_80101EC8; block-scoped named locals temp_v0 (=0x3E8) and temp_v1_3 (=-var_a0); LZC-result locals v0_m/v1_m/idx/hi; plain-C arithmetic (magnitude-squared head, table lookup, *0x50/100 and /16 arms, four /1000 stores, return var_t0-0x44C)

## T1 semantic purpose
- GTE island: performs the LZCS/LZCR leading-zero count that feeds the sqrt
  table index — real computation with NO C form (cop2 mtc2/swc2). Removing it
  changes output.
- Clobber list $12-$15: declares the register footprint of the hand-asm
  island. $12 ($t4) is literally written by the template; $13-$15 are the
  original island's footprint proven by target bytes (reload's mfhi scratch
  skips them and lands at $24 — reload1.c bad_spill_regs; they have zero
  pseudo uses in target, so an asm-level mention is the only mechanism the
  frozen compiler has to skip them). This is reconstruction of the original
  source's asm register list, not a steer; omitting them makes the TU LESS
  faithful.
- t2_base/t3_base: real base pointers for the four stores; target's own bytes
  address through exactly these bases (`sw $a2,0x134($t2)` / `sw $a0,0x134($t3)`,
  `addiu $t3,$t2,0x44C`). Program logic, both read multiple times.
- temp_v0 = 0x3E8: the /1000 divisor, read four times. temp_v1_3 = -var_a0:
  the negated delta, read twice. Both alive and consumed.
- v0_m/v1_m/idx/hi: the LZC post-processing arithmetic, all consumed.
- No construct in the diff is dead, unused, or address-of-only.

## T2 human-programmer
Every construct is what a PsyQ-era programmer writes: a GTE macro block for
the leading-zero count (SDK idiom), base pointers into a struct, a named
divisor constant, plain divisions. The COMPLETED-C sibling func_8002BC68
(same file, similarity 0.965) ships the byte-for-byte same shapes and was
accepted. The else arm is plain `/ 16` (this session REPLACED a hand-expanded
shift/if form with the natural division — moved TOWARD human-natural code).

## T3 GCC-internals justification
The C body's justification is program logic throughout; no C construct exists
"to make a pass behave". The one internals-referencing element is the
$13-$15 clobber widening, whose mechanism (reload1.c bad_spill_regs) is cited
as EVIDENCE that the original source named those registers — the sanctioned
evidentiary use under the 2026-07-28 owner/Judge ruling, identical in
spelling and comment to the sibling's accepted instance. It is not a pure-C
coercion claim and does not extend any pure-C family.

## T4 permuter/search provenance
No permuter or auto-search output is in the diff. Both session levers came
from documented rulings (the unspent clobber grant) and from reading the
sibling's accepted source + BB2_DBR_DEBUG dump forensics. The prior WIP
candidate (cheat-reviewer PASS on record) supplied the unchanged remainder.

## T5 family check
- The island matches the owner-authorized LZC canonical-island family
  (func_8001A67C / func_800274BC / func_8002BC68), with the island wording
  for THIS function pre-approved 2026-08-07 (docs/grind/decisions.md:3906)
  conditional on "otherwise matches" — now met (sandbox 0).
- The clobber list matches the 2026-07-28 grant naming this function.
- No forbidden family present: no pins, no barriers, no hardcoded-$N asm
  outside the granted island (in-island GPR is limited to the two
  `addu $t4` cop2 addressing preamble copies), no volatile coercion, no dead
  stores, no invented locals, no alias renames.

## T6 naming-announces-intent
Names are m2c-style value names (temp_a3, var_t0, t2_base…). No pad/dummy/
unused/spill/slack names; every named value is read.

SANCTIONED-FAMILY-CLAIMS:
  FAMILY: canonical GTE cop2 inline-asm island (LZCS/LZCR)
  SCOPE: "Inline `__asm__` in a C function body using opcodes that ONLY EXIST in asm form: GTE coprocessor ops (`ctc2`/`mtc2`/`mfc2`/`lwc2`/`swc2`/`.word 0x4XXXXXXX`), BIOS vector jumps (`j 0xA0`/`B0`/`C0`), cache/DMA register pokes (`0x1F8003xx`). Authentic — original devs wrote these."
  PRECEDENT: inline_asm_canonical.txt:267

  FAMILY: reload-spill clobber-list reconstruction ($13-$15 on the authorized island)
  SCOPE: "A clobber list may **only** be widened to registers the bytes force. Adding clobbers to steer allocation where the target's reload temp does *not* demonstrate the skip is a register pin in clobber spelling — the ordinary cheat."
  PRECEDENT: docs/grind/decisions.md:1845

ANNOTATION-CONFORMANCE: n/a — no FAKE construct in the diff. Neither claimed
family's rule mandates a /* FAKE */ annotation: the canonical island is real
no-C-form computation, and the reload-spill rule mandates the bytes-forced
evidence COMMENT, which is present on the island verbatim (mirroring the
sibling's mandated comment at src/code6cac_b.c:534-540; target's mfhi $t8 at
asm/funcs/func_8002BEA0.s:63 is the forcing byte).
