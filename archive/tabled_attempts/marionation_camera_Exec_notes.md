# marionation_camera_Exec — TABLED 2026-04-23

## Function: 155 instructions, code6cac.c line 223
## Best score: 3670 (permuter), +20 bytes (build)

## What's done
- Full C decompilation structurally correct
- GTE matrix load blocks working (2× inline asm .word blocks)
- GTE sqrt (ctc2/cfc2/swc2) implemented with register asm t4 + inline asm
- srav fix (removed (u32) cast)
- Scratchpad distance/scale logic matches structurally
- Struct copy via _MCE_Copy16 typedef

## C code
Best version saved in `permuter/marionation_camera_Exec/base.c` (attempt 10)
Integration script in `tmp/update_mario_cam.py`

## Blockers (5 compounding)
1. **Scratchpad stores**: GCC emits lui+ori+sw(0), target has lui+sw(offset) — 3× delete ori + 3× subst
2. **Register cascade**: $6→$3 throughout scale section, mfhi/mflo register assignments
3. **Scratchpad reads in scale section**: same lui+ori pattern, 3× more delete+subst pairs  
4. **Struct copy registers**: GCC routes through different temp regs vs target
5. **Delay slot scheduling**: multiple fill_delay needed (scale init, beqz, jal)

## Estimated regfix effort
30+ rules across all 5 blockers. gen-regfix cannot handle this function (SIZE MISMATCH
from inline asm .word directives inflating TEXT count). All rules would need manual
index counting from maspsx output.

## maspsx TEXT index reference
Saved in previous session's context. Key ranges:
- T015-T017: Distance store 1 (0x1F800024)
- T025-T027: Distance store 2 (0x1F800028)  
- T028-T041: Distance block 3 (complex restructuring)
- T087-T110: Scale section
- T111-T130: Struct copy + jal

## Why tabled
Compound regfix with 30+ manual rules, gen-regfix inoperable, 5 interacting blockers.
Could be revisited if gen-regfix gains inline-asm-aware instruction counting.
