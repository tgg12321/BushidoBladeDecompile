/* func_80018300 -- COMPLETED-INLINE-ASM-CANONICAL (manual lane, 2026-09-24).
 * Distance-constraint pass over a chain of 64-byte nodes. arg0+6 is the link
 * count, arg0+0xC the node array, arg0+0x10 a table of 16-byte links (word 0 =
 * rest length, word 1 = two packed node indices). For each link the node pair is
 * bisected (the midpoint replaces the node named by the low index half if
 * its word 6 is negative, else the node named by the high half, and that
 * node's words 3..5 are quartered) until every axis delta is within 3x the
 * rest length; the delta >> 3 is then squared on the
 * GTE, its length taken through the D_8008D118 byte-LUT integer sqrt (GTE LZC
 * above 0x400), and the delta >> 3 is scaled on the GTE by
 * f = ((len - rest) << 14) / len (GPF sf=1: (f * d) >> 12) into
 * the 0x1F8000BC output array. The last link is emitted after the loop without
 * the bisection.
 * `sandbox func_80018300 --disable all` = 0 (307/307).
 *
 * GTE ISLANDS: census member of the 2026-08-17 owner cluster ruling
 * (.claude/rules/cop2-addressing-preamble-cluster.md:61; registry row eeda6664b,
 * owner-instructed 2026-09-24, this function only). Each island is one PsyQ GTE
 * macro as spelled in PsyQ inline_o.h, the "DMPSX version 3" macro header
 * (Xeeynamo/croc@f30ff1ee include/psyq/inline_o.h, sha256 27a4abd6...81a9d6;
 * its $PSLibId$ is unexpanded, so no release is pinned), in three classes:
 * (a) 14 load/store islands (gte_ldlvl, gte_stlvnl, gte_ldlzc, gte_stlzc,
 * gte_lddp, gte_stlvl) = `move $12,%0` + the cop2 transfer; (b) 4 command
 * islands (gte_sqr0, gte_gpf12) = nop, nop + the cop2 command word; (c) 2
 * gte_nop islands = one bare `nop` each (inline_o.h:3068), filling the
 * post-loop LZC result delay (target mtc2 $t4,$30; nop; nop at 0x80018720-28).
 * Every statement clobbers "$12","$13","$14","$15","memory" as the header
 * writes it. That header is where the target's `addu $t4,<src>,$zero`
 * preamble comes from, and its clobber list is what the target's register
 * footprint shows: $t5-$t7 carry no value anywhere in the function,
 * count/out/data/radius sit in $s0/$s1/$t8/$t9 (with "$12","memory" they land
 * in $t7/$t8/$t5/$t6), and reload spills the constant island operands to $s2
 * (reload1.c bad_spill_regs <- regs_explicitly_used; measured on this body:
 * "$12","memory" alone scores 44 at 301 insns). Here each macro's statements
 * are joined into one __asm__ with the macro's own operand and clobber list,
 * and the header's `($12)` addressing is written `0($12)` (same encoding).
 * gte_sqr0 / gte_gpf12 carry the real cop2 words (0x4AA00428 SQR sf=0 lm=1,
 * 0x4B98003D GPF sf=1) in place of inline_o.h's DMPSX placeholders
 * 0x00000f3f / 0x000012bf; both words are the target's own bytes. */
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
            : : "r"((s32 *)0x1F800000) : "$12", "$13", "$14", "$15", "memory");
        /* gte_sqr0() -- inline_o.h:1749 */
        __asm__ volatile(
            "nop\n"
            "nop\n"
            ".word 0x4AA00428\n"
            : : : "$12", "$13", "$14", "$15", "memory");
        data += 4;
        count--;
        thresh = data[1];
        /* gte_stlvnl(r1) -- inline_o.h:2422 */
        __asm__ volatile(
            "move   $12, %0\n"
            "swc2   $25, 0($12)\n"
            "swc2   $26, 4($12)\n"
            "swc2   $27, 8($12)\n"
            : : "r"((s32 *)0x1F80000C) : "$12", "$13", "$14", "$15", "memory");
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
                : : "r"(sum) : "$12", "$13", "$14", "$15", "memory");
            p1 = (s32 *)(base + ((thresh >> 16) << 6));
            p2 = (s32 *)(base + ((thresh & 0xFFFF) << 6));
            /* gte_stlzc(r1) -- inline_o.h:2999 */
            __asm__ volatile(
                "move   $12, %0\n"
                "swc2   $31, 0($12)\n"
                : : "r"(lz) : "$12", "$13", "$14", "$15", "memory");
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
            : : "r"((s32 *)0x1F800000) : "$12", "$13", "$14", "$15", "memory");
        f = (s32)((len - radius) << 14) / (s32)len;
        radius = data[0];
        thresh = radius * 3;
        nthresh = -thresh;
        /* gte_lddp(r1) -- inline_o.h:443 */
        __asm__ volatile(
            "move   $12, %0\n"
            "mtc2   $12, $8\n"
            : : "r"(f) : "$12", "$13", "$14", "$15", "memory");
        /* gte_gpf12() -- inline_o.h:1875 */
        __asm__ volatile(
            "nop\n"
            "nop\n"
            ".word 0x4B98003D\n"
            : : : "$12", "$13", "$14", "$15", "memory");
        dx = p2[0] - p1[0];
        dy = p2[1] - p1[1];
        dz = p2[2] - p1[2];
        /* gte_stlvl(r1) -- inline_o.h:2403 */
        __asm__ volatile(
            "move   $12, %0\n"
            "swc2   $9, 0($12)\n"
            "swc2   $10, 4($12)\n"
            "swc2   $11, 8($12)\n"
            : : "r"(out) : "$12", "$13", "$14", "$15", "memory");
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
        : : "r"((s32 *)0x1F800000) : "$12", "$13", "$14", "$15", "memory");
    /* gte_sqr0() -- inline_o.h:1749 */
    __asm__ volatile(
        "nop\n"
        "nop\n"
        ".word 0x4AA00428\n"
        : : : "$12", "$13", "$14", "$15", "memory");
    /* gte_stlvnl(r1) -- inline_o.h:2422 */
    __asm__ volatile(
        "move   $12, %0\n"
        "swc2   $25, 0($12)\n"
        "swc2   $26, 4($12)\n"
        "swc2   $27, 8($12)\n"
        : : "r"((s32 *)0x1F80000C) : "$12", "$13", "$14", "$15", "memory");
    sum = *(s32 *)0x1F80000C + *(s32 *)0x1F800010 + *(s32 *)0x1F800014;
    if (sum < 0x400) {
        len = (&D_8008D118)[sum];
    } else {
        /* gte_ldlzc(r1) -- inline_o.h:645 */
        __asm__ volatile(
            "move   $12, %0\n"
            "mtc2   $12, $30\n"
            : : "r"(sum) : "$12", "$13", "$14", "$15", "memory");
        /* gte_nop() x2 -- inline_o.h:3068 */
        __asm__ volatile("nop" : : : "$12", "$13", "$14", "$15", "memory");
        __asm__ volatile("nop" : : : "$12", "$13", "$14", "$15", "memory");
        /* gte_stlzc(r1) -- inline_o.h:2999 */
        __asm__ volatile(
            "move   $12, %0\n"
            "swc2   $31, 0($12)\n"
            : : "r"(lz) : "$12", "$13", "$14", "$15", "memory");
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
        : : "r"((s32 *)0x1F800000) : "$12", "$13", "$14", "$15", "memory");
    f = (s32)((len - radius) << 14) / (s32)len;
    /* gte_lddp(r1) -- inline_o.h:443 */
    __asm__ volatile(
        "move   $12, %0\n"
        "mtc2   $12, $8\n"
        : : "r"(f) : "$12", "$13", "$14", "$15", "memory");
    /* gte_gpf12() -- inline_o.h:1875 */
    __asm__ volatile(
        "nop\n"
        "nop\n"
        ".word 0x4B98003D\n"
        : : : "$12", "$13", "$14", "$15", "memory");
    /* gte_stlvl(r1) -- inline_o.h:2403 */
    __asm__ volatile(
        "move   $12, %0\n"
        "swc2   $9, 0($12)\n"
        "swc2   $10, 4($12)\n"
        "swc2   $11, 8($12)\n"
        : : "r"(out) : "$12", "$13", "$14", "$15", "memory");
}
