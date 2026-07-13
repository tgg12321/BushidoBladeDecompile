# ESCALATION — owner ruling needed: GCC 2.7.2 decompals-fork divergence blocking `_spu_FiDMA`

**Origin:** Closer Phase 3 wrap-up (2026-07-13). Banked candidate:
`memory/closer/candidates/spu_writebyio_splice.c`. Function:
`DispUpdateStatusMessage` (queue: active, ASM-SUSPECT, dist 204) — the splice
region `0x800889D4..0x80088D0C` = Sony `spu.c` `_spu_FwriteByIO` +
`_spu_FiDMA` + `_spu_Fr_`.

## The exact question

Our build compiler is the open-source `decompals/mips-gcc-2.7.2` fork. On the
Sony-source wait-loop class

```c
while (<volatile MMIO read expr>) { ...; if (++i > 0xF00) break; }
```

**our fork SEGFAULTS (cc1 SIGSEGV)** — for any `while`/`for` spelling with a
volatile read in the condition and an early exit in the body. The original
PsyQ `cc1psx` compiles the identical source without crashing and emits the
**exact target bytes** (including the dead `addu $3,$3,-1` that reorg's
increment-compensation clause produces).

`_spu_FiDMA` therefore cannot be closed from the faithful Sony source in our
toolchain. The closest crash-avoiding spelling (if + do-while + `goto`) lands
**2 words short**: it loses the loop notes, so `reorg.c mostly_true_jump`
returns likely=0 and the increment-compensation `addiu -1` never fires.
`_spu_FwriteByIO` is already **bit-exact 132/132** in the same region; only the
`_spu_FiDMA` sub-region is blocked.

**Should this be accepted as a fork-divergence class ("our fork cannot emit
what the original compiler did") — a Ruling-2-style disposition — or must the
function reach byte-exact through some not-yet-found pure-C spelling in our
fork?**

## Mechanism evidence (independently reproducible)

- **Crash exhibit:** `tmp/closer/spu3/` — 15 measured variants; the faithful
  `while (volatile & 0x30) { if (++i>0xF00) break; }` SIGSEGVs cc1.
- **cc1psx counter-exhibit:** `tmp/closer/spu3/t_psx.s` — the ORIGINAL PsyQ
  compiler compiles that exact source and emits the target bytes verbatim,
  incl. the dead `addu $3,$3,-1`.
- **Why the workaround is 2 short:** goto/if-do-while spellings avoid the
  crash but drop loop notes → `mostly_true_jump` likely=0 → reorg.c:3615
  increment-compensation (the only producer of the `addiu -1`) never fires
  (DBRDBG-traced, `BB2_DBR_DEBUG=1`).
- **Reachability:** `_spu_FiDMA` is installed by address only
  (`g_snd_irq_data = 0x80088BA0`, the SPU DMA-complete IRQ callback);
  `_spu_Fr_` is unreferenced Sony dead code (S_SCA precedent).

This is the same *kind* of finding as the marionation fp-allocatability
fork-divergence noted in Phase 2 — "our fork cannot emit; the original
compiler can and did."

## The two named options for the owner

1. **Grant Ruling-2 (fork-divergence class).** Accept `_spu_FiDMA` as a
   documented fork-divergence: our decompals fork provably cannot compile the
   faithful Sony source (it crashes), the original `cc1psx` provably can and
   produces the target bytes. Disposition would follow the canonical-asm /
   authorized-exception path for the residual 2 words. **Constraint check:**
   this is NOT the `input-side compiler fix` that `no-compiler-divergence`
   forbids — it is a completion-class ruling, not a patch to the toolchain.

2. **Reject — require a pure-C close in our fork.** Keep the function ACTIVE
   in the queue for the Grinder; the standing search task becomes "find a
   loop-note-preserving spelling whose exit test does NOT target loop-end
   (blocking the `duplicate_loop_exit_test` that fires on the empty break
   arm) while keeping body order `[i++; limit; cond]`." A major search with
   no guarantee of existence.

## Related (lower-confidence, same family — NOT yet proven fork-class)

`func_80089F3C` (SpuSetReverbModeParam, queue: active, verdict C, dist 23) —
banked at `memory/closer/candidates/spusetreverbmodeparam_struct.c`. 20 of 23
masked words are self-heal struct-reloc addends; the **3 real words** are a
reload spill-slot ordering: our fork's reload runs TWO spill passes
("Spilling reg 8/64" ×2) leaking two transient slots before `sp58`'s; Sony's
compile leaked one → `sp58` lands at `sp+0x5C` vs target `sp+0x58`. This
*could* be another fork/reload divergence, but it is NOT proven fork-class —
a not-yet-found source shape may leak one slot. Next step is
instrumentation-side (patch cc1 `BB2_RELOAD_DEBUG` to identify the pass-1
stacked pseudo set), not an owner policy call. Listed here so the owner sees
the pattern; **no ruling requested on this one yet** — it stays a Grinder
reference item.
