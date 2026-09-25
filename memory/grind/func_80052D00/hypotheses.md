# func_80052D00 — hypotheses / measured alternatives (2026-09-25)

- `slope` local (`slope = div; ...; W->unk7C = slope; W->unk64 -= (W->unk60 * slope) >> 12;`)
  scored 0 but is NOT needed: placing `W->unk7C = (W->unk74 << 12) / W->unk70;`
  after the +0x90 count statement also scores 0 with no local. Landed form has no local.
- Slope statement first (before the three `+= 1000`), no local: 28 (387/385). Order matters.
- `W->unk78` computed before the `W->unk64 -=` statement: 11. Target reloads +0x7C for
  the +0x78 computation, so the +0x64 store must come first in source.
- `W->unk64 = W->unk64 % 2000 + W->unk78;` (one statement): 1 (384/385) — the target
  stores +0x64 twice, so the two statements are real. `%=` then `+=` = 0.
- Pre-initialised flags (`zdir = 1; if (...) { zdir = -1; ... }`, `swapped = 1; if (...) ... else swapped = 0;`)
  and plain if/else for every flag both score 0; landed if/else (each arm writes once).
- `*(s32 *)&W->unk88 == *(s32 *)&W->unk8C` and the union `W->unk88.w == W->unk8C.w` both
  score 0; landed the union (no cast of a field address).
- Separate break test `W->unk80 = f(); if (W->unk80 != 0) break;`: sandbox 0 but
  oracle FAIL (2 words, thread_jumps retarget; see evidence.md). Landed the
  assignment-in-condition form.
