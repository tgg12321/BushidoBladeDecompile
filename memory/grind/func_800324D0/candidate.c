/* func_800324D0 — CANDIDATE (s1, 2026-08-20): sandbox --disable all == 0, 68/68 insns.
 * Pin-free pure C. See evidence.md [s1] for the full RA mechanism chain.
 *
 * The load-bearing structure (do NOT "simplify" these away):
 *  - `base` (stream base pointer): once-written, read twice (base[4] and base+5).
 *    Because it has two real consumers, neither cse1 (it is not a reg-reg copy)
 *    nor combine (not single-use) can delete it, so it survives to local-alloc
 *    as a block-0 pseudo.
 *  - `ff` (the 0xFF default value): block-0 local read by 7 real sb stores.
 *    Local-alloc assigns ff -> $2 first (higher density), so the overlapping
 *    `base` gets $3. set_preference() on `ptr = base + 5` (PLUS lookthrough,
 *    reg_renumber substitution) then plants hard_reg_full_preferences[$3] on
 *    the walker pseudo; prune_preferences puts $3 into regs_someone_prefers of
 *    the higher-priority payload/command allocnos, which skip it in find_reg
 *    pass 0 (val -> $a1, cmd -> $a2), and the walker takes its preferred $v1.
 *  - Splitting `ff` from the stream byte `c` is what makes base/ff overlap in
 *    block 0 (the previous m2c body reused one variable for both).
 */
void func_800324D0(u8 *pad) {
    u8 *base;
    u8 ff;
    u8 *ptr;
    u8 c;
    u32 cmd;
    u8 val;

    base = *(u8 **)(pad + 0x58);
    ff = 0xFF;
    pad[0xA1] = ff;
    pad[0xA3] = ff;
    pad[0xA2] = ff;
    pad[0xA4] = ff;
    pad[0xAA] = 0;
    pad[0xA7] = 0;
    pad[0xA8] = 0;
    pad[0xA5] = 0;
    pad[0xA6] = ff;
    pad[0xAB] = ff;
    pad[0xAC] = ff;

    c = base[4];
    ptr = base + 5;
    if (c == 0) return;

    do {
        cmd = c;
        if (cmd == 0xFF) {
            ptr += 6;
        } else if (cmd < 0x80) {
            ptr++;
        } else {
            cmd -= 0x80;
            val = *ptr;
            ptr++;
            if (cmd < 12) {
                switch (cmd) {
                    case 0: pad[0xA1] = val; break;
                    case 1: pad[0xA3] = val; break;
                    case 2: pad[0xA7] = val; break;
                    case 3: pad[0xA8] = val; break;
                    case 4: pad[0xA9] = val; break;
                    case 5: pad[0xA5] = val; break;
                    case 6: pad[0xA6] = val; break;
                    case 7: pad[0xA2] = val; break;
                    case 8: pad[0xA4] = val; break;
                    case 9: pad[0xAA] = val; break;
                    case 10: pad[0xAB] = val; break;
                    case 11: pad[0xAC] = val; break;
                }
            }
        }
        c = *ptr;
        ptr++;
    } while (c != 0);
}
