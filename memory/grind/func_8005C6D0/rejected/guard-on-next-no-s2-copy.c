/* REJECTED (score 20, 118 insns). Guard written as `if ((s16)next < 0x18) { voice = next; ... }`
 * with `off` at the top of the outer loop.  Gets the duplicated guard AND no LICM hoist, and the
 * instruction COUNT is exactly right, but the guard tests `next` before the copy so the target's
 * `addu $s0,$s4,$zero` / `addu $s2,$v1,$zero` pair is replaced by a single `move s0,s3`, and the
 * i / next register roles come out swapped (s3<->s4) versus the target.
 */
extern s32 D_800EFC44;
extern s32 D_800EFC50;
extern void SpuGetAllKeysStatus(u8 *);
extern s32 SpuGetKeyStatus(s32);
extern s32 SsUtKeyOnV(s16, s16, s16, s16, s16, s16, s16, s16);
void func_8005C6D0(void) {
    extern s32 *D_800EFC38[];
    u8 keys[24];
    s16 i;
    s16 voice;
    s16 next;
    u16 vab;
    u16 *p;
    s32 off;
    u32 *ev;

    SpuGetAllKeysStatus(keys);
    next = 0;
    for (i = 0; (s16)i < 0x18; i = (s16)(i + 1)) {
        off = i * 8;
        p = *(u16 **)((u8 *)&D_800EFB78 + off);
        if (p != 0 && (s32)D_800EFC38[*p] < 0) {
            if ((s16)next < 0x18) {
                voice = next;
                do {
                    if (SpuGetKeyStatus(1 << voice) != 1) {
                        vab = *p;
                        if (vab == 6 && D_800EFC50 == D_800EFC44) {
                            vab = 3;
                        }
                        ev = &((u32 *)D_800EFC38[vab][0])[p[1]];
                        next = (s16)(voice + 1);
                        SsUtKeyOnV((s16)voice, (s16)vab,
                                   (s16)(*ev & 0x7F),
                                   (s16)((*ev >> 7) & 0xF),
                                   (s16)((*ev >> 11) & 0x7F),
                                   (s16)((*ev >> 18) & 0x7F),
                                   *((u8 *)&D_800EFB7D + off),
                                   *((u8 *)&D_800EFB7C + off));
                        break;
                    }
                    voice = (s16)(voice + 1);
                } while ((s16)voice < 0x18);
            }
        }
        *(s32 *)((u8 *)&D_800EFB78 + i * 8) = 0;
    }
}
