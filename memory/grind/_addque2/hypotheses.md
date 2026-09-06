# Hypothesis ledger — _addque2

## s1 (2026-09-06, recon)
- H1 aggregate merge (4 per-word splat scalars -> `GpuQueueItem[64]`, 0x60 records) —
  CONFIRMED 184 -> 29; every slot store byte-identical.
- H2 `volatile s32 *D_8009BF48` at the :720 site (MMIO pointee, GPU_STATUS) —
  CONFIRMED 29 -> 22 (loop.c hoisted the spin-wait read otherwise; .loop dump).
- H3 `volatile s32 *D_8009BF54` at :732/:795 (MMIO pointee, DMA2_CHCR) — CONFIRMED 22 -> 20.
- H4 volatile queue array (SOTN original `static volatile struct QueueItem`) —
  CONFIRMED 20 -> 2 raw; honest (un-granted, stripped) stays 18. Needs Ruling 4 grant.
- H5 copy loop `arg[i]` instead of `*p++` — CONFIRMED 2 -> 0 (loop.c giv init order).
- KILLED (instance): `*p++` pointer-walk copy loop, measured 2 with volatile queue live.
- KILLED (instance): non-volatile queue aggregate with H2+H3+H5, measured 18 honest.

## Frontier
1. Integration handoff (ruling-request filed s1): allowlist grant for D_80103680 under
   Ruling 4 + header-canonical `GpuQueueItem` in include/gpu.h + alias-suffix rows for
   D_80103684/88/8C. After the grant: re-apply candidate.c, sandbox must read 0 honest,
   verify-oracle ok, then candidate-ready.
2. If the Judge refuses the volatile grant: the residual is exactly 18 = reorg delay-slot
   filler (reorg.c:752) + sched volatile serialization; no sanctioned non-volatile
   family reproduces a store that refuses a delay slot — file the negative census then.
