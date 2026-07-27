# Evidence bank — func_80052B44

- WIP rejected_form: {'form': 'clean plain-locals + ctc2 %0 constraints, no pins/rule', 'score': 13, 'reason': "GCC picks lower regs ($v0/$v1/$a1/$a2) not target's $t0-$t4, and won't fill the jr delay slot — not a cheat, just non-matching; the gap is hand-asm register choice + delay fill."}

- s1 (2026-07-27, recon): baseline re-measured at current HEAD — canonical ASM-PARTIAL 8/14 (ctc2 regions [5,11]+[13,13]); sandbox --disable all = 13 (build 16 vs target 14, 1 rule dropped, pins stripped). Matches inherited floor.
- s1: NO-C-FORM PROOF for the tail — target ends `jr ra; ctc2 $zero,$7` (delay slot). ctc2 has no C analog, so it can only enter compilation as implicitly-volatile no-output inline asm; GCC 2.7.2 reorg.c can never fill a delay slot with an asm insn: stop_search_p (tools/gcc-2.7.2/reorg.c:730-735) stops the candidate search at ASM_INPUT/asm_noperands insns, and mark_referenced_resources (reorg.c:366-367) marks asm always-volatile so resource_conflicts_p (reorg.c:749-752) always conflicts. Pure-C distance mechanically cannot reach 0.
- s1: PRECEDENT — func_8007ED6C (inline_asm_canonical.txt:308, src/display.c:2253-2284) authorized canonical-body 2026-07-11 as verbatim-linked Sony PsyQ 4.0 LIBGTE object; its first 10 instructions are IDENTICAL to func_80052B44's (5x lw $t0-$t4 -> 5x ctc2 CR0-4). func_80052B44 = LIBGTE SetRotMatrix body + zero-translation (ctc2 $zero CR5-7). gte-wrapper-misroute-park 2026-06-10 boundary: loads feeding hardcoded GTE regs are MECHANICAL PACKAGING (zero-ambiguity canonical class); this function has zero general-purpose computation.
- s1: scan_hand_coded --single: LOW 0/8 — expected; S-signals target >=40-insn bodies (multu pacing, spills, masks); not evidentiary either way for a 14-insn GTE leaf.
- s1: sibling pin-debt cluster in text1b.c (separate queue items, same shape): func_80052A88 / func_80052B00 / func_80052B7C.

- == imported from memory/wip notes.md ==
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

