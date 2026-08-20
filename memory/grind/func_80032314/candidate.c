/* func_80032314 — best form as of grind session 2 (structural).
 * Honest floor: sandbox --disable all = 1 (was 15 at session start, 27 at s1 start).
 * build_insns == target_insns == 109. NO register pins, no cheats — the only
 * inline asm is the user-authorized canonical GTE-LZC island (verbatim from the
 * matched sibling func_800274BC, src/code6cac_b.c:292, authorized 2026-06-10).
 *
 * THE SESSION-2 WIN (15 -> 1): the residual 15 was a 3-cycle rotation on
 * $a1/$a2/$a3 caused by global.c allocno priority order. Priority formula
 * (read from the BB2_ALLOC_DEBUG instrumented cc1, global.c prune_preferences/
 * find_reg block): pri = floor_log2(n_refs) * n_refs / live_length * 10000 *
 * size. Measured at s2 start: walker(p74, 8 refs, len 82) = 2926 > mflo1(p115,
 * 2 refs, len 10) = 2000 > ent(p75, 6 refs, len 63) = 1904 -> allocation order
 * walker,mflo1,ent -> regs 5,6,7 (rotation). Target needs mflo1,ent,walker ->
 * 5,6,7. Removing EXACTLY ONE walker ref drops floor_log2 from 3 to 2:
 * pri 2*7/82 = 1707, landing in the required band (below ent 1904, above
 * t0(p72) which rises to 7 refs = 1666). The C edit: the radius byte read
 * (final compare block) is spelled `*(u8 *)(t0 + 2)` instead of `*a3` — same
 * value (a3 == t0+2 loop-invariantly; invisible to cse across the back-edge).
 * Measured: every register in all 109 insns now matches target; score 1.
 *
 * THE ONE REMAINING DIFF: our `lbu v1,2(t0)` vs target `lbu $v1,0x0($a3)`
 * (the radius read itself). This is ARITHMETICALLY FORCED for pure C: a
 * byte-matching build needs 8 walker appearances (7 insns + dual-ref latch),
 * flow-time n_refs can never be below the final appearance count, and 8 refs
 * -> pri 2926 -> walker allocated first -> rotation. ent (6 refs, len 63
 * byte-pinned) caps at 1904; mflo1 (2 refs, len 10 byte-pinned) at 2000.
 * See evidence.md s2 for the full proof and the sanctioned F1 chain-extender
 * endgame (stale reg_n_refs on ent AND mflo1) with exact arithmetic.
 *
 * THE SESSION-1 WIN (do not regress): GTE island as ONE mnemonic block
 * (mtc2/swc2), never `.word` — see evidence.md FINDING 2.
 */
void func_80032314(void) {
    u8 *t0 = &D_80104E88;
    s32 t1 = 0;
    u8 *a3 = &D_80104E88 + 2;
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
        u32 dist_sq = (u32)(dx * dx + dy * dy + dz * dz);
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
            s32 v1 = *(u8 *)(t0 + 2);
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
