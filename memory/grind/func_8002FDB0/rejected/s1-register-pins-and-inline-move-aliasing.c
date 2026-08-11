/* kengo:HIGH  |  nm_cpu/cpu_check_tubazeri  |  76i  |  x2 size collision */
s32 func_8002FDB0(s32 *arg0) {
    s32 stride;
    s32 v1, v2;
    s32 ret;

    stride = (s32)((s16 *)arg0)[2] * 264;

    /* Compute (point_a - center) into scratchpad SCR[0x60..0x68] and
     * (point_b - center) into SCR[0x70..0x78].  Source vectors live at
     * stride-offset slots in scratchpad (0xB4/0xB8/0xBC = center xyz;
     * 0xC0/0xC4/0xC8 = a xyz; 0xCC/0xD0/0xD4 = b xyz).
     *
     * Natural-C `*(s32 *)(0x1F8000XX + stride)` produces target's
     * lui+addu+lw 3-insn sequence; absolute-addr stores match
     * the lui+sw idiom from cpu_check_tubazeri. */
    v1 = *(s32 *)((u8 *)0x1F8000C0 + stride);
    v2 = *(s32 *)((u8 *)0x1F8000B4 + stride);
    *(s32 *)0x1F800360 = v1 - v2;

    v1 = *(s32 *)((u8 *)0x1F8000C4 + stride);
    v2 = *(s32 *)((u8 *)0x1F8000B8 + stride);
    *(s32 *)0x1F800364 = v1 - v2;

    v1 = *(s32 *)((u8 *)0x1F8000C8 + stride);
    v2 = *(s32 *)((u8 *)0x1F8000BC + stride);
    *(s32 *)0x1F800368 = v1 - v2;

    v1 = *(s32 *)((u8 *)0x1F8000CC + stride);
    v2 = *(s32 *)((u8 *)0x1F8000B4 + stride);
    *(s32 *)0x1F800370 = v1 - v2;

    v1 = *(s32 *)((u8 *)0x1F8000D0 + stride);
    v2 = *(s32 *)((u8 *)0x1F8000B8 + stride);
    *(s32 *)0x1F800374 = v1 - v2;

    v1 = *(s32 *)((u8 *)0x1F8000D4 + stride);
    {
        /* Pin v2 to $v0 ($2) — in the 6th block the diff result stays in
         * $v1 and stride goes in a different reg, so GCC normally uses $a0
         * for v2.  Target uses $v0 here. */
        register s32 v2_r asm("$2");
        v2_r = *(s32 *)((u8 *)0x1F8000BC + stride);
        /* Clobber $5 here so GCC defers q's lui+ori materialization until
         * AFTER the 6th-block loads — both insns then land in the 2nd lw's
         * load-delay slot (matching target's `lw $v0; lui $a1; ori $a1; subu`). */
        __asm__ volatile ("" ::: "$5");
        {
            register s32 *q asm("$5") = (s32 *)0x1F800360;
            register s32 *mp asm("$12");
            register s32 t5 asm("$13");
            register s32 t6 asm("$14");
            register s32 t7 asm("$15");
            v1 -= v2_r;
            *(s32 *)0x1F800378 = v1;
            __asm__ volatile ("move %0, %1" : "=r"(mp) : "r"(q));
            t5 = *mp;
            t6 = *(mp + 1);
            __asm__ volatile ("ctc2 %0, $0" :: "r"(t5));
            t7 = *(mp + 2);
            __asm__ volatile ("ctc2 %0, $2" :: "r"(t6));
            __asm__ volatile ("ctc2 %0, $4" :: "r"(t7));
        }
    }
    /* Load diff_b (SCR[0x70..0x78]) into GTE IR regs and run OP cross product. */
    {
        register s32 *q asm("$5") = (s32 *)0x1F800370;
        register s32 *mp asm("$12");
        __asm__ volatile ("move %0, %1" : "=r"(mp) : "r"(q));
        __asm__ volatile ("lwc2 $11, 8(%0)" :: "r"(mp));
        __asm__ volatile ("lwc2 $9,  0(%0)" :: "r"(mp));
        __asm__ volatile ("lwc2 $10, 4(%0)" :: "r"(mp));
        __asm__ volatile ("nop");
        __asm__ volatile ("nop");
        __asm__ volatile (".word 0x4B70000C");  /* op 0 (cross product) */
    }
    /* Store MAC1/MAC2/MAC3 to scratchpad SCR[0x80..0x88]. */
    {
        register s32 *q asm("$5") = (s32 *)0x1F800380;
        register s32 *mp asm("$12");
        __asm__ volatile ("move %0, %1" : "=r"(mp) : "r"(q));
        __asm__ volatile ("swc2 $25, 0(%0)" :: "r"(mp));
        __asm__ volatile ("swc2 $26, 4(%0)" :: "r"(mp));
        __asm__ volatile ("swc2 $27, 8(%0)" :: "r"(mp));
    }
    /* Read MAC2 from scratchpad and return slt(0, MAC2) — i.e. MAC2 > 0. */
    ret = *(s32 *)0x1F800384;
    return 0 < ret;
}

/* REJECTED (form, not bytes) — session 1, 2026-08-11.
 * This is the incoming HEAD body. It reaches the target bytes only with
 * cheat-asm KEPT (sandbox --disable all --keep-cheat-asm == 0); the honest
 * cheat-stripped score is 21. Its GTE tail is spelled with `register T x
 * asm("$N")` pins, `__asm__ volatile("move %0, %1")` INLINE_MOVE_ALIASING
 * blocks, and an `__asm__ volatile("" ::: "$5")` scheduling barrier — three
 * separate forbidden families. Superseded by candidate.c, which reaches the
 * same bytes at honest distance 0 with none of them. Do not revert to this.
 */
