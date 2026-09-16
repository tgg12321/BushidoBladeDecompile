# Evidence — func_80056CB8 (src/text1b.c)

## [s1] recon 2026-09-16

**Function status:** NOT YET STARTED. `src/text1b.c` carries
`INCLUDE_ASM("asm/funcs", func_80056CB8);` (asm-until-matched state, no
cheats). `sandbox --disable all` reports `no_c_body: true`, `score: 204`
(== `target_insns`) — the "distance" is simply the raw target instruction
count, not a diff against any candidate C. This is a from-scratch
reconstruction, not a residual-chasing session.

**Signature (confirmed via caller + asm):** `void func_80056CB8(void *arg0)`
(possibly a typed character/fighter struct pointer). Caller evidence:
`memory/grind/func_80055B60/evidence.md:13` region R9 — `func_80055B60`
calls it as `jal func_80056CB8(s2)` (single arg, plain jal, return value
unused at the call site). Asm prologue: `$a0` is moved into `$s7` and held
live across the whole function (`addu $s7,$a0,$zero` at
`asm/funcs/func_80056CB8.s:3`), consistent with a single struct-pointer
parameter used throughout.

**OBJECT MODEL:** (per the brief's mandatory recon requirement — every
flagged DATA MODEL symbol addressed)

- `D_8009A820` (byte, MATCHES): already `extern u8 D_8009A820;` at
  `src/text1b.c:2183`. No header decl, but the in-TU convention for this
  exact class of byte table is already established just above in the same
  file: `func_80056FE8`'s body (`src/text1b.c:1867-1874`) reads sibling
  tables `D_8009A830`/`D_8009A838`/`D_8009A840` via
  `*(u8 *)(((s32)(&D_8009A83X)) + idx)` pointer-cast arithmetic rather than
  a declared array. Our target's asm does the identical thing:
  `lui $at,%hi(D_8009A821); addu $at,$at,$fp; lbu $v0,%lo(D_8009A821)($at)`
  (`asm/funcs/func_80056CB8.s:29-32`) — a `%hi/%lo`-relocated base address
  plus a register offset, i.e. exactly `*(u8 *)((s32)&D_8009A821 + fp)`.
  No declaration change needed; write the read in this file's own idiom.
  Score not separately measurable in isolation (whole-function distance is
  204 either way at this stage) — logged as MATCHES-unmeasured (isolating
  this one symbol from a 204-insn from-scratch body isn't meaningful; the
  measurement that matters is the eventual full-body sandbox score).
- `D_8009A821` (byte, MATCHES): identical situation, same idiom, same file
  region (`src/text1b.c:2184`), same asm shape
  (`asm/funcs/func_80056CB8.s:56-59` — the second array read at
  `.L80056DA8`).
- `D_800F6608` (Rec44 struct, MATCHES + naming correction): already
  `extern Rec44 D_800F6608;` in `include/code6cac.h:283`,
  `w0`/`w4`/`w8`/`wC` are `s32` members at struct offsets 0/4/8/0xC
  (`code6cac.h:265-274`). The census's separate symbol `D_800F6610` is
  `D_800F6608 + 0x8` — i.e. it is NOT a distinct global, it is
  `D_800F6608.w8`. Confirmed both arithmetically (`0x800F6610 -
  0x800F6608 == 8`) and by exact sibling precedent:
  `func_80057094` (`src/text1b.c:1889`, matched, in the SAME file) reads
  `D_800F6608.w0 - *(s32*)(arg0+0xF4)` and `D_800F6608.w8 -
  *(s32*)(arg0+0xFC)` — the identical midpoint-anchor `ratan2` idiom our
  target's asm uses at `.L80056D6C` (`asm/funcs/func_80056CB8.s:50-57`:
  `lw $v1,%lo(D_800F6608)($v1)` for w0, `lw $v0,%lo(D_800F6610)($v0)` for
  w8, then `jal ratan2` with `subu`-computed deltas against
  `0xF4($s1)`/`0xFC($s1)`). **This IS the DATA MODEL declaration-fix
  hypothesis** (hypothesis #1 below) — write `D_800F6608.w8`, never
  declare a new `D_800F6610` symbol.
- `Judge` (s16, MATCHES): already `extern s16 Judge;` declared twice in
  this TU (`src/text1b.c:1967`, `:2169`). It is a sine/cosine-style lookup
  table accessed via raw pointer arithmetic on `&Judge`, NOT array
  indexing syntax — and this file already has the exact idiom matched and
  in production at `func_80057CC8` (`src/text1b.c:2045-2046`):
  `*(&Judge + (ang_mid & 0xFFF))` / `*(&Judge + (((s16)ang_mid + 0x400) &
  0xFFF))`. Our target's asm computes the identical shape twice
  (`.L80056D94`-area, `asm/funcs/func_80056CB8.s:61-90`): a 12-bit-masked
  angle word shifted left 1 (`andi $v0,$s0,0xFFF; sll $v0,$v0,1`) added to
  `%hi/%lo(Judge)`, then `lh` — i.e. `*(&Judge + (angle & 0xFFF))` — done
  TWICE, once for the base angle (`$s5`, offset 0) and once for angle+0x400
  (`$s4`, offset `+0x400` before the same mask), mirroring
  `func_80057CC8`'s `ang_mid` / `ang_mid+0x400` pair exactly. **Hypothesis
  #2 below: reuse this exact idiom verbatim for the two Judge reads.**

**Structural map of the asm (204 insns, `asm/funcs/func_80056CB8.s`):**
- Prologue: saves `$s0-$s7,$fp,$ra` (0xA8-byte frame), `$s7 = arg0`.
- `v1 = (lhu arg0[0x3E8]) & 3` (a 0..3 mode/counter field); `s6 = v1<<1`
  (loop start counter, saved to `0x60($sp)`), `fp = v1<<2` (byte-table
  index base, `%hi/%lo` relocation offset for the `D_8009A820/21` reads).
  Sets up three saved stack pointers (`0x68/0x70/0x78($sp)`) to
  `sp+0x28`, `sp+0x58`, and the scratchpad address `0x1F8002B8` — the
  scratchpad literal is almost certainly an argument to `func_80053614`
  (see below), not a scratchpad POKE by this function itself (no
  `mtc2`/`swc2`/cop2 opcodes anywhere in the 218-line file — this is
  ordinary ALU/mult/branch code, ZERO GTE-band signals, `scan_hand_coded`
  tier LOW per the canonical gate, so this is an ordinary ratan2/table
  lookup ALU function, not canonical-asm territory).
- **Outer loop `.L80056D24 .. .L80056FB4`** runs exactly TWICE
  (`s6` starts at `v1<<1`, increments by 1 each iter, loop test is
  `s6 < initial_s6 + 2`; `fp` increments by 2 each iter, NOT 4 — so the
  two `D_8009A820/21` byte-table reads inside the loop use a *different*
  index stride (`fp`, +2/iter) than the byte-table's own natural element
  size implies; this needs a probe next session, not assumed).
- Inside the loop: reads `arg0[0x6A]` (`s16`, compared against 0x13/6 to
  pick between `arg0->field0x1CA` [dereferenced through `$s1`, itself set
  from either `arg0` directly or `*(s32*)arg0` gated by a `D_8009A821`
  byte's bit `0x1000`] and a `ratan2(D_800F6608.w0 - s1[0xF4], D_800F6608.w8
  - s1[0xFC])` call) to build an angle word `s0`.
  Two Judge-table lookups (`s5`/`s4`, angle and angle+0x400) feed two
  `mult` ops against a `D_8009A820`-derived scaled byte, producing values
  written into a `0x28/0x2C`-based local stack struct (two `sra 12`
  fixed-point scalings against `s1[0xB8]`/`s1[0xBC]`, both offset by
  `-0x320`).
  Calls **`func_80053614`** TWICE per iteration (already matched, in this
  TU: `void func_80053614(s32 *arg0, s32 *arg1, s32 arg2, s32 arg3, s32
  arg4)`, `src/text1b.c:1513`) with a 5-slot stack-built argument block
  (`0x10/0x18/0x1C/0x20/0x28/0x2C/0x30($sp)` written before each call) —
  this is a coordinate/rect probe helper, return value accumulated into a
  bitmask `s0` (`or`, then `+1`).
  A conditional block (only when the first `func_80053614` call fails —
  `beqz $s0`) does an angle-blend fixed-point computation reading
  `0x0($s5)`/`0x0($s4)` a second time with a `*24` (`x*5-x, <<2, +x`
  =`*31`? — actually `(x<<5 - x)<<2 + x = (31x)<<2+x = 125x`... needs a
  careful re-derivation next session, NOT assumed) scale factor.
  After the two `func_80053614` calls, a distance/threshold gate chain
  (`0x38/0x40($sp)` vs `arg0[0xB8]/[0xC0]`, squared and summed, compared
  against `0x3D0900`) followed by a second gate (`arg0[0xBC]` vs
  `arg0[0x4C]`, range `[0,0x3E8]` either direction) picks a small integer
  `s0 in {0,4,5}` written to `arg0[0x444 + s6]` (byte array indexed by the
  outer loop counter, NOT `fp`).
- Epilogue: standard restore, `jr $ra`.

**Callee inventory:** `ratan2` (external, s32 atan2-style, used elsewhere
in this TU), `func_80053614` (matched, in-TU, 5 args). No unmatched
callees — nothing blocks a from-scratch C reconstruction on the callee
side.

**No canonical-asm signal.** `canonical func_80056CB8` → verdict `C`,
`hand_coded_tier: LOW` (no S1/S2/S6 signal). `distance 204` is
whole-function size, not hand-asm evidence
([[canonical-gate-distance-not-evidence]]). This is an ordinary — if
large — pure-C reconstruction target. No GTE/cop2 opcodes, no redundant
masks (packed-multiply-cluster S8 signal absent), no hardcoded-register
tells.

- [s1] sandbox --disable all: no_c_body=true, score=204=target_insns (function has never had a C draft; distance is raw size, not a residual)

- [s1] canonical: verdict C, hand_coded_tier LOW, no S1/S2/S6 signal - ordinary pure-C target, not canonical-asm territory

- [s1] signature confirmed via caller (func_80055B60 region R9, memory/grind/func_80055B60/evidence.md:13): void func_80056CB8(void *arg0), called as func_80056CB8(s2), return value unused

- [s1] D_8009A820/D_8009A821 byte tables already extern u8-declared in this TU (src/text1b.c:2183-2184) with an established cast-pointer-arithmetic access idiom used two functions earlier in the same file (func_80056FE8, src/text1b.c:1867-1874) for the sibling tables D_8009A830/38/40

- [s1] D_800F6608 (Rec44) already declared in include/code6cac.h:265-283; D_800F6610 is D_800F6608.w8, not a separate symbol (see hypothesis 1)

- [s1] Judge already declared extern s16 Judge (src/text1b.c:1967,2169) with a matched pointer-arithmetic table-lookup idiom in func_80057CC8 (src/text1b.c:2045-2046) directly reusable for this function's two lookups

- [s1] callee func_80053614 is matched/in-TU (src/text1b.c:1513, 5 args: s32*,s32*,s32,s32,s32), called twice per outer-loop iteration with a stack-built 7-word argument block; ratan2 is the other callee, external, used elsewhere in this TU - no callee blocks a from-scratch draft

- [s1] outer loop runs exactly twice (s6 starts at (arg0[0x3E8]&3)<<1, increments by 1, loop test s6 < initial+2); fp (byte-table index base) increments by 2/iter - open structural question, see hypothesis 3

- [s1] zero GTE/cop2 opcodes, zero redundant pre-shift masks (packed-multiply-cluster S8 signal absent) anywhere in the 204-insn body - ordinary ALU/mult/branch/call code
