/* Distance 0 but BANNED: carries both the semantically-null `if ((s16)voice < 0x18)`
 * guard and the entry_off/vol_off pair. Layer-1 FAIL 2026-09-10 09:29; the driver
 * rejects any candidate-ready re-declaring either. Kept for the record only. */
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
    s32 vol_off;
    s32 entry_off;
    u32 *ev;

    SpuGetAllKeysStatus(keys);
    next = 0;
    for (i = 0; (s16)i < 0x18; i = (s16)(i + 1)) {
        entry_off = i * 8;
        p = *(u16 **)((u8 *)&D_800EFB78 + entry_off);
        if (p != 0 && (s32)D_800EFC38[*p] < 0) {
            voice = next;
            if ((s16)voice < 0x18) {
                vol_off = i * 8;
                for (; (s16)voice < 0x18; voice = (s16)(voice + 1)) {
                    if (SpuGetKeyStatus(1 << voice) != 1) {
                        vab = *p;
                        if (vab == 6 && D_800EFC50 == D_800EFC44) {
                            vab = 3;
                        }
                        ev = &((u32 *)D_800EFC38[vab][0])[p[1]];
                        SsUtKeyOnV((s16)voice, (s16)vab,
                                   (s16)(*ev & 0x7F),
                                   (s16)((*ev >> 7) & 0xF),
                                   (s16)((*ev >> 11) & 0x7F),
                                   (s16)((*ev >> 18) & 0x7F),
                                   *((u8 *)&D_800EFB7D + vol_off),
                                   *((u8 *)&D_800EFB7C + vol_off));
                        next = (s16)(voice + 1);
                        break;
                    }
                }
            }
        }
        *(s32 *)((u8 *)&D_800EFB78 + i * 8) = 0;
    }
}
