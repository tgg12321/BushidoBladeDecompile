/* REJECTED (score 26, 114 insns). `off = i * 8` at the top of the outer loop used by ALL FOUR
 * sites (pool load, pool clear, both lbu volumes).  loop.c leaves %hi(D_800EFB78) alone (good)
 * but jump.c cross-jumps the `if (voice < 0x18)` guard into the do-while bottom test (emits
 * `j` to the shared test) so the target's duplicated guard (sll/sra/slti/beqz) is missing, and
 * the clear site reuses s2 instead of recomputing sll/sra 13 at the join.
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
            voice = next;
            if ((s16)voice < 0x18) {
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
        *(s32 *)((u8 *)&D_800EFB78 + off) = 0;
    }
}
