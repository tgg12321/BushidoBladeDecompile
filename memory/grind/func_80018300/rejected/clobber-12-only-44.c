/* REJECTED ablation (2026-09-24): every island's clobber list is "$12",
 * "memory" (no $13-$15). sandbox --disable all = 44 at 301/307 insns.
 * Receipt for the inline_o.h clobber list.
 * Code is candidate.c with ONLY that change; the in-body comments are copied
 * verbatim from candidate.c and describe the candidate, not this variant. */
void func_80018300(s32 *arg0) {
    s32 *out;
    s32 *data;
    u8 *base;
    s32 count;
    s32 thresh;
    s32 radius;
    s32 nthresh;
    s32 *p1, *p2;
    s32 dx, dy, dz;
    u32 sum;
    u32 len;
    /* FAKE: n.b.! must be 17-24 bytes (s32 [5] and [6] are byte-identical).
     * OVERSIZED-LOCALS carve-out (.claude/rules/dead-vars-local-array.md, owner
     * ruling 2026-07-13), prong 2 (extend the LIVE locals object): lz[0] is the
     * GTE LZC output, written by gte_stlzc and read back. Frame math from
     * asm/funcs/func_80018300.s alone: frame 0x28, three saves $s0-$s2 at
     * 0x18/0x1C/0x20 (ALIGN8(12) = 16), no calls so no outgoing-args area, locals
     * region 0x00-0x17 = 24 bytes; the ONLY $sp traffic in it is the island's
     * `swc2 $31,0($t4)` ($t4 = $sp) and `lw $v1,0($sp)` -- 4 bytes. A fully
     * written 4-byte object gives ALIGN8(4)+16 = 0x18 != 0x28 (measured: lz[1]
     * and lz[4] score 8, lz[5] and lz[6] score 0, lz[7] scores 8).
     * lever-exhaustion:
     * memory/grind/func_80018300/hypotheses.md (phantom-slot probes: 19
     * single spelling swaps (14 instruction-neutral, 5 not) +
     * assignment-as-value + store-base forms, none move `vars=`). */
    s32 lz[6];
    s32 f;

    out = (s32 *)0x1F8000BC;
    count = *(s16 *)((u8 *)arg0 + 6) - 1;
    data = *(s32 **)((u8 *)arg0 + 0x10);
    base = *(u8 **)((u8 *)arg0 + 0xC);
    /* FAKE: the link's packed node-index word is staged through `thresh`,
     * which holds nothing yet here (and, in the loop, nothing between the range
     * check and its reassignment after the divide); the word is consumed by the
     * two node-pointer statements before `thresh = radius * 3` overwrites it.
     * mechanism: global.c find_reg -- as its own pseudo the word has no conflict
     * with dx/dy/p2 and takes the lowest free reg ($a1); sharing thresh's pseudo
     * seats it in $t1 as the target does (measured: own `pair` variable 8).
     * Family: staged-value-reused-variable (owner ruling 2026-07-03).
     * lever-exhaustion: memory/grind/func_80018300/hypotheses.md. */
    thresh = data[1];
    radius = data[0];
    p1 = (s32 *)(base + ((thresh >> 16) << 6));
    p2 = (s32 *)(base + ((thresh & 0xFFFF) << 6));
    thresh = radius * 3;
    nthresh = -thresh;
    dx = p2[0] - p1[0];
    dy = p2[1] - p1[1];
    dz = p2[2] - p1[2];
    while (count > 0) {
        while (dx < nthresh || thresh < dx || dy < nthresh || thresh < dy ||
               dz < nthresh || thresh < dz) {
            dx = (p2[0] + p1[0]) / 2;
            dy = (p2[1] + p1[1]) / 2;
            dz = (p2[2] + p1[2]) / 2;
            if (p2[6] < 0) {
                p2[0] = dx;
                p2[1] = dy;
                p2[2] = dz;
                p2[3] /= 4;
                p2[4] /= 4;
                p2[5] /= 4;
            } else {
                p1[0] = dx;
                p1[1] = dy;
                p1[2] = dz;
                p1[3] /= 4;
                p1[4] /= 4;
                p1[5] /= 4;
            }
            dx = p2[0] - p1[0];
            dy = p2[1] - p1[1];
            dz = p2[2] - p1[2];
        }
        *(s32 *)0x1F800000 = dx >> 3;
        *(s32 *)0x1F800004 = dy >> 3;
        *(s32 *)0x1F800008 = dz >> 3;
        /* gte_ldlvl(r1) -- inline_o.h:308 */
        __asm__ volatile(
            "move   $12, %0\n"
            "lwc2   $9, 0($12)\n"
            "lwc2   $10, 4($12)\n"
            "lwc2   $11, 8($12)\n"
            : : "r"((s32 *)0x1F800000) : "$12", "memory");
        /* gte_sqr0() -- inline_o.h:1749 */
        __asm__ volatile(
            "nop\n"
            "nop\n"
            ".word 0x4AA00428\n"
            : : : "$12", "memory");
        data += 4;
        count--;
        thresh = data[1];
        /* gte_stlvnl(r1) -- inline_o.h:2422 */
        __asm__ volatile(
            "move   $12, %0\n"
            "swc2   $25, 0($12)\n"
            "swc2   $26, 4($12)\n"
            "swc2   $27, 8($12)\n"
            : : "r"((s32 *)0x1F80000C) : "$12", "memory");
        sum = *(s32 *)0x1F80000C + *(s32 *)0x1F800010 + *(s32 *)0x1F800014;
        if (sum < 0x400) {
            len = (&D_8008D118)[sum];
            p1 = (s32 *)(base + ((thresh >> 16) << 6));
            p2 = (s32 *)(base + ((thresh & 0xFFFF) << 6));
        } else {
            /* gte_ldlzc(r1) -- inline_o.h:645; the next link's node pointers
             * are computed in the LZC result delay, before gte_stlzc. */
            __asm__ volatile(
                "move   $12, %0\n"
                "mtc2   $12, $30\n"
                : : "r"(sum) : "$12", "memory");
            p1 = (s32 *)(base + ((thresh >> 16) << 6));
            p2 = (s32 *)(base + ((thresh & 0xFFFF) << 6));
            /* gte_stlzc(r1) -- inline_o.h:2999 */
            __asm__ volatile(
                "move   $12, %0\n"
                "swc2   $31, 0($12)\n"
                : : "r"(lz) : "$12", "memory");
            /* FAKE: `len` carries the LZC count and then the table shift
             * before it takes the root; `sum` takes the table byte once its
             * last read (the shifted index) is done. Each staged value is read
             * by the next statement and each variable's old value is dead at
             * the write. mechanism: local-alloc.c/global.c seat order -- the
             * target keeps the count, the shift and the root in $v1 and the
             * byte in sum's $a0 (ablations on this body: fresh shift local
             * 36, fresh byte expression 38, no len = lz[0] copy 6).
             * Family: staged-value-reused-variable (owner ruling 2026-07-03);
             * same sum-for-byte reuse as func_8002F2D0 (src/code6cac_b.c).
             * lever-exhaustion:
             * memory/grind/func_80018300/hypotheses.md. */
            len = lz[0];
            len = 0x16 - (len & ~1);
            sum = (&D_8008D118)[sum >> len];
            len = (u32)(sum << 16) >> (0x10 - ((s32)len >> 1));
        }
        /* gte_ldlvl(r1) -- inline_o.h:308 */
        __asm__ volatile(
            "move   $12, %0\n"
            "lwc2   $9, 0($12)\n"
            "lwc2   $10, 4($12)\n"
            "lwc2   $11, 8($12)\n"
            : : "r"((s32 *)0x1F800000) : "$12", "memory");
        f = (s32)((len - radius) << 14) / (s32)len;
        radius = data[0];
        thresh = radius * 3;
        nthresh = -thresh;
        /* gte_lddp(r1) -- inline_o.h:443 */
        __asm__ volatile(
            "move   $12, %0\n"
            "mtc2   $12, $8\n"
            : : "r"(f) : "$12", "memory");
        /* gte_gpf12() -- inline_o.h:1875 */
        __asm__ volatile(
            "nop\n"
            "nop\n"
            ".word 0x4B98003D\n"
            : : : "$12", "memory");
        dx = p2[0] - p1[0];
        dy = p2[1] - p1[1];
        dz = p2[2] - p1[2];
        /* gte_stlvl(r1) -- inline_o.h:2403 */
        __asm__ volatile(
            "move   $12, %0\n"
            "swc2   $9, 0($12)\n"
            "swc2   $10, 4($12)\n"
            "swc2   $11, 8($12)\n"
            : : "r"(out) : "$12", "memory");
        out += 3;
    }
    *(s32 *)0x1F800000 = dx >> 3;
    *(s32 *)0x1F800004 = dy >> 3;
    *(s32 *)0x1F800008 = dz >> 3;
    /* gte_ldlvl(r1) -- inline_o.h:308 */
    __asm__ volatile(
        "move   $12, %0\n"
        "lwc2   $9, 0($12)\n"
        "lwc2   $10, 4($12)\n"
        "lwc2   $11, 8($12)\n"
        : : "r"((s32 *)0x1F800000) : "$12", "memory");
    /* gte_sqr0() -- inline_o.h:1749 */
    __asm__ volatile(
        "nop\n"
        "nop\n"
        ".word 0x4AA00428\n"
        : : : "$12", "memory");
    /* gte_stlvnl(r1) -- inline_o.h:2422 */
    __asm__ volatile(
        "move   $12, %0\n"
        "swc2   $25, 0($12)\n"
        "swc2   $26, 4($12)\n"
        "swc2   $27, 8($12)\n"
        : : "r"((s32 *)0x1F80000C) : "$12", "memory");
    sum = *(s32 *)0x1F80000C + *(s32 *)0x1F800010 + *(s32 *)0x1F800014;
    if (sum < 0x400) {
        len = (&D_8008D118)[sum];
    } else {
        /* gte_ldlzc(r1) -- inline_o.h:645 */
        __asm__ volatile(
            "move   $12, %0\n"
            "mtc2   $12, $30\n"
            : : "r"(sum) : "$12", "memory");
        /* gte_nop() x2 -- inline_o.h:3068 */
        __asm__ volatile("nop" : : : "$12", "memory");
        __asm__ volatile("nop" : : : "$12", "memory");
        /* gte_stlzc(r1) -- inline_o.h:2999 */
        __asm__ volatile(
            "move   $12, %0\n"
            "swc2   $31, 0($12)\n"
            : : "r"(lz) : "$12", "memory");
        /* FAKE: same staged len/sum reuse as the loop's LZC arm above. */
        len = lz[0];
        len = 0x16 - (len & ~1);
        sum = (&D_8008D118)[sum >> len];
        len = (u32)(sum << 16) >> (0x10 - ((s32)len >> 1));
    }
    /* gte_ldlvl(r1) -- inline_o.h:308 */
    __asm__ volatile(
        "move   $12, %0\n"
        "lwc2   $9, 0($12)\n"
        "lwc2   $10, 4($12)\n"
        "lwc2   $11, 8($12)\n"
        : : "r"((s32 *)0x1F800000) : "$12", "memory");
    f = (s32)((len - radius) << 14) / (s32)len;
    /* gte_lddp(r1) -- inline_o.h:443 */
    __asm__ volatile(
        "move   $12, %0\n"
        "mtc2   $12, $8\n"
        : : "r"(f) : "$12", "memory");
    /* gte_gpf12() -- inline_o.h:1875 */
    __asm__ volatile(
        "nop\n"
        "nop\n"
        ".word 0x4B98003D\n"
        : : : "$12", "memory");
    /* gte_stlvl(r1) -- inline_o.h:2403 */
    __asm__ volatile(
        "move   $12, %0\n"
        "swc2   $9, 0($12)\n"
        "swc2   $10, 4($12)\n"
        "swc2   $11, 8($12)\n"
        : : "r"(out) : "$12", "memory");
}
