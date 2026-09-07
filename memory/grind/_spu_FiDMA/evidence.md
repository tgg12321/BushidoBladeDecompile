# Evidence bank — _spu_FiDMA

## Session 1 (2026-09-07, recon)

**FIDMA-E1 — OBJECT MODEL: every flagged global in the brief's DATA MODEL section
was checked against the target bytes; all three MATCH their HEAD declaration, and
the matching C body was produced with zero declaration changes.**
- `D_800A2CDC` (@0x800A2CDC, census `g_spu_base_addr`) — **MATCHES.** HEAD declares
  `extern s32 D_800A2CDC;` (src/main.c:75). The target reads it once with
  `lw %lo(D_800A2CDC)` into `$a0` and then addresses the SPU register file off that
  value with `lhu/sh 0x1AA($a0)`, i.e. it is a scalar *value* holding a base
  address, not an aggregate. The established HEAD idiom in this same TU
  (src/main.c:1620-1691, `*(volatile u16 *)(D_800A2CDC + off)`) reproduces exactly
  that addressing. Measured: score 0 with the `s32` declaration untouched, so
  there is no declaration fix to make here — widening it to a struct/array would
  be a change with no byte to justify it.
- `_spu_transferCallback` (@0x800A2D14, census `g_spu_init_flag` — the census row's
  name is WRONG for this symbol; symbol_addrs.txt and the ratified grant note both
  call it the SPU transfer callback slot) — **MATCHES.** HEAD declares
  `extern volatile s32 _spu_transferCallback;` (src/main.c:74) under the ratified
  `volatile_extern_allowlist.txt:33-34` grant. The target's DOUBLE load
  (`lw %lo` at 80088C18 for the `beqz`, then a second `lw %lo` at 80088C2C before
  the `jalr`) is precisely what the `volatile` qualifier emits, and the source
  spelling `if (_spu_transferCallback) { ((void (*)(void))_spu_transferCallback)(); }`
  reproduces both loads. Measured: score 0, declaration untouched. Do NOT re-type
  it as `void (*volatile)(void)` — that would be a second C handle for the same
  memory and is unnecessary, since the cast form already matches.
- `D_800A2D2C` (@0x800A2D2C, census `g_spu_xfer_pending`) — **MATCHES.** HEAD
  declares `extern s32 D_800A2D2C;` (src/main.c:100). Target reads it with a single
  `lw %lo` and `bnez`, i.e. a plain non-volatile scalar guard; `if (D_800A2D2C == 0)`
  reproduces it. Measured: score 0, declaration untouched.
No symbol required a declaration fix; the flagged signals were all "no header decl"
(for `_spu_transferCallback` / `D_800A2D2C` the brief looked in include/*.h, but both
are declared TU-locally at the top of src/main.c) rather than a shape mismatch.

**FIDMA-E2 — The function is MATCHED. Honest floor 48 -> 0 in one measurement.**
`sandbox _spu_FiDMA --disable all` = `{"score": 0, "target_insns": 48,
"build_insns": 48}` and the full clean-driver build re-verified
`{"ok": true, "build_matches": true}` (SHA1 == 62efab4f73f992798c43e8c730aa43baa10bb4fa),
so the whole EXE still links byte-identical with the INCLUDE_ASM replaced by C.
Zero cheat constructs, zero regfix/asmfix rules, zero declaration changes.

**FIDMA-E3 — The winning body is the banked psyz transplant seed, applied verbatim
modulo one rename.** `memory/grind/_spu_FiDMA/psyz-seed.c` (authored 2026-08-18 from
Xeeynamo/psyz `decomp/src/libspu/spu.c:161-181`) was correct as written; the only
edit needed was `D_800A2D14` -> `_spu_transferCallback`, the symbol's current name
after the 2026-08 naming wave. Every "open question" the seed listed resolved
favourably on the first measurement: the timeout loop survives (`sltiu $v1,0xF01`
plus the trailing `addiu $v1,$v1,-1` are both emitted), and the volatile reload
before the `jalr` survives -O2. **Lesson for future sessions: measure the banked
transplant seed before deriving anything.**

**FIDMA-E4 — The 2026-07 "fork segfault" finding for this function is STALE and no
longer reproduces.** `memory/closer/phase3-progress.md:353` recorded that our
decompals GCC 2.7.2 fork SEGFAULTED on Sony's literal
`while (<volatile read> & 0x30) { i++; if (i > 0xF00) break; }` spelling — "any
while/for + break/goto-out spelling" — and that finding is what routed the function
toward the fork-divergence-inline-asm carve-out
(`.claude/rules/fork-divergence-inline-asm.md:135`, "first confirmed case"). That
EXACT spelling compiles cleanly on today's cc1 and emits the exact target bytes.
`docs/grind/decisions.md:10312-10313` had already flagged the suspicion ("the probe
grid predates the 2026-08-07 cc1 rebuild") — this session confirms it measurably.
The 15 variants in tmp/closer/spu3/ and the whole loop-rotation /
`duplicate_loop_exit_test` / `reorg.c:3615` analysis were artifacts of the
pre-rebuild compiler and carry no weight on the current chassis.
**Blast radius:** the fork-divergence rule's only named confirmed case was
`_spu_FiDMA`; anything else parked or reasoned against the "volatile-read while-loop
with an early exit crashes cc1" class should be re-measured before it is trusted.
