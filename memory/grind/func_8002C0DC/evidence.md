# Evidence bank — func_8002C0DC

## Session 1 (2026-08-13, modality: recon) — CLOSED at distance 0

### Baseline
- `canonical func_8002C0DC` -> verdict **C**, `asm_insns: 0`, `total: 84`,
  `distance: 23`, reason "pure-C distance 23 <= 50 — pure-C target". No
  canonical-asm question exists for this function.
- `sandbox func_8002C0DC --disable all` (HEAD, before any edit) -> **score 23**,
  `target_insns: 84`, `build_insns: 89`, `rules_dropped: 13`.
- 13 regfix rules live at `regfix.txt:229-244`: one `reorder 12,11 @ 11-12`
  ("reorder ptr load before field_D8 load in loop body") plus a 12-rule `subst`
  block renaming registers across the post-loop multiply chain (TEXT 25-36).

### What the function does
`func_8002C0DC` is a two-entry loop over the 0x44C-stride object array based at
`D_80101EC8`, followed by a guarded state poke.
1. `s2 = func_8002BC68(D_800A371C)` — the return is a timestamp-like value
   compared much later against `D_800A371C + 0xC8`.
2. For each of the two entries `e`: read the partner pointer out of field 0x00,
   compute `dx = partner->0xD8 - e->0xD8`, `dy = partner->0xE0 - e->0xE0`, and
   call `func_8001F860(e, ratan2(dx, dy))` — i.e. face each fighter toward its
   opponent.
3. Recompute a *selected* entry `p = &tbl[D_800A38AE]` via the compiler's
   1100x (0x44C) multiply-shift chain (`x<<4 +x <<2 +x <<2 -x <<2`).
4. If `D_800A376E == 0 && D_800A3758 == 0xFF && p->0xAA == (s16)p->0x40`, call
   `func_8002AB08(1)`.
5. If `(s16)p->0x40 >= (u8)D_800A38E8` and `< (u8)p->0xAA` and
   `D_800A371C + 0xC8 < s2`, then `p->0x286 = 4` and `(*(void**)p)->0x286 = 5`.

### The measured defect (objdump of the sandbox object vs asm/funcs/*.s)
Build was 89 insns to target's 84, with a FOURTH callee-save. Exact surplus:

| surplus insn | why |
|---|---|
| `sw $s3,0x1C($sp)` / `lw $s3,0x1C($sp)` | fourth callee-save needed |
| `addiu $s1,$s0,0xE0` (loop entry) | second induction variable materialized |
| `addiu $s1,$s1,0x44C` (per iteration) | its increment |
| `nop` before `bnez` on D_800A376E | load-delay, `lh` issued late |

Frame was 0x28 vs target 0x20. Build consumed the two clustered fields as
`lw $a0,-8($s1)` / `lw $a1,0($s1)`; target consumes them as `lw $a0,0xD8($s0)` /
`lw $a1,0xE0($s0)` off the single walking pointer. Register roles were rotated
one slot (build s2=counter, s3=return; target s1=counter, s2=return) — that
rotation is what the 12 `subst` rules in regfix.txt were compensating for
downstream.

### Root cause (CONFIRMED)
The HEAD body walked the array by hand (`var_s0 += 0x44C;`) with an independent
counter, under three `register T x asm("sN")` pins. Two hand-written induction
variables let loop.c's `strength_reduce` / `combine_givs` create a SECOND derived
pointer for the +0xD8/+0xE0 cluster (they are within a 16-bit displacement of
each other, so they merged onto one new giv at +0xE0 rather than onto the
existing base). That surplus live pointer forced the fourth callee-save, the
0x28 frame, and the whole downstream register rotation.

### The fix (CONFIRMED, first probe)
Rewrite the loop as an ordinary indexed `for (i = 0; i < 2; i++)` with a
per-iteration element pointer `e = (u8 *)&D_80101EC8 + i * 0x44C`, and delete
all three register pins. That leaves exactly ONE basic induction variable — `i`,
which survives elimination because the bound test reads it — so all three
references off the element (+0x00, +0xD8, +0xE0) combine onto a single derived
pointer, which is target's `$s0`.

`sandbox --disable all` -> **score 0**, `build_insns: 84` == `target_insns: 84`.

### Verification that this is NOT a masked zero
Per [[sandbox-zero-retire-fails]], score 0 alone is not sufficient evidence. The
sandbox object was disassembled (`tmp/grind/func_8002C0DC/s1/build.txt`) and
compared instruction-by-instruction against `asm/funcs/func_8002C0DC.s`: all 84
instructions agree on opcode, register operands, immediates, field offsets,
branch displacements, delay-slot occupant, frame size and callee-save set. The
only zeroed fields are link-time relocations (`lui`/`%lo` pairs, `jal` targets).
No register residual is hiding behind the mask.

### Secondary finding — the load-delay nop resolved for free
The fifth surplus instruction (a `nop` in the load-delay of `lh D_800A376E`
before its `bnez`) was NOT separately attacked. It disappeared as a consequence
of the same fix: with the fourth callee-save gone the scheduler issues the
`lh D_800A376E` early, immediately after the `lh D_800A38AE` and ahead of the
1100x shift chain — exactly the target ordering. Lesson worth carrying: a
surplus-induction-variable defect can present as *several* apparently unrelated
residuals (frame size, register rotation, a stray load-delay nop) that all
collapse together once the extra live value is removed. Do not attack such
residuals individually before checking the callee-save count and frame size
against target.

### Purity
The accepted form adds nothing. It strictly REMOVES three `register asm("sN")`
pins and replaces a hand-rolled pointer walk with an index walk. No dead code,
no dead stores, no volatile, no aliases, no wrappers, no inline asm, no
sanctioned-exception family claimed. Full six-test vetting in `self_vet.md`.

### OPEN — integration handoff (NOT a defect in the C)
The 13 regfix rules at `regfix.txt:229-244` were written against the old, wrong
codegen. With the correct codegen in place they are actively harmful and must be
retired before the full-build oracle check. This session may not touch
`regfix.txt` and may not run `retire`, so this is flagged for the operator/driver
rather than done here.
