# func_80052B44 (text1b.c) — BLOCKED: GTE leaf wrapper, canonical-asm authorization

## TL;DR
GTE matrix-load wrapper: 5 `lw matrix[0..4]` -> `ctc2` into GTE control regs
$0-$4, then `ctc2 zero,$5/$6/$7` (last in the `jr ra` delay slot). `canonical`
routes **ASM-PARTIAL** (8/14 insns are canonical cop2 `ctc2` ops). HEAD matches
via 5 `register asm("$8".."$12")` pins + 1 `fill_delay` regfix rule — both cheats.

## Why it's not pure-C (measured, pins+rule neutralized)
Clean form (plain `s32 t0=matrix[0]; ... __asm__("ctc2 %0,$N" :: "r"(t0))`,
no pins, no rule) full-build distance **13**:
1. **Register rotation, anti-preference.** Target loads into `$t0..$t4` (regs
   8-12). GCC naturally picks the LOWER-numbered `$v0,$v1,$a1,$a2,$a0` (its
   ascending preference). Target uses HIGHER regs than GCC prefers — no pure-C
   lever forces that (it's the opposite of the allocator's bias); this is the
   hallmark of hand-written GTE asm. Also GCC reorders the base-aliasing
   `lw a0,0(a0)` last, adding a load-delay `nop`.
2. **Delay-slot fill.** Target puts the final `ctc2 zero,$7` in the `jr ra`
   delay slot (the `fill_delay @ 13 <- 12` rule). GCC emits `ctc2 zero,$7; jr ra;
   nop` — won't fill the slot with the trailing volatile cop2 op on its own.

## Endpoint
This is a GTE leaf wrapper (pure cop2 body + thin hand-coded load prologue) —
the COMPLETED-INLINE-ASM-CANONICAL class. Per project policy GTE leaf wrappers
are authorized by the orchestrator/user, NOT pure-C-ground by a worker. Blocked
for canonical-asm authorization. Sibling wrappers (func_80052A88, func_80052B7C,
func_8007ED6C, func_80052B00) were "Retired to pure C" in 2026-05-11 commits —
worth checking whether those used the same pins (i.e. whether they are genuinely
COMPLETED-C or carry the same debt).
