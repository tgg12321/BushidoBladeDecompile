/* func_800204C0 — honest bucket COMPLETED-INLINE-ASM-CANONICAL (allowlist line required;
 * no registry row in tools/grinder/owner_cluster_grants.txt yet — see ledger). Pure-C body
 * (97 insns, byte-exact with zero coercion) + the same four PsyQ SDK GTE macro islands as
 * func_800203B4 (owner grant 2026-09-01, widened cop2 materialize-then-copy anchor;
 * func_800204C0 is named by name as a confirmed handwritten-tagged carrier in that grant,
 * pre-slim-2026-10-01:docs/grind/decisions.md:17959 and pre-slim-2026-10-01:.claude/rules/cop2-addressing-preamble-cluster.md:155):
 * gte_SetRotMatrix, gte_ldlv0, cop2 MVMVA (.word 0x4A486012), gte_stlvnl — character-identical
 * to the func_800203B4 spelling in this file. Load-bearing measured facts (s1, do not tidy):
 *  - the counter head MUST be `+= 1` then a fresh re-read for the `& 7` test: cse.c forwards
 *    the stored value (andi on the stored reg) and copies the first load (`move $v1,$v0`
 *    in the beqz delay slot); the HImode read-modify-write also reserves the 8-byte
 *    phantom frame slot (vars=32) — a `cnt = *p + 1` local spelling gives vars=24 and no
 *    copy (sandbox 10, rejected/head-local-cnt-frame24.c);
 *  - `mul` is an ordinary signed `/ 150` (magic 0x1B4E81B5, shift 4) and `/ 0x1000`.
 * Full ledger: memory/grind/func_800204C0/ (sandbox --disable all == 0, 122/122). */
void func_800204C0(u8 *arg0) {
    s32 mac[3];
    s16 out[3];
    s32 pid;
    s32 src;
    s32 mul;
    s32 tx, ty, tz;

    pid = *(s16 *)(arg0 + 4);
    if (*(s16 *)(arg0 + 0x350) != 0) {
        *(s16 *)(arg0 + 0x350) += 1;
        if ((*(s16 *)(arg0 + 0x350) & 7) == 2) {
            src = *(s32 *)((((s32)*(s16 *)(arg0 + 0x352)) << 2) +
                           game_GetPlayerData(pid));
            /* PsyQ libgte inline macro gte_SetRotMatrix(r0) --- inline_c.h:297-310: loads
             * the 5 packed rotation-matrix words at r0 into cop2 control regs $0..$4.
             * Macro body hardcodes $12-$14 (published clobbers, inline_c.h:310); the
             * `move $12, %0` preamble and $15 are the granted func_800203B4 spelling. */
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
                :: "r"(src) : "$12", "$13", "$14", "$15");
            /* PsyQ libgte inline macro gte_ldlv0(r0) --- inline_c.h:101-110: lhu/lhu/sll/or
             * pack of VX0/VY0 into one word, mtc2 to $0, lwc2 VZ0 into $1 (macro body,
             * published clobbers $12/$13), then the 2-cycle GTE load delay carried as
             * explicit nops (maspsx does NOT supply them in the full-build context —
             * measured on func_800203B4, 2026-09-01). */
            __asm__ volatile(
                "move   $12, %0\n"
                "lhu    $14, 4($12)\n"
                "lhu    $13, 0($12)\n"
                "sll    $14, $14, 16\n"
                "or     $13, $13, $14\n"
                "mtc2   $13, $0\n"
                "lwc2   $1, 8($12)\n"
                "nop\n"
                "nop\n"
                :: "r"(arg0 + 0x354) : "$12", "$13", "$14");
            /* GTE MVMVA sf=1, mx=rotation, v=V0, cv=none, lm=0 --- cop2 command 0x0486012. */
            __asm__ volatile(".word 0x4A486012");
            mul = ((0x96 - *(s16 *)(arg0 + 0x350)) << 12) / 150;
            /* PsyQ libgte inline macro gte_stlvnl(r0) --- inline_c.h:1111-1117: store
             * MAC1/MAC2/MAC3 ($25/$26/$27) to r0. */
            __asm__ volatile(
                "move   $12, %0\n"
                "swc2   $25, 0($12)\n"
                "swc2   $26, 4($12)\n"
                "swc2   $27, 8($12)\n"
                :: "r"(mac) : "$12");
            tx = (mac[0] * mul) / 0x1000;
            out[0] = tx;
            ty = (mac[1] * mul) / 0x1000;
            out[1] = ty;
            tz = (mac[2] * mul) / 0x1000;
            out[2] = tz;
            if ((s16)ty >= 0x801) {
                out[0] = -tx;
                out[1] = -ty;
                out[2] = -tz;
            }
            func_80032854(pid, 4, (u8 *)0x1F8000A8 + pid * 0x108 + *(s16 *)(arg0 + 0x352) * 0xC, out);
        }
        if (*(s16 *)(arg0 + 0x350) >= 0x96) {
            *(s16 *)(arg0 + 0x350) = 0;
        }
        *(s16 *)(arg0 + 0x350) = 0;
    }
}
