# SELF-VET — _addque2
CONSTRUCTS: aggregate-merge declaration (GpuQueueItem[64] replacing D_80103680/84/88/8C), `extern volatile GpuQueueItem D_80103680[64];`, `extern volatile s32 *D_8009BF48;` (:720), `extern volatile s32 *D_8009BF54;` (:732, :795), `(u8 *)` cast at _reset's memset, `D_8009BF68[0] = (s32)func;`
## T1 semantic purpose: aggregate = the object's real shape (0x60 records, census + stride evidence). volatile queue = original Sony semantics (SOTN sys.c:95) — the slots are read by _exeque from DMA-IRQ context; without it the compiler may reorder the slot stores against the head-index publish. volatile MMIO pointees = GPU_STATUS / DMA2_CHCR hardware registers; without it loop.c hoists the spin-wait read (proven infinite loop in the .loop dump). memset cast = type adaptation only. D_8009BF68[0] = the granted array spelling of that symbol.
## T2 human-programmer: yes — this is a transcription of Sony's sys.c `_addque2` onto BB2's older record layout; every qualifier is one Sony wrote.
## T3 GCC-internals justification: mechanisms (reorg.c resource_conflicts_p, sched.c volatile deps, loop.c invariant_p) are recorded as EXPLANATION of the measurements; the constructs are justified by program semantics (hardware registers, ISR-shared queue) independent of them.
## T4 permuter/search provenance: none — hand-derived from the SOTN reference and the target listing; no permuter run. The `g_`-named spelling that bypasses the detector is NOT submitted.
## T5 family check: aggregate merge (no-new-park-categories.md:238 entry, prongs a-e addressed in evidence.md); Ruling 4 volatile class (commit c80d976e) for D_80103680 — GRANT REQUIRED, not yet held -> ruling-request; mmio-volatile-type-level for the two pointees (ordinary C).
## T6 naming-announces-intent: no pad/dummy/unused names; struct members func/arg/count/data.
SANCTIONED-FAMILY-CLAIMS:
  FAMILY: aggregate merge (Per-word splat symbol -> aggregate merge)
  SCOPE: "two or more splat-invented `D_<addr>` scalars may be replaced by a single aggregate declaration. Prongs, ALL mandatory: (a) the object model is established by evidence independent of and predating the byte-chasing session"
  PRECEDENT: .claude/rules/no-new-park-categories.md:238
  FAMILY: Ruling 4 ground-truth-codegen volatile class (extern volatile T G on Sony census module state)
  SCOPE: "For census-proven Sony library module state (symbol identity reloc-proven against the verbatim-linked SDK object), where the Sony object's code is MEASURED unreachable without `volatile` (non-volatile build demonstrably collapses ordering/re-reads), volatile is legal as ORIGINAL SEMANTICS"
  PRECEDENT: c80d976e
  FAMILY: type-level MMIO volatile
  SCOPE: "0x1F801000-0x1F802FFF" window (GPU_STATUS 0x1F801814, DMA2_CHCR 0x1F8010A8) per .claude/rules/mmio-volatile-type-level.md
  PRECEDENT: volatile_extern_allowlist.txt:76
ANNOTATION-CONFORMANCE: n/a — no FAKE construct (no family here mandates a /* FAKE */ line).
