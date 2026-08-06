/* PutRobShadow — C body removed by Campaign 4 (INCLUDE_ASM conversion).
 * The build DISCARDED this body (asmfix replace_with_asmfile substituted
 * asm/funcs/PutRobShadow.s verbatim), so it never reached the binary.
 * This body is a real prior decomp attempt — archaeology for the C SHAPE when this function is decompiled.
 * NOTE: its signature was never checked by anything — do not trust it.
 *
 * !! WARNING — FORBIDDEN CONSTRUCTS BELOW: 49 volatile-coercion cast(s) OUTSIDE the hardware-MMIO range (scratchpad 0x1F8000xx and/or &D_xxxxxxxx — see mmio-volatile-type-level.md, which excludes scratchpad from the carve-out).
 * These are NOT sanctioned and must NOT be carried back into src/.
 * See .claude/rules/register-asm-pins.md (diagnostic-only, never
 * committable), inline-asm-injection.md (hardcoded-$N) and
 * inline-move-aliasing.md (archived tombstone). This file is outside
 * every detector's scope, so nothing will flag them for you.
 * Resume from the C SHAPE only — control flow, field offsets, the
 * algorithm — and re-derive in PURE C.
 * Campaign plan R2.
 */
void PutRobShadow(void) {
    s32 *t0 = (s32 *)0x1F8002B8;
    s32 *d_tbl = &D_80102314;
    s32 v1, v0, a2;
    s32 d_v1, d_a0, d_v0, d_a1;
    s32 sum_v0_2, sum_a1_2;

    *(volatile s32 *)0x1F800360 = 0;
    *(volatile s32 *)0x1F800364 = 0;
    *(volatile s32 *)0x1F800368 = 0;
    *(volatile s32 *)0x1F800370 = 0;
    *(volatile s32 *)0x1F800374 = 0;
    *(volatile s32 *)0x1F800378 = 0;

    if (D_800A3824 & 1) {
        v1 = *(volatile s32 *)0x1F80004C;
        v0 = *(volatile s32 *)0x1F800048;
        a2 = *(volatile s32 *)0x1F800050;
        *(volatile s32 *)0x1F800364 = v1;
        v1 = v1 + *(volatile s32 *)0x1F800058;
        *(volatile s32 *)0x1F800360 = v0;
        v0 = v0 + *(volatile s32 *)0x1F800054;
        *(volatile s32 *)0x1F800360 = v0;
        *(volatile s32 *)0x1F800364 = v1;
        d_v1 = D_80102100;
        d_a0 = D_80102104;
        *(volatile s32 *)0x1F800368 = a2;
        a2 = a2 + *(volatile s32 *)0x1F80005C;
        *(volatile s32 *)0x1F800370 = D_801020FC;
        *(volatile s32 *)0x1F800370 = D_801020FC + D_80102108;
        d_v0 = D_8010210C;
        d_a1 = D_80102110;
    } else {
        v1 = *(volatile s32 *)0x1F800004;
        v0 = *(volatile s32 *)0x1F800000;
        a2 = *(volatile s32 *)0x1F800008;
        *(volatile s32 *)0x1F800364 = v1;
        v1 = v1 + *(volatile s32 *)0x1F800010;
        *(volatile s32 *)0x1F800360 = v0;
        v0 = v0 + *(volatile s32 *)0x1F80000C;
        *(volatile s32 *)0x1F800360 = v0;
        *(volatile s32 *)0x1F800364 = v1;
        d_v1 = D_801020DC;
        d_a0 = D_801020E0;
        *(volatile s32 *)0x1F800368 = a2;
        a2 = a2 + *(volatile s32 *)0x1F800014;
        *(volatile s32 *)0x1F800370 = D_801020D8;
        *(volatile s32 *)0x1F800370 = D_801020D8 + D_801020E4;
        d_v0 = D_801020E8;
        d_a1 = D_801020EC;
    }
    *(volatile s32 *)0x1F800368 = a2;
    *(volatile s32 *)0x1F800374 = d_v1;
    *(volatile s32 *)0x1F800378 = d_a0;
    *(volatile s32 *)0x1F800374 = d_v1 + d_v0;
    *(volatile s32 *)0x1F800378 = d_a0 + d_a1;

    if (D_800A3824 & 2) {
        t0[0xA8/4] += *(volatile s32 *)0x1F800060;
        t0[0xAC/4] += *(volatile s32 *)0x1F800064;
        t0[0xB0/4] += *(volatile s32 *)0x1F800068;
        t0[0xB8/4] += d_tbl[0x234/4];
        t0[0xBC/4] += d_tbl[0x238/4];
        t0[0xC0/4] += d_tbl[0x23C/4];
        t0[0xA8/4] += *(volatile s32 *)0x1F80006C;
        t0[0xAC/4] += *(volatile s32 *)0x1F800070;
        t0[0xB0/4] += *(volatile s32 *)0x1F800074;
        t0[0xB8/4] += d_tbl[0x240/4];
        sum_v0_2 = t0[0xBC/4] + d_tbl[0x244/4];
        sum_a1_2 = d_tbl[0x248/4];
    } else {
        t0[0xA8/4] += *(volatile s32 *)0x1F800024;
        t0[0xAC/4] += *(volatile s32 *)0x1F800028;
        t0[0xB0/4] += *(volatile s32 *)0x1F80002C;
        t0[0xB8/4] += d_tbl[0x210/4];
        t0[0xBC/4] += d_tbl[0x214/4];
        t0[0xC0/4] += d_tbl[0x218/4];
        t0[0xA8/4] += *(volatile s32 *)0x1F800030;
        t0[0xAC/4] += *(volatile s32 *)0x1F800034;
        t0[0xB0/4] += *(volatile s32 *)0x1F800038;
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
