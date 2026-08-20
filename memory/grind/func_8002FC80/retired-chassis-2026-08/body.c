s32 func_8002FC80(s32 *a0, s32 *a1, s32 *a2) {
    s32 *va;
    s32 *vb;
    s32 *out;
    s32 ret;
    /* CHEAT (audit 2026-08-18): the hardcoded-address __asm__ sw/lw islands
     * and $2/$3 register pins below are the ARCHIVED-FORBIDDEN scratchpad-gte
     * shape — NOT canonical (no inline_asm_canonical.txt entry; only cop2 ops
     * qualify). The prior comment here claiming this as "the canonical
     * scratchpad-write idiom" was false: the pure-C exhibit for the same
     * store is code6cac.c:3095 (COMPLETED-C). Everything in this block must
     * reach zero for COMPLETED-C. */
    {
        volatile s32 *va0 = (volatile s32 *)a0;
        volatile s32 *va1 = (volatile s32 *)a1;
        volatile s32 *va2 = (volatile s32 *)a2;
        register s32 v0 asm("$2");
        register s32 v1 asm("$3");
        v0 = va1[0]; v1 = va0[0]; v0 = v0 - v1;
        __asm__ volatile ("sw %0, 0x1F800360" :: "r"(v0));
        v0 = va1[1]; v1 = va0[1]; v0 = v0 - v1;
        __asm__ volatile ("sw %0, 0x1F800364" :: "r"(v0));
        v0 = va1[2]; v1 = va0[2]; v0 = v0 - v1;
        __asm__ volatile ("sw %0, 0x1F800368" :: "r"(v0));
        v0 = va2[0]; v1 = va0[0]; v0 = v0 - v1;
        __asm__ volatile ("sw %0, 0x1F800370" :: "r"(v0));
        v0 = va2[1]; v1 = va0[1]; v0 = v0 - v1;
        __asm__ volatile ("sw %0, 0x1F800374" :: "r"(v0));
        v0 = va2[2]; v1 = va0[2]; v0 = v0 - v1;
        __asm__ volatile ("sw %0, 0x1F800378" :: "r"(v0));
    }
    /* Load diff_a from scratchpad into GTE coef regs $0, $2, $4.
     * Pin source pointer to $a3 ($7) so lui+ori materialization uses $a3,
     * matching target's `lui $a3; ori $a3; addu $t4, $a3, $zero; ...`. */
    va = (s32 *)0x1F800360;
    {
        register s32 *q asm("$7") = va;
        register s32 *mp asm("$12");
        register s32 t5 asm("$13");
        register s32 t6 asm("$14");
        register s32 t7 asm("$15");
        __asm__ volatile ("move %0, %1" : "=r"(mp) : "r"(q));
        t5 = *mp;
        t6 = *(mp + 1);
        __asm__ volatile ("ctc2 %0, $0" :: "r"(t5));
        t7 = *(mp + 2);
        __asm__ volatile ("ctc2 %0, $2" :: "r"(t6));
        __asm__ volatile ("ctc2 %0, $4" :: "r"(t7));
    }
    /* Load diff_b from scratchpad into GTE IR regs and run OP cross product.
     * Pin source ptr to $a3 + move to $t4 to match target's lui+ori+addu pattern. */
    vb = (s32 *)0x1F800370;
    {
        register s32 *q asm("$7") = vb;
        register s32 *mp asm("$12");
        __asm__ volatile ("move %0, %1" : "=r"(mp) : "r"(q));
        __asm__ volatile ("lwc2 $11, 8(%0)" :: "r"(mp));
        __asm__ volatile ("lwc2 $9,  0(%0)" :: "r"(mp));
        __asm__ volatile ("lwc2 $10, 4(%0)" :: "r"(mp));
        __asm__ volatile ("nop");
        __asm__ volatile ("nop");
        __asm__ volatile (".word 0x4B70000C");  /* op 0 (cross product) */
    }
    /* Store MAC1/MAC2/MAC3 to scratchpad SCR[0x80..0x88], then call enemy-id
     * resolver with cross[0] and the scratchpad probe at 0x1F800388.
     * Out ptr pinned to $v0 so it survives until the lw $a0, 0($v0) call-arg
     * load (target reuses $v0 as both swc2 base and call-arg base). */
    {
        register s32 *q asm("$2") = (s32 *)0x1F800380;
        register s32 *mp asm("$12");
        s32 arg1_val;
        register s32 flag asm("$3");
        __asm__ volatile ("move %0, %1" : "=r"(mp) : "r"(q));
        __asm__ volatile ("swc2 $25, 0(%0)" :: "r"(mp));
        __asm__ volatile ("swc2 $26, 4(%0)" :: "r"(mp));
        __asm__ volatile ("swc2 $27, 8(%0)" :: "r"(mp));
        __asm__ volatile ("lw %0, 0x1F800388" : "=r"(arg1_val));
        ret = ratan2(q[0], arg1_val);
        __asm__ volatile ("lw %0, 0x1F800384" : "=r"(flag));
        if (flag > 0) {
            ret += 0x800;
        }
    }
    (void)out;
    return ret;
}
