# SELF-VET — _spu_FiDMA

CONSTRUCTS: none (ordinary C only). Enumerated for completeness, every construct in
the diff: (1) `if (D_800A2D2C == 0) { _spu_Fw1ts(); }` guarded call; (2) MMIO
read-modify-write `*(volatile u16 *)(D_800A2CDC + 0x1AA) = *(volatile u16 *)(D_800A2CDC + 0x1AA) & ~0x30;`;
(3) `u32 timeout` local with a bounded `while (... & 0x30) { timeout++; if (timeout > 0xF00) break; }`
spin; (4) `((void (*)(void))_spu_transferCallback)()` — call through the Sony
callback slot; (5) early `return;` in the callback arm with the `DeliverEvent(0xF0000009, 0x20)`
fallthrough. No FAKE construct is present anywhere in the diff.

## T1 semantic purpose: Every construct is load-bearing.
(1) skips the SPU-wait when the module's ready flag is already set — removing it
changes which hardware writes happen. (2) is the actual hardware side effect of
the function: it clears the transfer-mode bits in SPUCNT. (3) is the completion
spin with Sony's 0xF00 bounded timeout; deleting it lets the handler dispatch
before the SPU has finished. (4) dispatches the user-installed transfer callback;
(5) selects between the callback and the SPU DMA-complete event delivery. Delete
any one of them and the function's observable behaviour changes.

## T2 human-programmer: Yes to all five. This is the published shape of Sony's
LIBSPU `_spu_FiDMA` (psyz `decomp/src/libspu/spu.c:161`): guard, clear the mode
bits, bounded wait for them to read back clear, then callback-or-event. A reader
handed only "this is the SPU DMA-completion interrupt handler" would write this
body. Nothing in it prompts a "why is this here?".

## T3 GCC-internals justification: No construct in the diff is justified by a GCC
internal. No pass name (allocator, scheduler, DCE, combine, reorg, loop) appears
in the reasoning for why any line exists — the reasoning is entirely "this is what
the handler does". The prior ledger knowledge about `duplicate_loop_exit_test` /
`reorg.c:3615` was diagnostic context from a 2026-07 campaign; the form measured
here is the plain Sony spelling and needed no such lever (see evidence entry
FIDMA-E4: the 2026-07 fork segfault on this exact spelling no longer reproduces
post-2026-08-07 cc1 rebuild).

## T4 permuter/search provenance: No permuter or automated search was run this
session. The body is a hand transplant of the human-authored psyz reference source
banked at `memory/grind/_spu_FiDMA/psyz-seed.c`, applied verbatim modulo the
`D_800A2D14` -> `_spu_transferCallback` symbol rename. It measures 0 because it is
the original source shape, not because a search found a spelling detectors miss.

## T5 family check: No construct matches any forbidden family, by analogy or
otherwise. There is no register pin, no `__asm__`, no scheduling barrier, no
alias rename, no dead local, no dead store, no pad array, no `do {} while (0)`,
no opaque constant holder, no empty-body `if`, no `if (1)` wrapper, no dead goto,
no duplicated statement into arms, no aggregate merge, no declaration-order trick.
`timeout` is written and read on every iteration and controls the loop exit.

## T6 naming-announces-intent: The only local is `timeout`, which names what it is
— the Sony bounded-spin counter, compared against 0xF00. No `pad`/`dummy`/`unused`/
`spill`/`slack`/`_buf`/`tail` naming appears in the diff.

SANCTIONED-FAMILY-CLAIMS: none — the diff is ordinary C and claims no exception
family. Two pre-existing declarations it consumes are noted for the reviewer, and
neither is introduced or modified by this diff:
  - `extern volatile s32 _spu_transferCallback;` (src/main.c:74) is a HEAD
    declaration carrying its own ratified grant at
    `volatile_extern_allowlist.txt:33-34` (operator-audited 2026-07-10).
  - `*(volatile u16 *)(D_800A2CDC + off)` is the established HEAD idiom for the
    SPU register file in this same TU (src/main.c:1620-1691, unchanged lines).

ANNOTATION-CONFORMANCE: n/a — no FAKE construct.
