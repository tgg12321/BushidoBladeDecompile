---
name: reload-spill-reg-reveals-asm-clobbers
description: "A reload-emitted scratch register at an unexpected regno (target `mfhi $t8` where our build emits `mfhi $13`) is EVIDENCE, not a diff: reload1.c puts every explicitly-mentioned hard reg into bad_spill_regs and then picks spill regs ascending among unused call-clobbered regs. The skipped regs prove the original TU mentioned them at asm level — reconstruct them as an inline-asm clobber list, never as a regfix subst."
paths: [".claude/rules/reload-spill-reg-reveals-asm-clobbers.md"]
# on-demand only: surfaced via codegen-technique-index (auto-loads on src/*.c)

metadata:
  type: reference
---

# The reload spill register is a fingerprint of the original source's asm clobbers

> **Historical framing note (2026-08-30):** this rule predates the removal of the
> regfix/asmfix rule system (retired at zero rules; machinery deleted). Where the
> symptom text says a function "carries a rule", read it as "the honest build shows
> this diff shape vs target". The technique itself is unchanged.

## Symptom

Near a canonical inline-asm island (GTE, BIOS trampoline, hand-written block)
your build emits a compiler-generated scratch register that the target spells
differently, and the difference is **only** the register number:

```
ours:    mfhi $13        sra $3,$13,5
target:  mfhi $t8        sra $v1,$t8,5     ($t8 == $24)
```

Nothing about the C changes this. The standing temptation is a `regfix.txt`
`subst "mfhi\s+\$13" "mfhi\t$24"` — that is the cheat this rule replaces
(func_8002BEA0 carries exactly that pair at `regfix.txt:469-470`).

## Mechanism (`tools/gcc-2.7.2/reload1.c`, frozen compiler)

`order_regs_for_reload()` decides which hard registers reload may spill into:

- **lines 3644-3663** — every hard register in `regs_explicitly_used[]` is added
  to `bad_spill_regs`, i.e. reload refuses to spill into it. (The
  `#ifndef SMALL_REGISTER_CLASSES` guard around this is **live** on MIPS:
  `mips.h` defines neither `SMALL_REGISTER_CLASSES` nor `REG_ALLOC_ORDER` —
  confirmed by grep.)
- **lines 3690-3694** — the remaining spill candidates are ordered **ascending
  by regno** among the unused call-clobbered registers.

`regs_explicitly_used` is populated from **explicit hard-register mentions in
the RTL**, and inside a compiled TU the only construct that mentions a hard
register explicitly is an **`__asm__` block's register list / clobber list**.

So the arithmetic runs backwards, and it closes:

- With only `$12` mentioned, the first free call-clobbered reg is `$13` (`$t5`)
  -> our build's `mfhi $13`.
- With `$12`-`$15` all mentioned, `$13`-`$15` are bad spill regs and `$16`-`$23`
  are callee-saved, so the next candidate is **`$24` (`$t8`)** -> the target's
  `mfhi $t8`.

## Why this is evidence and not a coincidence

The deduction is closed, not circumstantial (func_8002BC68 ruling, 2026-07-28):

1. `$13`-`$15` have **zero pseudo uses** anywhere in the target function — any
   pseudo use would have ranked them by use-count and changed other bytes too.
2. They are not fixed regs, not the frame pointer, not eliminable.
3. Explicit RTL mention is therefore the **only** route by which the frozen
   compiler can skip them — and only asm-level constructs mention hard regs.

Conclusion: the original translation unit provably named `$13`-`$15` at asm
level in that function. Writing them back is **reconstruction of the original
source's register footprint**, not a steer — omitting them makes our TU less
faithful, not more.

## What to write

Widen the **clobber list of the canonical island that is already authorized**,
in the existing island's own spelling:

```c
__asm__ volatile(
    "addu  $t4, %1, $zero\n"
    "mtc2  $t4, $30\n"
    "nop\n"
    "nop\n"
    "addu  $t4, $sp, $zero\n"
    "swc2  $31, 0($t4)\n"
    : "=m"(sp_tmp) : "r"(v) : "$12","$13","$14","$15");
```

A conservative `t4`-`t7` clobber set is ordinary human practice for a hand-asm
GTE macro, so it passes the human-writable test.

## Bounds — what this does NOT license

- It is **not** a pure-C technique and extends no SOTN pure-C family. It applies
  only where the residual is provably unreachable from C and the island is
  already routed ASM-PARTIAL / already authorized.
- A clobber list may **only** be widened to registers the bytes force. Adding
  clobbers to steer allocation where the target's reload temp does *not*
  demonstrate the skip is a register pin in clobber spelling — the ordinary
  cheat.
- It never enlarges the island itself. Address arithmetic on a C-visible value
  belongs in C, not in the template ([[inline-asm-injection]]; this is the exact
  ground on which func_8002EA24's broader block was refused on 2026-07-30).

## Confirmed cases + one UNSPENT grant

- **`func_8002BC68`** (src/code6cac_b.c) — grant applied; the mandated comment
  sits at `src/code6cac_b.c:773-782`. Function is off the queue.
- **`func_8002BEA0`** (src/code6cac_b.c) — **the grant is EXPLICITLY IN SCOPE and
  has never been spent.** The 2026-07-28 ruling names it by line: "func_8002BEA0's
  regfix.txt lines 607-608 carry the identical `mfhi $13 -> $24` subst cheat this
  ruling retires." Those rules are live today at `regfix.txt:469-470`
  (line numbers drifted), and the queue entry is **parked** with a `park_reason`
  written BEFORE the ruling that never mentions it. Anyone picking this function
  up should retire that subst pair via the clobber list first, then work the
  genuinely-remaining floor-4 residual (a pure-C RA/strength-reduce diff in the
  `/100` division subtree, outside the GTE block).

Ruling text: `docs/grind/decisions.md:1845` (commit `104fc679`). Sibling
precedent for the island itself: `func_8001A67C`, `func_800274BC`
(`inline_asm_canonical.txt:266-267`, owner-authorized 2026-06-10).

## Related
- [[canonical-asm-authorization-recipe]] — how to author the island the clobber
  list attaches to.
- [[inline-asm-injection]] — the line this rule must not cross; the clobber list
  is licensed by the target's own reload behaviour, template text never is.
- [[no-compiler-divergence]] — names canonical-asm authorization as the sole
  legitimate escape for a provably no-C-form residual.
- [[cop2-addressing-preamble-cluster]] — the neighbouring GTE island family in
  the same file.
