# Evidence bank — func_80058580

- [s1] Current state: whole-body cheat = asmfix.txt:221 'replace_with_asmfile asm/funcs/func_80058580.s'; src/text1b.c:11893 is a (void)args stub. Queue distance 2989 ~= the full 2991-insn body (nothing matched). Verdict ASM-STRUCTURAL is a distance>500 auto-route; 2026-06-09 canonical audit REJECTED hand-coded (0-1/5 signals) - this is compiled C awaiting decomp.

- [s1] Scale: 2991 instructions, 0x80058580-0x8005B438 (asm/funcs/func_80058580.s, 3239 lines), 228 labels, 16 backward branches, 31 calls to 9 unique callees, 3 jr $v0 switch dispatches + jr $ra epilogue (asm line 3236). Largest single function in the queue alongside func_80023F08.

- [s1] ALL 9 callees already have C bodies: func_80079154 x13 (LCG rand, text1b_b.c:1868), func_80055B44 x6 (AI-script installer, text1b.c:11648 - sets +0x3B4=ptr,+0x3BC,+0x3B8,+0x3C8=0,+0x3CC=-1), func_8007E11C x5, file_GetFlag1 x2, func_80057094/571C0/57ACC/57E84 + single_game_getEnemyCharId x1 each. func_80057E84 is itself the sibling asmfile-replaced stub (text1b.c:11888).

- [s1] Function role: CPU-AI decision/think routine on robot struct s0=arg0. Fields: +0x0 opponent ptr, +0x6A u16 state, +0x443 char id, +0x430 u32 flags (masks 0x15100/0x8/0x10/0x20/0x40), +0x425/426/427 AI plan bytes, +0x3F3 pace counter, +0x43A angle (wrap +0x800 & 0xFFF to signed, asm lines 346-353), +0x43C threshold, +0x42A/2C/2E s16 pos vs opponent +0xF4/+0xFC s32 pos (3D dist-squared compare, asm lines 159-177 / 0x587C4-0x58808), +0x3B4..0x3CC installed-script block.

- [s1] Region map: R0 entry gates 0x58580-0x58624 (opp-state in {4,0x14} -> 0x599A0 skip; charid==0x16 -> 0x58A14). R1 script-trigger select 0x58624-0x58A2C (picks a1 in {D_8009A870/74/78/80/890}, calls func_80055B44(s0,a1,mode,0)). R2 0x58A2C-0x590D0 mask-select writes +0x3CC in {0x1000,0x2000,0x4000}, opp-state in {2,0x29,0x13,6} gates, rand-driven picks. R3 inner loop .L80058F1C. R4 0x590D0-0x59794 (unread bulk, func_8007E11C call sites). R5 loop 0x59794-0x597F8. R6 switch#1 0x59DC8. R7 loop cluster 0x59BB0/0x59C40/0x59C74. R8 switch#2 0x5A188 + switch#3 0x5A410. R9 big loops 0x5A124-0x5A360 and GIANT 0x5A640-0x5AC40 (~640 insns, the hardest region). R10 shared exits 0x5B404/0x5B408, epilogue 0x5B434.

- [s1] Switch dispatches: jr $v0 at 0x59DC8 via jtbl_8001585C (9 cases 0x59DD0..0x59F5C, small bodies); at 0x5A188 via jtbl_80015884 (5 cases, entries 0/2 and 1/3 SHARED -> C cases with fallthrough/shared labels A190/A238, +A2C0); at 0x5A410 via jtbl_8001589C (6 tiny sequential cases A418..A4B4, sltiu 0x6 bound).

- [s1] THE JTBL WALL: all 3 jump tables are hand-materialized const u32 arrays with absolute addresses in src/text1a_b_pre_rodata.c:322-353 @ 0x1585C/0x15884/0x1589C. bb2.ld links text1b.o(.rodata) at line 65, far AFTER text1a_b_pre_rodata.o (line 58) - a real C switch in text1b.c emits its jtbl at the wrong address. Needs the replay_camera_rob_back_loose2 asmfix bridge pattern (jtbl-rodata-split-infrastructure, [infra-rule: jtbl-infra] escape documented in memory/project/rodata-cleanup-progress.md:408-415) or evidence-based TU re-attribution.

- [s1] .L80058BD0 (asm line 440) is NOT a loop head: it is a 2-insn shared exit stub (sw $v0,0x3CC($s0); j .L800590D0) targeted by 7 LATER branch sites (0x58C68/0x58CD8/0x58CF0/0x58D14/0x58D1C/0x58DB4/0x590B8) - a jump2 cross-jump merged store+jump tail. C source must present per-path '+0x3CC = 0x2000' stores and let cross-jump merge them (cross-jump-store-tail-merge rule applies).

- [s1] Char-property table D_80099D88: stride 24 (sll 1 + addu + sll 3), indexed by charid +0x443, lhu at +0 with masks 0x8000/0x8F00/0xBF00/0xF00/0x300 (e.g. asm lines 105-115, 194-206, 252-263, 331-341). 20 %hi refs. Heaviest global: D_800A387C x28 refs. Div-pacing idiom at 0x586CC: div by ((s16)(+0x438<<16)>>24)+2 with break-7 guard = modulo counter pacing.

- [s1] m2c reference decompile SUCCEEDED clean: tmp/blitz/m2c_80058580.c (1321 lines, 0 decompilation errors, 5 switches / 4 loops / 65 gotos) using synthesized jtbl rodata tmp/blitz/jtbl_80058580.s (jtbl case labels exist in the .s as 'jlabel .LXXXXXXXX'). This is the scaffold source for session 2.

- [s1] No transplant shortcut: memory project note slog-kengo-dead-end records func_80058580 (2991) has NO Kengo equivalent at function, sub-region, or callee-signature level. First-principles decomposition only.

- [s1] Templates: text1b.c cpu_* AI siblings (cpu_side_move_dir_*, cpu_check_tubazeri_2, cpu_get_dist - same robot struct + D_80099D88 table) are the completed-C style reference for field spelling (raw offset casts, u8/s16 loads); same-file helpers func_80055B44/57094/571C0/57ACC show the established extern/type conventions this function must interoperate with.

- [s1] Trailing 0x00000000 words inside jtbl_8001585C[10] and jtbl_80015884[6] declarations are inter-object alignment padding, not case entries (real case counts 9 and 5 per the sltiu bounds) - a cc1-emitted switch jtbl will NOT contain them; the pre-rodata arrays absorb the pad today. Any jtbl-infra bridge must account for those 4-byte pads.
