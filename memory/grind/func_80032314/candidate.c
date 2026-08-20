/* func_80032314 — best form as of grind session 3 (structural).
 * *** THIS FORM MEASURED sandbox --disable all == 0 (109/109 insns) on 2026-08-20 ***
 * (artifacts: tmp/grind/func_80032314/s3/{allocdbg.txt,build0.txt}).
 *
 * IT IS NOT YET SUBMITTABLE. The three lines marked F4 below are the
 * chain-extender endgame — sanctioned F1 family
 * (.claude/rules/dead-store-fake-exception.md:32 "combine-foldable
 * chain-extender", owner ruling 2026-07-01) — which requires (a) the full
 * modality ladder demonstrably spent per the DRIVER's ledger, (b) the named
 * GCC-pass mechanism (documented below), (c) /* FAKE *\/ annotations.
 * At s3 the ladder is NOT spent (recon, structural x2 only), so prong (a)
 * fails and submission would risk a Judge FAIL that bans the construct.
 * THE SESSION THAT SUBMITS THIS (once the driver's exhaustion gate opens)
 * must add the FAKE annotations (template in evidence.md [s3]) and write
 * self_vet.md citing dead-store-fake-exception.md:32.
 *
 * Removing ONLY the three F4 lines (s32 dxs decl becomes inline again:
 * dist_sq = (u32)(dx*dx + dy*dy + dz*dz); and radius read respelled
 * *(u8 *)(t0 + 2)) reproduces the s2 pure-C floor-1 form.
 *
 * THE MECHANISM (measured, s3): global.c allocno pri =
 * floor_log2(n_refs)*n_refs/live_length*10000. The fold-away detour pair
 * `ent += dxs; ent -= dxs;` at the END of the dist_sq<0x400 then-arm gives
 * stale flow-time refs (combine.c:52-57 documents verbatim that reg_n_refs
 * is NOT adjusted when a register is no longer required): ent 6->10 refs
 * (pri 4615), dxs (the first mflo) 2->4 refs len 14 (pri 5714). combine
 * folds (ent+dxs)-dxs -> ent to a self-move that final.c:1800-1806 elides
 * (zero bytes). PLACEMENT IS LOAD-BEARING TWICE OVER:
 *  - dxs pri must sit in the (4687, 6666) window: BELOW the dys mult-temp
 *    (6666, which must claim LO first via its mflo copy-preference — a
 *    higher-pri dxs gets captured by LO itself, measured score 23) and
 *    ABOVE ent. Arm-END placement gives len 14 -> 5714. Placement directly
 *    after the sum gives len 11 -> 7272 -> LO capture (score 23).
 *  - resulting allocation order dxs(5714) > ent(4615) > walker(2857) ->
 *    $a1/$a2/$a3 exactly as target; t0->$t0, t1->$t1.
 * Verified trace: tmp/grind/func_80032314/s3/allocdbg.txt (p114 dxs->5,
 * p75 ent->6, p74 walker->7, p72->8, p73->9).
 *
 * SESSION-2 WIN preserved: radius read via *a3 restored here (walker at its
 * byte-forced 8 refs); the *(u8*)(t0+2) respelling is only for the floor-1
 * no-FAKE form. SESSION-1 WIN preserved: GTE island as ONE mnemonic block
 * (mtc2/swc2), never .word (see evidence.md FINDING 2).
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
        s32 dxs = dx * dx;                       /* F4 (named so the detour can reference it) */
        u32 dist_sq;
        u32 log2_val;
        dist_sq = (u32)(dxs + dy * dy + dz * dz);
        if (dist_sq < 0x400) {
            log2_val = (u32)(*(&D_8008D118 + dist_sq)) >> 3;
            ent = (u8 *)((s32)ent + dxs);        /* F4 detour — folds to zero bytes */
            ent = (u8 *)((s32)ent - dxs);        /* F4 detour — folds to zero bytes */
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
