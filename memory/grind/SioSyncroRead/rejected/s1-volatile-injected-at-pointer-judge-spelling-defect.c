/* REJECTED (spelling, not bytes): s1's sandbox-0 form, superseded by the s2 candidate.
 * The Judge's 2026-08-25 10:34 ruling (docs/grind/decisions.md:11002) PASSed the D_800F1AE0
 * Ruling-4 grant but flagged this form's SPELLING as a defect: it injects volatility at the
 * pointer over NON-volatile decls (`extern u16 D_800F1AE0;` + `volatile u16 *p_ae0`, and
 * `volatile s32 *st = flag` over non-volatile D_800F1AFC). volatile_extern_allowlist.txt +
 * engine/volatile_cheats.py:68-72 cover pattern-3 decl-level volatility ONLY; pointer/cast
 * injection is never covered. The s2 respelling moves volatile onto the granted decls
 * (extern volatile u16 D_800F1AE0; extern volatile s32 D_800F1AFC;) and has every handle
 * COPY volatility (twin SioSyncroWrite pattern); re-measured sandbox 0, 160/160.
 * Do not re-propose this injection spelling for any symbol.
 * Era decls: extern s32 D_800F1AFC; extern u16 D_800F1AE0; (both non-volatile). */
s32 SioSyncroRead(u8 *arg0, s32 arg1) {
    s32 r_arg1 = arg1;
    s32 *flag = &D_800F1AFC;
    s32 count;
    s32 retries = 0;
    s32 pkt_len;
    s32 spu;
    s32 (*cb)(s32, s32);

    if (*flag != 0) return -1;
    goto main_work;

cleanup_A:
    {
        u16 saved = *((volatile u16 *)(spu + 0xA));
        *((volatile u16 *)(spu + 0xA)) = 0x50;
        {
            volatile u16 *p_ae0 = &D_800F1AE0;   /* <-- THE DEFECT: injects volatile over non-volatile decl */
            *((volatile u16 *)(spu + 8)) = *p_ae0;
        }
        *((volatile u16 *)(spu + 0xE)) = D_800F1AE6;
        *((volatile u16 *)(spu + 0xA)) |= 0x10;
        *((volatile u16 *)(spu + 0xA)) = saved;
        *((volatile u16 *)(spu + 0xA)) &= 0xFFDF;
        DeliverEvent(0xF000000B, 0x8000);
        {
            volatile s32 *p_b04a = &D_800F1B04;
            return r_arg1 - *p_b04a;
        }
    }

cleanup_B:
    {
        s32 base = D_800A3044;
        *((volatile u16 *)(base + 0xA)) &= 0xFFDF;
        DeliverEvent(0xF000000B, 0x100);
        {
            volatile s32 *p_b04b = &D_800F1B04;
            return r_arg1 - *p_b04b;
        }
    }

main_work:
    {
        volatile u16 *p_ae2 = &D_800F1AE2;
        u32 mode = *p_ae2;
        pkt_len = *(s16 *)((s32)D_800A3074 + ((mode & 0x300) >> 7));
    }
    flag[2] = r_arg1;
    flag[1] = (s32)arg0;
    *flag = 0;
    *((volatile u16 *)(D_800A3044 + 0xA)) |= 0x20;

    count = 0;
    if (flag[2] == 0) goto final_cleanup;
    {
        volatile s32 *st = flag;   /* <-- THE DEFECT: injects volatile over non-volatile D_800F1AFC */

        do {
            spu = D_800A3044;
            if ((*((volatile u16 *)(spu + 4))) & 0x38) goto cleanup_A;
            if (!((*((volatile u16 *)(spu + 4))) & 2)) {
                do {
                    cb = D_800F1AE8;
                    if (cb != 0) {
                        s32 prev = retries;
                        retries += 1;
                        if (cb(1, prev) == 0) goto cleanup_B;
                    }
                } while (!((*((volatile u16 *)(D_800A3044 + 4))) & 2));
            }

            *((u8 *)D_800F1B00) = *((u8 *)D_800A3044);
            st[1]++;
            count += 1;
            st[2]--;
            if (count == pkt_len) {
                count = 0;
                *((volatile u16 *)(D_800A3044 + 0xA)) ^= 2;
            }
        } while (st[2] != 0);
    }

final_cleanup:
    *((volatile u16 *)(D_800A3044 + 0xA)) &= 0xFFDF;

return_val:
    {
        volatile s32 *p_b04 = &D_800F1B04;
        return r_arg1 - *p_b04;
    }
}
