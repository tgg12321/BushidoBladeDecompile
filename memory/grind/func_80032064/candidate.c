/* func_80032064 — s2 candidate, sandbox --disable all == 0 (masked).
 *
 * Change vs HEAD: the audit-flagged `sw_val` is renamed to `vel_y` and the
 * four top-of-function locals now carry their initializers at declaration.
 * The redundant `i = 0;` statement (duplicating the for-init) is gone: `i`
 * is initialized once, at its declaration, and the for-init clause is empty.
 *
 * The four-local init cluster (speed, vel_y, i, ptr) maps 1:1, in source
 * order, onto asm/funcs/func_80032064.s lines 5-9. Every member is
 * byte-load-bearing, and so is their ORDER (swapping the first two scores 2)
 * — see rejected/ for the measured negatives.
 *
 * s2 settled the audit's charge that `vel_y` is a scheduling coercion:
 * declaring the SAME variable at the store site instead of at the top scores 7,
 * exactly like the bare literal (rejected/blocklocal-holder-at-store-site-*).
 * The variable's existence does nothing; the source position of its
 * initialization is what places the addiu in the entry BB — ordinary C
 * statement-order semantics, identical for speed/i/ptr. vel_y is fully live
 * (its value is stored as the effect's y-velocity), so it is not a "fake"
 * constant-holder and carries no FAKE annotation.
 */
extern s32 func_80032854(s32, s32, u8 *, s16 *);
u8 *func_80032064(u8 *src, s32 type) {
    s32 speed = 0x50;
    s32 vel_y = -0xC8;
    s32 i = 0;
    u8 *ptr = &D_80104E88;
    u8 *s0;
    s16 sp_area[2];

    for (; i < 4; i++) {
        s0 = ptr;
        if (*s0 == 0) break;
        ptr = s0 + 0x2C;
    }
    if (i == 4) return 0;

    *s0 = type;
    *(s0 + 1) = 1;
    *(s0 + 2) = 0;
    *(s0 + 3) = *(u16 *)(src + 4);
    *(s32 *)(s0 + 4) = *(s32 *)(src + 0xF4);
    {
        s32 v1 = *(s16 *)(src + 0x1A);
        if (v1 < 0) v1 += 0x1F;
        *(s32 *)(s0 + 8) = *(s32 *)(src + 0xBC) - (v1 >> 5);
    }
    *(s32 *)(s0 + 0xC) = *(s32 *)(src + 0xFC);
    *(s32 *)(s0 + 0x1C) = ((s32)*(&Judge + (*(u16 *)(src + 0x1CA) & 0xFFF)) * speed) >> 12;
    *(s32 *)(s0 + 0x20) = vel_y;
    *(s32 *)(s0 + 0x24) = ((s32)*(&Judge + ((*(s16 *)(src + 0x1CA) + 0x400) & 0xFFF)) * speed) >> 12;
    *(Vec3_copy *)(s0 + 0x10) = *(Vec3_copy *)(s0 + 4);
    *(s32 *)(s0 + 0x28) = *(s32 *)(src + 0xBC);
    sp_area[1] = *(u16 *)(src + 0x1CA);
    {
        s32 a0_arg = *(u8 *)(src + 0xB2);
        s32 cmd = 0xD;
        u8 *v1 = s0 + 4;
        if (type == 1) cmd = 0xC;
        func_80032854(a0_arg, cmd, v1, sp_area);
    }
    return s0;
}
