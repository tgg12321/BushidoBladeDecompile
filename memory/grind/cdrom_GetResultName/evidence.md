# Evidence bank — cdrom_GetResultName

- Audit diagnosis (regressions.md): goto-done + void*ret accumulator (src/system.c:103-113) matches forbidden family 'goto-end-with-ret-val accumulator + shared label'; fails tests #1 and #5. Worker should attempt the two-direct-return form: if (idx < 0x7) { return (void *)g_cd_result_table[idx]; } return &g_str_none; — for this simple no-saved-register leaf, GCC 2.7.2 + find_cross_jump produces a single shared jr-ra epilogue naturally, so the simple form is highly likely byte-identical. If sandbox shows it isn't, record the specific diff and seek a clean structural lever (not goto-done).  (committed code flagged by the re-audit patrol; review and re-do in pure C if confirmed. The byte-correct construct stays on main until a clean replacement lands.)

- [s1] [fable-blitz 2026-07-07] Flagged construct at src/system.c:103-113: void *ret; if (idx < 0x7) { ret = g_cd_result_table[idx]; goto done; } ret = &g_str_none; done: return ret; - forbidden goto-end-with-ret-val accumulator per judge_constraints[0]

- [s1] [fable-blitz 2026-07-07] Target asm (build/bb2.elf @ 0x800801b4, 13 insns, leaf, no frame): andi a0,0xff; sltiu v0,a0,7; beqz v0,ELSE (sll v0,a0,2 delay); lui/addu at; lw v0,g_cd_result_table(at) [0x800A125C]; j DONE; nop; ELSE: lui/addiu v0,g_str_none (0x80016074); DONE: jr ra; nop

- [s1] [fable-blitz 2026-07-07] Instruction-for-instruction identical to cdrom_GetCmdName @ 0x80080180 except the sltiu bound (7 vs 28) and table %lo (0x125c vs 0x11dc) - whatever clean form closes the sibling closes this one

- [s1] [fable-blitz 2026-07-07] Judge constraint's own hypothesis (two direct returns; GCC 2.7.2 shared jr-ra epilogue arises naturally) is consistent with the asm: MIPS epilogue is textual, so per-return v0-set + jump-to-end IS the natural two-return codegen; the goto-done spelling buys nothing

- [s1] [fable-blitz 2026-07-07] No cross-jump concern: the two arm tails (lw v0 vs lui/addiu v0) share no suffix, so find_cross_jump has nothing to merge or unmerge in either spelling
