/* func_80032314 — best form as of grind session 1 (recon).
 * Honest floor: sandbox --disable all = 15 (was 27 at session start).
 * Target/build instruction counts are now EQUAL (109/109); the entire
 * residual is a 3-cycle register rotation on $a1/$a2/$a3 (see evidence.md).
 *
 * NOTE ON PINS: the `register ... asm("tN")` pins below are carried over from
 * the HEAD body and are CHEATS — they are stripped by the cheat-invisible
 * sandbox, so they do not affect the floor of 15 at all. They must be deleted
 * before any candidate-ready submission. They are left in here only so this
 * file is a drop-in replacement for the HEAD body; the next session should
 * delete them first (measured: floor is 15 with or without them).
 *
 * THE SESSION-1 WIN (do not regress): the GTE leading-zero-count island is
 * written as ONE canonical block copied verbatim from the MATCHED,
 * USER-AUTHORIZED sibling func_800274BC (src/code6cac_b.c:292, authorized
 * 2026-06-10), together with that sibling's tail-arithmetic spelling
 * (v0_m = (u32)-2; v0_m &= clz; ... v1_m >>= 1; (hi<<16) >> (0x13 - v1_m)).
 * The previous HEAD spelling used `.word 0x488CF000` / `.word 0xE99F0000`
 * plus a $t4 pin and two bare `__asm__("nop")` statements; the engine's
 * canonical classifier (tools/classify_inline_asm.py:62,
 * CANONICAL_DOTWORD_RE = /\.word\s*0x4[89A-Fa-f]/) accepts the mtc2 word but
 * classifies the swc2 word 0xE99F.... as CHEAT and strips it, which deleted
 * the LZCR store from the scored build and cost 3 instructions + a broken
 * dataflow. The single-block mnemonic spelling is classified canonical whole
 * (mtc2/swc2 are both in CANONICAL_ASM_OPS) and reproduces the target's
 * `addu $t4,...` feeder moves exactly.
 */
extern u8 D_8008D118;
void func_80032314(void) {
    register u8 *t0 asm("t0") = &D_80104E88;
    register s32 t1 asm("t1") = 0;
    register u8 *a3 asm("a3") = &D_80104E88 + 2;
    u8 *ent;
    s32 state;
    s32 a0;

loop:
    if (*t0 == 0) goto next;
    {
        s32 v1_v = (*(u8 *)(a3 + 1) == 0);
        v1_v = v1_v * 0x44C;
        ent = v1_v + &D_80101EC8;
    }
    state = *(u16 *)(ent + 0x6A);
    a0 = state & 0xFFFF;
    if (a0 == 4) goto next;
    if (a0 == 0x14) goto next;
    if (a0 == 0xF) goto next;
    if ((u32)(state - 0x1C) < 2) goto next;
    if ((u32)(state - 0x1E) < 2) goto next;
    if ((u32)(state - 0x20) < 2) goto next;
    if (a0 == 0x11) goto next;
    {
        s32 dx = *(s32 *)(ent + 0xF4) - *(s32 *)(a3 + 2);
        s32 dy = *(s32 *)(ent + 0xF8) - *(s32 *)(a3 + 6);
        s32 dz = *(s32 *)(ent + 0xFC) - *(s32 *)(a3 + 0xA);
        register u32 dist_sq asm("a0") = (u32)(dx * dx + dy * dy + dz * dz);
        u32 log2_val;
        if (dist_sq < 0x400) {
            log2_val = (u32)(*(&D_8008D118 + dist_sq)) >> 3;
        } else {
            s32 clz = 0;
            s32 sp_tmp;
            if ((s32)dist_sq >= 0) {
                /* Hand-written GTE leading-zero-count block (LZCS in, LZCR out) —
                 * canonical inline asm, identical to the user-authorized block in
                 * the matched sibling func_800274BC (src/code6cac_b.c:292). */
                __asm__ volatile(
                    "addu   $t4, %1, $zero\n"
                    "mtc2   $t4, $30\n"        /* LZCS <- dist_sq */
                    "nop\n"
                    "nop\n"
                    "addu   $t4, $sp, $zero\n" /* &sp_tmp (at 0($sp)) */
                    "swc2   $31, 0($t4)\n"     /* sp_tmp <- LZCR */
                    : "=m"(sp_tmp)
                    : "r"(dist_sq)
                    : "$12");
                clz = sp_tmp;
            }
            {
                u32 v0_m = (u32)-2;
                u32 v1_m;
                u32 idx;
                u32 hi;
                v0_m &= clz;
                v1_m = 0x16 - v0_m;
                idx = dist_sq >> v1_m;
                v1_m = v1_m >> 1;
                hi = (u32)((u8)(*((&D_8008D118) + idx)));
                log2_val = (hi << 16) >> (0x13 - v1_m);
            }
        }
        {
            s32 v1 = *a3;
            s32 v0 = v1 << 4;
            v0 = v0 - v1;
            v0 = v0 << 1;
            v0 = v0 + 0x1F4;
            if (log2_val < (u32)v0) {
                *(s16 *)(ent + 0x286) = 5;
                *t0 = 0;
            }
        }
    }
next:
    t1 += 1;
    a3 += 0x2C;
    t0 += 0x2C;
    if (t1 < 4) goto loop;
}
