---
name: bridge-signature
paths: ["asmfix.txt"]
description: "As of 2026-05-17, only 1 of 147 bridged functions has a stub-vs-extern signature mismatch (motion_SetExMotion), and it's load-bearing. The original 41 issues from 2026-05-10 have been resolved as the decomp agent retired bridges and properly adopted canonical signatures during each conversion."
metadata:
  type: audit
---

# Bridge signature audit

**Status (2026-05-17):** Bridge count dropped from 209 to 147 since the
original audit. The 41 mismatched-signature issues have been almost
entirely resolved -- only **1 informational issue remains**
(`motion_SetExMotion` in src/text1b.c), and that one is *load-bearing*:
removing the redundant local `extern s32 motion_SetExMotion(s32);` at
src/text1b.c:14571 breaks SHA1, because GCC's codegen at the s32-typed
callers below it depends on the override. Fixing this properly requires
fully decompiling the function (not just touching the signature).

**When to read this:** Before starting real decomp work on any of the 147
remaining `replace_with_asmfile` bridged functions. The C stub signature
is *usually* correct now (the agent has been disciplined about adopting
canonical signatures during conversions), but always re-run
`python3 tools/audit_bridge_signatures.py` to confirm before starting.

## Why this matters (the round-6 trap, generalized)

When we bridged 209 functions across rounds 5-11, every stub got a default
placeholder signature like `void func_XXX(s32 arg0, s32 arg1, s32 arg2, s32 arg3)`.
That signature was never validated against caller usage. The build only
matches SHA1 because:

1. **Stubs in file A**, **callers in file B** → cross-TU. GCC compiles each
   `.c` file independently. The caller's TU uses its own `extern` decl, not
   the stub. If the extern says `s32 func(...)` and the stub says `void
   func(...)`, the caller emits `move $sX, $v0` after the call. The stub's
   asmfile (the real asm) actually does write `$v0`. So the cross-TU
   disagreement is invisible.

2. **Same file, but a forward `extern` decl** above the caller line
   overrides the stub for that one call site (GCC uses the most-recent
   visible decl). 13 functions are kept matching only by this trick.

3. **K&R `()` and varargs `...`** on the stub or extern accept any args.

If a future agent does real decomp on one of these and removes the local
`extern` or "cleans up" the void return, sibling callers in the same TU
regress (the round-6 76-byte shift on `func_800693CC`).

## Audit run / tool

```bash
python3 tools/audit_bridge_signatures.py
```

- Reads `asmfix.txt` for `replace_with_asmfile` entries (209 funcs)
- For each: finds the C stub def, all `extern` decls, all call sites
- Classifies: ACTIVE (would break build), LATENT (suppressed by local extern,
  but stub is misleading), INFORMATIONAL (stub vs cross-TU extern mismatch)
- Writes `tmp/bridge_signature_audit.json` (full per-func evidence)

**Original state (2026-05-10):** 0 ACTIVE, 13 latent, 38 informational
across 41 functions.

**Current state (2026-05-17):** 1 informational issue
(`motion_SetExMotion`), 0 latent, 0 active.  The audit has been
substantially closed out as the decomp agent retired bridges and
properly chose canonical signatures during conversion.  The remaining
issue is *load-bearing* -- attempting to remove the redundant local
`extern s32 motion_SetExMotion(s32);` at src/text1b.c:14571 causes
SHA1 mismatch because GCC's codegen depends on it.  It can only be
resolved by fully decompiling motion_SetExMotion (which is still a
bridged asm function).

The original 41-issue table is preserved below for historical reference,
but the audit JSON at `tmp/bridge_signature_audit.json` reflects the
current state and is what to consult.

## Historical (original 41 issues, 2026-05-10) -- now resolved

These were the latent / informational issues found in the original
audit.  All have been cleared except motion_SetExMotion (which moved
from "informational" classification to a known load-bearing artifact).
The list is kept for blame-trace purposes.

| Function | Current stub | Canonical (from extern + caller use) |
|----------|-------------|-------------------------------------|
| func_8005FC9C | `void(s32, s32)` | `s32(s32, s32)` |
| func_80073728 | `void(s32)` | `s32(GameObj*, s32)` |
| func_8006D808 | `void(s32, s32, s32)` | `void(s32*, s32*, s32*, s32, s32)` |
| func_80055138 | `void(s32, s32)` | `void(s32, s32, s32)` |
| func_8005D814 | `void(s32, s32)` | `s32(s16*, s32, s32, s32)` |
| func_8005C8A8 | `void(s32)` | `s32(s32, s32, s32, s32)` |
| func_8006CFBC | `void(s32, s32, s32, s32)` | `s32(s32)` |
| func_80054604 | `void(s32, s32, s32, s32)` | `void(s32, s32, s32, s32, s32, s32, s32)` |
| func_8007F24C | `void(s32, s32, s32, s32)` | `void(s32*, s32*, s32*, s32*, s32*, s32*, s32*, s32*)` |
| func_80052754 | `void(s32, s32, s32, s32)` | `s32(s32, s32, s32)` |

## Informational issues (38) — cross-TU stub vs extern mismatch

These are stub-in-one-file disagreeing with `extern` in another file. The
build still matches because each TU compiles against its own decl. But:

- If you bring the stub up to a real decompilation, **adopt the extern's
  signature**, not the stub's. The extern is what the actual asm function
  honors (verified by the matching build).
- The `display.c` family has a common pattern: `void* func(s32, void*)`
  stubs that callers in `code6cac.c` extern as `void func(s32, s32)`.
  The asm returns a pointer in `$v0` and the calling code in `code6cac`
  ignores it — so the canonical is the void* version.

Full list in `tmp/bridge_signature_audit.json`. Notable patterns:

- **`text1b.c` accumulator returns**: many text1b.c stubs are `void(s32,s32,s32,s32)`
  but the extern in the same file (forward-declared above the caller) is
  `s32 func()` or `s32 func(s32, ...)`. The asm produces values in `$v0`
  used by sibling callers.
- **`s16 *` vs `s32` widening**: `func_80045B68` extern in text1b.c
  declares third arg `s16 *`, stub has `s32`. Real type is `s16 *`.
- **`u8` return narrowing**: several `func_8006XXXX` externs declare
  `u8` return (`func_80068D88`, `func_800678A8`, `func_80067200`,
  `func_80067D14`, `func_80063BD0`, `func_80065800`). Stubs say `void`
  or `s32`. The asm likely zero-extends an 8-bit value; check the
  function's `lbu` / `andi 0xff` epilogue when decomping.

## How to use this when decomping

1. **Before starting** a bridged function, look it up in
   `tmp/bridge_signature_audit.json` (or re-run
   `tools/audit_bridge_signatures.py`).
2. **Adopt the canonical signature** for the new C function — return type
   and arity from the JSON's `inferred_signature` field, exact arg types
   from `extern_evidence` entries.
3. **Caller-audit will block integration** if the new signature still
   doesn't match max-observed caller arity. That's the safety net; the
   audit just tells you what to write in the first place.
4. After matching, **clean up the redundant local extern**: if your new
   signature matches what the local extern said, the extern is now
   redundant and can be deleted (it was only there to override the
   broken stub).

## How to keep this in sync

- Re-run `python3 tools/audit_bridge_signatures.py` after any pure-C
  conversion of a bridged function (the count should drop by 1).
- If counts move in the wrong direction, a new stub was added without
  signature-vetting — investigate.
