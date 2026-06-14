# func_8007F1A8 (display.c) — GTE outer-product wrapper, needs canonical-asm authorization

## TL;DR
Pure GTE/cop2 wrapper, NO C form. `canonical` gate = **ASM-PARTIAL, 16/22 insns
are cop2 ops** (cfc2, ctc2, lwc2, swc2, c2). Already byte-matches with its
`__asm__(".word ...")` body (prior match commit 59b98c97). The ONLY remaining
work is formal **canonical-asm authorization** (add to `inline_asm_canonical.txt`)
— a worker cannot make that call; flagged for orchestrator/user.

## Why it has no C form
Body = save 3 GTE control regs (cfc2 $t5/$t6/$t7), load a 3-vector matrix from
*a0 into the GTE control regs (lw -> ctc2 $0/$2/$4), load a 3-vector from *a1
into GTE data regs (lwc2 $9/$10/$11), run GTE op `0x4B70000C` (outer-product,
op 0 variant), store the 3 MAC results (swc2 $25/$26/$27) to *a2, restore the
3 control regs. Every compute insn is a cop2 op with hardcoded GTE register
numbers ($0/$2/$4 control, $9/$10/$11 data, $25/$26/$27 result) the compiler
cannot reach from C. The 3 `lw` and the `jr` are the only non-cop2 insns and
are mechanical packaging.

## This is an established auto-authorize pattern
Direct sibling of the whole 8007Fxxx display.c GTE-wrapper cluster already in
`inline_asm_canonical.txt` (func_8007F0BC, func_8007F0E4, func_8007F21C,
func_8007F2AC, func_8007F00C, func_8007F034, func_8007F098, func_8007F200,
gte_GetScreenXY, func_8007E8AC ...). All auto-authorized 2026-06-07/11 as
"pure cop2 ops, no C form". func_8007F1A8 and its twin func_8007F150 are the
two outer-product (op 0) variants of the same family that were matched but never
added to the registry.

## Ready-to-paste registry line (for the authorizer)
```
func_8007F1A8  # GTE outer-product wrapper (display.c, op-0 variant): cfc2 $t5/$t6/$t7 (save control) -> lw $t0/$t1/$t2 from *a0 -> ctc2 to $0/$2/$4 (matrix) -> lwc2 $9/$10/$11 from *a1 (vector) -> nop -> GTE op 0x4B70000C (outer product, op 0) -> swc2 $25/$26/$27 (MAC1-3) to *a2 -> ctc2 $t5/$t6/$t7 (restore control) -> jr ra. Pure cop2 ops with hardcoded GTE register choices, no C form. Sibling of func_8007F150 (same op, different... ) and the 8007Fxxx cluster. Retired as canonical inline asm. Auto-authorize per gte-wrapper-misroute-park.
```
(NB: the twin func_8007F150 likely needs the same line — check the board.)

## Status
Source is at HEAD and already byte-matches (oracle green). Card BLOCKED pending
canonical-asm authorization. Per CLAUDE.md this is an orchestrator AUTO-HANDLE
(GTE leaf wrapper), not a user escalation — but it is outside a worker's authority.
