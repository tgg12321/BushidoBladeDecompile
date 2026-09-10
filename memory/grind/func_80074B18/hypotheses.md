# Hypothesis ledger — func_80074B18

## s1 (2026-09-10, recon)
- H1 CONFIRMED — OT index as a scalar assigned before the call (`ot = 0xB; if (arg1) ot = 0x15;`)
  instead of a ternary in the argument. 46 -> 32. Mechanism: jump/cse arm-folding of the add.
- H2 CONFIRMED — `s16 n` for the column count. 32 -> 1. Mechanism: with s32 n combine folds the
  duplicated inner-loop entry test via reg_nonzero_bits (combine.c:724-733, :8989-8995) after
  loop.c refuses the hoist ("life 1, savings 1 not desirable", loop.c:1631). s16 n yields the
  target's preheader copy + real slt + spill.
- H3 KILLED (instance) — struct-typed cursors with `p++` inside the AddPrim argument and/or `t++`
  before the call (v3 47, v4 42, v5 38 vs v2 32). Plain `AddPrim(...); p += 0x10; t += 0xC;` is
  the matching order; sched2 hoists the increments itself.
- H4 KILLED (instance) — `u8 n` (3): extra andi at the preheader copy.
- H5 CONFIRMED — the last instruction (nop at 0x80074CF0) is the maspsx `.L`-label load-delay
  blind spot; gated object = 0 diffs, relink SHA1 == oracle (tmp/grind/func_80074B18/s1/).
  Disposition: INTEGRATION HANDOFF (docs/grind/decisions.md 2026-09-10 entry), mirrors func_80027640.

Frontier: none on the C axis — the C is final. Remaining step is the operator-landed
maspsx_label_nop_funcs.txt line (fidelity gate, add-scope-allow denylist).

## [s1] Selecting the AddPrim OT index as a scalar before the call (ot = 0xB; if (arg1) ot = 0x15;) instead of a ternary inside the call argument reproduces the target's li/li/sll/addu shape
- mechanism: jump/cse fold the +0x2C/+0x54 add into both arms of a ternary argument; a scalar assigned in an if keeps the index as a register value, one sll+addu
- probe: sandbox v1 (ternary) vs v2 (scalar)
- result: 46 -> 32, pairdiff_v1/v2.txt
- verdict: CONFIRMED

## [s1] Declaring the column count as s16 n (not s32) reproduces the target's preheader copy (sw v1,40(sp)), real slt a3,zero,v1 spilled to 48(sp), and beqz on the reload
- mechanism: with s32 n, loop.c refuses to hoist the duplicated inner-loop entry test (loop.c:1631, 'life 1, savings 1 not desirable' on the 77-insn outer loop) and combine folds it into the jump via reg_nonzero_bits of the two-set pseudo (combine.c:724-733, GT 0 -> NE 0 at combine.c:8989-8995); s16 n (PROMOTE_MODE SImode pseudo) yields the target shape
- probe: sandbox v2 (s32) vs v6 (s16) vs v7 (u8)
- result: 32 -> 1 (u8: 3, extra andi)
- verdict: CONFIRMED

## [s1] Struct-typed cursors with p++ inside the AddPrim argument and/or t++ written before the call, measured on the HEAD chassis with no FAKE constructs, score 47/42/38 against the plain 'AddPrim(...); p += 0x10; t += 0xC;' order at 32
- mechanism: biv-increment placement relative to the call; sched2 already hoists the four increments above the jal for the plain order
- probe: sandbox v3, v4, v5
- result: 47, 42, 38 vs 32
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 938d5bdf3, -mel -msoft-float, s32 n, no FAKE constructs

## [s1] The single remaining instruction (nop at 0x80074CF0 across the .L merge label before sw $s3,0x14($a3)) is the maspsx .L-label load-delay blind spot and is closed by the per-function maspsx_label_nop_funcs.txt opt-in
- mechanism: ASPSX inserted a hazard nop between lw $a3,0x10($sp) and its base-register store consumer across a label; maspsx is_label() matches $L not .L; the per-function gate (tools/maspsx/maspsx/__init__.py:832-838) re-emits it
- probe: tmp/grind/func_80074B18/s1/build_gated.sh (scratch copy of the list) + pdiff + link_gated.sh
- result: gated object: 0 differing instructions; relink SHA1 62efab4f73f992798c43e8c730aa43baa10bb4fa == oracle
- verdict: CONFIRMED
