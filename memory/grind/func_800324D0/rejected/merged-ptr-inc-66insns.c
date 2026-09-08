/* REJECTED [s22 2026-09-07, rederive] - the short-command and payload `ptr++`
  * sites merged into one, tail duplicated into the first eight arms.
  * Purpose: the ONLY quantity standing between this function and a match under
  * -msoft-float is 3 loop instructions (loop.c:1631 wants insn_count <= 58, the
  * RA fix wants 61), so a shape that folds two real instructions into one was
  * the direct attack on the deficit.
  * MEASURED: build_insns 66 != target_insns 68 at every K, score 10 at K=8 and
  * 26-32 below it. It merges two instructions the TARGET KEEPS - which is itself
  * a useful positive fact: the original body provably has two separate `ptr++`
  * sites, one on the short-command path and one on the payload path, so the
  * base loop insn_count of 47 is not reducible by merging them. */
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
            ptr++;
            if (c >= 0x80) {
                switch (c - 0x80) {
                    case 0: pad[0xA1] = val; c = *ptr; ptr++; continue;
                    case 1: pad[0xA3] = val; c = *ptr; ptr++; continue;
                    case 2: pad[0xA7] = val; c = *ptr; ptr++; continue;
                    case 3: pad[0xA8] = val; c = *ptr; ptr++; continue;
                    case 4: pad[0xA9] = val; c = *ptr; ptr++; continue;
                    case 5: pad[0xA5] = val; c = *ptr; ptr++; continue;
                    case 6: pad[0xA6] = val; c = *ptr; ptr++; continue;
                    case 7: pad[0xA2] = val; c = *ptr; ptr++; continue;
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
