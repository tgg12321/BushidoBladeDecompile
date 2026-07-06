# func_8001E6E4 — TL;DR

Distance 21 -> 19 (reviewer-approved, `candidate.c` applied). Remaining 19 is a
SINGLE root cause: target's stack frame is 112 bytes with the `CamWork local`
struct starting at sp+24; the honest pure-C reconstruction only produces a
104-byte frame with `local` at sp+16 (8-byte / 2-word gap). Every differing
instruction is just this same +8 sp-offset shift on the exact same opcodes —
there is no other structural divergence anywhere in the function.

## What's confirmed

- **No corresponding dead store exists in target.** Checked byte-by-byte via
  objdump AND via a fresh m2c re-decompile of `asm/funcs/func_8001E6E4.s` —
  zero loads/stores touch sp+0..sp+23 anywhere in the function. The
  `dead-vars-local-array` written-never-read carve-out requires target-side
  dead stores as prerequisite #1; that prerequisite is not met here, so that
  carve-out does not apply.
- **Unreferenced padding arrays are mechanically inert here, not just
  forbidden.** Tested `s32 pre_pad[N];` for N = 2, 4, 20 words (8/16/80
  bytes) — ALL produced byte-identical output with ZERO effect on frame size
  or sandbox score. GCC fully dead-code-eliminates a truly-unreferenced local
  regardless of declared size. So this isn't just a policy violation to
  avoid — it doesn't even work as a mechanical trick for this case.
- **Struct-field address-taking DOES mechanically work** (confirmed: folding
  a `s32 pad_lead[2]` as the FIRST member of `CamWork` and re-pointing the
  call-site pointer to `&local.vx` closed the gap to sandbox 0 / SHA1 ==
  oracle). This mirrors the already-committed `func_8001A538` MATRIX fix
  (`7f37c94c`) mechanically, but **cheat-reviewer FAILed it** because unlike
  MATRIX (a real, externally-verified, multiply-reused PsyQ SDK struct),
  `CamWork.pad_lead` has no such grounding — it's a single-use invented
  struct with no evidence for what those bytes represent. Saved as
  `rejected/dead-vars-local-array_pad_lead.diff`. Do not re-propose this
  exact form without NEW evidence (e.g. discovering the true shared struct
  elsewhere in the codebase with externally-verifiable field semantics for
  those leading 8 bytes).

## Sibling clue (not this function, but relevant)

`func_8001E404` (same file, same CamBuf/CamWork idiom, still in the queue at
distance 23 despite a 2026-05-26 commit message claiming full SHA1 match) has
the IDENTICAL unresolved 8-byte-short frame with an unreferenced
`s32 pre_pad[2];` already sitting in the committed tree. Since both siblings
need the same +8 leading reservation, whatever the TRUE answer is (a real
shared struct? a different original decomposition entirely?) likely closes
both at once. Worth a joint investigation session across both functions
rather than treating them independently. Flagged to orchestrator, not
resolved here (out of this worker's assigned scope).

## Next lever to try (not yet attempted)

- Search the decomp.me corpus (`tools/decomp_me_scrape.py`) for scratches
  matching this exact camera-struct-pass-by-address-to-3-callees shape —
  might surface the real original struct layout used by another PSX/PsyQ
  title targeting the same toolchain class.
- Check whether `D_800F5328`/`D_800F6608` (the two source tables `s2` can
  point to) are themselves declared with a known struct type elsewhere that
  reveals genuine leading fields (haven't checked their OWN declared type/
  size beyond raw pointer arithmetic).
