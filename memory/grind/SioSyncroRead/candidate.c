/* SioSyncroRead — s1 candidate — SANDBOX 0 THIS SESSION (2026-08-25, 160/160)
 * Measured: sandbox SioSyncroRead --disable all => score 0, build_insns 160 == target_insns 160,
 * with this exact body in src/main.c (rules_dropped 6 by the sandbox; the 6 committed regfix
 * rules are calibrated to the OLD rule-era body and must be retired at integration).
 * BLOCKER (why s1 returned ruling-request, not candidate-ready): p_ae0 is a volatile handle
 * to D_800F1AE0, which has NO volatile_extern_allowlist.txt row. Measured this session:
 * the non-volatile handle lets combine.c fold the address into the load (159i, addiu lost);
 * volatile is byte-required. Identical Ruling-4 evidence class as D_800F1AE2's existing grant
 * (allowlist line 46, granted 2026-08-19 off the SioSyncroWrite twin). Grant question filed
 * in outcome ruling_question.
 * Depends on existing extern decls at src/main.c (D_800F1AFC, volatile D_800F1B00/B04,
 * D_800A3044, D_800F1AE0 (u16), volatile u16 D_800F1AE2, D_800F1AE6, D_800F1AE8, D_800A3074).
 * Modeled on the MATCHED twin SioSyncroWrite (COMPLETED-C 2026-08-19, same Sony LIBCOMB COMB
 * module): volatile st[] increment/decrement (expand_increment dead-reload fingerprint),
 * FAKE pointer handles for combine symbol-fold defeat, subword D_800A3074 table read.
 */
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
            /* FAKE: redundant second handle to D_800F1AE0, mechanism: MEM_VOLATILE_P blocks combine.c's single-use symbol-address fold into the load (non-volatile handle measured folded: 159i vs target 160), lever-exhaustion: memory/grind/SioSyncroRead/hypotheses.md [s1-H2]. GRANT PENDING: D_800F1AE0 has no volatile_extern_allowlist.txt row yet â€” Ruling-4-class evidence banked, see ledger */
            volatile u16 *p_ae0 = &D_800F1AE0;
            *((volatile u16 *)(spu + 8)) = *p_ae0;
        }
        *((volatile u16 *)(spu + 0xE)) = D_800F1AE6;
        *((volatile u16 *)(spu + 0xA)) |= 0x10;
        *((volatile u16 *)(spu + 0xA)) = saved;
        *((volatile u16 *)(spu + 0xA)) &= 0xFFDF;
        DeliverEvent(0xF000000B, 0x8000);
        {
            /* FAKE: duplicated return compute (re-merged by cross-jump), mechanism: flow.c reg_n_refs priority lift for the saved-arg1 pseudo in global.c allocno_compare, lever-exhaustion: this session's shared-label form measured 9/160 with s3<->s4 seat swap (solver model tmp/ra_solver_work/SioSyncroRead.model.json: p74 3refs/len102 vs p78 3refs/len56) */
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
            /* FAKE: duplicated return compute (re-merged by cross-jump), mechanism: flow.c reg_n_refs priority lift for the saved-arg1 pseudo in global.c allocno_compare, lever-exhaustion: as cleanup_A duplicate */
            volatile s32 *p_b04b = &D_800F1B04;
            return r_arg1 - *p_b04b;
        }
    }

main_work:
    {
        /* FAKE: redundant second handle to D_800F1AE2, mechanism: combine.c symbol-fold defeat (address forced into a pseudo, so lui/%lo is not folded into the load base), lever-exhaustion: twin precedent SioSyncroWrite s4-M1 (main.c:3044); this session's direct-global form measured 158i/24 (s1 variant A) */
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
        volatile s32 *st = flag;

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
        /* FAKE: redundant second handle to D_800F1B04, mechanism: combine.c symbol-fold defeat (address forced into a pseudo, so lui/%lo is not folded into the load base), lever-exhaustion: twin precedent SioSyncroWrite s4-M4 (main.c:3104); this session's direct-global form measured 158i/24 (s1 variant A) */
        volatile s32 *p_b04 = &D_800F1B04;
        return r_arg1 - *p_b04;
    }
}
