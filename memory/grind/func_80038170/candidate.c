/* Candidate: func_80038170 (code6cac_c_mid.c) — session s1 (2026-07-28)
 * Honest floor: 5 (was 14/12) — layer-1 cheat-reviewer PASS on everything here.
 * IN PLACE in src/code6cac_c_mid.c as of s1.
 *
 * THE FRAME GAP IS SOLVED (was the WIP-era "unrecoverable phantom local"):
 *   out[0x42]/[0x43] = (&D_8008F19C)[s3*2+0 / +1]  (same-base pair, one table —
 *   same shape as the D_8008F1A8 pairs) makes GCC 2.7.2 allocate an 8-byte
 *   compiler stack temp inside the if(s3) conditional (phantom-frame-slots-gcc272
 *   mechanism: counted by get_frame_size, register-allocated away, ZERO stores).
 *   vars 8 -> 16, frame 48 -> 56 == target. No dead decls, no extra insns.
 *   Reviewer: PASS ("the MORE correct spelling"; D_8008F19D is a splat per-byte
 *   auto-symbol for D_8008F19C+1).
 *   Trigger conditions (measured, s1 probes): needs the full array-ref pair off
 *   ONE symbol with shared variable index, in TWO statements, inside a
 *   CONDITIONAL scope (outer-block pairs reuse one slot = the pre-existing 8).
 *   Named-pointer staging (u8 *t = &D_8008F19C + s3*2) KILLS the temp (vars=8).
 *   Two-symbol spelling (19C/19D) KILLS the temp (vars=8).
 *
 * RESIDUAL 5 = prologue init/save pair order ONLY: ours s0,s1,s2,s3,ra; target
 *   s0,s3,s2,s1,ra (offsets + frame all correct). The zero-inits' source order
 *   drives it. Both order levers are review-FAILed:
 *     - s32 s3=0,s2=0,s1=0 (decl-order flip)  -> rejected/decl-order-prologue-flip.c
 *     - s1 = s2 = s3 = 0;  (chained, RTL right-to-left) -> rejected/chained-zeroing-order.c
 *   With either lever the floor is 1, and that 1 is a PROVEN text-only artifact:
 *   source D_8008F19C+1 vs reference-.o D_8008F19D — linked words identical
 *   (lui 0x0980 / lbu imm 0xF19D both ways; verified via word-level diff vs
 *   asm/funcs raw words: only the jal reloc word differs pre-link). Once build/
 *   regenerates from this src, sandbox reads 0.
 *   => s1 outcome: ruling-request on the init-order question.
 */

void func_80038170(u8 *out) {
    s32 s1, s2, s3;
    s32 i;
    s32 mask;
    s32 bit;

    s1 = 0;
    s2 = 0;
    s3 = 0;
    mask = D_80106A50;

    for (i = 0; i < 0x1B; i++) {
        bit = 1 << i;
        if (mask & bit) {
            s32 v = (&D_8008F204)[i];
            switch (v) {
                case 0: s1++; break;
                case 1: s2++; break;
                case 2: s3++; break;
            }
        }
    }

    i = 0x3F;
    out[0] = 0x53;
    out[1] = 0x43;
    out[2] = 0x11;
    out[3] = 0x01;

    {
        u8 *p = out + 0x3F;
        do {
            p[4] = 0;
            i--;
            p--;
        } while (i >= 0);
    }

    func_80079194(out + 4, &D_8008F1C0);

    out[0x22] = (&D_8008F1A8)[s1 * 2 + 0];
    out[0x23] = (&D_8008F1A8)[s1 * 2 + 1];
    out[0x3C] = (&D_8008F1A8)[s2 * 2 + 0];
    out[0x3D] = (&D_8008F1A8)[s2 * 2 + 1];

    if (s3 > 0) {
        out[0x40] = D_800A3200;
        out[0x41] = D_800A3201;
        out[0x42] = (&D_8008F19C)[s3 * 2 + 0];
        out[0x43] = (&D_8008F19C)[s3 * 2 + 1];
    }

    i = 0x1B;
    {
        u8 *p = out + 0x1B;
        do {
            p[0x44] = 0;
            i--;
            p--;
        } while (i >= 0);
    }

    i = 0;
    {
        u16 *src = (u16 *)&D_800109EC;
        u8 *dst = out;
        do {
            *(u16 *)(dst + 0x60) = *src;
            src++;
            i++;
            dst += 2;
        } while (i < 0x10);
    }

    i = 0;
    {
        u8 *outer_src = (u8 *)&D_80010A2C;
        u8 *outer_dst = out;
        do {
            s32 j = 0;
            u16 *dst = (u16 *)(outer_dst + 0x80);
            u16 *src = (u16 *)outer_src;
            do {
                *dst = *src;
                src++;
                j++;
                dst++;
            } while (j < 0x40);
            outer_src += 0x80;
            i++;
            outer_dst += 0x80;
        } while (i <= 0);
    }
}
