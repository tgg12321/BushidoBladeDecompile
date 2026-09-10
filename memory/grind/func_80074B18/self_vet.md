# SELF-VET — func_80074B18
CONSTRUCTS: none
## T1 semantic purpose: every statement is program logic — column count `s16 n = 5 / 8`, OT index `ot = 0xB / 0x15`, row loop over `D_800A36A0->u8[0x65] + 3`, per-tile SetTile / colour+size copy / SetSemiTrans / position / AddPrim, cursor advance, cursor write-back. Removing any statement changes the drawn output.
## T2 human-programmer: `x = default; if (flag) x = other;` twice, two nested for loops, raw-offset field access in the file's established style, `s16` counters/count (the game's tile fields are u16/s16; a short column count is ordinary). Nothing a reader would ask "why is this here?" about.
## T3 GCC-internals justification: none needed. The evidence.md pass attribution explains WHY the s32 spelling missed (combine nonzero_bits); it is not the reason the s16 spelling exists — `s16 n` is a plain type choice with no FAKE annotation and no lever naming.
## T4 permuter/search provenance: no permuter, no search; seven hand-written spellings measured (evidence.md table).
## T5 family check: no register pins, no asm, no volatile, no dead store, no constant holder, no alias, no do-while(0), no pad, no duplicated statement, no split read. The maspsx gate-list entry is a toolchain fidelity gate (not a C construct) and is NOT staged by this session.
## T6 naming-announces-intent: locals are p, t, i, j, n, ot — descriptive, all read and written.
SANCTIONED-FAMILY-CLAIMS: none
ANNOTATION-CONFORMANCE: n/a — no FAKE construct
