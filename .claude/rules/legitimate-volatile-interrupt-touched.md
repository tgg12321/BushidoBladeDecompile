---
name: legitimate-volatile-interrupt-touched
paths: [".claude/rules/legitimate-volatile-interrupt-touched.md"]
# on-demand only: surfaced via codegen-technique-index (auto-loads on src/*.c)
description: "NARROW SANCTIONED CARVE-OUT (user policy 2026-06-08): `extern volatile T G;` is allowed ONLY for globals asynchronously mutated by an identifiable IRQ/MMIO writer AND read at a use-site shape that demonstrably requires CSE-defeat (spin-wait / double-read-across-sequence-point / IRQ-mutated-loop-bound). SOTN-grounded. Does NOT relax the broader `volatile`-coercion ban in [[inline-asm-policy]] for any other case."
metadata:
  type: rule
---

# The `extern volatile T G;` narrow exception (IRQ-touched globals)

> **SCOPE AMENDMENT (owner ruling 2026-07-01):** for addresses in the
> PSX hardware I/O-register range (`0x1F801000-0x1F802FFF`), volatile is
> now TYPE-LEVEL hardware semantics — no use-site shape test, all read
> shapes including single-read probes — per [[mmio-volatile-type-level]]
> (SOTN declares register pointers volatile at the declaration; evidence
> in [[sotn-family-research-2026-07-01]]). THIS rule's two-prong +
> three-shape criterion now governs GAME-STATE memory only (KSEG0 RAM,
> scratchpad). Everything below is unchanged for that class.

**This rule documents a NARROW carve-out to the default ban on
`extern volatile T D_xxxxxxxx;` declarations codified in
[[inline-asm-policy]] (expanded catalog, 2026-05-31). It is deliberately
scoped to a SPECIFIC class of memory under a SPECIFIC class of use-site
constructs, and exists ONLY because SOTN ships the construct openly for
exactly this purpose. It is NOT a precedent for any other
codegen-coercion device, nor a general relaxation of the volatile ban,
nor a justification for `*(volatile T *)&G` inline casts on globals
that fall outside the carve-out. The default posture for
`extern volatile T D_xxxxxxxx;` REMAINS FORBIDDEN; this rule documents
the specific exception, not a relaxation of the default.**

## Sibling carve-out precedent

This is the SECOND time the project has added a narrow exception to a
default-forbidden cheat catalog based on SOTN-master-branch evidence —
the first was [[do-while-zero-exception]] (2026-06-04). Both carve-outs
share the same shape: (1) SOTN-master-branch direct evidence, (2) a
strictly-scoped trigger criterion, (3) the cheat-reviewer agent is the
strictness mechanism, defaulting to FAIL. The fact that this rule
exists does NOT lower the bar for the next proposed exception; each
must be researched and sanctioned on its own evidence.

## The two-pronged criterion (BOTH must hold; neither alone sufficient)

A `extern volatile T G;` declaration on a `D_xxxxxxxx` global is
sanctioned ONLY when BOTH of the following hold:

1. **G is asynchronously mutated by hardware or an IRQ handler.**
   Concretely: G's address appears as a write target inside a function
   installed via `InterruptCallback` / `VSyncCallback` /
   `EnterCriticalSection`-wrapped callback (or a function called from
   such an installed handler); OR G is an MMIO address in the
   PSX-mapped hardware range (`0x1F8xxxxx`). **The worker MUST cite
   the writer's source location (function name + file:line); a bare
   assertion that the global "is touched by an interrupt" is NOT
   acceptable evidence.**

2. **The use-site source exhibits a construct the C standard permits
   to be CSE'd / hoisted into a use-once read, but where the IRQ
   writer can change G between the would-be-merged reads.**
   Concretely, the use-site shape is ONE OF (exact list, not
   illustrative):
   - **spin-wait:** `while (G ...) {}` (or `do { } while (G ...);`,
     `for (; G ...; ) {}`) — the loop has no body work besides the
     condition test, and the IRQ writer's mutation is the only way the
     loop terminates.
   - **double-read-across-sequence-point:** `x = G; foo(); y = G;`
     where `foo()` is a sequence point the compiler may treat as
     opaque, but where the worker can demonstrate G changes between
     reads (typically because `foo()` triggers or enables the IRQ).
   - **IRQ-mutated-loop-bound:** `for (i = 0; i < G; ...)` where G is
     IRQ-mutated during the loop (the IRQ shortens / lengthens the
     iteration count).

   Other use-site shapes default-FAIL pending fresh SOTN research.

## Required commit-message format

A commit landing an `extern volatile T G;` under this carve-out MUST
include the following block in the body (the cheat-reviewer's audit
verifies each field):

```
Match: <func> (<file>.c) — extern volatile s32 <G> for IRQ-touched global

IRQ writer: <function>():<file>:<line> — installed via <SysSetCallback|VSyncCallback|InterruptCallback|EnterCriticalSection|MMIO 0x1F8xxxxx>
Use-site construct: <spin-wait|double-read-across-sequence-point|IRQ-mutated-loop-bound>
Cheat-reviewer: PASS (criteria 1 and 2 independently verified)
```

The reviewer cross-checks each field against the actual source: the
cited IRQ-writer function must exist at the cited file:line and must
actually write G; the cited use-site construct must exist in the
modified C source at the location implied by the function name.

## What is NOT covered (forbidden generalizations)

This exception does NOT sanction:

- **Non-IRQ-touched globals.** Plain game-state memory mutated only by
  ordinary (non-callback) code is NOT covered — it's standard pure-C
  territory. The detector's `D_xxxxxxxx` plain-volatile ban applies
  unchanged. "Most games touch most state from many places" is NOT a
  generalization of this rule.
- **"I think this might be IRQ-touched" without citation.** The IRQ
  writer must be NAMED with a function name AND a file:line. Hand-wave
  evidence ("by analogy to the libcd pattern", "it looks like the
  hardware writes this") defaults to FAIL.
- **Use-sites WITHOUT a spin-wait / double-read / IRQ-mutated-loop-bound
  shape.** A read that GCC would not CSE / hoist in the first place
  does not need volatile to behave correctly — adding volatile in that
  case is the same CSE-defeat cheat the broader ban exists to prevent.
  The reviewer FAILs if the use-site construct doesn't match one of
  the three documented shapes.
- **Other syntactic spellings of CSE-defeat.** `*(volatile T *)&G`
  inline casts on non-IRQ globals stay forbidden (the case 2 detector
  in `engine/volatile_cheats.py` is unchanged). Alias-renames
  (`extern volatile T G_v asm("G");` with name != sym) stay forbidden.
  Macro-hidden `__asm__` blocks stay forbidden. The narrow carve-out
  here covers ONE specific syntactic spelling
  (`extern volatile T D_xxxxxxxx;` scalar) under ONE specific class of
  use sites; everything else in the expanded cheat catalog is
  unchanged.
- **Implicit relaxation of the broader volatile-coercion family.** The
  `volatile`-as-CSE-defeat policy from [[inline-asm-policy]] expanded
  catalog (2026-05-31) is unchanged for everything outside the
  two-pronged criterion above. The cheat-reviewer enforces the SAME
  family check by default; this rule is the SINGLE published
  exception.

## SOTN evidence (2026-06-07 research)

The carve-out is grounded in direct SOTN master-branch evidence — 8
distinct instances of `extern volatile T G;` on IRQ-touched globals,
all matching the two-pronged criterion above. The contrast — SOTN's
IRQ handler that mutates 5 globals declared plain `u32` — proves SOTN
does NOT prophylactically qualify all IRQ-touched memory; volatile is
applied ONLY at use-sites that demonstrably require CSE-defeat.

**Sanctioned instances:**

- `libetc/vsync.c:8,55` — `extern volatile s32 Vcount;`. VBlank IRQ
  counter. Use-site: `while (Vcount < count) {}` spin-wait — the
  exemplar of pattern (1).
- `libcd/c_009.c:9` — `extern volatile s32 D_80098894;`. CD-ROM ring
  buffer head, modified by `StCdInterrupt`. Use-site: read-then-call
  (CD callback may advance the head between reads).
- `libcd/c_011.c:8-11,79` — MMIO DMA register spin-waits. MMIO range
  `0x1F8xxxxx`; use-site is a polling spin-wait on the DMA
  status bit.
- `dra/dra_bss.h:86` + `sound.c:391` — IRQ-mutated loop bound. Use-site
  shape is `for (i = 0; i < G; i++)` where the sound IRQ updates G
  during the loop.
- `dra/dra_bss.h:114` — switch state-machine cursor advanced by CD
  callback. Use-site is a re-read of the cursor across a sequence
  point (the cursor advances on CD completion).

**Contrasting evidence (volatile NOT applied):**

`src/dra/47BB8.c` `VSyncHandler` mutates 5 globals from inside an IRQ
handler, but ALL 5 are declared plain `u32` (not volatile). The
read-sites of those 5 globals do NOT need CSE-defeat — they're
single-read or the reads occur with EnterCriticalSection-bracketed
critical sections. This is the proof that SOTN's rule is the
two-pronged criterion above, NOT "IRQ-touched ⇒ always volatile".

Full research memo: `memory/project/sotn-volatile-interrupt-research-2026-06-07.md`.

## How the strictness mechanism works (four layers)

1. **The default ban remains in place.** [[inline-asm-policy]]'s
   expanded catalog still forbids `extern volatile T D_xxxxxxxx;`
   by default. The engine's `engine/volatile_cheats.py` detector
   still detects it.
2. **The allowlist (`volatile_extern_allowlist.txt`) lists symbols
   that bypass the engine detector.** Adding a symbol to the allowlist
   is a structured artifact tracking each sanctioned use; the file is
   committed and visible.
3. **The cheat-reviewer agent (`.claude/agents/cheat-reviewer.md`)
   audits each addition.** Its family check (test #5) defaults to
   FAIL unless the commit message documents (a) the IRQ-writer citation
   and (b) the use-site construct name from the three-element list.
   The reviewer cross-checks each claim against the actual source.
4. **The commit message preserves the audit trail.** Even after the
   commit lands and the allowlist file shows the new entry, the
   commit message documents WHY the entry was added — making future
   audits straightforward.

The four layers together: detector backstops syntax, allowlist tracks
sanctioned uses, cheat-reviewer enforces the criterion, commit message
preserves the audit. Any one of them in isolation is insufficient.

## Confirmed cases

- **D_8009BF7C** (display.c, granted 2026-06-11) — GPU packet ring
  read-index. IRQ writer: `func_8007D6D8` (writes the symbol,
  asm/funcs/func_8007D6D8.s:131) installed via
  `irq_AcknowledgeVblank(2, func_8007D6D8)` (src/display.c:890).
  Qualifying use-site: `func_8007DB20`'s
  `while (D_8009BF78 != D_8009BF7C) { ... }` spin-wait (display.c:980).
  Exact SOTN analog: `libcd/c_009.c`'s `extern volatile s32 D_80098894`
  (CD-ROM ring buffer head advanced by `StCdInterrupt`). Bonus
  mechanism evidence: target asm STORES zero to the symbol and
  immediately RELOADS it (func_8007D9C4 head) — codegen only producible
  from a volatile declaration. Granted via the user's standing
  three-question test; unblocked func_8007D9C4 to COMPLETED-C with
  HEAD's unchanged source.

- **g_sys_dma_region / D_800A2634** (ings2.c, granted 2026-06-12) — VSync
  IRQ frame counter. IRQ writer: `D_800832F8()` does `++D_800A2634`
  (src/ings2.c:338), installed via
  `irq_EnableInterrupts(0, D_800832F8)` (src/ings2.c:330). Qualifying
  use-site: `func_80082A14`'s `do { ... } while (g_sys_dma_region < a0);`
  spin-wait (ings2.c:96-105). Closed sys_VSync to COMPLETED-C
  (commit 1acf14cc) after 7 stuck sessions. **Mechanism note for future
  diagnosis:** GCC 2.7.2 sched.c `read_dependence(mem,x) =
  MEM_VOLATILE_P(x) && MEM_VOLATILE_P(mem)` — a read-read scheduling
  edge exists only when BOTH reads are volatile. If a target block looks
  UNSCHEDULED (strictly source-ordered, with a genuine load-delay nop)
  around reads of an IRQ-touched counter while your build interleaves
  the chains, check whether the SECOND read's symbol is a non-volatile
  handle of memory that qualifies under this carve-out (here the same
  address already had a volatile handle, D_800A2634 at ings2.c:320 — the
  grant unified the second handle, g_sys_dma_region).

- **func_80078B3C / func_80078B70** (text1b_b.c, B3C closed 2026-06-14;
  B70 is the same-pattern sibling) — `*(volatile s32 *)(D_8009BD68 + ...)`
  RMW of the PS1 interrupt-controller I_MASK register. D_8009BD68 = .word
  `0x1F801070` (asm/data/7D920.data.s:23913 — the I_STAT/I_MASK MMIO base;
  +4 = I_MASK). This is the **computed-MMIO-address** category (a pointer
  cast on a hardware-register address), NOT an `extern volatile T D_xxx`
  game-state coercion — `reviewer_precheck` reports `new_extern_volatile:
  []`. **Diagnostic symptom:** a `drain_delay` regfix rule pulling a `sw`
  out of the `jr ra` delay slot on an MMIO-base `base[k] = base[k] | ...`
  RMW — cc1's `fill_simple_delay_slots` fills the slot with the store on a
  NON-volatile pointer, but the target keeps the nop because the original
  access was volatile. Closing form: `volatile s32 *base =
  (volatile s32 *)D_8009BD68; base[1] = base[1] | (&table)[v];`
  (the bit-table `D_8009BD70` stays NON-volatile — plain data, not MMIO).
  Same API family as the COMPLETED siblings func_80078B04 / func_80078BA8
  (root-counter MMIO at D_8009BD6C = 0x1F801100). retire dropped the 1
  drain_delay rule; SHA1 == oracle.

## Symbol-level grants (clarified 2026-06-11, func_8007D9C4 decision)

A grant is PER SYMBOL, not per use-site — matching both C semantics
(volatile is a declaration property; every consumer inherits it) and the
SOTN evidence model (their volatile declarations are file/header-scope).
The three-shape use-site catalog gates whether the SYMBOL qualifies: at
least ONE consumer must exhibit a cataloged shape (with the IRQ-writer
prong verified as always). Once granted, OTHER consumers of the same
symbol (e.g. a store-then-readback at an init site) are collateral of the
declaration and do NOT each need a cataloged shape. They DO inherit the
audit trail: the allowlist comment names the qualifying consumer.

## How to use the exception when it applies

If you believe the two-pronged criterion applies to a function you're
working on:

1. **Identify the IRQ writer.** Grep for the global's symbol used as a
   write target inside functions that are themselves installed via
   `SysSetCallback` / `VSyncCallback` / `InterruptCallback` /
   `EnterCriticalSection` / `CdCallback` / etc. Cite the function name
   and file:line.
2. **Identify the use-site construct.** Confirm the read shape matches
   ONE of the three documented forms above. If your use site is some
   other shape, the carve-out does NOT apply — re-park or surface a
   policy question.
3. **Draft the commit message** in the exact format documented above.
4. **Invoke the cheat-reviewer** ([[review-discipline-before-commit]]).
   The reviewer will FAIL if either citation is missing, hand-wave-y,
   or unverifiable in the actual source. Burden of proof is on the
   worker, not on the reviewer.
5. **Add the global's symbol to `volatile_extern_allowlist.txt`** in
   the same commit. The engine detector reads this file at run-time
   to bypass the listed symbols. Format: one symbol per line, with a
   trailing comment naming the function and IRQ writer (free-form, but
   convention is `# <func> — IRQ writer: <writer>:<file>:<line>`).
6. **Reference this rule in the commit message** so the policy basis
   is linked from git history.

## Related rules

- [[inline-asm-policy]] — the expanded ban this rule narrowly amends.
  The default forbid remains; this rule documents the SOLE exception.
- [[do-while-zero-exception]] — sibling carve-out, same model
  (SOTN-grounded, narrow trigger, cheat-reviewer enforces FAIL-default).
- [[review-discipline-before-commit]] — the cheat-reviewer architecture;
  its family check honors this exception only when the commit-message
  audit fields are present and verifiable.
- [[no-new-park-categories]] — the broader "cheats by any spelling"
  policy this carve-out narrowly amends; the policy posture is
  unchanged for everything outside the two-pronged criterion.
- [[inline-asm-injection]] — sibling family-level coercion (alias
  renames, `*(volatile T *)&G` casts) that stays forbidden under this
  rule; the carve-out does NOT extend to those spellings.
