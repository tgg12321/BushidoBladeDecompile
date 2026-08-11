# Hypothesis ledger — func_8002FDB0

## Session 1 (recon, 2026-08-11)

### H1 — CONFIRMED
**Statement.** The 22-instruction GTE tail (target insns 68-89) is the verbatim
expansion of PsyQ libgte `inline_c.h` macros — `gte_SetRotMatrix`, `gte_ldlvl`,
`gte_op0`, `gte_stlvnl` — whose SDK bodies hardcode `$12`-`$15` and open with
`move $12, %0`; it is therefore hand-written SDK asm text, not compiler output
from any C.
**Mechanism.** Not a GCC mechanism at all — that is the point. Three redundant
constant-materialize-then-copy pairs, a repeated fixed `$12`-`$15` footprint,
and two unfilled cop2 load-delay `nop`s are all shapes no GCC pass produces.
**Probe.** Rewrote the tail as three canonical GTE inline-asm islands containing
the SDK macro bodies (one `__asm__ volatile` per macro, `"r"` operand, `$12`-`$15`
clobbers), deleted every register pin and every `move %0,%1` aliasing block, and
measured `sandbox --disable all`.
**Result.** 21 -> 3, `build_insns` 90 == `target_insns` 90, and the three islands
reproduce target instructions 68-89 exactly. Also removed the need for the
`__asm__ volatile("" ::: "$5")` scheduling barrier: the first island's `lui/ori`
now schedules into the sixth block's load-delay slot on its own.
**Verdict.** CONFIRMED.

### H2 — CONFIRMED
**Statement.** The 63-instruction head (stride computation plus six
subtract-and-store blocks) is ordinary pure C and needs no coercion.
**Mechanism.** `*(s32 *)((u8 *)0x1F8000XX + stride)` is expanded by aspsx/maspsx
into the target's `lui $at; addu $at,$v0,$at; lw` triple (absolute address with
index register), and `*(s32 *)0x1F8003XX = a - b;` into the `lui $at; sw` pair.
**Probe.** objdump of the cheat-stripped sandbox object at session start,
compared instruction by instruction against `asm/funcs/func_8002FDB0.s`.
**Result.** Head instructions 1-62 already matched byte-exactly with cheat-asm
stripped, before any edit this session.
**Verdict.** CONFIRMED.

### H3 — CONFIRMED
**Statement.** The final 3-instruction residual is a register-allocation
consequence of REUSING `v1`/`v2` across all six subtract blocks; giving the sixth
block its own locals lets it take `$v0`, which `stride` frees at its last use.
**Mechanism.** `v2`'s pseudo, live across five prior blocks, is already tied to
`$a0` by the time the sixth block is allocated, so the sixth block's difference
spills to a fresh pseudo that gets the newly-free `$v0`. A distinct pseudo for
the sixth block's second operand takes `$v0` directly and leaves the difference
in `$v1`, as target has it.
**Probe.** Declared `s32 w1, w2;` and rewrote the sixth block to use them.
**Result.** 3 -> **0**. build_insns 90 == target_insns 90; full objdump
comparison against `asm/funcs/func_8002FDB0.s` shows all 90 instructions
identical.
**Verdict.** CONFIRMED.

## Frontier (for the next session / the operator)

The bytes are proven at honest distance 0 with zero regfix/asmfix rules, zero
register pins, and zero `move %0,%1` aliasing blocks. What remains is a
DISPOSITION question, not a matching question:

1. **Integration handoff — authorize the canonical GTE islands.** The finished
   state is COMPLETED-INLINE-ASM-CANONICAL, so `func_8002FDB0` needs an entry in
   `inline_asm_canonical.txt` (grind sessions may not touch that file). See
   `docs/grind/decisions.md` (2026-08-11 entry) for the exact operator steps.
   Precedent: `inline_asm_canonical.txt:263` (`func_800274BC`, same file,
   owner-authorized 2026-06-10) is the same shape — a canonical GTE island
   carrying the original island's GPR scaffolding.
2. **If the owner instead rules the SDK-macro scaffolding inadmissible**, the
   only remaining question is whether `move $12,%0` + `lw $13,0($12)` can come
   from C at all. It cannot: no C expression makes GCC materialize a constant
   into one register and then copy it to a second with no consumer. That would
   make the function a canonical-asm-or-nothing case, i.e. the same handoff by a
   different route.
3. **Sibling sweep (cheap, high leverage).** `src/code6cac_b.c` and
   `src/code6cac.c` carry many more functions built from the same
   pin + `move %0,%1` + per-instruction-cop2 spelling (e.g. `code6cac_b.c:1452`,
   `:1651`, `:1726`, `code6cac.c:1960`, `:2025`). If this session's island
   reconstruction is accepted, the same rewrite very likely retires that whole
   cluster; the reconstruction recipe is in `candidate.c`'s header comment.

## [s1] The 22-instruction GTE tail (target insns 68-89) is the verbatim expansion of PsyQ libgte inline_c.h macros gte_SetRotMatrix / gte_ldlvl / gte_op0 / gte_stlvnl, whose SDK bodies hardcode $12-$15 and open with `move $12, %0` — hand-written SDK asm text, not compiler output from any C.
- mechanism: Not a GCC mechanism, and that is the point: the target bytes contain three redundant `lui $a1; ori $a1; addu $t4,$a1,$zero` materialize-then-copy sequences (GCC never copies a freshly materialized constant to a second register with no other consumer), a fixed $12/$13/$14/$15 footprint repeated across all three islands under no register pressure, and two unfilled cop2 lwc2 load-delay nops. All three are shapes no GCC pass emits, and the sequences match the published PsyQ inline_c.h macro bodies instruction for instruction.
- probe: Deleted every register pin, every `__asm__ volatile("move %0, %1")` aliasing block and the `__asm__ volatile("" ::: "$5")` barrier from func_8002FDB0, and rewrote the tail as three multi-instruction canonical GTE __asm__ islands carrying the SDK macro bodies ("r" operand, $12-$15 clobber list) plus one `.word 0x4B70000C` cop2 OP command; measured `sandbox func_8002FDB0 --disable all`.
- result: 21 -> 3, build_insns 90 == target_insns 90; objdump shows target instructions 68-89 reproduced exactly. The first island's lui/ori also scheduled into the sixth subtract block's load-delay slot on its own, retiring the scheduling-barrier cheat the incoming form needed for that.
- verdict: CONFIRMED

## [s1] The 63-instruction head (stride computation plus the six scratchpad subtract/store blocks) is ordinary pure C requiring no coercion of any kind.
- mechanism: `*(s32 *)((u8 *)0x1F8000XX + stride)` is expanded by aspsx/maspsx into target's `lui $at; addu $at,$v0,$at; lw` triple (absolute address with an index register), and `*(s32 *)0x1F8003XX = a - b;` into the `lui $at; sw` pair — the addressing idiom, not a codegen accident.
- probe: objdump of the cheat-stripped sandbox object (tmp/sandbox/func_8002FDB0/code6cac_b.o) at session start, compared instruction by instruction against asm/funcs/func_8002FDB0.s.
- result: Head instructions 1-62 already matched byte-exactly with all cheat-asm stripped, before any edit this session. The whole honest 21 lived in the tail.
- verdict: CONFIRMED

## [s1] The final 3-instruction residual is caused by REUSING v1/v2 across all six subtract blocks; giving the sixth block its own locals lets its second operand take $v0, which `stride` frees at its last use.
- mechanism: v2's pseudo is live across five prior blocks and already tied to $a0 by the time the sixth block is allocated, so the sixth difference goes to a fresh pseudo that picks up the newly free $v0 (build: `lw $a0,0xBC($at); subu $v0,$v1,$a0; sw $v0`). A distinct pseudo for the sixth block's second operand takes $v0 directly and leaves the difference in $v1 (target: `lw $v0,0xBC($at); subu $v1,$v1,$v0; sw $v1`).
- probe: Declared `s32 w1, w2;` and rewrote the sixth subtract block to use them instead of v1/v2; re-ran `sandbox func_8002FDB0 --disable all`.
- result: 3 -> 0. build_insns 90 == target_insns 90, and a full 90-instruction objdump comparison against asm/funcs/func_8002FDB0.s shows every instruction identical.
- verdict: CONFIRMED
