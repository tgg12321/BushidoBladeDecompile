/* REJECTED [s22 2026-09-07, rederive] - operand read hoisted to the loop head,
  * tail duplicated into the first six arms.
  * Purpose: extending val's live range lowers the operand carrier's global.c
  * priority, so the walker wins $v1 at a LOWER duplication count - which is the
  * whole game under -msoft-float, where the loop.c:1631 hoist of the 0xFF
  * head-test constant needs loop insn_count <= 58 and each duplicated arm costs
  * +2 over a base of 47.
  * MEASURED: it does exactly that - the RA flip moves from K=7 (plain chassis)
  * to K=6 - but it is not enough twice over. 47 + 12 = 59 insns, so the 0xFF
  * still de-hoists (`Insn 81: regno 85 (life 1), move-insn savings 1 not
  * desirable`, s22 .loop dump), and the hoisted lbu costs a byte of its own:
  * score 6 vs the plain chassis's 5 (honest 4 vs 3). K=5 (57 insns) DOES hoist
  * but the allocator has not flipped yet: 30.
  * Banked as the measured proof that the val-livelen lever is worth exactly one
  * duplicated arm, and one arm is not the three instructions needed. */
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
        val = *ptr;
        if (c == 0xFF) {
            ptr += 6;
        } else if (c < 0x80) {
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
