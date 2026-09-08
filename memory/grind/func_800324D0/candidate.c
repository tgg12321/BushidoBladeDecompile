/* kengo:HIGH  |  is_pad/Pad_Prs  |  111i */
void func_800324D0(u8 *pad) {
    u8 *ptr;
    u8 c;
    u8 val;

    ptr = *(u8 **)(pad + 0x58);
    pad[0xA1] = 0xFF;
    pad[0xA3] = 0xFF;
    pad[0xA2] = 0xFF;
    pad[0xA4] = 0xFF;
    pad[0xAA] = 0;
    pad[0xA7] = 0;
    pad[0xA8] = 0;
    pad[0xA5] = 0;
    pad[0xA6] = 0xFF;
    pad[0xAB] = 0xFF;
    pad[0xAC] = 0xFF;

    c = ptr[4];
    /* FAKE: the header advance `ptr += 5` is spelled as a four-step chain,
     * mechanism: combine folds the four `addiu` insns back to the single
     * `addiu $v1,$v1,5` the target carries (zero emitted bytes, build_insns
     * 68 == target_insns 68), and the only surviving effect is the extra
     * reg_n_refs count flow.c records BEFORE the fold, which lifts the
     * walker allocno's global.c allocno_compare priority above the payload
     * carrier's so find_reg seats the walker in $v1,
     * lever-exhaustion: memory/grind/func_800324D0/hypotheses.md s1-s23 */
    ptr++;
    ptr++;
    ptr++;
    ptr += 2;
    while (c != 0) {
        if (c == 0xFF) {
            /* FAKE: same combine-foldable chain-extender as above, applied
             * to the 0xFF command's `ptr += 6` advance, mechanism: combine
             * folds `addiu 1; addiu 5` back to the target's single
             * `addiu $v1,$v1,6`, contributing reg_n_refs inside the loop
             * (loop depth 2) without adding a final instruction,
             * lever-exhaustion: memory/grind/func_800324D0/hypotheses.md */
            ptr++;
            ptr += 5;
        } else if (c < 0x80) {
            ptr++;
        } else {
            val = *ptr;
            ptr++;
            /* FAKE: the loop tail `c = *ptr; ptr++;` is duplicated into the
             * first five command arms instead of being reached by falling
             * out of the switch, mechanism: flow.c's reg_n_refs census
             * counts the duplicated walker references before global.c's
             * allocno_compare ranks the allocnos, and jump2's cross-jump
             * pass (after reload) re-merges the identical tails so not one
             * duplicated instruction materialises,
             * lever-exhaustion: memory/grind/func_800324D0/hypotheses.md */
            switch (c - 0x80) {
                case 0: pad[0xA1] = val; c = *ptr; ptr++; continue;
                case 1: pad[0xA3] = val; c = *ptr; ptr++; continue;
                case 2: pad[0xA7] = val; c = *ptr; ptr++; continue;
                case 3: pad[0xA8] = val; c = *ptr; ptr++; continue;
                case 4: pad[0xA9] = val; c = *ptr; ptr++; continue;
                case 5: pad[0xA5] = val; break;
                case 6: pad[0xA6] = val; break;
                case 7: pad[0xA2] = val; break;
                case 8: pad[0xA4] = val; break;
                case 9: pad[0xAA] = val; break;
                case 10: pad[0xAB] = val; break;
                case 11: pad[0xAC] = val; break;
            }
        }
        c = *ptr;
        ptr++;
    }
}
