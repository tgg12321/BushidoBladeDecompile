---
name: mmio-volatile-type-level
paths: [".claude/rules/mmio-volatile-type-level.md"]
description: "Owner ruling 2026-07-01: volatile on PSX I/O-register addresses (0x1F801000-0x1F802FFF) is type-level hardware semantics — any access shape, no FAKE annotation. Game-state globals keep the two-prong gate."
metadata:
  type: rule
  tier: blocking
---

# Hardware-MMIO volatile is TYPE-LEVEL

For the PSX hardware I/O-register range `0x1F801000-0x1F802FFF`, `volatile` belongs on the declaration as
ordinary hardware semantics; every access shape is legitimate, including single-read probes. SOTN declares
register pointers volatile once (`libetc/intr.c:42-44` `static volatile u16* i_stat = (u16*)0x1F801070;`) and
probes inherit it (`GetIntrMask`, `_status`).

**Not covered:** scratchpad `0x1F800000-0x1F8003FF` and all KSEG0 RAM — those keep the
[[legitimate-volatile-interrupt-touched]] two-prong gate. `*(volatile T *)&D_xxx` casts on game RAM, alias
renames and macro-hidden asm stay forbidden ([[inline-asm-policy]]).

**Covered spellings** (address must be verifiable — a literal, a cited `.data` word, or a documented symbol):
`volatile T* p = (T*)0x1F801xxx;`; computed `*(volatile T *)(BASE + k)`; `extern volatile T D_xxx;` resolving
into the range.

**Requirements:**
1. Cite how the address lands in the range (e.g. `D_8009BD88 = .word 0x1F801070, asm/data/7D920.data.s:<line>`).
2. No FAKE annotation (correct typing); a `/* I_STAT */`-style register comment is encouraged.
3. `extern volatile` spellings still go through `volatile_extern_allowlist.txt`:
   `# <sym> — MMIO <register> 0x1F801xxx (type-level grant, mmio-volatile-type-level)`.

Volatile MMIO typing is not a codegen wand: a remaining gap with correct typing is ordinary pure-C work.

Example: `volatile s32 *base = (volatile s32 *)D_8009BD68; base[1] = base[1] | (&table)[v];` (I_MASK RMW,
func_80078B3C).
