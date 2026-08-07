/* KILLED (s7, 27/92, from the candidate's 8/91).  The Sony reference spells
 * the timeout test as ONE short-circuit `||`:
 *     if (Alarm.unk0 < VSync(-1) || Alarm.unk4++ > 0x3C0000)
 * where sessions 1-6 spell it as two ifs with `goto do_timeout`.  The two are
 * semantically identical and compile to the same two-branch CFG, so this
 * looked like a free faithfulness win.  It is not: it costs 19 points and one
 * instruction (92 vs target's 91), isolated against an otherwise byte-for-byte
 * identical candidate.
 *
 * Related kills in the same batch: c2 (only the exits changed to the
 * reference's `while (1) { ... break; }`, keeping the inner gotos) explodes to
 * 61/129; c4 (c1 + c2) to 74/131.  Both are pathological block duplication,
 * not near-misses.  The candidate's `do { ... } while (a0 == 0)` + `check:`
 * scaffolding stands.
 *
 * What the reference DID confirm about the tail: target's
 * `j / li v0,-1 / move v0,zero / bnez v0 / li v0,-1` at idx 68-72 is the
 * inlined-`get_alarm()` return-value shape, i.e. the candidate's `v0` temp and
 * `check:` label ARE the original's structure. */
/* c1 = 8/91 candidate, ONLY the timeout test respelled as the Sony
 * reference's `||` short-circuit (everything else byte-for-byte candidate). */
s32 saEft01Init(s32 a0) {
    s32 v0;
    s32 cnt;
    s32 ret;
    s32 k;
    s32 *tbl_11dc;
    u8 *idx_1494;
    s32 *tbl_125c;

    D_800F19B8 = sys_VSync(-1) + 0x3C0;
    tbl_11dc = D_800A11DC;
    idx_1494 = &D_800A1494;
    tbl_125c = D_800A125C;
    D_800F19BC = 0;
    D_800F19C0 = &D_800162C0;

    do {
        k = 0x3C0000;
        if (D_800F19B8 < sys_VSync(-1) || (cnt = D_800F19BC, D_800F19BC = cnt + 1, k < cnt)) {
        tslTm2LoadImage_2(&D_800161B8);
        {
            s32 arg4;
            arg4 = tbl_125c[idx_1494[0]];
            debug_printf(&D_800161C8, D_800F19C0, tbl_11dc[D_800A11D5], arg4, tbl_125c[idx_1494[1]]);
        }
            cdrom_ClearIrq();
            v0 = -1;
        } else {
            v0 = 0;
        }
        if (v0 != 0) {
            ret = -1;
            break;
        }
        k = 0x1000000;
        cnt = k;
        ret = *D_800A14C0 & cnt;
        if (ret == 0) {
            break;
        }
        ret = 1;
    } while (a0 == 0);
    return ret;
}
