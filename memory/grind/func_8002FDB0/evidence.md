# Evidence bank — func_8002FDB0

## Session 1 (recon, 2026-08-11)

### Baseline measurements
- `canonical func_8002FDB0` → **ASM-PARTIAL**, 10/90 insns canonical-asm, regions
  [69,69] [71,72] [76,79] [83,85], reasons: GTE/cop2 ops (c2 / ctc2 / lwc2 / swc2).
- `sandbox func_8002FDB0 --disable all` at session start → **21** (0 regfix/asmfix
  rules; 326-352 cheat-asm constructs stripped file-wide). build_insns 83 vs
  target_insns 90.
- `sandbox --disable all --keep-cheat-asm` at session start → **0**. So the
  incoming src form already produced the exact target bytes; the entire debt was
  the FORM (register pins + `move %0,%1` INLINE_MOVE_ALIASING blocks), not the
  logic. The 21 was an artifact of stripping those pins/moves.

### The function
`s32 func_8002FDB0(s32 *arg0)` — 90 instructions, leaf, no stack frame.
1. `stride = ((s16 *)arg0)[2] * 264` (emitted as sll5 + addu + sll3).
2. Six identical `SCR[dst] = SCR[src_a + stride] - SCR[src_b + stride]` blocks
   writing the two difference vectors:
   `(0xC0,0xC4,0xC8) - (0xB4,0xB8,0xBC)` → `0x1F800360/364/368`, and
   `(0xCC,0xD0,0xD4) - (0xB4,0xB8,0xBC)` → `0x1F800370/374/378`.
   All six compile byte-exactly from plain
   `*(s32 *)((u8 *)0x1F8000XX + stride)` reads and `*(s32 *)0x1F8003XX = a - b;`
   stores — the `lui $at; addu $at,$v0,$at; lw` triples are aspsx/maspsx
   expanding an absolute address with an index register, not a codegen accident.
3. A GTE tail: SetRotMatrix-from-SCR[0x360], load-long-vector-from-SCR[0x370],
   GTE `OP` (cop2 cmd 0x0170000C, outer/cross product), store MAC1..3 to
   SCR[0x380].
4. `return 0 < *(s32 *)0x1F800384;` (the trailing `slt $v0,$zero,$v0` in the
   `jr $ra` delay slot).

### CONFIRMED — the GTE tail is verbatim PsyQ libgte inline-MACRO expansion
Target instructions 68-89 decompose exactly into three published PsyQ
`inline_c.h` macro bodies plus one cop2 command word:

    gte_SetRotMatrix(r):  move $12,%0 ; lw $13,0($12) ; lw $14,4($12) ;
                          ctc2 $13,$0 ; lw $15,8($12) ; ctc2 $14,$2 ; ctc2 $15,$4
    gte_ldlvl(r):         move $12,%0 ; lwc2 $11,8($12) ; lwc2 $9,0($12) ;
                          lwc2 $10,4($12) ; nop ; nop
    gte_op0():            cop2 0x0170000C      (.word 0x4B70000C)
    gte_stlvnl(r):        move $12,%0 ; swc2 $25,0($12) ; swc2 $26,4($12) ;
                          swc2 $27,8($12)

Four independent signals that this is SDK macro text and not compiler output:
- Three redundant `lui $a1,0x1F80 ; ori $a1,$a1,0x3X0 ; addu $t4,$a1,$zero`
  sequences. GCC materializes a constant into a register and then copies it to a
  SECOND register with no other consumer exactly never; the copy is the macro's
  own `move $12, %0`.
- A fixed `$12/$13/$14/$15` register footprint repeated identically across all
  three islands, with no pseudo pressure that would force it.
- Two unfilled `nop`s (the cop2 lwc2 load delay) sitting between the lwc2 triple
  and the `OP` command — a scheduler would have filled or omitted them.
- The pointer operands are the three compile-time scratchpad constants
  0x1F800360 / 0x1F800370 / 0x1F800380, re-materialized per macro use, which is
  what an `"r"`-constrained macro argument produces.

### CONFIRMED — the whole function reaches honest distance 0 in this form
Rewriting the tail as three canonical GTE inline-asm ISLANDS (one
multi-instruction `__asm__ volatile` per SDK macro, `"r"` operand, `$12`-`$15`
clobber list, zero register pins, zero `move %0,%1` aliasing blocks) and
splitting the sixth subtract block onto its own locals gives:

    sandbox func_8002FDB0 --disable all  ->  score 0, build_insns 90 == target 90

objdump of `tmp/sandbox/func_8002FDB0/code6cac_b.o` was compared instruction by
instruction against `asm/funcs/func_8002FDB0.s`: all 90 identical, including the
`lui/ori` of the first island's pointer scheduling into the sixth block's
load-delay slot (which now falls out naturally — the incoming form needed a
`__asm__ volatile("" ::: "$5")` scheduling-barrier cheat to force it).

The engine keeps these islands because `engine/inlineasm.py::_block_category`
classifies a block as `canonical` when ANY instruction in the template is
canonical (a cop2 op here), so the honest `--disable all` score is a real
measurement of this form, not a stripped-away one.

### CONFIRMED — the last 3-instruction residual is a locals-split lever
With `v1`/`v2` reused for the sixth subtract block the score sat at **3**:
build emitted `lw $a0,0xBC($at) ; subu $v0,$v1,$a0 ; sw $v0,0x378($at)` where
target has `lw $v0,0xBC($at) ; subu $v1,$v1,$v0 ; sw $v1,0x378($at)`.
Giving the sixth block its own two locals (`w1`, `w2`) drops it to **0**:
`stride` takes its last use as the index of that block's second address, so
`$v0` frees exactly there, and a fresh allocno (rather than the recycled
`v2` pseudo already tied to `$a0` by five prior blocks) takes it.

### Disposition constraint (not a matching problem)
`tools/audit_asm_cheats.py --func func_8002FDB0` reports
`UNAUTHORIZED: ... multi-insn __asm__ block in code6cac_b.c:1923 (7 insns)
containing non-§6.1-whitelisted instructions` — i.e. the finished state is
COMPLETED-INLINE-ASM-CANONICAL and requires an `inline_asm_canonical.txt`
entry, a surface a grind session may not touch. Precedent for the exact shape:
`func_800274BC` (same file) at `inline_asm_canonical.txt:263`, a canonical GTE
island carrying its original island's GPR scaffolding (`addu $t4, %1, $zero`),
owner-authorized 2026-06-10.

- [s1] canonical func_8002FDB0 = ASM-PARTIAL, 10/90 insns canonical-asm (GTE c2 / ctc2 / lwc2 / swc2), regions [69,69] [71,72] [76,79] [83,85].

- [s1] Session-start honest floor: sandbox --disable all = 21 (0 regfix/asmfix rules), build_insns 83 vs target_insns 90.

- [s1] Session-start sandbox --disable all --keep-cheat-asm = 0: the incoming src body already produced the exact target bytes, so the entire debt was the FORM (register pins + move-aliasing blocks + a scheduling barrier), never the logic.

- [s1] End-of-session honest floor: sandbox --disable all = 0, build_insns 90 == target_insns 90, with zero regfix/asmfix rules, zero register pins, zero `move %0,%1` blocks and zero scheduling barriers in the function.

- [s1] Byte-exactness verified independently of the score: mipsel-linux-gnu-objdump of tmp/sandbox/func_8002FDB0/code6cac_b.o compared instruction by instruction against asm/funcs/func_8002FDB0.s — all 90 identical.

- [s1] engine/inlineasm.py::_block_category classifies an __asm__ block as `canonical` when ANY instruction in its template is canonical (a cop2 op here), so the honest --disable all score measures this form rather than stripping it away.

- [s1] tools/audit_asm_cheats.py --func func_8002FDB0 reports UNAUTHORIZED: multi-insn __asm__ block in code6cac_b.c:1923 (7 insns) containing non-6.1-whitelisted instructions — the mechanical confirmation that the finished state is COMPLETED-INLINE-ASM-CANONICAL and needs an inline_asm_canonical.txt entry.

- [s1] Precedent for the exact construct shape: inline_asm_canonical.txt:263 — func_800274BC, same source file, owner-authorized 2026-06-10, a canonical GTE island carrying its original island's GPR scaffolding (`addu $t4, %1, $zero`) inside one multi-instruction __asm__ block alongside mtc2/swc2.

- [s1] The tail has no pure-C form: no C expression makes GCC materialize a constant into one register and then copy it to a second register with no consumer, and ctc2/lwc2/swc2/cop2 have no C form at all. The choice is canonical-asm authorization or nothing.

- [s1] Sibling cluster: src/code6cac_b.c and src/code6cac.c carry many more functions in the same pin + move-aliasing + per-instruction-cop2 spelling (code6cac_b.c:1452, :1651, :1726; code6cac.c:1960, :2025, :2066). The same island reconstruction likely retires the cluster if this ruling is favourable.
