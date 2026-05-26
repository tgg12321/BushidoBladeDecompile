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
    register s32 v0 asm("v0");                          /* tier-3 pin */
    __asm__ volatile (".word 0x48848800" :: "r"(a0));   /* mtc2 $a0, $17 */
    __asm__ volatile (".word 0x48859000" :: "r"(a1));   /* mtc2 $a1, $18 */
    __asm__ volatile (".word 0x48869800" :: "r"(a2));   /* mtc2 $a2, $19 */
    __asm__ volatile ("nop");                           /* GTE load-delay   */
    __asm__ volatile (".word 0x4B58002D");              /* avsz3            */
    __asm__ volatile (".word 0x48023800" : "=r"(v0));   /* mfc2 $v0, $7     */
    return v0;
}
```

`canonical` reports ASM-PARTIAL (the `.word` cop2 ops ARE recognized as GTE/tier-2).
The target asm (`asm/funcs/<func>.s`) annotates every line "handwritten instruction"
inside `glabel`/`endlabel`.

## Why distance is 1 (and why it's a mirage)

`sandbox --disable all` strips tier-3 inline asm before scoring. Two blocks here
strip: the `register asm("v0")` pin AND the standalone `__asm__("nop")` (a bare-nop
block has no tier-2 instruction, so the classifier calls the whole block tier-3 and
drops it). The **only** resulting diff vs. the canonical build is that missing
GTE load-delay nop -> distance 1. The pin is **not load-bearing**: the stripped
object shows no spurious `move` — GCC returns the `mfc2` result in `$v0` naturally.

So distance-1 is NOT "almost pure C." There is **no pure-C form at all**: `avsz3`/
`avsz4` (GTE Z-average), `mtc2`/`mfc2` (GTE register moves) have no C analog. The
engine just can't represent "needs a canonical nop" as anything but a 1-insn gap.

## Action — PARK (canonical-asm needing user sign-off)

Do **not** grind it in pure C, and do **not** `queue done` it (that would pass —
zero rules + SHA1 already matches — but leave the tier-3 pin in place, violating
Tier-4). The honest finished form is a **tier-1 file-scope `__asm__(glabel ...)`
canonical-asm block** added to `inline_asm_canonical.txt`, which folds the GTE ops
+ the load-delay nop into one canonical block (no pin, no stray-nop tier-3). That
is a canonical-asm retirement -> **needs user authorization**, exactly like the
sibling cluster `func_8007F87C` / `func_8007F5EC` / `motutil_GetWalkDir` (user-
authorized 2026-05-13, see `inline_asm_canonical.txt`).

```
& tools/eng.ps1 queue park <func> --reason "Canonical-asm (hand-written GTE wrapper), no pure-C form; ... needs user sign-off"
```

These come in clusters (avsz3/avsz4 twins, sin/cos kernels) — authorize them
together. Confirmed: `func_8007F10C` (avsz3) parked 2026-05-26; sibling
`func_8007F12C` (avsz4) is the identical case, next in queue.

## Related
- [[jtbl-rodata-split-infrastructure]] — the other "misrouted to `active`, really
  needs park/authorize" class (distance 0, asmfix-only, rodata-split jump table)
- [[canonical-asm-retirement]] — when/how a function legitimately ends as asm
- [[inline-asm-allowed]] — GTE cop2 ops are an accepted DONE state (tier-2)
- [[gte-3x3]] — matching the genuinely-C GTE mvmva wrappers (different: those have
  a real C body around the GTE block; these are pure GTE leaves)
