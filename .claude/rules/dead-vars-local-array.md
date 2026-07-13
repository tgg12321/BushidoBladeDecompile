---
name: dead-vars-local-array
paths: [".claude/rules/dead-vars-local-array.md"]
# on-demand only: surfaced via codegen-technique-index (auto-loads on src/*.c)
description: "FORBIDDEN as of 2026-05-31 (expanded 2026-06-01) — unused local arrays and (void)&scalar address-coercion. NARROW CARVE-OUT 2026-07-01: a WRITTEN-never-read local array is sanctioned (SOTN dra/62DEC.c ships u8 sp70[4] written 4x/read 0x, twice, in matched core) when the TARGET bytes contain the dead stores (oracle-enforced), after exhaustion, FAKE-annotated, dual-reviewed. OVERSIZED-LOCALS CARVE-OUT 2026-07-13: a locals object with an unwritten tail (written-prefix buffer) or, fallback, a dead pad local is sanctioned when the target frame equation PROVES the original declared locals strictly larger than the bytes it writes — frame-math proof + range annotation + exhaustion + dual review required."
metadata:
  type: reference
  status: forbidden
---

# FORBIDDEN — Frame coercion is a cheat (array AND scalar variants)

> **NARROW CARVE-OUT (owner ruling 2026-07-01):** a local array that is
> **WRITTEN but never read** is sanctioned as a last-resort matching
> construct. Direct SOTN-master evidence (matched, no NON_MATCHING
> markers): `u8 sp70[4]; sp70[2] = sp70[1] = sp70[0] = 0x80; sp70[3] = 0;`
> written-never-read in BOTH `func_80104790` and `func_80105078`
> (`src/dra/62DEC.c` — PSX core overlay), `s16 z[5]` written-never-read
> in `EntityStageNamePopup` (st/chi + st/nz0), amid announced-dead locals
> (`SVECTOR pad;`, `s32 stupid80;`, `s32 unused_interp;`); plus
> `volatile u32 pad[4]; // FAKE` (st/sel/stream.c:80). Prerequisites
> (cheat-reviewer FAILs if missing):
> 1. The TARGET function's bytes contain the corresponding dead stores /
>    frame (the oracle enforces this — the construct cannot fabricate a
>    match where the original had none).
> 2. Documented lever-exhaustion (WIP ledger).
> 3. `/* FAKE: ... */` annotation on the declaration or writes.
> 4. Layer-1 + layer-2 review.
>
> The UNWRITTEN-array form (`s32 buf[N];` never referenced) and the
> `(void)&local` scalar coercion remain FORBIDDEN below — SOTN's
> written-never-read instances all WRITE the array (real stores that
> survive in the emitted bytes); a never-referenced array emits nothing
> and is pure frame-size coercion with no SOTN analog in matched code
> beyond annotated `pad` declarations, which stayed out of scope pending
> their own evidence request. **That evidence request was made and ruled
> on 2026-07-13 — see the OVERSIZED-LOCALS carve-out below.**

> **OVERSIZED-LOCALS CARVE-OUT (owner ruling 2026-07-13):** a stack-locals
> object with an **unwritten tail** (a written-prefix buffer, e.g.
> func_80037540's `s32 sp[8]` with only `sp[0..5]` stored and the buffer
> passed live to a callee) — or, as the fallback when no live object
> exists to extend, a dead pad local — is sanctioned as a last-resort
> matching construct **when the target bytes PROVE the original declared
> it**. Granted on the func_80037540 evidence request that the 2026-07-01
> ruling explicitly anticipated. Evidence packet: census of 356 dead-pad
> declarations in matched reference-project code (SOTN 40 — incl.
> `u8 _pad[40]; // n.b.! needs to be 33-40 bytes (inclusive)`
> (boss/bo4/unk_45354.c:463), documenting the identical
> size-recoverable-only-as-a-range situation; `byte stackpad[40];`
> dra/6BF64.c:156; `s32 unused_stack[2];` unk_45354.c:115), re-verified
> line-by-line against SOTN source on disk by the default-FAIL Judge
> (docs/grind/decisions.md 2026-07-13 12:51).
>
> Prerequisites — the cheat-reviewer / Judge FAILs if ANY is missing:
> 1. **Frame-math proof from the target bytes alone** that the original
>    declared a locals object strictly larger than the bytes it writes:
>    frame size − callee-save bytes − outgoing-args bytes must exceed the
>    bytes actually stored in the locals region, such that the
>    fully-written form yields a strictly SMALLER frame. (func_80037540:
>    0x48 frame − 24 saves − 16 args = 32-byte locals region, only 24
>    written; ALIGN8(24)+16+24 = 0x40 ≠ 0x48 — no fully-written locals
>    set can produce the target frame.) This is what separates the
>    carve-out from ordinary frame coercion: the oracle-checked frame
>    ATTESTS the slack existed in the original. Where the equation does
>    NOT force the slack, declaring it remains FORBIDDEN.
> 2. **Prefer extending a live object.** If a semantically-live locals
>    object exists (an argv-style buffer partially filled and handed to a
>    callee), the unwritten tail extends IT; a separate dead pad local is
>    the fallback only when the function has no live locals object.
> 3. **Range annotation on the declaration** (SOTN `// n.b.!` convention):
>    ALIGN8 makes the declared size recoverable only as a range (25–32
>    bytes ⇒ `s32 [7]` and `[8]` are byte-identical); the annotation
>    states the frame derivation and the range, and names this carve-out.
> 4. **Documented lever-exhaustion** in the ledger/WIP (func_80037540:
>    4 grind sessions, 18,769 permuter samples, 800 frame-band candidates
>    mechanically classified, 0 clean — every alternative used volatile
>    coercion, address-escape, or extra stores absent from target).
> 5. **Layer-1 + layer-2 review** (manual path) or the Grinder's
>    default-FAIL Judge (autonomous path).
>
> Explicitly STILL FORBIDDEN, unchanged by this ruling: volatile-qualified
> dead pads (`volatile char pad;` — volatile coercion; func_80037540's
> byte-matching volatile form stays banked as a cheat in
> memory/grind/func_80037540/rejected/), `(void)&local` scalar coercion,
> `__asm__`-based frame tricks (func_800644FC's
> `__asm__ volatile("":"=m"(dummy_pad))` must be REPLACED by a plain
> carve-out-conformant pad when it is closed, not grandfathered), and any
> pad whose size the frame equation does not force.
>
> Family members waiting on this carve-out (close case-by-case under the
> five prerequisites when each reaches the queue top): func_80037540
> (closed with this ruling), func_8003DBE4 (`s32 buf[2]` dead local),
> file_LoadSectors/func_800165F8 (`s32 _pad[2]`), func_800644FC (asm
> dummy_pad — see above). NOTE for fully-dead pads: the
> `find_unused_local_arrays` detector still flags never-referenced
> arrays and `mark_done` will refuse — wiring a prerequisite-aware
> allowlist through the engine (with `engine test` kept green) is part
> of closing the FIRST fully-dead-pad family member; do not weaken the
> detector silently.

## Status

**This technique is FORBIDDEN as of 2026-05-31** (commit `1cd5c64`+ wires the
array detector) **with the scalar variant added 2026-06-01** after the
`func_8007CE0C` de-cheat investigation found that
`s32 pad; (void)&pad;` reproduced the same frame-coercion effect under a
syntactic spelling the array detector didn't catch. A function carrying any
of the following is no longer eligible for COMPLETED-C status:

- **Array form:** `T name[N];` (2 ≤ N ≤ 64) declared inside a function body
  and never referenced afterward. Detector:
  `engine/volatile_cheats.find_unused_local_arrays`.
- **Scalar address-coercion form:** `(void)&<name>;` as a body statement.
  There is no legitimate pure-C reason to write a discarded address-of —
  even taking the address legitimately is `&local` (no `(void)` cast); the
  cast-to-void exists purely to suppress unused-expression warnings around
  the DCE defeat. Detector:
  `engine/volatile_cheats.find_addr_coerced_locals`. Catches the agent's
  proposed `s32 stk_a,b,c,d; (void)&stk_a; ...` shape verbatim. Verified
  zero false positives on the current src/*.c (no legitimate uses of the
  pattern exist).

Both feed into `engine/inlineasm.func_cheat_asm_count`; `engine/queue.mark_done`
refuses to record completion for affected functions. Regression test:
`engine/test_engine.py::test_addr_coerced_locals` (6 cases, all green).

The array detector excludes struct-member declarations and arrays that ARE
referenced later in the body. The scalar detector triggers unconditionally
on any `(void)&local;` statement at function-body scope.

## Why this changed

The prior position (archived below) argued that `s32 buf[N];` was "what the
original source looked like" because GCC reserves the frame for declared
locals regardless of use. The flaw in that argument:

1. **We DON'T know what the original source had.** We only know the FRAME
   SIZE of the compiled output. Declaring an anonymous `buf[N]` is a GUESS
   that happens to produce the right size. SOTN-quality reconstruction
   either identifies what the locals semantically WERE (matrices, GPU
   packets, scratchpad areas) and reconstructs them with meaningful names,
   OR marks the function `INCLUDE_ASM` if pure-C can't recover them.

2. **The naming pattern reveals intent.** The flagged BB2 cases are named
   `pad`, `_pad`, `pad2`, `pre_pad`, `_spill`, `sp_area`, `sp_buf`,
   `sp_dummy`, `pad12`, `w`, `tail`, `buf`, `a`. These announce themselves
   as padding, not as reconstructed source. A SOTN-quality reconstruction
   would use names like `GpuPacketArea`, `MatrixScratch`, `OtBuffer`.

3. **The original rule already conceded the line.** The earlier audit
   explicitly REJECTED `s32 unused_slack[8]; (void)unused_slack;` as
   "dead-code injection whose ONLY purpose is to bloat the frame." Removing
   the `(void)` cast and the `unused_` prefix doesn't change the intent —
   only the auditor's ability to detect the pattern.

4. **SOTN's bar rejects this.** SOTN matches functions either via real C
   semantics (with locals that correspond to identifiable stack usage) or
   via `INCLUDE_ASM`. There is no "declare phantom locals" pattern in the
   community standard.

## What to do instead

If a function's frame size can't be matched in pure C with locals that have
**clear semantic meaning** (you can name what they hold from reading the
function's asm and callers), the function is not COMPLETED-C-matchable in pure C:

1. **`queue park`** the function with a clear rationale.
2. **Request canonical-asm authorization** via `inline_asm_canonical.txt`
   if the function is genuinely hand-written or has no recoverable C
   structure.
3. **Do NOT** add a `s32 pad[N];` workaround. Do NOT add `s32 buf[N];`.
   The detector will refuse completion.

Functions that legitimately use stack locals are unaffected — their arrays
are referenced in the body and the detector skips them.

## Currently affected functions (as of 2026-05-31)

44 functions across BB2's source carry one or more unused local arrays:
- `src/code6cac.c`: 7 functions (`sp_area`, `sp_buf`, `pad`, `a`, `_pad`,
  `pre_pad`, `tail`, `pad2`, `pad12`, `sp20`)
- `src/text1b.c`: 6 functions
- `src/sound.c`: 4 functions
- Plus singletons in `gpu.c`, `text1b_b.c`, `display.c`, `ings.c`,
  `text1a_c.c`, `code6cac_*.c`

These functions will be re-routed to `active` by `queue regen` and need a
genuine COMPLETED-C close (or canonical-asm authorization).

## InitHiraRmd_80047FBC — the case the original rule was written for

The function genuinely matched in pure C **without** the `s32 buf[8];` trick
once the other levers (pointer-walk prologue staging + dead `arg0 = 0;` —
itself now also forbidden, see [[register-alloc-pure-c]] Lever D update +
loop-local precompute) were applied. With Lever D also forbidden, this
cluster's COMPLETED-C status needs re-evaluation; queue regen will re-route it.

## Related

- [[register-alloc-pure-c]] — companion "Lever D dead self-assignment" is
  also now forbidden.
- [[split-read-defeats-hoist]] — companion "mark globals volatile" is also
  now forbidden.
- [[inline-asm-policy]] — the policy framework that volatile/frame coercion
  cheats now fall under (cheat-asm, never COMPLETED-C).
- [[canonical-asm-retirement]] — the LEGITIMATE escape for functions that
  genuinely cannot reach pure-C COMPLETED-C.

---

## ARCHIVAL — the original (now-deprecated) rationale

The text below is preserved for historical context. Do NOT apply this
technique going forward. The detector enforces the new bar.

### What this discovered (original framing — superseded)

The prior agent concluded that all 4 pure-C techniques producing the
target's frame=80 / vars=32 layout for the InitHiraRmd_80047FBC cluster
were "functionally equivalent to banned `unused_slack[8]`". This rule's
original claim refuted that — `s32 buf[8];` naturally produces frame=80/
vars=32 in GCC 2.7.2 without `(void)` cast or volatile decoration.

That observation is FACTUALLY TRUE. The reframing as "legitimate COMPLETED-C"
was the error. GCC reserving the frame for declared locals is a compiler
property, not a license to declare phantom locals as a matching technique.
