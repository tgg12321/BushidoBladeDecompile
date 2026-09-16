/* REJECTED s11 (rederive) — fresh m2c decompile of asm/funcs/_exeque.s
 * (m2c --target=mipsel-ido-c --valid-syntax) reconstructs the final
 * guard/clear/call block as ONE merged && condition with no pointer
 * indirection:
 *
 *   if (D_8009BF78 == D_8009BF7C && !(*D_8009BF54 & 0x01000000) &&
 *       D_8009BE7C != 0 && D_8009BE80 != 0) {
 *       D_8009BE7C = 0;
 *       ((s32 (*)(void))D_8009BE80)();
 *   }
 *
 * Measured on the s4-s10 do-while(0)-wrapped floor-2/187 chassis
 * (memory/grind/_exeque/candidate.c), substituting ONLY this final
 * block for the nested-if/pointer-local form: sandbox score 5/187
 * (WORSE than the banked 2/187). KILLED instance.
 */
