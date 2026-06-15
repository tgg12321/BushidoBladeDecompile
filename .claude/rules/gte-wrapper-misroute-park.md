---
name: gte-wrapper-misroute-park
paths: ["src/display.c", "src/code6cac.c"]
description: "Hand-written GTE leaf wrappers (avsz3/avsz4, mtc2/mfc2) misroute to the active queue as ASM-PARTIAL distance-1; the gap is only the stripped GTE load-delay nop and the pin is non-load-bearing — park as canonical-asm needing user sign-off, don't grind in pure C."
metadata:
  type: reference
---

# GTE leaf wrappers misroute to `active` with distance 1 — park as canonical-asm, don't grind

## Symptom

A `queue next` top item is a tiny GTE leaf function: verdict **ASM-PARTIAL**,
**distance 1**, **0 regfix/asmfix rules**, sitting in the `active` bucket. The
body is a hand-written GTE sequence written as per-instruction `.word` asm, e.g.:

```c
s32 func_8007F10C(s32 a0, s32 a1, s32 a2) {
    register s32 v0 asm("v0");                          /* cheat-asm pin */
    __asm__ volatile (".word 0x48848800" :: "r"(a0));   /* mtc2 $a0, $17 */
    __asm__ volatile (".word 0x48859000" :: "r"(a1));   /* mtc2 $a1, $18 */
    __asm__ volatile (".word 0x48869800" :: "r"(a2));   /* mtc2 $a2, $19 */
    __asm__ volatile ("nop");                           /* GTE load-delay   */
    __asm__ volatile (".word 0x4B58002D");              /* avsz3            */
    __asm__ volatile (".word 0x48023800" : "=r"(v0));   /* mfc2 $v0, $7     */
    return v0;
}
```

`canonical` reports ASM-PARTIAL (the `.word` cop2 ops ARE recognized as canonical GTE asm).
The target asm (`asm/funcs/<func>.s`) annotates every line "handwritten instruction"
inside `glabel`/`endlabel`.

## Why distance is 1 (and why it's a mirage)

`sandbox --disable all` strips cheat-asm before scoring. Two blocks here
strip: the `register asm("v0")` pin AND the standalone `__asm__("nop")` (a bare-nop
block has no canonical-asm instruction, so the classifier calls the whole block cheat-asm
and drops it). The **only** resulting diff vs. the canonical build is that missing
GTE load-delay nop -> distance 1. The pin is **not load-bearing**: the stripped
object shows no spurious `move` — GCC returns the `mfc2` result in `$v0` naturally.

So distance-1 is NOT "almost pure C." There is **no pure-C form at all**: `avsz3`/
`avsz4` (GTE Z-average), `mtc2`/`mfc2` (GTE register moves) have no C analog. The
engine just can't represent "needs a canonical nop" as anything but a 1-insn gap.

## Status (2026-05-26): the misroute is FIXED + the action is now AUTO-authorize

Two changes landed so this class no longer wastes a worker run:

1. **Gate fix** — `canonical._verdict` now excludes structural `nop`/`jr` from the
   ASM-WHOLE denominator, so a pure GTE leaf classifies **ASM-WHOLE → `authorize`**
   (not ASM-PARTIAL → active). It is never handed to a worker.
2. **Auto-authorize policy** (user, 2026-05-26) — GTE leaf wrappers are pure cop2
   ops with **no C form** (zero ambiguity), so the orchestrator **authorizes them
   itself**, no escalation. (Contrast canonical-body *hand-coded* asm, which still needs a
   user judgment call.)

   **Boundary clarification (2026-06-10, from the func_80052754 retro-audit):**
   the auto-authorize scope covers cop2 ops + MECHANICAL packaging only --
   loads/stores that exist solely to feed or drain the hardcoded GTE register
   encodings (e.g. `lw $t0..$t2` before a `ctc2`, `swc2` to an out-pointer).
   A leaf whose body includes **general-purpose COMPUTATION with a direct C
   form** (e.g. `addu` sums of mfc2 outputs -- `return t0 + t1 + t2;`) is NOT
   in the zero-ambiguity class: it needs explicit user sign-off. The
   distinction is input/output PACKAGING (no C can target those registers)
   vs output ARITHMETIC (C exists; whether the original was hand-written is
   a judgment call). Workers must NOT re-adjudicate their own NEEDS_USER on
   this boundary -- escalate it. (`func_80052754` itself was explicitly
   user-approved 2026-06-10 after the retro-audit surfaced the pathway
   violation.)

## Action — auto-authorize + convert to clean canonical inline asm

The finished form is **pin-removed canonical inline asm GTE**, NOT a glabel block:

1. **Remove the `register s32 v0 asm("v0")` pin** → plain `s32 v0;`. The pin is
   the only cheat-asm debt; GCC returns the `mfc2` result in `$v0` naturally (the
   stripped sandbox already showed no spurious move). The `.word` cop2 ops stay
   (canonical inline asm); the load-delay `nop` stays (canonical GTE timing).
2. **`verify-oracle --rebuild`** → SHA1 must still == oracle (it does; pin was
   non-load-bearing).
3. **Add the function to `inline_asm_canonical.txt`** (the canonical-asm registry)
   with a one-line GTE justification.
4. **`& tools/wteng.ps1 main queue done <func>`** (0 rules + SHA1 match → done).

Confirmed: `func_8007F10C` (avsz3) + `func_8007F12C` (avsz4) auto-authorized this
way 2026-05-26 — pins removed, SHA1 == oracle, both in `inline_asm_canonical.txt`,
both `queue done`. (NB: a canonical-body file-scope glabel block is an alternative finished
form, but pin-removal → canonical inline asm is lower-risk — no maspsx `.set` handling needed.)

## Related
- [[jtbl-rodata-split-infrastructure]] — the other "misrouted to `active`, really
  needs park/authorize" class (distance 0, asmfix-only, rodata-split jump table)
- [[canonical-asm-retirement]] — when/how a function legitimately ends as asm
- [[inline-asm-allowed]] — GTE cop2 ops are an accepted DONE state (canonical inline asm)
- [[gte-3x3]] — matching the genuinely-C GTE mvmva wrappers (different: those have
  a real C body around the GTE block; these are pure GTE leaves)
