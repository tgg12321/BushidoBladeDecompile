---
name: dead-vars-local-array
paths: ["src/text1b.c", "src/*.c"]
description: "FORBIDDEN as of 2026-05-31. Declaring unused local arrays (`s32 buf[N];`) to force GCC to reserve frame bytes is a codegen-coercion cheat. SOTN rejects it. The engine flags every unused fixed-size local with names like `pad`, `_pad`, `buf`, `w`, `tail` and refuses completion."
metadata:
  type: reference
  status: forbidden
---

# FORBIDDEN — Frame coercion via unused local arrays is a cheat

## Status

**This technique is FORBIDDEN as of 2026-05-31** (commit `1cd5c64`+ wires the
detector). A function carrying any unused fixed-size local array is no longer
eligible for COMPLETED-C status. The engine's
`engine/volatile_cheats.find_unused_local_arrays` detector flags every such
declaration; `engine/inlineasm.func_cheat_asm_count` includes the count;
`engine/queue.mark_done` refuses to record completion.

The detector excludes struct-member declarations and arrays that ARE
referenced later in the body. Only genuinely-unused fixed-size locals are flagged.

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
