# Evidence bank — cdrom_GetCmdName

- Audit diagnosis (regressions.md): goto-end-with-ret-val accumulator pattern (void *ret; ... goto done; ... done: return ret;) is explicitly listed as a FORBIDDEN FAMILY in the cheat catalog — slipped through the mechanical gates because it uses entirely normal C syntax. Worker must redo with two direct return statements (if/return/return) or an equivalent pure-C form that has no goto+accumulator; the simpler form is the natural target.  (committed code flagged by the re-audit patrol; review and re-do in pure C if confirmed. The byte-correct construct stays on main until a clean replacement lands.)

- [s1] [fable-blitz 2026-07-07] Flagged construct at src/system.c:91-101: void *ret; if (idx < 0x1C) { ret = tbl[idx]; goto done; } ret = &g_str_none; done: return ret; - matches forbidden family goto-end-with-ret-val accumulator per judge_constraints[0]

- [s1] [fable-blitz 2026-07-07] Target asm (build/bb2.elf @ 0x80080180, 13 insns, leaf, no frame): andi a0,0xff; sltiu v0,a0,28; beqz v0,ELSE (sll v0,a0,2 in delay); lui/addu at; lw v0,g_cd_cmd_table(at); j DONE; nop; ELSE: lui v0,0x8001; addiu v0,24692 (g_str_none=0x80016074); DONE: jr ra; nop

- [s1] [fable-blitz 2026-07-07] Shape analysis: GCC 2.7.2 MIPS emits the epilogue TEXTUALLY (FUNCTION_EPILOGUE macro, not RTL), so multiple return statements each set v0 and jump to the single end-of-function label - there is no RTL return insn for jump.c to convert 'j DONE' into 'jr ra'. Two direct returns should reproduce the j-DONE + fallthrough-else + shared jr-ra shape byte-for-byte

- [s1] [fable-blitz 2026-07-07] Delay-slot check: sll v0,a0,2 in the beqz delay slot is reorg.c speculating the then-arm's index shift (safe: v0 dead on else path until its lui) - natural for if(idx<28) return tbl[idx]; no fence construct needed

- [s1] [fable-blitz 2026-07-07] The lw v0 -> j DONE -> nop sequence: nop fills the j's jump delay slot (nothing schedulable after the load); no maspsx load-delay nop is implicated because the jump does not consume v0

- [s1] [fable-blitz 2026-07-07] Sibling cdrom_GetResultName (src/system.c:103-113, asm @ 0x800801b4) is instruction-for-instruction identical except bound 7 vs 28 and table 0x800A125C vs 0x800A11DC - one clean form closes both
