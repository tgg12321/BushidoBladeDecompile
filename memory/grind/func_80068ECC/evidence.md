# Evidence bank — func_80068ECC

## s1 (2026-07-27, recon) — MATCHED at sandbox 0
- Function: bit-remap into global D_8009BC04 (41 target insns, leaf, no branches).
  Bit map: dst bits 0-2,7 <- arg0 bits 0-2,7 direct; dst 3,4,5 <- arg0 4,5,6 (>>1, three
  SEPARATE `srl $v1,$a0,1` in target — no CSE); dst 6 <- arg0 3 (<<3).
- Inherited src body carried cheat-asm (register $2 pin + 3 CSE-barrier asms); honest
  floor with them stripped = 13 (39/41 insns: the two extra srls CSE-merged away, $a1
  stolen by the hoisted srl, pointer pushed to $a2).
- Measured facts (all via sandbox --disable all + objdump of the sandbox .o):
  1. In-place `v &= ~K; v |= b;` keeps v in $v0 with `and $v0,$v0,$v1` two-address
     shape. Compound `v = (v & ~K) | b;` allocates an intermediate -> score 31. KILLED.
  2. GCC 2.7.2 combine does NOT canonicalize mask-then-shift `(arg0 & K) >> 1`
     -> emits `andi;sra` (s32) / `andi;srl` (u32). Target needs `srl;andi` -> shift-first
     spelling mandatory.
  3. Signed shift-first `(arg0 >> 1) & 8` emits `sra` (no ashiftrt->lshiftrt conversion
     under mask). Score 1 with it as the odd one out. KILLED.
  4. CSE hashes on RTL structure: `((u32)arg0 >> 1)`, `((u16)arg0 >> 1)`,
     `((u8)arg0 >> 1)` are three DISTINCT expressions (zero_extend widths differ), so no
     CSE merge; combine then folds each zero_extend into the following andi mask
     (mask fits inside the narrowed width) -> three independent `srl;andi` pairs,
     byte-matching target. **Score 0, 41/41 insns.**
- Closing form: candidate.c (applied in src/text1b.c). Pure C — no asm, no pins, no
  volatile, no dead code; all casts are live value-range-narrowing with real semantics.
