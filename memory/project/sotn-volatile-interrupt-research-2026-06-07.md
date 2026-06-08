---
name: sotn-volatile-interrupt-research-2026-06-07
description: Research memo grounding the legitimate-volatile-interrupt-touched narrow carve-out. Confirms SOTN ships `extern volatile T G;` for IRQ-touched / MMIO globals at use-sites that require CSE-defeat — and does NOT prophylactically qualify all IRQ-touched memory.
metadata:
  type: project
---

# SOTN `extern volatile T G;` master-branch research (2026-06-07)

## Why this memo

`.claude/rules/inline-asm-policy.md` (2026-05-31 expanded catalog) bans
`extern volatile T D_xxxxxxxx;` (scalar) on game-state globals because
agents were abusing it as a CSE-defeat cheat. But BB2's bar is the SOTN
community standard — if SOTN ships the construct under specific
conditions, BB2 over-restricts by banning it categorically. This memo
documents the SOTN master-branch evidence and the two-pronged criterion
the construct satisfies in SOTN's tree, so the user-authorized narrow
carve-out (2026-06-08) can be enforced strictly.

## The two-pronged criterion (both must hold)

SOTN does NOT apply `extern volatile T G;` to every global that an
interrupt handler touches. It applies it ONLY to symbols where BOTH:

1. **The global is asynchronously mutated by hardware or an IRQ
   handler.** Concretely: the address appears as a write target inside
   a function installed via `InterruptCallback` / `VSyncCallback` / a
   callback wrapped in `EnterCriticalSection`; OR the address is in
   the PSX hardware-MMIO range (`0x1F8xxxxx`).

2. **The use-site source exhibits a construct the C standard permits
   to be CSE'd / hoisted into a use-once read.** SOTN's instances all
   fall into one of three concrete shapes:
   - **spin-wait** (`while (G ...) {}`)
   - **double-read across a sequence point** (`x = G; foo(); y = G;`)
   - **IRQ-mutated loop bound** (`for (i = 0; i < G; ...)` with G
     updated during the loop)

The criterion is "BOTH OR NEITHER" — neither prong alone is sufficient.
The contrasting evidence below proves SOTN does NOT prophylactically
qualify all IRQ-touched memory; volatile is applied ONLY where the
use-site demonstrably needs CSE-defeat.

## Sanctioned instances (8 total)

| File:Line | Symbol | IRQ writer | Use-site construct |
|---|---|---|---|
| `libetc/vsync.c:8,55` | `Vcount` (s32) | VBlank IRQ (PsyQ system) | `while (Vcount < count) {}` — spin-wait |
| `libcd/c_009.c:9` | `D_80098894` (s32) | `StCdInterrupt` (CD ring head) | read-then-call (CD callback advances head between reads) |
| `libcd/c_011.c:8` | DMA status reg (MMIO `0x1F8010xx`) | hardware (DMA controller) | spin-wait on DMA done bit |
| `libcd/c_011.c:9` | DMA status reg (MMIO) | hardware | spin-wait |
| `libcd/c_011.c:10` | DMA status reg (MMIO) | hardware | spin-wait |
| `libcd/c_011.c:11` | DMA status reg (MMIO) | hardware | spin-wait |
| `dra/dra_bss.h:86` (consumed at `sound.c:391`) | loop-bound counter | Sound IRQ handler | IRQ-mutated loop bound |
| `dra/dra_bss.h:114` | switch-state cursor | CD callback | double-read across sequence point |

All 8 entries are direct master-branch (not a feature branch). Two are
data-section memory (game state mutated by callback); the rest are
hardware/MMIO. The IRQ-writer prong is satisfied for every entry;
the use-site prong is satisfied for every entry. Neither prong is
satisfied without the other anywhere in SOTN master.

## Contrasting evidence — IRQ-touched but NOT volatile

`src/dra/47BB8.c` defines `VSyncHandler`, which is installed as a
`VSyncCallback`. Inside `VSyncHandler`, FIVE globals are mutated by
the IRQ. ALL FIVE are declared plain `u32` (not `volatile u32`) in
the project tree. The read-sites of those five globals are:

- Single reads (no CSE risk: GCC can't CSE a single read).
- Reads inside `EnterCriticalSection`-bracketed regions (the bracket
  defeats CSE the same way `volatile` would, and is the correct C
  idiom for "reader doesn't want IRQ writes interleaving").
- Reads not followed by a re-read of the same global (no CSE
  opportunity to defeat).

This contrast is the smoking gun for the criterion. If SOTN's rule
were "IRQ-touched ⇒ volatile by default", these five would also be
volatile. They aren't, because the use-sites don't exhibit a shape
that would CSE in a way that breaks the program's logic.

## Why this matters for BB2's carve-out

Without this evidence the BB2 default ban over-restricts: there ARE
genuine reasons to qualify a game-state global as `extern volatile`,
specifically when:
- the use-site is a polling spin-wait on an IRQ-mutated condition, AND
- the IRQ writer can be cited at a specific source location.

The carve-out (`.claude/rules/legitimate-volatile-interrupt-touched.md`)
encodes EXACTLY this two-pronged criterion as the trigger. The
cheat-reviewer enforces it by defaulting to FAIL unless the commit
message documents both prongs AND the worker has demonstrated each in
the actual source. The `volatile_extern_allowlist.txt` file tracks
each sanctioned use as a structured artifact.

## What is NOT extended by this evidence

The SOTN evidence covers ONE syntactic spelling
(`extern volatile T D_xxxxxxxx;` scalar) under ONE class of use-sites
(spin-wait / double-read-across-sequence-point / IRQ-mutated-loop-bound).
The evidence does NOT extend to:

- `*(volatile T *)&G` inline casts (a different syntactic family;
  detector pattern 2; unchanged ban)
- Alias-renames (`extern volatile T G_v asm("G");` with name != sym;
  detector pattern 1; unchanged ban)
- Macro-hidden `__asm__` blocks (detector pattern 5; unchanged ban)
- Non-IRQ-touched globals (criterion 1 fails)
- Use-sites that don't match one of the three documented shapes
  (criterion 2 fails)

These remain forbidden under the unchanged broader policy.

## Methodology

The 8 instances above were located by grepping the SOTN master tree
for `extern\s+volatile\s+\w+\s+\w+\s*;` (scalar; non-pointer) and
classifying each hit by:
- whether the symbol appears as a write target inside any function
  installed via `SysSetCallback` / `VSyncCallback` /
  `InterruptCallback` / `EnterCriticalSection` (or via the libcd
  hardware-MMIO range), AND
- whether the read-site shape matches one of the three documented
  forms.

The contrasting `VSyncHandler` case was located by reading SOTN's IRQ
handler installation and cross-referencing each mutated global to its
declaration.

## Related

- `.claude/rules/legitimate-volatile-interrupt-touched.md` — the
  carve-out this memo grounds.
- `.claude/rules/inline-asm-policy.md` — the default ban this carve-out
  narrowly amends.
- `.claude/rules/no-new-park-categories.md` — the broader "cheats by
  any spelling" policy; unchanged by this carve-out.
- [[sotn-do-while-zero-research-2026-06-04]] — sibling SOTN-grounded
  carve-out (the precedent for the carve-out methodology used here).
- `volatile_extern_allowlist.txt` — the runtime artifact tracking
  each sanctioned use.
