/* REJECTED [s22 2026-09-07, rederive] - operand read moved into the
  * else-of-0xFF arm (read on the short-command path but not the 0xFF path),
  * tail duplicated into the first seven arms.
  * Purpose: a semantically narrower version of the val-hoist lever - extend the
  * operand carrier's live range without reading past the 0xFF command's payload.
  * MEASURED: score 7 at K=7 (24 at K=4/K=5, 27 at K=6), i.e. it does NOT move
  * the RA flip below K=7 the way the full loop-head hoist does, and it costs +2
  * on the score. Strictly dominated by both the plain chassis (5) and the
  * loop-head hoist (6). */
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
    ptr += 5;
    while (c != 0) {
        if (c == 0xFF) {
            ptr += 6;
        } else {
            val = *ptr;
            if (c < 0x80) {
                ptr++;
            } else {
                ptr++;
                switch (c - 0x80) {
                    case 0: pad[0xA1] = val; c = *ptr; ptr++; continue;
                    case 1: pad[0xA3] = val; c = *ptr; ptr++; continue;
                    case 2: pad[0xA7] = val; c = *ptr; ptr++; continue;
                    case 3: pad[0xA8] = val; c = *ptr; ptr++; continue;
                    case 4: pad[0xA9] = val; c = *ptr; ptr++; continue;
                    case 5: pad[0xA5] = val; c = *ptr; ptr++; continue;
                    case 6: pad[0xA6] = val; c = *ptr; ptr++; continue;
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
    }
}
