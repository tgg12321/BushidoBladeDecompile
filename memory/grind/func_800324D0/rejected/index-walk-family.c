/* func_800324D0 — REJECTED FAMILY: index-based walk (rederive, brief-session
 * 10, 2026-08-20). The one structurally-different derivation of the stream
 * walk never measured before: fixed base pointer + integer index instead of
 * the self-incrementing pointer. Three spellings, all measured this session
 * (sandbox --disable all), ALL shape-broken at build 69 vs target 68:
 *
 *   1. plain index (base fixed, i = 5, c = base[i] tail):        26, 69/68
 *   2. + staged tail (cmd = base[i]; c = cmd;):                  14, 69/68
 *   3. + biased base (base += 5; i = 0) + staged tail:           13, 69/68
 *      (this file's text)
 *
 * Why it dies (read from s10/sbs_biased_index13.txt):
 * - Spelling 3 matches the ENTIRE head, lines 0-19, including
 *   `addiu $3,$3,5` IN the beqz delay slot with the walker-init in $3 —
 *   exactly target's $v1 geometry. First time any spelling has put the
 *   walker-init in target's register.
 * - The extra 69th insn is the strength-reduction header copy `move $6,$3`:
 *   loop.c turns base[i] + i-increments into a walking-pointer giv, a NEW
 *   pseudo initialized by a reg-reg copy from base at loop entry. Target has
 *   no such copy (its walker is one continuous web in $v1).
 * - The copy would become a deletable no-op ONLY if the giv were allocated
 *   $3 — which needs the cmd head/arm webs to skip $3 in find_reg pass 0:
 *   the exact condition proven impossible in s2 (set_preference cannot plant
 *   a walker pref) / s6 (cascade partition). The giv's allocno has the same
 *   low density as the pointer-form walker (s1 arithmetic kill applies
 *   verbatim), and the copy cannot plant a $3 preference because base is a
 *   GLOBAL pseudo (live across the beqz edge, block 0 -> preheader), so
 *   set_preference sees no renumbered hard reg at planting time.
 * - Register residual is the SAME walker<->cmd 2-swap (ours giv=$6/cmd=$3 vs
 *   target $3/$6) in every index spelling — no new seam, the wall relocated
 *   into loop.c's giv creation but is the same find_reg cascade.
 *
 * The masked scores 14/13 < 15 are alignment-shift accounting on a mis-shaped
 * 69-insn body, NOT floor progress: a 69-insn build can never byte-match, and
 * per the ledger's uniform standard build != 68 is dead. Spellings 1/2
 * differ from 3 only in the delay-slot fill (their walker-init
 * `addiu $6,$3,5` is a cross-register add, ineligible for the slot; 3's
 * same-register `addiu $3,$3,5` fills it).
 */
void func_800324D0(u8 *pad) {
    u8 *base;
    u32 i;
    u8 c;
    u32 cmd;
    u8 val;

    base = *(u8 **)(pad + 0x58);
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

    c = base[4];
    base += 5;
    i = 0;
    while (c != 0) {
        if (c == 0xFF) {
            i += 6;
        } else if (c < 0x80) {
            i++;
        } else {
            cmd = c - 0x80;
            val = base[i];
            i++;
            {
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
        cmd = base[i];
        c = cmd;
        i++;
    }
}
