---
name: mmio-volatile-type-level
paths: [".claude/rules/mmio-volatile-type-level.md"]
# on-demand only: surfaced via codegen-technique-index (auto-loads on src/*.c)
description: "OWNER RULING 2026-07-01: volatile on hardware-MMIO-range addresses (0x1F801000-0x1F802FFF I/O registers) is legitimate TYPE-LEVEL hardware semantics — all read/write shapes, including single-read probes. No use-site shape test, no FAKE annotation (it isn't fake). SOTN declares register pointers volatile once; GetIntrMask/_status/bit-test probes are volatile by default. Game-state globals keep the two-prong gate."
metadata:
  type: rule
---

# Hardware-MMIO volatile is TYPE-LEVEL (no use-site shape test)

**Owner ruling 2026-07-01** (evidence base:
[[sotn-family-research-2026-07-01]]): for memory in the PSX hardware
I/O-register range, `volatile` belongs on the DECLARATION as ordinary
hardware semantics — every access shape it covers is legitimate,
**including single-read probes**. This replaces the three-shape
use-site test of [[legitimate-volatile-interrupt-touched]] FOR
MMIO-RANGE ADDRESSES ONLY; game-state globals keep the unchanged
two-prong + three-shape gate.

## Why (the SOTN structural finding)

SOTN does not gate volatile per use-site at all. Hardware registers are
declared volatile ONCE and every read inherits it:

- `src/main/psxsdk/libetc/intr.c:42-44` — `static volatile u16* i_stat =
  (u16*)0x1F801070; static volatile u16* g_InterruptMask =
  (u16*)0x1F801074;`
- Same idiom: `libcd/registers.h:4-7`, `libapi/counter.c:15-16`,
  `libetc/intr_dma.c:12-13`, `libgpu/sys.c:88-95`.

Consequently single-read MMIO probes carry volatile semantics with zero
per-site justification in SOTN:

- `intr.c:69` — `u16 GetIntrMask(void) { return *g_InterruptMask; }`
- `libgpu/sys.c:549` — `u_long _status(void) { return *GPU_STATUS; }`
- `libgpu/sys.c:889` — `return ((u_long)*GPU_STATUS >> 0xC) & 1;` —
  read-once / test-bit / return-bool.

## Scope (exact)

**Covered addresses:** the PSX hardware I/O-register range
`0x1F801000-0x1F802FFF` (interrupt controller, DMA, timers, CD, GPU,
SPU, expansion). **Explicitly NOT covered:** scratchpad RAM
`0x1F800000-0x1F8003FF` (fast RAM, not registers — volatile there is
game-state coercion unless an IRQ writer is cited under the two-prong
gate) and all KSEG0 game RAM.

**Covered spellings** (the address must be verifiable — a literal, a
cited `.data` word, or a symbol whose value is documented in the split):

- `volatile T* p = (T*)0x1F801xxx;` pointer declarations
- `*(volatile T *)(BASE + k)` computed-MMIO-address casts (already
  precedented by func_80078B3C/B70, 2026-06-14)
- `extern volatile T D_xxxxxxxx;` where D_'s address resolves into the
  covered range

**Requirements:**

1. **Address verification** — the worker cites how the address lands in
   the covered range (e.g. `D_8009BD88 = .word 0x1F801070,
   asm/data/7D920.data.s:<line>`). The reviewer verifies the citation.
2. **No FAKE annotation** — this is correct typing, not a coercion; a
   brief `/* I_STAT */`-style comment naming the register is encouraged.
3. **`extern volatile` spellings still go through
   `volatile_extern_allowlist.txt`** (the engine detector's audit
   artifact) — the GRANT CRITERION under this rule is simply "address
   verifiably in the MMIO range"; no use-site shape or IRQ-writer
   citation needed. Allowlist comment format: `# <sym> — MMIO <register
   name> 0x1F801xxx (type-level grant, mmio-volatile-type-level)`.

## What stays unchanged (non-extension)

- **Game-state globals** (KSEG0 RAM, scratchpad): the
  [[legitimate-volatile-interrupt-touched]] two-prong criterion
  (verifiable IRQ writer + spin-wait / double-read / IRQ-mutated-bound
  use-site) applies unchanged. This rule does not touch it.
- **`*(volatile T *)&D_xxxxxxxx` casts on game RAM** — forbidden,
  unchanged ([[inline-asm-policy]]).
- **Alias renames, macro-hidden asm** — forbidden, unchanged.
- A volatile MMIO declaration is NOT a general codegen wand: if the
  bytes still don't match with correct hardware typing, the remaining
  gap is ordinary pure-C work. (Known case: `func_80078EC0`'s wall was
  the jump-opt boolean fold, which volatile does NOT defeat — it closed
  2026-07-01 via [[dead-store-fake-exception]] instead, with NO volatile
  needed for the match.)

## Related

- [[sotn-family-research-2026-07-01]] — evidence base.
- [[legitimate-volatile-interrupt-touched]] — the game-state-global
  gate this rule sits beside (and narrows to non-MMIO memory).
- [[sotn-volatile-interrupt-research-2026-06-07]] — the earlier
  research; its three-shape catalog was derived from game-state
  `extern volatile` instances and remains correct for that class.
