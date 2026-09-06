# SELF-VET — _spu_Fr_
CONSTRUCTS: (1) `extern volatile s32 *D_800A2CE0/D_800A2CE4/D_800A2CE8;` declaration change (pointee-volatile pointers to DMA4 MADR/BCR/CHCR, 0x1F8010C0/C4/C8); (2) `*(volatile u16 *)(D_800A2CDC + 0x1A6/0x1AA)` SPU-register accesses (identical idiom to the already-matched _spu_init/_spu_t in the same file); nothing else — no locals, no FAKE constructs.
## T1 semantic purpose: (1) The pointees are hardware DMA registers; volatile is the correct type for MMIO stores (every store must be emitted, in program order). Real semantics, not a byte-shaping no-op. (2) same — SPU control registers via the base pointer.
## T2 human-programmer: Sony's own spu.c stores through these register pointers (SOTN libspu/spu.c `_spu_r_`); the MMIO rule's cited SOTN idiom is `static volatile u16* i_stat = (u16*)0x1F801070;`. A reader asks no "why is this here" — it is the textbook way to declare DMA register pointers.
## T3 GCC-internals justification: The declaration is justified by hardware semantics (DMA channel 4 registers). The reorg.c:760 mechanism is recorded in evidence.md as pass ATTRIBUTION of the residual, not as the reason the construct exists.
## T4 permuter/search provenance: No permuter. Hand-derived from the DATA MODEL signal (three pointer globals with no header declaration, census names "pointer to SPU DMA ... register") and the SOTN reference body.
## T5 family check: MMIO-range type-level volatile, `.claude/rules/mmio-volatile-type-level.md`, spelling "`volatile T* p = (T*)0x1F801xxx;` pointer declarations" — the pointer's value is a documented `.data` word in the covered range. Not scalar volatile on a game-state global; not an alias rename; not a cast on `&G`.
## T6 naming-announces-intent: No new names; splat names retained (D_800A2CE0/E4/E8) with a comment naming the Sony symbols and registers.
SANCTIONED-FAMILY-CLAIMS:
  FAMILY: type-level MMIO volatile (mmio-volatile-type-level)
  SCOPE: "**Covered addresses:** the PSX hardware I/O-register range `0x1F801000-0x1F802FFF` (interrupt controller, DMA, timers, CD, GPU, SPU, expansion)."
  PRECEDENT: `.claude/rules/mmio-volatile-type-level.md:45`
  ADDRESS VERIFICATION: `asm/data/7D920.data.s:33123` (.word 0x1F8010C0), `asm/data/7D920.data.s:33129` (.word 0x1F8010C4), `asm/data/7D920.data.s:33135` (.word 0x1F8010C8). Detector clause: `engine/volatile_cheats.py:30`.
ANNOTATION-CONFORMANCE: n/a — no FAKE construct (the MMIO rule's requirement 2 states "No FAKE annotation — this is correct typing"; a register-naming comment is present at the declaration).
