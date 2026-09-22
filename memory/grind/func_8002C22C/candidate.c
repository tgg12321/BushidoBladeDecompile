/* func_8002C22C (PutRobShadow) — s2 candidate (structural, 2026-09-22).
 * sandbox --disable all = 199 (target 252 insns, build 246 insns). NOT a match.
 * Live-staged in src/code6cac_b.c as of this session. Progress this session:
 * duplicated the s1 candidate's top-level unconditional 6-word scratchpad
 * zero-init (0x1F800360/364/368/370/374/378 = 0;) into BOTH if/else arms
 * instead of leaving it above the branch. s1 measured 211; this form measures
 * 199 (build_insns 230 -> 246, closer to target's 252). See evidence.md /
 * hypotheses.md for the CSE block-extension mechanism this targets and the
 * killed do-while(0) follow-up probe.
 */
extern s32 D_80102314; /* record 1 of a 2-elem table, stride 0x44C from D_80101EC8 (func_8002C61C's s1+0x44C);
                         * fields accessed base+offset here, unlike record 0's individually-named scalars.
                         * TODO once scope-granted: promote to include/code6cac.h (out of this session's
                         * scope_allow surface, so declared file-local for now). */
void func_8002C22C(void) {
    s32 *t0 = (s32 *)0x1F8002B8;
    s32 *d_tbl = &D_80102314;
    s32 v1, v0, a2;
    s32 d_v1, d_a0, d_v0, d_a1;
    s32 sum_v0_2, sum_a1_2;

    if (D_800A3824 & 1) {
        *(s32 *)0x1F800360 = 0;
        *(s32 *)0x1F800364 = 0;
        *(s32 *)0x1F800368 = 0;
        *(s32 *)0x1F800370 = 0;
        *(s32 *)0x1F800374 = 0;
        *(s32 *)0x1F800378 = 0;
        v1 = *(s32 *)0x1F80004C;
        v0 = *(s32 *)0x1F800048;
        a2 = *(s32 *)0x1F800050;
        *(s32 *)0x1F800364 = v1;
        v1 = v1 + *(s32 *)0x1F800058;
        *(s32 *)0x1F800360 = v0;
        v0 = v0 + *(s32 *)0x1F800054;
        *(s32 *)0x1F800360 = v0;
        *(s32 *)0x1F800364 = v1;
        d_v1 = D_80102100;
        d_a0 = D_80102104;
        *(s32 *)0x1F800368 = a2;
        a2 = a2 + *(s32 *)0x1F80005C;
        *(s32 *)0x1F800370 = D_801020FC;
        *(s32 *)0x1F800370 = D_801020FC + D_80102108;
        d_v0 = D_8010210C;
        d_a1 = D_80102110;
    } else {
        *(s32 *)0x1F800360 = 0;
        *(s32 *)0x1F800364 = 0;
        *(s32 *)0x1F800368 = 0;
        *(s32 *)0x1F800370 = 0;
        *(s32 *)0x1F800374 = 0;
        *(s32 *)0x1F800378 = 0;
        v1 = *(s32 *)0x1F800004;
        v0 = *(s32 *)0x1F800000;
        a2 = *(s32 *)0x1F800008;
        *(s32 *)0x1F800364 = v1;
        v1 = v1 + *(s32 *)0x1F800010;
        *(s32 *)0x1F800360 = v0;
        v0 = v0 + *(s32 *)0x1F80000C;
        *(s32 *)0x1F800360 = v0;
        *(s32 *)0x1F800364 = v1;
        d_v1 = D_801020DC;
        d_a0 = D_801020E0;
        *(s32 *)0x1F800368 = a2;
        a2 = a2 + *(s32 *)0x1F800014;
        *(s32 *)0x1F800370 = D_801020D8;
        *(s32 *)0x1F800370 = D_801020D8 + D_801020E4;
        d_v0 = D_801020E8;
        d_a1 = D_801020EC;
    }
    *(s32 *)0x1F800368 = a2;
    *(s32 *)0x1F800374 = d_v1;
    *(s32 *)0x1F800378 = d_a0;
    *(s32 *)0x1F800374 = d_v1 + d_v0;
    *(s32 *)0x1F800378 = d_a0 + d_a1;

    if (D_800A3824 & 2) {
        t0[0xA8/4] += *(s32 *)0x1F800060;
        t0[0xAC/4] += *(s32 *)0x1F800064;
        t0[0xB0/4] += *(s32 *)0x1F800068;
        t0[0xB8/4] += d_tbl[0x234/4];
        t0[0xBC/4] += d_tbl[0x238/4];
        t0[0xC0/4] += d_tbl[0x23C/4];
        t0[0xA8/4] += *(s32 *)0x1F80006C;
        t0[0xAC/4] += *(s32 *)0x1F800070;
        t0[0xB0/4] += *(s32 *)0x1F800074;
        t0[0xB8/4] += d_tbl[0x240/4];
        sum_v0_2 = t0[0xBC/4] + d_tbl[0x244/4];
        sum_a1_2 = d_tbl[0x248/4];
    } else {
        t0[0xA8/4] += *(s32 *)0x1F800024;
        t0[0xAC/4] += *(s32 *)0x1F800028;
        t0[0xB0/4] += *(s32 *)0x1F80002C;
        t0[0xB8/4] += d_tbl[0x210/4];
        t0[0xBC/4] += d_tbl[0x214/4];
        t0[0xC0/4] += d_tbl[0x218/4];
        t0[0xA8/4] += *(s32 *)0x1F800030;
        t0[0xAC/4] += *(s32 *)0x1F800034;
        t0[0xB0/4] += *(s32 *)0x1F800038;
        t0[0xB8/4] += d_tbl[0x21C/4];
        sum_v0_2 = t0[0xBC/4] + d_tbl[0x220/4];
        sum_a1_2 = d_tbl[0x224/4];
    }
    t0[0xBC/4] = sum_v0_2;
    t0[0xC0/4] += sum_a1_2;
    t0[0x13C/4] = ((t0[0xA8/4] * 3) + t0[0xB8/4]) >> 4;
    t0[0x140/4] = ((t0[0xAC/4] * 3) + t0[0xBC/4]) >> 4;
    t0[0x144/4] = ((t0[0xB0/4] * 3) + t0[0xC0/4]) >> 4;
}
