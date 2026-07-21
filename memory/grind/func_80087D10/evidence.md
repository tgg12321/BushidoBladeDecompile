# Evidence bank — func_80087D10

## s1 (recon, 2026-07-21)

- **Baseline (inherited src form, memory-barrier cheat stripped by sandbox): 8/18.**
  Old src carried `__asm__ volatile("" ::: "memory")` between the base `lw` and the
  `sh D_80102806` — a forbidden scheduling-barrier cheat; sandbox strips it, honest floor 8.
  Barrier REMOVED this session (src is now barrier-free; file cheat-asm count 79 → 78).
- **Canonical: verdict C** (18 insns, pure-C target).
- **Target shape** (tmp/grind/func_80087D10/s1/target.s): EXACT source order, zero scheduler
  movement: `andi v0,a0,0xff; sll v0,2; [lui/addu/lw base]; [lui/sh D_80102806]; andi a0,0xff00;
  sra a0,8; 5-insn *176 mul chain (sll1/addu/sll2/subu/sll4) in v0; addu v1,v1,v0; lh v0,88(v1)`.
  Store is the direct **$at-macro** form (lui at/sh) — NOT the CAC pointer (a3) form.
- **Core diff mechanism:** our build sinks the base `lw` macro past the sh/andi/sra cluster;
  everything else already matches. Post-RA the lw sinks exactly until the first insn that
  writes its address register — i.e. the sink boundary is a register anti-dep, and the sink
  distance is controlled by which reg holds the table index:
  - old form (`p = base + slot*176`, fresh single-set `p` and `base`): index in **v1**, lw
    writes v1 → lw free to sink to just before its consumer → sank 7 slots → distance 8.
  - V1 form (`base += slot*176`, multi-set `base`): index in **v0** (matches target), mul chain
    writes v0 → anti-dep stops the sink right before the mul chain → sank 3 slots → **distance 6**.
- **V1 = current floor 6 = candidate.c:** drop `p`, make `base` multi-set
  (`base += slot * 176; return *(s16 *)(base + 0x58);`). Natural C, no FAKE needed.
  Consistent with [[staged-value-reused-variable]]'s mechanism (single-set fresh dest gets
  load-late launch priority; multi-set turns it off) — here achieved by pure simplification.
- **Statement order in C is INERT:** store-first, slot-before-store, load-first all score 6
  with identical bytes. Placement of the sh/lw is decided entirely by the scheduler.
- **Single-use pointer store folds PRE-sched (KILLED):** `s16 *ptr = &D_80102806; *ptr = a0;`
  (single use) → cse folds to the direct symbol MEM before sched1; output byte-identical to
  direct store (still 6; tmp/grind/func_80087D10/s1/built_v3_single_use_ptr.s). So the CAC
  pointer-alias mechanism does NOT transfer to a single-access store: no reload-time REG_EQUIV
  path materializes, no alias opacity at sched time. (CAC closed only because TWO accesses
  keep the pointer live → addiu a3 form; D10 target is the $at form, per CAC judge notes.)
- **Param-reuse slot (`a0 = (a0 & 0xff00) >> 8;`) scores 7 (KILLED):** C-level anti-dep raises
  the sh cluster's priority further; worse than fresh `slot` local.
- **Sibling context:** func_80087CAC COMPLETED-C via sanctioned pointer-alias-fake-exception
  (judge f5fb1d22 lineage); its exhaustion sweeps live in tmp/grind/func_80087CAC/s2/.
  func_80087D58 (return +0x5A) is a byte-twin of D10 modulo the lh offset and still carries the
  same barrier cheat — whatever closes D10 closes D58 identically.
- **What remains for 0:** keep the 3-insn base-lw macro from sinking past the 3-insn
  sh/andi/sra group (target: lw at slots 3-5; ours: slots 7-9). Everything else matches,
  including all register allocation.

- [s1] canonical: verdict C, 18 insns; baseline honest floor 8 (inherited form carried a forbidden __asm__ memory-barrier cheat between lw and sh - removed this session, src now barrier-free, file cheat-asm count 79->78)

- [s1] target (s1/target.s) is EXACT source order with zero scheduler movement: index andi/sll, base lw macro, direct $at-macro sh of D_80102806, andi/sra slot, 5-insn *176 chain (sll1/addu/sll2/subu/sll4), addu, lh 0x58

- [s1] entire residual diff at floor 6 = the 3-insn base-lw macro sits after the sh/andi/sra cluster instead of before it; all register allocation already matches

- [s1] post-RA lw sink boundary = first insn writing the lw's address register: with index in v1 (old form) it sank 7 slots (floor 8); with index in v0 (multi-set base) the mul chain's v0 writes stop it at 3 slots (floor 6)

- [s1] sibling func_80087CAC closed via sanctioned pointer-alias-fake-exception ONLY because its store+reload keeps the pointer live (addiu a3 form); D10 target is the single-access $at form, so that lever is unavailable here (confirmed by H2 measurement)

- [s1] func_80087D58 is a byte-twin (lh offset 0x5A) still carrying the same barrier cheat; any D10 closure transfers verbatim
