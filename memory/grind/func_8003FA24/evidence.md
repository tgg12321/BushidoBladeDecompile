# func_8003FA24 evidence

## Session 1 — 2026-09-22

- Queue discipline: selected the required top active item; no cherry-picking.
- Canonical gate: C, LOW hand-coded tier, 263 target instructions.
- Baseline: whole-body INCLUDE_ASM, honest sandbox distance 263 with no C body.
- Reconstructed behavior from `asm/funcs/func_8003FA24.s`: copy packed triples into 8-byte point records; scan a packed geometry-command stream to calculate output size; emit scratchpad packet records; initialize a scene/object descriptor; return an aligned output cursor.
- Corrected two pre-existing type models while deriving the candidate: the scene record's leading object ID is 16-bit, and the copied scene quad is a mixed-field 16-byte record rather than four semantically uniform words. These corrections remain candidate-local until completion.
- Best honest candidate: `candidate.c`, sandbox `129/263`, 262 build instructions vs 263 target.
- Structurally matched region: entry through point-copy and the first stream scan is opcode-identical apart from register allocation and the eight-byte frame displacement.
- Remaining source-level gap: packet-emission loop hoists constants and uses a different signed-count/control-flow shape; target frame is 0x50 while the honest typed candidate is 0x48.
- Full oracle and completion audits were not run because the function is not score 0.
- Main was restored to INCLUDE_ASM as required for all incomplete functions.

