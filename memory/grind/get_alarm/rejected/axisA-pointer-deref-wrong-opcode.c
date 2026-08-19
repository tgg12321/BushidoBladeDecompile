/* REJECTED (s2, structural). Axis A: reach target's 3-insn materialized
 * D_8009BF68 load by declaring it a pointer and dereferencing.
 *   extern s32 *D_8009BF68;   ... debug_printf(&D_80016044, *D_8009BF68, ...)
 * Result: score 12, build_insns=91 (== target's 91 — the insertion IS closed!)
 * but the middle instruction is WRONG:
 *   ours:   lui $r,%hi(BF68); lw $r,%lo(BF68)($r); lw $a1,0($r)   (load PTR, deref)
 *   target: lui $v0,%hi(BF68); addiu $v0,$v0,%lo(BF68); lw $a1,0($v0)  (la ADDR, load)
 * Target materializes the ADDRESS of D_8009BF68 (addiu), not a pointer value
 * load (lw). A pointer type gives lui;lw;lw — right count, wrong opcode at the
 * BF68 slot (addiu vs lw) → +diffs, score 12. So BF68 is NOT a pointer global.
 * KILLS the "pointer deref" spelling of H-A1/H-A2. */
