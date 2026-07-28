# Hypothesis ledger — func_80037B00

## [s1] The +8 phantom frame is not independently addressable; it materializes only when register pressure keeps a local alive across the loop, so it's downstream of the register-rotation axis.
- mechanism: Target has addiu sp,-8/addiu sp,+8 with ZERO memory accesses at sp+0..7 and zero jals (outgoing-args=0), so the 8 bytes are a vars slot. GCC 2.7.2 provably never DCEs local-array stores (file_LoadSectors DCE tests), so any pure-C written-local emits stores that target lacks; unused-local is dead-vars-local-array (forbidden except the 2026-07-01 carve-out which requires target dead stores — target has none). Only surviving mechanism: pin-induced reload pressure keeps a coalesced-away scalar alive, producing the frame as a side-effect of the register axis.
- probe: Confirmed by ledger evidence (WIP note #2) + this session's asm read; no fresh measurement needed.
- result: Axis is contingent, not independent. Any register-axis fix that keeps a natural live-but-memory-dead local produces the frame implicitly.
- verdict: CONFIRMED

## [s1] do-while loop restructure lowers the score.
- mechanism: Move outer/inner test to bottom to match target's bnez-with-delay-slot-advance shape.
- probe: Ledger: prior session measured do-while pin-free → sandbox=17 weighted, build_insns=33.
- result: Score WORSE than goto form (17 > 15 weighted, 33 < 34 build_insns — lost one inner-loop insn). Do-while collapses one slt/bnez pair GCC produces in the goto form.
- verdict: KILLED
