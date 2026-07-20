/* s9 rederive frontier probe #2 (from s8 live frontier):
 *   Split-init accumulation FALSE arm — build `v` via three separate `|=`
 *   statements instead of a single OR-tree expression, with the goal of
 *   producing arm-distinct RTL SETs that jump2/find_cross_jump refuses to
 *   merge past the final `or`. Split-init accumulation is sanctioned per
 *   feedback/split-init-accumulation-sanctioned (user 2026-06-13).
 *
 * Two variants measured:
 *   (a) shared gnd_load_tex(v) OUTSIDE arms, FALSE arm split-init:
 *         v  = *((u8*)player+0x1A);      // b
 *         v |= (s32)*((u8*)player+0x18) << 16;
 *         v |= (s32)*((u8*)player+0x19) << 8;
 *       -> sandbox --disable all: score=11, build_insns=81 (target 82)
 *
 *   (b) per-arm gnd_load_tex(v) INSIDE arms, FALSE arm split-init:
 *         v  = *((u8*)player+0x1A);      // b
 *         v |= (s32)*((u8*)player+0x18) << 16;
 *         v |= (s32)*((u8*)player+0x19) << 8;
 *         gnd_load_tex(v);
 *       -> sandbox --disable all: score=11, build_insns=81
 *
 *   (c) same as (b) but b LAST in accumulation:
 *         v  = (s32)*((u8*)player+0x18) << 16;
 *         v |= (s32)*((u8*)player+0x19) << 8;
 *         v |= *((u8*)player+0x1A);
 *       -> sandbox --disable all: score=11, build_insns=81 (identical)
 *
 * Disasm (variant b, tail — see tmp/grind/gnd_init_80041688/s9/split_init_per_arm.dis):
 *   TRUE:  15b8 move v1,v0; 15bc sll a0,v0,0x10; 15c0 sll v0,v1,0x8;
 *          15c4 j 15e4; 15c8 or a0,a0,v0 (delay)
 *   FALSE: 15cc lbu v0,0x18(s0);  <- r=v0
 *          15d0 lbu v1,0x1A(s0);  <- b=v1  (matches target b in $v1!)
 *          15d4 lbu a0,0x19(s0);  <- g=a0
 *          15d8 sll v0,v0,0x10;
 *          15dc or  v1,v1,v0;     <- b | (r<<16), in $v1
 *          15e0 sll a0,a0,0x8;
 *          15e4 jal gnd_load_tex; (cross-jump absorbed one FALSE `or` here)
 *          15e8 or  a0,v1,a0 (delay)  <- b|(r<<16)|(g<<8), final or matches target shape
 *
 * Key finding: split-init DOES land `b` in $v1 through the FALSE-arm ORs,
 * producing target's final-or shape `or a0,v1,a0`. BUT the FALSE-arm lbu
 * emission order is [r,b,g] — still not target's [b,r,g]. And jump2/find_cross_jump
 * merged one `or` into the shared jal delay slot, dropping build_insns 82->81.
 *
 * The three `|=` statements do NOT produce enough arm-distinct RTL to defeat
 * jump2 merge past the jal (both arms' final delay-slot or has 3-operand shape
 * `or a0,X,Y` after combine folds intermediates back).
 *
 * Rejected. Regresses baseline 2 -> 11 without unlocking target byte layout.
 *
 * Note: this exhausts the last non-cheat structural rederive axis. Combined
 * with s7's case-exhaustion proof (loop1-b-staging dichotomy: dead-store OR
 * RA-cascade) and s8's shared-call-shape KILL, the pseudo-78-fusion path is
 * closed to legitimate C forms.
 */
void gnd_init_80041688_split_init_false_variant_b(s32 arg0, s32 arg1) {
    s32 *player;
    s32 i;
    u8 *p;
    u8 *q;
    s32 b, r, g, v;
    volatile s32 sp10[8];
    extern s32 func_800486FC(void);

    player = (s32 *)g_player_ptrs[arg0];
    if (player == NULL) return;

    p = (u8 *)player + 0x94;
    if (arg1) { p[1] |= 1; } else { p[1] &= ~1; }

    i = 1;
loop1:
    p += 0x68;
    if (*(s16 *)(p + 2) >= 0) {
        if (arg1) p[1] |= 1;
        else      p[1] &= ~1;
    }
    i++;
    if (i < 18) goto loop1;

    q = (u8 *)player + 0x10D5;
loop2:
    if (*(s32 *)(q + 0x57) == 0) goto after2;
    if (arg1) *q |= 1;
    else      *q &= ~1;
    q += 0x68;
    goto loop2;
after2:

    if (func_800486FC()) {
        r = *((u8 *)player + 0x18);
        g = *((u8 *)player + 0x19);
        b = *((u8 *)player + 0x1A);
        v = func_8004881C(b, g, r);
        gnd_load_tex((v << 16) | (v << 8) | v);
    } else {
        v  = *((u8 *)player + 0x1A);
        v |= (s32)*((u8 *)player + 0x18) << 16;
        v |= (s32)*((u8 *)player + 0x19) << 8;
        gnd_load_tex(v);
    }
    (void)sp10;
}
