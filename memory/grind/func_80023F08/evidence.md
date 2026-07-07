# Evidence bank — func_80023F08

- [s1] Current state: whole-body cheat = asmfix.txt:103 'replace_with_asmfile asm/funcs/func_80023F08.s'; src stub in code6cac.c. Queue distance 2981 ~= full 2983-insn body. ASM-STRUCTURAL verdict is the distance>500 auto-route; 2026-06-09 canonical audit REJECTED hand-coded - compiled C awaiting decomp.

- [s1] Scale/shape: 2983 insns (0x80023F08-0x80026DA0), 226 labels, 0x250-byte frame, 15 backward branches, SINGLE exit (jr $ra @ 0x26D9C), 65 calls to 38 unique callees. Crucially NO jr $v0 switch dispatch -> NO jtbl-infra wall (unlike sibling monster func_80058580). Structurally the EASIER of the two 2900+ monsters.

- [s1] Function role: per-player CPU move-command executor. Entry (asm lines 2-84): s1 = &D_80101EC8[arg0*0x44C] (17x->69x->276x->275x->1100x=0x44C strength-reduced stride, lines 5-11); +0x3C call counter++; copies 6 words arg1[0x0..0x14] -> s1+0x24..0x38; then 4x inline bit-repack of +0x2C/30/34/38: (v&0xFFF)|((v&0x8000)>>3)|((v&0x7000)<<1) (input-encoding conversion), gated by D_800A38BA!=0 and arg0 in {0,1} && s16 +0x6 == 0.

- [s1] SHARED-STRUCT payoff: D_80101EC8 stride 0x44C is the SAME robot-AI struct func_80058580 receives by pointer (both use u16 state +0x6A, opponent ptr +0x0; func_80058580 uses +0x425..0x449, this uses +0x24..0x82). code6cac_b.c:893 (COMPLETED) already spells 'var_s0 = &D_80101EC8 + idx * 0x44C' - raw-offset convention, no struct header; one offset map serves both monsters.

- [s1] Six GCC-2.7.2 movstrsi inline block-move expansions = the 12 paired backward branches: (0x24EF8/0x24F4C), (0x24FA4/0x24FF8), (0x252FC/0x25350), (0x253B0/0x25408), (0x26294/0x262E8), (0x26390/0x263E8). Each = runtime '(src|dst)&3' test dispatching lwl/lwr unaligned loop vs lw/sw aligned loop + 4-byte tail (asm lines 1100-1143). Copy size 0x84 = one move record; e.g. src = D_800A3888[arg0] + rec*0x84 (33x<<2 stride, lines 1091-1099) -> sp+0x9C, second copy -> sp+0x18.

- [s1] Block-move mechanism lead: the RUNTIME alignment dispatch means cc1 could not prove either pointer's alignment -> source was almost certainly builtin memcpy(dst_ptr, src_ptr, 0x84) on plain pointers, NOT a struct assignment (struct assign of a 4-aligned 0x84 type would emit the aligned loop only). Note bb2_memcpy in matched siblings (code6cac.c:1520) is an extern CALLED function - different idiom; no matched-src precedent found yet for the INLINE expansion.

- [s1] Real (non-blockmove) loops: 0x24448/0x24460 exit stubs + 0x244C4 move-record list walker (record ptr +0x50: level byte +0x8 vs +0x40, count u16 +0xA, data +0xC; input-bit test (1 << s16 +0xA) against record halfwords, lines 397-409) and 0x24E40 region state 0x15/0x33 handling reading byte table D_8008D9EC[+0xA].

- [s1] Direction-select if-chain at 0x244F4 (lines 410-445): (rec & 0x30) in {0,0x10,0x20,0x30} selects one of the four repacked words +0x2C/+0x30/+0x38/+0x34 into s2; (rec & 0x2000) zeroes it; state +0x6A==0x11 && D_800A38AE==arg0 also zeroes. m2c reconstructs this as if-chain (2 'switch' in m2c output are if-trees; no real jtbl).

- [s1] Command-echo global cluster written on several exit paths (lines 340-346, 370-381): D_800A376E (u16 flag=1), D_800A36D8 (u32), D_800A36CA (u16), D_800A381C (u16) - plus per-struct +0x7C (u32 cmd), +0x80/+0x82 (u16), +0x4C, +0x5E. Heavy callees: func_80021424 x10, func_80021A98(idx, +0x7C, s16 +0x80) x7, func_8001F860 x4 - all with existing C bodies.

- [s1] Init/side-effect singleton callees confirm role and give semantic anchors: md_game_rob_data_init, efc_rob_Init, single_game_SetAbilityData, camera_set_zoom, game_GetPlayerData, cpu_set_move_command_and_dir, cpu_check_tubazeri_2, cpu_check_same_dir_timer, scratchpad_Save/Restore, hirahira_w_ctrl, coli_check_circle_hit_line. 13 refs to Judge (sine table) = trig/angle math sections.

- [s1] m2c reference decompile SUCCEEDED clean first try (no jtbl needed): tmp/blitz/m2c_80023F08.c (1337 lines, 0 errors, 12 do + 12 while loops, 21 gotos, 47 stack locals). The 0x250 frame layout (record staging buffers sp+0x18 and sp+0x9C, 0x84 bytes each) is recoverable from m2c's sp-offset locals.

- [s1] No transplant shortcut: slog-kengo-dead-end memory records func_80023F08 (2983) has NO Kengo equivalent at function, sub-region, or callee-signature level. First-principles only.

- [s1] Unread bulk regions for later sessions: 0x240A0-0x24414 and ~0x25430-0x26290 (the big state-machine middle with most singleton calls); everything read so far is ordinary field-compare/branch code in the style of matched cpu_* siblings.
