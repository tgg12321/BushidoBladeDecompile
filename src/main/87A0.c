/* 12 game functions, among them pad_ResetState. .text 0x80017FA0 (ROM 0x87A0). Start boundary:
 * LEGACY (the splat 6CAC segment edge). */
#define INCLUDE_ASM_USE_MACRO_INC 1
#include "common.h"
#include "include_asm.h"
#include "bb2.h"
#include "bb2_const.h"

/* Extern data declarations */
extern s32 g_pad_buf;

/* Extern function declarations */






extern s32 memcpy(s32 *, s32, s32);
extern s32 func_80054434(void);






























extern s32 rand();









extern void func_80018300(s32 *);

/* --- Functions from 6CAC segment (0x80017FA0 - 0x8003EDC0) --- */

/* func_80017FA0 - copies scaled fields out of the block at a0[3] into scratchpad
 * RAM (0x1F800000). ptr[0] is written scaled by 128; ptr[1] is the group count, and
 * each group writes three words scaled by 4 at a 0x18 stride plus one word
 * taken from the 0x68 array. Nothing happens when a0[3] is null.
 *
 * The outer loop's entry guard is spelled against the live counter,
 * `if (i < ptr[1])`, not `if (ptr[1] > 0)`: `i` then survives to frame layout,
 * so mips.c:compute_frame_size emits the target's empty 8-byte leaf frame
 * (`addiu sp,sp,-8` in the beqz delay slot / `addiu sp,sp,8`) while `i` lives
 * entirely in a register (the "folded loop-guard compare" producer of
 * .claude/rules/phantom-slot-frame-lever.md; same spelling as
 * src/code6cac_c2.c func_8003DBE4). The outer loop stays a real do-while: it
 * must keep its loop notes, because the target's `ac_base` store
 * (`sw v0,0xAC(t3)` with `addiu t3,t3,4`) is loop.c's reduced form. */
void func_80017FA0(s32 *a0) {
    s32 *scr = (s32 *)0x1F800000;
    s32 temp;
    s32 *ptr;

    temp = a0[3];
    if (temp == 0) {
        goto end;
    }
    ptr = (s32 *)temp;

    scr[0x2E] = ptr[0] << 7;

    {
        s32 i = 0;
        if (i < ptr[1]) {
            s32 *p68 = ptr;
            s32 *ac_base = (s32 *)0x1F800000;
            s32 sp_off = 0;
            do {
                s32 j = 0;
                s32 data_off = i << 5;
                s32 sp_inner = sp_off;
            /* FAKE: inner counted loop spelled goto-formed rather than
             * `do { ... } while (j < 2);` -- GCC 2.7.2 loop.c only analyses
             * NOTE_INSN_LOOP_BEG-delimited loops, which the front end emits for
             * for/while/do statements only. Under the do-while spelling
             * strength_reduce combines the three scratchpad stores' addresses
             * as DEST_ADDR givs of `sp_inner` and hoists one biased base (57
             * insns against the target's 61); as a goto loop each store keeps
             * its absolute address and maspsx expands it to the target's
             * `lui $at ; addu $at,$a1,$at ; sw $2,%lo($at)` shape
             * (tools/maspsx/maspsx/__init__.py:1183). */
            inner:
                {
                    s32 *dp = (s32 *)((u8 *)ptr + data_off);
                    *(s32 *)(0x1F800064 + sp_inner) = dp[2] << 2;
                    data_off += 0x10;
                    *(s32 *)(0x1F800068 + sp_inner) = dp[3] << 2;
                    j++;
                    *(s32 *)(0x1F80006C + sp_inner) = dp[4] << 2;
                    sp_inner += 0xC;
                }
                if (j < 2) {
                    goto inner;
                }
                ac_base[0x2B] = *(s32 *)((u8 *)p68 + 0x68) << 2;
                p68 = (s32 *)((u8 *)p68 + 4);
                sp_off += 0x18;
                i++;
                ac_base++;
            } while (i < ptr[1]);
        }
    }

    scr[0x18] = ((s32 *)a0[3])[1];
end:
    ;
}
/* func_80018094 -- loads the GTE rotation/translation from the MATRIX at arg0[1],
 * runs func_80017FA0, writes that MATRIX's translation minus arg1[10..12] to
 * scratchpad, scales it by a factor derived from its length (byte-LUT integer
 * sqrt, GTE LZC above 0x400; 0x100 beyond 250000), copies the MATRIX to arg1+5 and
 * runs func_80018300.
 *
 * COMPLETED-INLINE-ASM-CANONICAL: three PsyQ GTE inline-asm islands
 * (gte_SetRotMatrix, gte_SetTransMatrix, gte_Lzc), granted for this function by
 * the owner cop2 cluster ruling (.claude/rules/cop2-addressing-preamble-cluster.md,
 * SetRotMatrix/long-vector sub-family); everything around them is C. The LZC
 * island's operand list is exactly the granted form
 * `: "=m"(sp_tmp[0]) : "r"(lut) : "$2", "$12"`.
 *
 * Two register details shape the C around the LZC island:
 * (1) the pre-island `move $a0,$a1` (target parks it in the `beqz` delay slot):
 *     the `lut = sum_sq;` copy survives cse only if a NOTE_INSN_LOOP_END sits between
 *     the small arm's terminating BARRIER and the LZC arm's label, which is what an `if`
 *     with no else whose body is `do { ...; goto lzc_done; } while (0);` emits
 *     (tools/gcc-2.7.2/cse.c:8100-8125, the follow-jumps gate's backward walk).
 * (2) the small arm's LUT byte in $v0 (not $a0): `lut` and `sum_sq` have identical
 *     conflict sets and both prefer $4, so global.c's allocno_compare priority alone
 *     decides which takes $a0. A do-while(0) around the LZC arm raises that region's
 *     flow.c loop depth, so the same references count for more and `lut` wins the
 *     sort (BB2_ALLOC_DEBUG on the instrumented cc1).
 * Each of the three do-while(0) wraps and the 16-byte sp_tmp is load-bearing.
 */
typedef struct { s32 pad[9]; s32 x, y, z; } ScrV;
#define SCRV ((ScrV *)0x1F800000)
void func_80018094(s32 *arg0, s32 *arg1) {
    /* FAKE: frame layout -- unwritten tail sp_tmp[1..3] on the live LZC-output object;
     * sp_tmp must be 9-16 bytes (s32[3] and s32[4] are byte-identical). From
     * asm/funcs/func_80018094.s: frame 0x30 = outgoing args 0x10 + locals 0x10 +
     * callee-saves 0x10 (s0/s1/ra at 0x20/0x24/0x28); the only locals traffic in the
     * whole target is the island's `swc2 $31,0($t4)` with $t4 = $sp+0x10 and the
     * matching `lw $v1,0x10($sp)`, i.e. 4 bytes written of a 16-byte locals region. A
     * 4-byte locals set yields ALIGN8(4)+16+16 = 0x28 != 0x30 (function.c
     * assign_stack_local / mips.c compute_frame_size). OVERSIZED-LOCALS carve-out of
     * .claude/rules/dead-vars-local-array.md (extend the live locals object, not a
     * dead pad): sp_tmp[0] is the live LZC output. */
    s32 sp_tmp[4];
    s32 dx, dy, dz;
    s32 sum_sq;
    s32 scale;
    s32 *dst;
    u32 lut;

    /* PsyQ libgte inline macro gte_SetRotMatrix(r0) --- inline_c.h:297-310 (same spelling
     * as func_80019310 / func_800300B4; "memory" clobber ADDED, not SDK text -- precedent
     * src/code6cac_b.c:1116-1123 (func_8002D320's lwc2-read island: `"r"(vin) : "$12", "memory"`),
     * the same committed precedent FUNCTION that func_80019310's own CLOBBER PROVENANCE
     * paragraph cites in this file. */
    __asm__ volatile(
        "move   $12, %0\n"
        "lw     $13, 0($12)\n"
        "lw     $14, 4($12)\n"
        "ctc2   $13, $0\n"
        "ctc2   $14, $1\n"
        "lw     $13, 8($12)\n"
        "lw     $14, 12($12)\n"
        "lw     $15, 16($12)\n"
        "ctc2   $13, $2\n"
        "ctc2   $14, $3\n"
        "ctc2   $15, $4\n"
        :: "r"(arg0[1]) : "$12", "$13", "$14", "$15", "memory");
    /* PsyQ libgte inline macro gte_SetTransMatrix(r0) --- inline_c.h:360-369. */
    __asm__ volatile(
        "move   $12, %0\n"
        "lw     $13, 20($12)\n"
        "lw     $14, 24($12)\n"
        "ctc2   $13, $5\n"
        "lw     $15, 28($12)\n"
        "ctc2   $14, $6\n"
        "ctc2   $15, $7\n"
        :: "r"(arg0[1]) : "$12", "$13", "$14", "$15", "memory");

    func_80017FA0(arg0);

    dx = ((s32 *)arg0[1])[5] - arg1[10];
    SCRV->x = dx;
    dy = ((s32 *)arg0[1])[6] - arg1[11];
    SCRV->y = dy;
    dz = ((s32 *)arg0[1])[7] - arg1[12];
    SCRV->z = dz;

    sum_sq = (dx * dx) + (dy * dy) + (dz * dz);

    if (sum_sq > 250000) {
        scale = 0x100;
    } else if (sum_sq < 0) {
        scale = 0;
    } else {
        {
            /* FAKE: do{...}while(0) around the whole else-arm body -- flow.c
             * life_analysis / basic_block_loop_depth (tools/gcc-2.7.2/flow.c:440-471):
             * NOTE_INSN_LOOP_BEG/END raise the block's loop depth, and every reference in
             * the region is then weighted by that depth in `reg_n_refs[regno] += loop_depth`
             * (flow.c:2081), the numerator of global.c's allocno_compare priority.
             * Family: do-while-zero-exception. */
            do {
            /* FAKE: the LZC island's input operand staged through `lut`, the local the LZC arm
             * already owns for its LUT byte, mechanism: cse.c cse_end_of_basic_block's
             * follow-jumps gate (see the small arm's note) lets this copy survive to RA, where
             * global.c find_reg seats it at $a0 and reorg.c fills the `beqz` delay slot with it
             * -- reproducing the target's pre-island `move $a0,$a1` with no asm-operand device.
             * Liveness (bound 3): `lut` holds nothing at this point (its LZC-arm write comes
             * later), and the staged value is consumed by the island BEFORE that write, so the
             * borrow is safe in both directions.
             * Family: staged-value-reused-variable. */
            lut = sum_sq;
            if (sum_sq < 0x400) {
                /* FAKE: do{...}while(0) around the small arm's body, with the arm exited by
                 * `goto lzc_done` so this `if` has NO else, mechanism: cse.c
                 * cse_end_of_basic_block's follow-jumps gate -- expand_end_loop emits
                 * NOTE_INSN_LOOP_END after the `goto`'s BARRIER and before the if's false
                 * label, and the gate's backward walk (tools/gcc-2.7.2/cse.c:8112-8118)
                 * stops on a LOOP_END note, so cse1 AND cse2 refuse to extend the block into
                 * the LZC arm and the `lut = sum_sq;` island-input copy survives.
                 * Family: do-while-zero-exception. */
                do {
                    sum_sq = (u8)(g_sqrt_table_u8[sum_sq]) >> 3;
                    goto lzc_done;
                } while (0);
            }
            {
                s32 shift_a, shift_b;
                /* FAKE: third do{...}while(0), around the LZC arm's body -- the same
                 * flow.c loop-depth weighting lifts `lut`'s three in-arm references
                 * (the island operand, the LUT byte set, the <<16 use) from weight 2 to
                 * weight 3, so allocno_n_refs[lut] goes 8 -> 11 while sum_sq's goes 19 -> 21,
                 * and global.c's allocno_compare priority
                 * (floor_log2(n_refs)*n_refs/live_length*10000) becomes 47142 for `lut` versus
                 * 40000 for `sum_sq`. `lut` is then allocated first and takes $a0, sum_sq $a1,
                 * exactly as the target seats them, and the small arm's LUT byte is free to
                 * stay in its own short-lived pseudo at $v0. One wrap level is not enough:
                 * each of the three wraps is load-bearing and none subsumes another.
                 * Family: do-while-zero-exception. */
                /* PsyQ libgte inline macro gte_Lzc(r1,r2) --- gtemac.h:174-178, which
                 * expands to gte_ldlzc(r1) (inline_c.h:228-231, `mtc2 %0,$30`), two
                 * gte_nop() (inline_c.h:1346-1347), then gte_stlzc(r2)
                 * (inline_c.h:1318-1322, `swc2 $31,0(%0)`).  DISCLOSURE OF THE ADDRESSING
                 * PREAMBLE: the two `addu $t4, ..., $zero` moves and the `addiu $v0,$sp,0x10`
                 * are NOT part of those macros own text -- they are the operand-addressing
                 * preamble the original build inline expansion emitted around them (the
                 * `addu $t4,$aN,$zero` + cop2 idiom of the 28-function cluster in
                 * .claude/rules/cop2-addressing-preamble-cluster.md, of which this function
                 * is an enumerated member).  They are written literally here because
                 * the island must reproduce those bytes; `"=m"(sp_tmp[0])` names the frame
                 * slot the `addiu $v0,$sp,0x10` computes, and `"r"(lut)` the ldlzc input.
                 * The operand list is exactly the granted form -- no extra output, tied, or
                 * clobber operand. */
                do {
                __asm__ volatile(
                    "addu   $t4, %1, $zero\n"
                    "mtc2   $t4, $30\n"
                    "nop\n"
                    "nop\n"
                    "addiu  $v0, $sp, 0x10\n"
                    "addu   $t4, $v0, $zero\n"
                    "swc2   $31, 0($t4)\n"
                    : "=m"(sp_tmp[0])
                    : "r"(lut)
                    : "$2", "$12");
                {
                    s32 lw_v1 = sp_tmp[0];
                    s32 li_v0 = -2;
                    li_v0 = lw_v1 & li_v0;
                    shift_a = 0x16 - li_v0;
                }
                shift_b = shift_a >> 1;
                lut = (u8)(g_sqrt_table_u8[sum_sq >> shift_a]);
                sum_sq = ((s32)(lut << 16)) >> (0x13 - shift_b);
                } while (0);
            }
        lzc_done:
            scale = ((sum_sq << 6) / 500) + 0xC0;
            } while (0);
        }
    }
    SCRV->x = (SCRV->x * scale) >> 1;
    SCRV->y = (SCRV->y * scale) >> 1;
    SCRV->z = (SCRV->z * scale) >> 1;

    dst = arg1;
    *(MATRIX *)(dst + 5) = *(MATRIX *)arg0[1];
    func_80018300(dst);
}
/* func_80018300 -- COMPLETED-INLINE-ASM-CANONICAL.
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
 *
 * GTE ISLANDS: census member of the owner cop2 cluster ruling
 * (.claude/rules/cop2-addressing-preamble-cluster.md; per-function registry row,
 * commit eeda6664b). Each island is one PsyQ GTE
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
 * (reload1.c bad_spill_regs <- regs_explicitly_used; "$12","memory" alone does
 * not match). Here each macro's statements
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
    /* FAKE: frame layout -- lz must be 17-24 bytes (s32 [5] and [6] are
     * byte-identical; [1], [4] and [7] are not). OVERSIZED-LOCALS carve-out
     * (.claude/rules/dead-vars-local-array.md), extending the LIVE locals object:
     * lz[0] is the GTE LZC output, written by gte_stlzc and read back. Frame math
     * from asm/funcs/func_80018300.s alone: frame 0x28, three saves $s0-$s2 at
     * 0x18/0x1C/0x20 (ALIGN8(12) = 16), no calls so no outgoing-args area, locals
     * region 0x00-0x17 = 24 bytes; the ONLY $sp traffic in it is the island's
     * `swc2 $31,0($t4)` ($t4 = $sp) and `lw $v1,0($sp)` -- 4 bytes. A fully
     * written 4-byte object gives ALIGN8(4)+16 = 0x18 != 0x28. */
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
     * seats it in $t1 as the target does.
     * Family: staged-value-reused-variable. */
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
            len = g_sqrt_table_u8[sum];
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
             * byte in sum's $a0 (a fresh shift local, a fresh byte expression
             * or dropping the len = lz[0] copy each break the match).
             * Family: staged-value-reused-variable; same sum-for-byte reuse
             * as func_8002F2D0 (src/code6cac_b.c). */
            len = lz[0];
            len = 0x16 - (len & ~1);
            sum = g_sqrt_table_u8[sum >> len];
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
        len = g_sqrt_table_u8[sum];
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
        sum = g_sqrt_table_u8[sum >> len];
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
void func_800187F4(s16 *arg0, s32 *arg1);
void func_80019310(s16 *arg0, s32 *arg1);
/* func_800187F4 -- COMPLETED-INLINE-ASM-CANONICAL.
 * Node-chain integrator. func_8001924C calls it for each 16-byte record (arg0;
 * +0xC enables collision) whose flag bit 0 is clear, with the record's descriptor
 * (arg1: +0 table of 8-byte anchor vectors, +4 s16 node count, +0xC the 64-byte
 * nodes). func_80018094 first sets up the GTE rotation/translation. Per node
 * (words 0-2 position, 3-5 velocity, 6 state, 7/8 force counts, 9-12 packed
 * force-table indices): state >= 0 springs the node toward its GTE-transformed
 * anchor (or snaps to it at 0) and ends there; state -0xFF..-1 first pulls the
 * position toward the anchor and then integrates like state < -0xFF: the indexed
 * scratchpad forces are added / subtracted, the node is pushed out of the ground and out of each
 * collision ellipsoid (inside when its distances to the two foci sum below the
 * bound; lengths via the D_8008D118 byte-LUT integer sqrt, with the GTE
 * leading-zero count above 0x400; the push applied on the GTE with GPF/GPL), and
 * the velocity is damped by 7/8 with 0x190 added to Y.
 *
 * GTE ISLANDS: each island is one PsyQ Run-time Library 4.3 inline_o.h macro
 * (or gtemac.h gte_Lzc), written statement for statement as the header writes
 * it: the copy pinned in engine/gtemacro.py PINNED (silent-hill-decomp@a1f407cb
 * include/psyq/inline_o.h, sha256 76f28032...; gtemac.h 9fe028fd...), class route
 * inline-asm-policy.md § Owner ruling 2026-09-26. The seven gte_rtv0tr / gte_sqr0 /
 * gte_gpf0 / gte_gpl12 units carry the post-DMPSX command word in place of the
 * header's DMPSX placeholder, under § Per-function grant: func_800187F4 (owner
 * ruling Q29): 0x0000027f -> 0x4A480012, 0x00000f3f -> 0x4AA00428,
 * 0x000012ff -> 0x4B90003D, 0x0000133f -> 0x4BA8003E. */
typedef struct {
    s32 d0[3];      /* 0x00 delta to focus 0 (GTE input) */
    s32 d1[3];      /* 0x0C delta to focus 1 (GTE input) */
    s32 vel[3];     /* 0x18 velocity / GTE vector */
    s32 dpos[3];    /* 0x24 displacement added to every node */
    s32 sq[3];      /* 0x30 GTE squares */
    s32 pos[3];     /* 0x3C node position */
    s32 cpos[3];    /* 0x48 node position >> 5 */
    s32 unk54[3];   /* 0x54 */
    s32 nsph;       /* 0x60 ellipsoid count */
    s32 sph[3][6];  /* 0x64 ellipsoid foci 0 and 1 */
    s32 rad[3];     /* 0xAC ellipsoid bounds (sum of the two focus distances) */
    s32 ground;     /* 0xB8 ground height */
    s32 force[0][3]; /* 0xBC force table, indexed by the node's packed bytes */
} Scr1F800000;
#define SCR ((Scr1F800000 *)0x1F800000)
void func_800187F4(s16 *arg0, s32 *arg1) {
    s32 *node;
    s32 i;
    s32 count;
    s32 vx, vy, vz;
    s32 bits, bits2;
    s32 r;
    s32 vy_new;
    s32 tot, pen;
    /* FAKE: frame layout -- lz must be 17-24 bytes (s32 [5] and [6] are
     * byte-identical). OVERSIZED-LOCALS carve-out
     * (.claude/rules/dead-vars-local-array.md), extending the LIVE locals object:
     * lz[0] and lz[1] are the two GTE leading-zero-count outputs, written by
     * gte_stlzc and read back. Frame from asm/funcs/func_800187F4.s alone: frame 0x78 = outgoing args
     * 0x10 + locals 0x40 + ten saves $s0-$s7/$fp/$ra at 0x50-0x74; the only locals
     * traffic is lz[0]/lz[1] at sp+0x10/0x14 and the count spill at sp+0x48.
     * Of the 0x40, 8 are the spill slot and 32 (0x28-0x47) are the four 8-byte
     * phantom slots of the combine orphan-USE loop-guard pseudos (the frame of the
     * lz[2] form: 0x68); the 24 bytes left (sp+0x10-0x27) are this object's:
     * lz[0]/lz[1] written by gte_stlzc, then a 16-byte unwritten tail.
     * lz[2] gives frame 0x68, lz[3]/lz[4] 0x70, lz[5]/lz[6] 0x78,
     * lz[7]/lz[8] 0x80. */
    s32 lz[6];

    func_80018094((s32 *)arg0, arg1);
    count = *(s16 *)((u8 *)arg1 + 4);
    node = (s32 *)arg1[3];

    for (i = 0; i < count; i++, node += 16) {
        /* Ruling 11 (reused local): three values, all loop indices -- the
         * add-force loop's, the subtract-force loop's and the ellipsoid loop's. */
        s32 idx;
        /* Ruling 11 (reused local): two values, both force counts -- node
         * word 7 (forces added) and node word 8 (forces subtracted). */
        s32 nforce;

        SCR->pos[0] = node[0];
        SCR->pos[1] = node[1];
        SCR->pos[2] = node[2];
        if (node[6] >= 0) {
            /* gte_ldv0(r1) -- inline_o.h 4.3 :16-20 */
            __asm__ volatile ("move  $12,%0": :"r"(arg1[0] + i * 8):"$12","$13","$14","$15","memory");
            __asm__ volatile ("lwc2  $0,($12)": : :"$12","$13","$14","$15","memory");
            __asm__ volatile ("lwc2  $1,4($12)": : :"$12","$13","$14","$15","memory");
            /* gte_rtv0tr() -- inline_o.h 4.3 :451-455; post-DMPSX word 0x4A480012
             * for the header placeholder 0x0000027f (owner Q29) */
            __asm__ volatile ("nop   ": : :"$12","$13","$14","$15","memory");
            __asm__ volatile ("nop   ": : :"$12","$13","$14","$15","memory");
            __asm__ volatile (".word 0x4A480012": : :"$12","$13","$14","$15","memory");
            /* gte_stlvnl(r1) -- inline_o.h 4.3 :904-909 */
            __asm__ volatile ("move  $12,%0": :"r"(SCR->vel):"$12","$13","$14","$15","memory");
            __asm__ volatile ("swc2  $25,($12)": : :"$12","$13","$14","$15","memory");
            __asm__ volatile ("swc2  $26,4($12)": : :"$12","$13","$14","$15","memory");
            __asm__ volatile ("swc2  $27,8($12)": : :"$12","$13","$14","$15","memory");
            if (node[6] > 0) {
                SCR->pos[0] += SCR->dpos[0];
                node[3] = node[3] / 2 + ((((SCR->vel[0] << 7) - SCR->pos[0]) * node[6]) >> 8);
                node[0] = SCR->pos[0] + node[3];
                SCR->pos[1] += SCR->dpos[1];
                node[4] = node[4] / 2 + ((((SCR->vel[1] << 7) - SCR->pos[1]) * node[6] + (0x100 - node[6]) * 25) >> 8);
                node[1] = SCR->pos[1] + node[4];
                SCR->pos[2] += SCR->dpos[2];
                node[5] = node[5] / 2 + ((((SCR->vel[2] << 7) - SCR->pos[2]) * node[6]) >> 8);
                node[2] = SCR->pos[2] + node[5];
            } else {
                node[0] = SCR->vel[0] << 7;
                node[1] = SCR->vel[1] << 7;
                node[2] = SCR->vel[2] << 7;
            }
            continue;
        }
        if (node[6] >= -0xFF) {
            /* gte_ldv0(r1) -- inline_o.h 4.3 :16-20 */
            __asm__ volatile ("move  $12,%0": :"r"(arg1[0] + i * 8):"$12","$13","$14","$15","memory");
            __asm__ volatile ("lwc2  $0,($12)": : :"$12","$13","$14","$15","memory");
            __asm__ volatile ("lwc2  $1,4($12)": : :"$12","$13","$14","$15","memory");
            /* gte_rtv0tr() -- inline_o.h 4.3 :451-455; post-DMPSX word 0x4A480012
             * for the header placeholder 0x0000027f (owner Q29) */
            __asm__ volatile ("nop   ": : :"$12","$13","$14","$15","memory");
            __asm__ volatile ("nop   ": : :"$12","$13","$14","$15","memory");
            __asm__ volatile (".word 0x4A480012": : :"$12","$13","$14","$15","memory");
            /* gte_stlvnl(r1) -- inline_o.h 4.3 :904-909 */
            __asm__ volatile ("move  $12,%0": :"r"(SCR->vel):"$12","$13","$14","$15","memory");
            __asm__ volatile ("swc2  $25,($12)": : :"$12","$13","$14","$15","memory");
            __asm__ volatile ("swc2  $26,4($12)": : :"$12","$13","$14","$15","memory");
            __asm__ volatile ("swc2  $27,8($12)": : :"$12","$13","$14","$15","memory");
            SCR->pos[0] -= (((SCR->vel[0] << 7) - SCR->pos[0]) * node[6]) >> 8;
            SCR->pos[1] -= (((SCR->vel[1] << 7) - SCR->pos[1]) * node[6]) >> 8;
            SCR->pos[2] -= (((SCR->vel[2] << 7) - SCR->pos[2]) * node[6]) >> 8;
        }
        vx = node[3];
        vy = node[4];
        vz = node[5];
        nforce = node[7];
        bits = node[9];
        for (idx = 0; idx < nforce; idx++) {
            s32 *f_add;

            f_add = SCR->force[bits & 0xFF];
            vx += f_add[0];
            vy += f_add[1];
            vz += f_add[2];
            if (idx == 3) {
                bits = node[10];
            } else {
                bits >>= 8;
            }
        }
        nforce = node[8];
        bits2 = node[11];
        for (idx = 0; idx < nforce; idx++) {
            s32 *f_sub;

            f_sub = SCR->force[bits2 & 0xFF];
            vx -= f_sub[0];
            vy -= f_sub[1];
            vz -= f_sub[2];
            if (idx == 3) {
                bits2 = node[12];
            } else {
                bits2 >>= 8;
            }
        }
        SCR->vel[0] = vx;
        SCR->vel[1] = vy;
        SCR->vel[2] = vz;
        if (*(s32 *)((u8 *)arg0 + 0xC) != 0) {
            /* Ruling 11 (reused local): two values, both Y deltas -- the
             * node's depth below the ground, then the Y delta to focus 0. */
            s32 delta;

            delta = SCR->pos[1] - SCR->ground;
            if (delta > 0) {
                if (delta > 0x3200) {
                    vy_new = vy - 0x400;
                } else {
                    vy_new = vy - delta / 8;
                }
                SCR->vel[1] = vy_new;
            }
            SCR->cpos[0] = SCR->pos[0] >> 5;
            SCR->cpos[1] = SCR->pos[1] >> 5;
            SCR->cpos[2] = SCR->pos[2] >> 5;
            for (idx = 0; idx < SCR->nsph; idx++) {
                s32 dx0, dz0, dy1, dx1, dz1;
                s32 sq2, dist2;
                /* Ruling 11 (reused local): three values -- a copy of the
                 * squared length for the leading-zero-count macro (a GTE-macro
                 * input copy, owner ruling Q28), then the focus-0 table byte,
                 * then the focus-1 table byte. */
                s32 temp;
                /* Ruling 11 (reused local): two values -- the focus-0 squared
                 * distance, then the distance (scaled to its push factor below). */
                s32 work;

                r = SCR->rad[idx];
                delta = SCR->cpos[1] - SCR->sph[idx][1];
                if (delta < -r || r < delta) {
                    continue;
                }
                SCR->d0[1] = delta;
                dx0 = SCR->cpos[0] - SCR->sph[idx][0];
                if (dx0 < -r || r < dx0) {
                    continue;
                }
                SCR->d0[0] = dx0;
                dz0 = SCR->cpos[2] - SCR->sph[idx][2];
                if (dz0 < -r || r < dz0) {
                    continue;
                }
                SCR->d0[2] = dz0;
                /* gte_ldlvl(r1) -- inline_o.h 4.3 :104-109 */
                __asm__ volatile ("move  $12,%0": :"r"(SCR->d0):"$12","$13","$14","$15","memory");
                __asm__ volatile ("lwc2  $9,($12)": : :"$12","$13","$14","$15","memory");
                __asm__ volatile ("lwc2  $10,4($12)": : :"$12","$13","$14","$15","memory");
                __asm__ volatile ("lwc2  $11,8($12)": : :"$12","$13","$14","$15","memory");
                /* gte_sqr0() -- inline_o.h 4.3 :646-650; post-DMPSX word 0x4AA00428
                 * for the header placeholder 0x00000f3f (owner Q29) */
                __asm__ volatile ("nop   ": : :"$12","$13","$14","$15","memory");
                __asm__ volatile ("nop   ": : :"$12","$13","$14","$15","memory");
                __asm__ volatile (".word 0x4AA00428": : :"$12","$13","$14","$15","memory");
                /* gte_stlvnl(r1) -- inline_o.h 4.3 :904-909 */
                __asm__ volatile ("move  $12,%0": :"r"(SCR->sq):"$12","$13","$14","$15","memory");
                __asm__ volatile ("swc2  $25,($12)": : :"$12","$13","$14","$15","memory");
                __asm__ volatile ("swc2  $26,4($12)": : :"$12","$13","$14","$15","memory");
                __asm__ volatile ("swc2  $27,8($12)": : :"$12","$13","$14","$15","memory");
                work = SCR->sq[0] + SCR->sq[1] + SCR->sq[2];
                temp = work;
                if (work < 0x400) {
                    work = g_sqrt_table_u8[work] >> 3;
                } else {
                    /* Ruling 11 (reused local): two values, both bit counts --
                     * the leading-zero count, then the table shift. */
                    s32 nbits;

                    /* gte_Lzc(r1,r2) -- gtemac.h 4.3 :174-178 = gte_ldlzc :207-210,
                     * gte_nop :1095-1097 twice, gte_stlzc :1074-1077 */
                    __asm__ volatile ("move  $12,%0": :"r"(temp):"$12","$13","$14","$15","memory");
                    __asm__ volatile ("mtc2  $12,$30": : :"$12","$13","$14","$15","memory");
                    __asm__ volatile ("nop   ": : :"$12","$13","$14","$15","memory");
                    __asm__ volatile ("nop   ": : :"$12","$13","$14","$15","memory");
                    __asm__ volatile ("move  $12,%0": :"r"(&lz[0]):"$12","$13","$14","$15","memory");
                    __asm__ volatile ("swc2  $31,($12)": : :"$12","$13","$14","$15","memory");
                    nbits = lz[0];
                    nbits = 0x16 - (nbits & ~1);
                    temp = g_sqrt_table_u8[work >> nbits];
                    work = (temp << 16) >> (0x13 - (nbits >> 1));
                }
                if (work >= r) {
                    continue;
                }
                dy1 = SCR->cpos[1] - SCR->sph[idx][4];
                if (dy1 < -r || r < dy1) {
                    continue;
                }
                SCR->d1[1] = dy1;
                dx1 = SCR->cpos[0] - SCR->sph[idx][3];
                if (dx1 < -r || r < dx1) {
                    continue;
                }
                SCR->d1[0] = dx1;
                dz1 = SCR->cpos[2] - SCR->sph[idx][5];
                if (dz1 < -r || r < dz1) {
                    continue;
                }
                SCR->d1[2] = dz1;
                /* gte_ldlvl(r1) -- inline_o.h 4.3 :104-109 */
                __asm__ volatile ("move  $12,%0": :"r"(SCR->d1):"$12","$13","$14","$15","memory");
                __asm__ volatile ("lwc2  $9,($12)": : :"$12","$13","$14","$15","memory");
                __asm__ volatile ("lwc2  $10,4($12)": : :"$12","$13","$14","$15","memory");
                __asm__ volatile ("lwc2  $11,8($12)": : :"$12","$13","$14","$15","memory");
                /* gte_sqr0() -- inline_o.h 4.3 :646-650; post-DMPSX word 0x4AA00428
                 * for the header placeholder 0x00000f3f (owner Q29) */
                __asm__ volatile ("nop   ": : :"$12","$13","$14","$15","memory");
                __asm__ volatile ("nop   ": : :"$12","$13","$14","$15","memory");
                __asm__ volatile (".word 0x4AA00428": : :"$12","$13","$14","$15","memory");
                /* gte_stlvnl(r1) -- inline_o.h 4.3 :904-909 */
                __asm__ volatile ("move  $12,%0": :"r"(SCR->sq):"$12","$13","$14","$15","memory");
                __asm__ volatile ("swc2  $25,($12)": : :"$12","$13","$14","$15","memory");
                __asm__ volatile ("swc2  $26,4($12)": : :"$12","$13","$14","$15","memory");
                __asm__ volatile ("swc2  $27,8($12)": : :"$12","$13","$14","$15","memory");
                sq2 = SCR->sq[0] + SCR->sq[1] + SCR->sq[2];
                if (sq2 < 0x400) {
                    dist2 = g_sqrt_table_u8[sq2] >> 3;
                } else {
                    /* Ruling 11 (reused local): two values, both bit counts --
                     * the leading-zero count, then the table shift. */
                    s32 nbits2;

                    /* gte_Lzc(r1,r2) -- gtemac.h 4.3 :174-178 = gte_ldlzc :207-210,
                     * gte_nop :1095-1097 twice, gte_stlzc :1074-1077 */
                    __asm__ volatile ("move  $12,%0": :"r"(sq2):"$12","$13","$14","$15","memory");
                    __asm__ volatile ("mtc2  $12,$30": : :"$12","$13","$14","$15","memory");
                    __asm__ volatile ("nop   ": : :"$12","$13","$14","$15","memory");
                    __asm__ volatile ("nop   ": : :"$12","$13","$14","$15","memory");
                    __asm__ volatile ("move  $12,%0": :"r"(&lz[1]):"$12","$13","$14","$15","memory");
                    __asm__ volatile ("swc2  $31,($12)": : :"$12","$13","$14","$15","memory");
                    nbits2 = lz[1];
                    nbits2 = 0x16 - (nbits2 & ~1);
                    temp = g_sqrt_table_u8[sq2 >> nbits2];
                    dist2 = (temp << 16) >> (0x13 - (nbits2 >> 1));
                }
                tot = work + dist2;
                if (tot >= r) {
                    continue;
                }
                /* gte_lddp(r1) -- inline_o.h 4.3 :144-147 */
                __asm__ volatile ("move  $12,%0": :"r"(1):"$12","$13","$14","$15","memory");
                __asm__ volatile ("mtc2  $12,$8": : :"$12","$13","$14","$15","memory");
                /* gte_ldlvl(r1) -- inline_o.h 4.3 :104-109 */
                __asm__ volatile ("move  $12,%0": :"r"(SCR->vel):"$12","$13","$14","$15","memory");
                __asm__ volatile ("lwc2  $9,($12)": : :"$12","$13","$14","$15","memory");
                __asm__ volatile ("lwc2  $10,4($12)": : :"$12","$13","$14","$15","memory");
                __asm__ volatile ("lwc2  $11,8($12)": : :"$12","$13","$14","$15","memory");
                /* gte_gpf0() -- inline_o.h 4.3 :721-725; post-DMPSX word 0x4B90003D
                 * for the header placeholder 0x000012ff (owner Q29) */
                __asm__ volatile ("nop   ": : :"$12","$13","$14","$15","memory");
                __asm__ volatile ("nop   ": : :"$12","$13","$14","$15","memory");
                __asm__ volatile (".word 0x4B90003D": : :"$12","$13","$14","$15","memory");
                pen = (r - tot) << 17;
                if (pen > 0x400000) {
                    pen = 0x400000;
                }
                if (work != 0) {
                    work = pen / work;
                }
                /* gte_ldlvl(r1) -- inline_o.h 4.3 :104-109 */
                __asm__ volatile ("move  $12,%0": :"r"(SCR->d0):"$12","$13","$14","$15","memory");
                __asm__ volatile ("lwc2  $9,($12)": : :"$12","$13","$14","$15","memory");
                __asm__ volatile ("lwc2  $10,4($12)": : :"$12","$13","$14","$15","memory");
                __asm__ volatile ("lwc2  $11,8($12)": : :"$12","$13","$14","$15","memory");
                /* gte_lddp(r1) -- inline_o.h 4.3 :144-147 */
                __asm__ volatile ("move  $12,%0": :"r"(work):"$12","$13","$14","$15","memory");
                __asm__ volatile ("mtc2  $12,$8": : :"$12","$13","$14","$15","memory");
                /* gte_gpl12() -- inline_o.h 4.3 :726-730; post-DMPSX word 0x4BA8003E
                 * for the header placeholder 0x0000133f (owner Q29) */
                __asm__ volatile ("nop   ": : :"$12","$13","$14","$15","memory");
                __asm__ volatile ("nop   ": : :"$12","$13","$14","$15","memory");
                __asm__ volatile (".word 0x4BA8003E": : :"$12","$13","$14","$15","memory");
                if (dist2 != 0) {
                    dist2 = pen / dist2;
                }
                /* gte_ldlvl(r1) -- inline_o.h 4.3 :104-109 */
                __asm__ volatile ("move  $12,%0": :"r"(SCR->d1):"$12","$13","$14","$15","memory");
                __asm__ volatile ("lwc2  $9,($12)": : :"$12","$13","$14","$15","memory");
                __asm__ volatile ("lwc2  $10,4($12)": : :"$12","$13","$14","$15","memory");
                __asm__ volatile ("lwc2  $11,8($12)": : :"$12","$13","$14","$15","memory");
                /* gte_lddp(r1) -- inline_o.h 4.3 :144-147 */
                __asm__ volatile ("move  $12,%0": :"r"(dist2):"$12","$13","$14","$15","memory");
                __asm__ volatile ("mtc2  $12,$8": : :"$12","$13","$14","$15","memory");
                /* gte_gpl12() -- inline_o.h 4.3 :726-730; post-DMPSX word 0x4BA8003E
                 * for the header placeholder 0x0000133f (owner Q29) */
                __asm__ volatile ("nop   ": : :"$12","$13","$14","$15","memory");
                __asm__ volatile ("nop   ": : :"$12","$13","$14","$15","memory");
                __asm__ volatile (".word 0x4BA8003E": : :"$12","$13","$14","$15","memory");
                /* gte_stlvl(r1) -- inline_o.h 4.3 :898-903 */
                __asm__ volatile ("move  $12,%0": :"r"(SCR->vel):"$12","$13","$14","$15","memory");
                __asm__ volatile ("swc2  $9,($12)": : :"$12","$13","$14","$15","memory");
                __asm__ volatile ("swc2  $10,4($12)": : :"$12","$13","$14","$15","memory");
                __asm__ volatile ("swc2  $11,8($12)": : :"$12","$13","$14","$15","memory");
            }
        }
        node[3] = (SCR->vel[0] * 7) >> 3;
        node[0] = SCR->pos[0] + SCR->dpos[0] + node[3];
        node[4] = ((SCR->vel[1] * 7) >> 3) + 0x190;
        node[1] = SCR->pos[1] + SCR->dpos[1] + node[4];
        node[5] = (SCR->vel[2] * 7) >> 3;
        node[2] = SCR->pos[2] + SCR->dpos[2] + node[5];
    }
}
void func_8001924C(s16 *arg0, s32 arg1) {
    s32 i = 0;
    s16 *s0;
    /* FAKE: pointer alias -- g_file_data_buf's address held in an integer local. Referenced directly, its
     * lui/addiu is scheduled after `move s0,a0` (score 2); held as a u8 * the addu operands swap (score 2). */
    s32 buf;

    if (i < arg1) {
        buf = (s32)g_file_data_buf;
        s0 = arg0;
        do {
            if (*(u8 *)((u8 *)s0 + 2) & 1) {
                s16 val = s0[0];
                func_80019310(s0, (s32 *)(val * 52 + buf));
            } else {
                s16 val = s0[0];
                func_800187F4(s0, (s32 *)(val * 52 + buf));
            }
            i++;
            s0 = (s16 *)((u8 *)s0 + 16);
        } while (i < arg1);
    }
}
/* func_80019310 - GTE rotate-and-scale of an SVECTOR array into a 0x40-stride VECTOR
 * table, then a 32-byte MATRIX copy into the descriptor. Pure-C body plus four PsyQ SDK
 * GTE macro islands, each cited to its Sony libgte macro NAME and its header line range
 * in inline_c.h (the PsyQ inline-GTE header; the copy cited is rood-reverse's
 * include/psx/inline_c.h):
 *   gte_SetRotMatrix(r0)   -- inline_c.h:297-310
 *   gte_SetTransMatrix(r0) -- inline_c.h:360-369
 *   gte_ldv0(r0)           -- inline_c.h:16-20
 *   gte_stlvnl(r0)         -- inline_c.h:1111-1117
 * plus the raw cop2 MVMVA sf=1/mx=rot/v=V0/cv=TR command (.word 0x4A480012), which is the
 * gte_rtv0()-class operation encoded directly. The islands use the same `move $12, %0`
 * macro-body spelling as func_800203B4 (owner grant, widened cop2
 * materialize-then-copy anchor; func_80019310 is named in that grant record,
 * pre-slim-2026-10-01:docs/grind/decisions.md:17921).
 *
 * CLOBBER PROVENANCE (do not read the "memory" clobbers as SDK text): of the four macros
 * above, ONLY gte_stlvnl publishes "memory" in its own clobber list (inline_c.h:1116);
 * gte_SetRotMatrix, gte_SetTransMatrix and gte_ldv0 publish only "$12","$13","$14" (or no
 * clobber list at all, for gte_ldv0). The "memory" clobber on those three islands is ADDED
 * here, and is cited to the committed same-file precedent func_8002D320
 * (src/code6cac_b.c), whose lwc2 read island carries exactly that added truthful
 * clobber; func_800300B4 carries the same addition. It is truthful in each case: islands 1-2 read the MATRIX through $12, island 3
 * reads the SVECTOR through $12, island 4 writes out[] which the C below reads. Its
 * byte-visible effect is on island 1, where it makes GCC re-read the MATRIX pointer before
 * the SetTransMatrix island (target 0x8001934C).
 *
 * COMPLETED-INLINE-ASM-CANONICAL (listed in inline_asm_canonical.txt). */
void func_80019310(s16 *arg0, s32 *arg1) {
    s32 out[3];
    s32 i;
    s32 *dst;

    /* PsyQ libgte inline macro gte_SetRotMatrix(r0) --- inline_c.h:297-310. Macro body
     * hardcodes $12-$14 (published clobbers, inline_c.h:310); the `move $12, %0` preamble
     * and $15 are the granted func_800203B4 spelling. The "memory" clobber is ADDED, not
     * SDK text -- precedent src/code6cac_b.c:935 (func_8002D320). */
    __asm__ volatile(
        "move   $12, %0\n"
        "lw     $13, 0($12)\n"
        "lw     $14, 4($12)\n"
        "ctc2   $13, $0\n"
        "ctc2   $14, $1\n"
        "lw     $13, 8($12)\n"
        "lw     $14, 12($12)\n"
        "lw     $15, 16($12)\n"
        "ctc2   $13, $2\n"
        "ctc2   $14, $3\n"
        "ctc2   $15, $4\n"
        :: "r"(*(s32 *)(arg0 + 2)) : "$12", "$13", "$14", "$15", "memory");
    /* PsyQ libgte inline macro gte_SetTransMatrix(r0) --- inline_c.h:360-369. Translation
     * vector words 20/24/28 into cop2 control regs $5..$7; published clobbers are
     * "$12","$13","$14" (inline_c.h:369). The "memory" clobber is ADDED, not SDK text --
     * precedent src/code6cac_b.c:935 (func_8002D320). */
    __asm__ volatile(
        "move   $12, %0\n"
        "lw     $13, 20($12)\n"
        "lw     $14, 24($12)\n"
        "ctc2   $13, $5\n"
        "lw     $15, 28($12)\n"
        "ctc2   $14, $6\n"
        "ctc2   $15, $7\n"
        :: "r"(*(s32 *)(arg0 + 2)) : "$12", "$13", "$14", "$15", "memory");

    dst = (s32 *)arg1[3];
    for (i = 0; i < ((s16 *)arg1)[2]; i++) {
        /* PsyQ libgte inline macro gte_ldv0(r0) --- inline_c.h:16-20. lwc2 VXY0/VZ0; the
         * macro publishes NO clobber list at all, so both the "$12" and the "memory"
         * clobbers here are ADDED, not SDK text -- precedent src/code6cac_b.c:935
         * (func_8002D320), whose lwc2 read island carries exactly this pair. The 2-cycle
         * GTE load delay is carried as the two explicit nops. */
        __asm__ volatile(
            "move   $12, %0\n"
            "lwc2   $0, 0($12)\n"
            "lwc2   $1, 4($12)\n"
            "nop\n"
            "nop\n"
            :: "r"((s32 *)(arg1[0] + i * 8)) : "$12", "memory");
        /* Sony libgte macro gte_mvmva(sf,mx,v,cv,lm) --- inline_c.h:816-817, whose body is
         * gte_mvmva_core(r0) at inline_c.h:809-814 (`nop; nop; .word <literal>`). Our
         * instance is gte_mvmva(1,0,0,0,0): sf=1, mx=rotation, v=V0, cv=TR, lm=0. It is
         * spelled as a bare `.word 0x4A480012` (the ASPSX 2.34 encoding of that cop2
         * command) rather than through the macro because the macro composes its literal in
         * a different word encoding (0x000013bf | sf<<25 | ...) that this assembler does
         * not accept. The two GTE-latency nops the macro body places ahead of the
         * command are carried at the tail of the preceding gte_ldv0 island above. */
        __asm__ volatile(".word 0x4A480012");
        /* PsyQ libgte inline macro gte_stlvnl(r0) --- inline_c.h:1111-1117. Stores
         * MAC1/MAC2/MAC3. This is the ONE island whose "memory" clobber IS the macro's own
         * published clobber list (inline_c.h:1116); "$12" is added with the preamble. */
        __asm__ volatile(
            "move   $12, %0\n"
            "swc2   $25, 0($12)\n"
            "swc2   $26, 4($12)\n"
            "swc2   $27, 8($12)\n"
            :: "r"(out) : "$12", "memory");
        dst[0] = out[0] << 7;
        dst[1] = out[1] << 7;
        dst[2] = out[2] << 7;
        dst[3] = 0;
        dst[4] = 0;
        dst[5] = 0;
        dst = (s32 *)((u8 *)dst + 0x40);
    }
    *(MATRIX *)(arg1 + 5) = **(MATRIX **)(arg0 + 2);
}
void func_8001945C(void) {
    D_80106A50.color[0] = 0x11;
    D_80106A50.color[1] = 0x44;
    D_80106A50.color[2] = 0x88;
}
s32 func_80019488(void) {
    return (D_80106A50.color[0] & 0xF) | ((D_80106A50.color[1] & 0xF) << 4) | ((D_80106A50.color[2] & 0xF) << 8);
}
void func_800194C0(s32 arg0) {
    D_800A3912 = arg0 & 0xF;
    D_800A3913 = (arg0 >> 4) & 0xF;
    D_800A3914 = (arg0 >> 8) & 0xF;
}
void pad_ResetState(void) {
    g_pad_state.type[0] = 4;
    g_pad_state.type[1] = 4;
    g_pad_state.held = 0;
    g_pad_state.pressed = 0;
    g_pad_state.released = 0;
    g_pad_state.unheld = -1;
}
void pad_ResetStateMarkValid(void) {
    pad_ResetState();
    g_pad_state.valid[0] = 1;
    g_pad_state.valid[1] = 1;
}
void func_80019568(s32 arg0) {
    PadState pad;
    s32 pkts[4];
    u8 *packets;
    s32 i;
    s32 held;
    s32 old_held;

    held = 0;
    i = 0;
    packets = (u8 *)&pkts[0];
    pkts[0] = g_pad_buf;
    pkts[1] = g_pad_buf_plus_0x4;
    pkts[2] = g_pad_buf_plus_0x24;
    pkts[3] = g_pad_buf_plus_0x28;
    do {
        u8 *rec = &packets[i * 8];
        s32 valid = 0;
        s32 bits;

        if (rec[0] == 0) {
            s32 type_m1;

            pad.type[i] = rec[1] >> 4;
            valid = 1;
            /* FAKE: the `pad.valid[i] = valid;` store is written into BOTH arms rather
             * than once after the join (family: duplicated-statement-into-arms,
             * .claude/rules/duplicated-statement-into-arms.md).  mechanism:
             * loop.c scan_loop (loop.c:695-716) only creates a movable for the
             * `1`-holding pseudo when it has a single set or consecutive sets;
             * the loop-top default plus this in-arm set are non-consecutive, so
             * no movable exists and the `addiu $v0,$zero,1` stays in the loop
             * filling target's lhu load-delay slot. A bare literal, a computed
             * `valid = (rec[0] == 0)` or a single store after the join do not
             * match. */
            pad.valid[i] = valid;
            type_m1 = (s16)((u16)pad.type[i] - 1);

            switch (type_m1) {
            case 4:
            case 6:
                pad.type[i] = 4;
            case 1:
            case 2:
            case 3:
                bits = ~((rec[2] << 8) | rec[3]);
                break;
            case 0:
            case 5:
            case 7:
            default:
                bits = 0;
                break;
            }
        } else {
            pad.type[i] = 4;
            pad.valid[i] = valid;
            bits = 0;
        }

        held = ((u32)held >> 16) | (bits << 16);
        i++;
    } while (i < 2);

    pad.held = held;
    func_8001B138(&pad.held);

    if (D_800A3834 == 1 && arg0 == 0) {
        s32 mode = D_800A38DC;

        switch (mode) {
        case 4:
        case 5:
            if (g_pad_state.valid[1] == 0) {
                pad.held |= 0x08000800;
            }
        case 0:
        case 1:
        case 2:
        case 3:
        case 6:
            if (g_pad_state.valid[0] == 0) {
                pad.held |= 0x08000800;
            }
            break;
        }
    }

    func_8003A728((s32)&pad);

    i = 0;
    do {
        g_pad_state.type[i] = pad.type[i];
        g_pad_state.valid[i] = pad.valid[i];
        i++;
    } while (i < 2);

    old_held = g_pad_state.held;
    g_pad_state.held = pad.held;
    g_pad_state.pressed = pad.held & ~old_held;
    g_pad_state.unheld = ~pad.held;
    g_pad_state.released = ~pad.held & old_held;
}
