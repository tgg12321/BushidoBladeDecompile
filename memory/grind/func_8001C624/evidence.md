# Evidence bank — func_8001C624

- Audit diagnosis (regressions.md): Three distinct cheat constructs found — two unannotated do-while(0) wrappers (one with variant spelling `0 != 0`) and a dead load/store round-trip pair — all permuter-discovered, all framed in the commit by GCC-scheduling intent rather than program spec. Worker must redo the function in pure C, dropping the do-while wrappers and the t/u round-trips; the struct-copy Blk16/Blk12 pattern and the outer sequencing are clean and can stay.  (committed code flagged by the re-audit patrol; review and re-do in pure C if confirmed. The byte-correct construct stays on main until a clean replacement lands.)

- [s1] [fable-blitz 2026-07-07] Flagged constructs located: inner do{...}while(0) at src/code6cac.c:1410-1415, outer do{...}while(0 != 0) variant spelling spanning src/code6cac.c:1397-1432, t/u load/store round-trip at src/code6cac.c:1418-1424

- [s1] [fable-blitz 2026-07-07] The t/u round-trip is NOT dead in the target: asm/6CAC.s:5101+5103 is lw $v0,0x10($sp) then sw $v0,0x10($sp) (local[0] self-copy) and asm/6CAC.s:5102+5117 is lw/sw $v1,0x18($sp) (local[2] self-copy). Target bytes literally contain the round-trip stores, so the construct cannot be DROPPED without changing bytes - it must be RESPELLED with semantic purpose

- [s1] [fable-blitz 2026-07-07] Natural respelling: the PsyQ libgte setVector comma-assign macro idiom - setVector on the 3-word local with (local[0], local[1]-0x384, local[2]) produces exactly the vx/vz self-assigns plus adjusted vy the target emits; human-plausible spec-driven code that eliminates the 'dead t/u temps' framing entirely

- [s1] [fable-blitz 2026-07-07] Target schedules the round-trip tail LATE: sw 0x18($sp) at asm/6CAC.s:5117 and the addiu -0x384 / sw 0x14($sp) at asm/6CAC.s:5118-5119 land AFTER the six zero stores D_80101FDC..D_80101FF4 (asm/6CAC.s:5105-5116), while the round-trip loads sit BEFORE them (5101-5104). Current source order does not interleave this way, so the wrappers are plausibly load-bearing scheduling fences, not inert

- [s1] [fable-blitz 2026-07-07] Function is straight-line (zero branches between prologue and the final jal, asm/6CAC.s:5019-5147), so the reorg.c LABEL_OUTSIDE_LOOP_P mechanism is absent; any wrapper effect is loop-note scheduling/RA weighting - sanctioned for ANY codegen effect w/ inline FAKE annotation per the FINAL 2026-07-06 ruling (.claude/rules/do-while-zero-exception.md), but the current form is NESTED (inner 1410 inside outer 1397) which carries the extra single-level-insufficient documentation duty

- [s1] [fable-blitz 2026-07-07] Judge-clean parts confirmed: Blk16/Blk12 struct copies (src/code6cac.c:1411-1412,1417) match the elementwise lw/lw/sw/sw pairs at asm/6CAC.s:5064-5100 including the odd-word load-delay nop at asm/6CAC.s:5081 (Blk12's unpaired third word)
