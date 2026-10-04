---
name: legitimate-volatile-interrupt-touched
paths: [".claude/rules/legitimate-volatile-interrupt-touched.md"]
description: "NARROW CARVE-OUT: `extern volatile T G;` only for a game-state global with a cited IRQ writer AND a spin-wait / double-read / IRQ-mutated-loop-bound use site. Volatile LOCALS: SOTN citation or target-byte proof."
metadata:
  type: rule
  tier: blocking
---

# `extern volatile T G;` — IRQ-touched globals (narrow exception)

The default ban on `extern volatile T D_xxxxxxxx;` and every other volatile-as-CSE-defeat spelling
([[inline-asm-policy]]) stands. This is the single published exception for GAME-STATE memory (KSEG0 RAM,
scratchpad). Hardware I/O registers (`0x1F801000-0x1F802FFF`) are volatile at type level instead
([[mmio-volatile-type-level]]). A verified matched-SOTN citation can admit a construct this text refuses, with
Q53's prerequisites ([[sotn-precedent-suffices]] Q55).

## The two-pronged criterion (BOTH required)

1. **G is asynchronously mutated by an IRQ handler.** G's address is a write target inside a function
   installed via `InterruptCallback` / `VSyncCallback` / `SysSetCallback` / `CdCallback` /
   `EnterCriticalSection`-wrapped callback (or called from one). **Cite the writer: function name +
   file:line.** A bare assertion fails.
2. **A use site has one of exactly three shapes** where the IRQ can change G between would-be-merged reads:
   - **spin-wait** — `while (G ...) {}` / `do {} while (G ...);` with no body work besides the test;
   - **double-read-across-sequence-point** — `x = G; foo(); y = G;` where G demonstrably changes between reads;
   - **IRQ-mutated-loop-bound** — `for (i = 0; i < G; ...)` with G changed by the IRQ during the loop.

   Any other shape default-FAILs. SOTN applies volatile only at such use sites (its `VSyncHandler` writes 5
   plain `u32` globals) — IRQ-touched alone is not enough.

A grant is PER SYMBOL: once one consumer qualifies, other consumers of the same symbol inherit the declaration.

**Owner ruling Q95 (2026-10-02) — one field-level row, `GpuCtx.unk08`.** The LIBGPU SYS draw-pending flag
(GpuCtx +0x08, `src/main/psxsdk/libgpu/sys.c`) may be `volatile s32` though its use site (_exeque's test-then-clear before
the drawsync callback) is none of the three shapes. Grounds: _exeque is also the DMA-2 IRQ callback; non-volatile,
reorg fills the callback's `jalr` delay slot with the clear (2/187), and only volatile, asm or extra jumps keep
it out (`1323cbc23^:memory/grind/_exeque/evidence.md` [s13b], [s14]). Only this field; other GpuCtx members stay plain.

**Not covered:** non-IRQ globals; uncited "might be IRQ-touched"; other use-site shapes; `*(volatile T *)&G`
casts on non-qualifying globals; alias renames (`extern volatile T G_v asm("G");`); macro-hidden `__asm__`.

**Mechanism note.** GCC 2.7.2 sched.c `read_dependence` adds a read-read edge only when BOTH reads are
volatile — a strictly source-ordered target block around an IRQ counter can mean a second, non-volatile handle
of the same memory.

## Procedure

1. Cite the IRQ writer and name the use-site shape.
2. Add the symbol to `volatile_extern_allowlist.txt` in the same commit
   (`<symbol>  # <func> — IRQ writer: <writer>:<file>:<line>`); the engine detector reads it.
3. Commit body:
   ```
   IRQ writer: <function>():<file>:<line> — installed via <callback mechanism>
   Use-site construct: <spin-wait|double-read-across-sequence-point|IRQ-mutated-loop-bound>
   Cheat-reviewer: PASS (criteria 1 and 2 independently verified)
   ```
4. The `cheat-reviewer` verifies each field against the source ([[review-discipline-before-commit]]).

Example: `D_8009BF7C` (display.c) — writer `func_8007D6D8` installed via `irq_AcknowledgeVblank`; use site
`while (D_8009BF78 != D_8009BF7C) { ... }` in `func_8007DB20`.

## Volatile locals (owner rulings Q48 + Q50, 2026-09-30)

A `volatile T x;` automatic local holding a value the function uses is admitted ONLY by route A or B, and
always with (4). Record: decisions.md 2026-09-30 OWNER RULING — volatile locals: SOTN precedent or
target-byte proof.

- **Route A — SOTN precedent (Q50).** A citation meeting conditions (1)-(4) of [[sotn-precedent-suffices]]
  (PS1-build file, the same thing when read, matched SOTN code), recorded in the
  ledger and an inline `/* SOTN: <file>:<line> @<commit> */` tag, with Q53 as tiered by Q91
  ([[sotn-precedent-suffices]]): `/* FAKE: ... */`-labelled, simplest known form, fresh layer-2; the
  ledger record and simpler-spelling write-ups are hygiene. Only SOTN counts.
- **Route B — target-byte proof (Q48).** All of:
  1. every access to the local in the original instructions is a store to / load from its `$sp` slot (none
     served from a register), listed by address and opcode;
  2. the same body without `volatile` (nothing else changed) serves at least one of those accesses from a
     register — score and diff banked;
  3. (1)-(2) banked in the function's ledger.
- **(4) Layer-2.** A fresh layer-2 `cheat-reviewer` checks the citation or the proof; a Judge PASS is not
  enough (manual path only). Sandbox 0 + oracle SHA1 as always.

This section does not govern unused `volatile` pad locals (phantom-frame-slot family), pointers to volatile
data, or `volatile` casts.

Related: [[mmio-volatile-type-level]] · [[do-while-zero-exception]] · [[inline-asm-policy]]
