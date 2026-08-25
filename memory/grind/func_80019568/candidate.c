/* candidate — func_80019568 — s1 (2026-08-25)
 *
 * BEST MEASURED: sandbox --disable all = 28 (target 141, build 143) with this file
 * applied verbatim.  HONEST-SUBMITTABLE FLOOR: 34 — obtained by changing the loop-1
 * exit test back to `} while (i < 2);` (build 141).
 *
 * WARNING: the `} while (i + 1 < 3);` spelling below is a DIAGNOSTIC probe, not a
 * submission form.  It blocks loop.c biv elimination of `i` (maybe_eliminate_biv_1's
 * REG case fails on the biv nested in the PLUS), which aligns the whole register file
 * with target (i->t0, mask->t1, const4->t2, jtbl->t3), but combine cannot fold
 * (lt (plus i 1) 3) -> (lt i 2) for signed <, so it emits 2 extra insns and reads as a
 * coercion.  Frontier F1 (hypotheses.md) = find the honest spelling with the same
 * elimination-blocking effect.  Remaining residuals: F2 (li-1 hoisted out of loop),
 * F3 (tail block: materialized &D_80102790 + early store + order 9C,94,98).
 */
void func_80019568(s32 arg0) {
    struct {
        s16 output[4];
        s32 voice_mask;
        s32 unk_1C;
        s32 unk_20;
        s32 unk_24;
        s32 packets[4];
    } sp;
    u8 *packets;
    s16 *output;
    s32 i;
    s32 voice_mask;
    s32 old_mask;
    s16 *base_addr;
    s16 *dst1;
    s16 *dst0;
    s16 *src;

    voice_mask = 0;
    i = 0;
    packets = (u8 *)&sp.packets[0];
    sp.packets[0] = D_800FF580;
    sp.packets[1] = D_800FF584;
    sp.packets[2] = D_800FF5A4;
    sp.packets[3] = D_800FF5A8;
    do {
        s32 bits;

        if (packets[i * 8] == 0) {
            s32 voice2;

            sp.output[i] = packets[i * 8 + 1] >> 4;
            sp.output[i + 2] = 1;
            voice2 = (s16)((u16)sp.output[i] - 1);

            if ((u32)voice2 < 8) {
                switch (voice2) {
                case 4:
                case 6:
                    sp.output[i] = 4;
                case 1:
                case 2:
                case 3:
                    bits = ~((packets[i * 8 + 2] << 8) | packets[i * 8 + 3]);
                    break;
                case 0:
                case 5:
                case 7:
                default:
                    bits = 0;
                    break;
                }
            } else {
                bits = 0;
            }
        } else {
            sp.output[i] = 4;
            sp.output[i + 2] = 0;
            bits = 0;
        }

        voice_mask = ((u32)voice_mask >> 16) | (bits << 16);
        i++;
    } while (i + 1 < 3);

    sp.voice_mask = voice_mask;
    func_8001B138(&sp.voice_mask);

    if (D_800A3834 == 1 && arg0 == 0) {
        s32 voice_state = D_800A38DC;

        if ((u32)voice_state < 7) {
            switch (voice_state) {
            case 4:
            case 5:
                if (D_8010278E == 0) {
                    sp.voice_mask |= 0x08000800;
                }
            case 0:
            case 1:
            case 2:
            case 3:
            case 6:
                if (D_8010278C == 0) {
                    sp.voice_mask |= 0x08000800;
                }
                break;
            }
        }
    }

    func_8003A728((s32)&sp.output[0]);

    i = 0;
    base_addr = &D_80102788;
    dst1 = base_addr + 2;
    dst0 = base_addr;
    src = &sp.output[0];

    do {
        dst0[0] = src[0];
        dst0++;
        dst1[0] = src[2];
        src++;
        i++;
        dst1++;
    } while (i < 2);

    old_mask = D_80102790;
    D_80102790 = sp.voice_mask;
    D_80102794 = sp.voice_mask & ~old_mask;
    D_8010279C = ~sp.voice_mask;
    D_80102798 = ~sp.voice_mask & old_mask;
}
