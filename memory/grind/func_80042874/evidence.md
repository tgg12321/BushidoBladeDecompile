# Evidence bank — func_80042874

## Session 1 (recon, 2026-08-12) — SOLVED to honest distance 0

### Baseline
- `canonical func_80042874` → verdict **C**, asm_insns 0, total 119, distance 21
  ("pure-C distance 21 <= 50 — pure-C target"). Not an asm-routing candidate.
- `sandbox func_80042874 --disable all` at HEAD → **21**, build_insns 120,
  target_insns 119, rules_dropped 10.
- HEAD's src body carried TWO cheats, both stripped by the sandbox (so neither was
  buying anything against the honest floor): `register s32 angC asm("$3")` and
  `cosA = (s16)*(volatile u16 *)(&Judge[...])`.
- regfix.txt lines 888-903 hold 10 rules for this function (mflo/sra/mult substs,
  one delete, two reorders, the `la $2,Judge` → `lui $at,%hi(Judge)` rewrite,
  one `insert_after "nop"`). They are written against the OLD codegen and MUST be
  retired by the operator (`engine retire func_80042874`) for the full build to
  match the new body. This session may not touch regfix.txt.

### The decisive structural fact: the solved sibling
`src/text1a_c.c` holds three consecutive rotation-matrix builders with identical
dataflow and different element orders:

| func | target insns | sandbox --disable all | rules | state |
|---|---|---|---|---|
| func_80042874 | 119 | 21 → **0** this session | 10 (stale) | this item |
| func_80042A88 | 114 | **0** | 0 | COMPLETED-C, off the queue, match commit `11ecbfa8` |
| func_80042C80 | 122 | 58 | 63 | still active in the queue (verdict ASM-SUSPECT) |

func_80042A88's accepted COMPLETED-C body (src/text1a_c.c:336-405) is the template
for this whole family. Its in-source header comment (src/text1a_c.c:275-334) records
four grind sessions' worth of mechanism notes and was the primary input to this
session. **Any future session on func_80042C80 should start there too** — C80 is the
same shape at score 58 with 63 rules, and the three transfers below are the obvious
first probes for it.

### What transferred (each measured)
1. **`u16 rawA` staging + a memory write between the load and the `(s16)` cast.**
   Target reads cosA as `lui at,%hi(Judge); addu at,at,a3; lhu v1,%lo(Judge)(at)`
   then `sll v1,16` / `sra v1,16` (target insns 57/59/61) — a three-insn
   zero-extend-then-sign-extend, not a single `lh`. Mechanism (from the A88 ledger):
   combine's `can_combine_p` will not combine a MEM into a later user across an insn
   that may write memory, so `simplify_shift_const` never sees the
   ashiftrt(ashift(zero_extend(mem),16),16) chain it would fold to a sign_extend.
   The intervening write here is the real store `a1[7] = sinA;`, which sched1 then
   hoists back out to its target position (target insn 106) at zero cost.
   MEASURED: dropping the staging local (plain `cosA = Judge[...]`, store still
   present) → score **6**, build_insns 116 (three insns short — exactly the collapsed
   `lh`). So the staging local is load-bearing and its effect is present in the
   TARGET bytes.
2. **`angC = a0[2]; sinC = Judge[angC & 0xFFF];` placed after the
   `negSinAxsinB_12 = (sinA * -sinB) >> 12;` statement.** a0[2] is the last use of
   the parameter pointer, so its position decides where $a0 dies; read there,
   local-alloc gives angC target's $v1 and the cos-index temp target's $v0
   (target insn 19 `lhu v1,0x4(a0)`).
3. **`idxB = (s16)angB + 0x400;` hoisted into a named intermediate right after sinB**
   (A88 does the same at src/text1a_c.c:359).

### The split-vs-combined fixpoint — RESOLVED here
The A88 ledger recorded an unresolved tension at score 12-13: writing the first
product as a SPLIT `prod = sinA * sinB;` … `prod12 = prod >> 12;` reproduces target's
schedule (mflo delayed past the cos-index chain) but flips angC/idxC into the wrong
registers, while the COMBINED `(sinA * sinB) >> 12` gets the registers but not the
schedule. This session reproduced that fixpoint exactly on func_80042874:
- staging (1) + SPLIT multiply, no idxB → **14**, build_insns 120. Residual was
  precisely the documented pair: angC/idxC mirror-swapped ($v0 ↔ $v1), the
  `sra` scheduled early with dest $a0 instead of $v0, and one surplus nop.
- staging (1) + COMBINED multiply + hoisted idxB (3) → **0**, build_insns 119.
**The idxB hoist is what lets the combined form keep both halves.** That is the
transferable finding: when the combined/split choice looks like a two-way fixpoint,
hoisting the OTHER angle's index into a named local is the tiebreaker.

### Byte-level verification (beyond the sandbox scorer)
The sandbox scorer ignores nops and masks branch/jump targets, so a score of 0 was
independently checked at the byte level:
- `mipsel-linux-gnu-readelf -sW tmp/sandbox/func_80042874/text1a_c.o` →
  `func_80042874` size **532 bytes = 133 words**, identical to the 133 instruction
  words in `asm/funcs/func_80042874.s` (the scorer's 119 is the nop-stripped count).
- `tmp/grind/func_80042874/s1/bytecmp.py` compares those 532 bytes word-by-word
  against the splat listing, skipping only the 28 words carrying relocations
  (all `%hi`/`%lo(Judge)`): **0 non-relocation word mismatches**.
This function is straight-line (no branches at all), so nothing is hidden by the
scorer's branch masking.

### Gotcha recorded for future sessions
Counting instructions by piping `objdump -d` through an awk range is unreliable here
(blank lines / column splits silently truncate the range and made the build look 14
nops short of target). Use `readelf -sW` symbol SIZE for length checks and compare
raw `.text` bytes for content — see `tmp/grind/func_80042874/s1/bytecmp.py`.
