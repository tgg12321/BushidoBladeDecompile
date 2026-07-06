# replay_camera_Init — WIP TL;DR

**Status:** honest pure-C distance 17 (down from HEAD's 18), 0 cheat constructs, 1 residual regfix rule (`fill_delay @ 26 <- 15`). NOT closable to 0 yet.

## What HEAD had (and why this is on the queue)
HEAD's committed body byte-matches the oracle but ONLY via 2 register-asm pins
(`saved_a1 asm("$7")`, `s0 asm("$8")`) + a bare `__asm__ volatile("":::"memory")`
scheduling barrier — all diagnostic-only/forbidden (register-asm-pins.md,
inline-asm-policy.md). That's why it's INCOMPLETE despite SHA1==oracle at HEAD.

## What I found
1. Removing the pins/barrier alone keeps distance at 18 (same as before) — the
   pins/barrier were score-inert for the honest metric anyway (as expected).
2. Restoring `s16 *s0 = &D_80101E62;` (a plain, non-register pointer cache,
   matching sibling `func_80036FD4`'s idiom) makes GCC cache the address once
   and reuse it for both the initial `*s0 != 0` check and the final `*s0 = 2`
   store — matching target's use of ONE register ($t0 in target) throughout.
3. **The core remaining gap is a register-rotation/scheduling wall.** Target's
   compiled form:
   - preserves the incoming `a1` into a fresh register (`$a3`) in the branch's
     delay slot, unconditionally, right at function entry
   - this frees `$a1`'s hardware register for the `SpecialCam` load (`cam_val`)
   - which in turn means the store to the (legitimately volatile, pre-existing)
     `D_80101E70` global is followed by a GENUINE re-load instruction (not
     folded), because by the time of the read, the register holding the
     just-stored value has already been reused for `D_8008EC38` doesn't apply — see the raw asm; the reload is real and uses a DIFFERENT register than the store).
4. I confirmed (2) is the load-bearing mechanism by DIAGNOSTIC-ONLY use of
   `register s32 saved_a1 asm("$7") = a1;` (never committed) via
   `sandbox --disable none --keep-cheat-asm`: with JUST that pin, build_insns
   became 39 (== target) and the reload appeared naturally, score dropped to 11.
   This PROVES the pin's target register assignment is what target's original
   compile produced — but per register-asm-pins.md this is diagnostic-only.
5. I could NOT find a pure-C structure that makes GCC choose this allocation
   without the pin. Tried: param-reuse (`a1 = ...; D_80101E6C = a1;`), plain
   `saved_a1` locals (before/after other decls), the sibling's shared-pointer
   trick (ruled out — target uses two SEPARATE symbol relocations for
   SpecialCam/D_8008EC38, confirmed via `objdump -dr`, so the shared-pointer
   shape doesn't apply here even though the two globals ARE adjacent in memory).
6. One reordering DID help: moving `D_80101E7C = a1;` to sit texually between
   the two loads let GCC's scheduler place it (using `a1` directly, no pin
   needed) at almost the right spot, dropping distance 18 -> 17. This is
   captured in `candidate.c`.

## Cheat-reviewer history
- Pass 1 (2026-07-05) FAILed an earlier version of this candidate that kept
  `s16 *s0 = &D_80101E62;` as a plain pointer-caching idiom (matching sibling
  `func_80036FD4`'s shape). Reviewer classified it as an unannotated
  pointer-alias-fake-exception shape (no lever-exhaustion doc, no named
  mechanism, no `/* FAKE */` annotation) and FAILed on that ground alone —
  everything else passed.
- Fix: removed the alias entirely, replaced with two direct `D_80101E62`
  accesses. Re-measured: distance UNCHANGED at 17 — the alias was not
  load-bearing for the score, so there was no reason to justify/annotate it.
  `candidate.c` now reflects this simpler, alias-free form.
- Pass 2 re-review requested on the corrected candidate; see meta.json for
  the recorded verdict.

## Rejected forms (do not re-derive)
See `meta.json.rejected_forms` for the full list with reasoning. Highlights:
- `*(volatile s32 *)&D_80101E70` cast at the read site — FORBIDDEN (case-2
  CSE-defeat cast, matches inline-asm-injection / legitimate-volatile-interrupt-touched
  ban; D_80101E70 has no IRQ-writer citation so doesn't qualify for the narrow carve-out).
- `volatile s32 *ecp = &D_80101E70;` pointer-indirection variant — empirically
  DOES force the reload, but is the same forbidden-by-spirit CSE-defeat family
  (no IRQ justification). Do not resurrect without a genuine two-pronged citation.

## Next steps for whoever resumes this
- The remaining ~17-point gap is almost entirely the a1/a3 register-rotation
  wall + the consequent lack of a real D_80101E70 reload. This matches the
  "register rotation" wall class documented in register-alloc-pure-c.md
  (Confirmed limits section) — consider a proper decomp-permuter run (not
  attempted this session — infra setup was deprioritized in favor of manual
  structural analysis) before declaring this un-closable in pure C.
- Do NOT re-try the rejected forms above.
- Apply `candidate.c`, confirm floor with `sandbox --disable all` (expect 17),
  then continue from there.
