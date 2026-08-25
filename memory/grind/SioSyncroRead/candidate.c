/* SioSyncroRead — s3 LANDED FORM — SANDBOX 0 (2026-08-25 s3, 160/160, rules_dropped 6)
 * This is the Judge-verified spelling (decl-level volatile per the binding constraint in
 * state.json / decisions.md 2026-08-25 10:34 PASS ruling): volatility lives on the DECLS
 * (`extern volatile s32 D_800F1AFC;` @ src/main.c:2891, `extern volatile u16 D_800F1AE0;`
 * @ src/main.c:2913) and every pointer in the body INHERITS it — nothing is injected at a
 * pointer/cast over a non-volatile decl. The two Ruling-4 allowlist rows
 * (volatile_extern_allowlist.txt:48-49) were landed in s3 under the driver-executed
 * integration-handoff scope grant (tools/grinder/scope_allow.txt: "SioSyncroRead
 * volatile_extern_allowlist.txt"); completion-gate cheat count re-measured 0 for both
 * SioSyncroRead and SioAnsyncRead after the rows landed.
 * Sibling coupling: SioAnsyncRead's `flag` local is aligned to `volatile s32 *` (the shared
 * D_800F1AFC decl is now volatile); measured byte-neutral (sandbox 0, 24/24, s3).
 * The 6 committed regfix rules are rule-era calibration debt: sandbox drops them
 * (rules_dropped 6) and they retire via the driver's `retire` at COMPLETED-C.
 * Superseded injection spelling banked at rejected/s1-volatile-injected-at-pointer-judge-
 * spelling-defect.c. Derivation history: evidence.md [s1] ladder (41 -> 0), [s3] landing.
 * Other extern deps (pre-existing in main.c): volatile s32 D_800F1B00/B04, s32 D_800A3044,
 * volatile u16 D_800F1AE2, u16 D_800F1AE6, D_800F1AE8 fn-ptr, s16 D_800A3074[4].
 */
s32 SioSyncroRead(u8 *arg0, s32 arg1) {
    s32 r_arg1 = arg1;
    volatile s32 *flag = &D_800F1AFC;
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
            /* FAKE: redundant second handle to D_800F1AE0, mechanism: MEM_VOLATILE_P blocks combine.c's single-use symbol-address fold into the load (non-volatile handle measured folded: 159i vs target 160), lever-exhaustion: memory/grind/SioSyncroRead/hypotheses.md [s1-H2]; volatility copied from the granted decl (Ruling-4 grant, docs/grind/decisions.md 2026-08-25 10:34) */
            volatile u16 *p_ae0 = &D_800F1AE0;
            *((volatile u16 *)(spu + 8)) = *p_ae0;
        }
        *((volatile u16 *)(spu + 0xE)) = D_800F1AE6;
        *((volatile u16 *)(spu + 0xA)) |= 0x10;
        *((volatile u16 *)(spu + 0xA)) = saved;
        *((volatile u16 *)(spu + 0xA)) &= 0xFFDF;
        DeliverEvent(0xF000000B, 0x8000);
        {
            /* FAKE: duplicated return compute (re-merged by cross-jump), mechanism: flow.c reg_n_refs priority lift for the saved-arg1 pseudo in global.c allocno_compare, lever-exhaustion: s1's shared-label form measured 9/160 with s3<->s4 seat swap (solver model tmp/ra_solver_work/SioSyncroRead.model.json: p74 3refs/len102 vs p78 3refs/len56) */
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
        /* FAKE: redundant second handle to D_800F1AE2, mechanism: combine.c symbol-fold defeat (address forced into a pseudo, so lui/%lo is not folded into the load base), lever-exhaustion: twin precedent SioSyncroWrite s4-M1 (main.c:3044); s1's direct-global form measured 158i/24 (variant A) */
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
        /* FAKE: redundant second handle to D_800F1B04, mechanism: combine.c symbol-fold defeat (address forced into a pseudo, so lui/%lo is not folded into the load base), lever-exhaustion: twin precedent SioSyncroWrite s4-M4 (main.c:3104); s1's direct-global form measured 158i/24 (variant A) */
        volatile s32 *p_b04 = &D_800F1B04;
        return r_arg1 - *p_b04;
    }
}
