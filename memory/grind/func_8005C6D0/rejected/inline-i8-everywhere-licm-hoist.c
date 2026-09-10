/* REJECTED (score 20, 121 insns). `i * 8` written inline at the pool load AND clear sites,
 * `off` only inside the voice guard.  Guard duplication and the `addu $s2,$v1,$zero` copy are
 * both correct, but with two inline uses of `(u8 *)&D_800EFB78 + i * 8` loop.c hoists the whole
 * symbol address into a callee-saved register (`lui/addiu` in the outer preheader, then
 * `addu v0,v1,s7 / lw 0(v0)`), costing an extra callee-saved save/restore pair and an 8-byte
 * frame.  The target keeps the `lui $at,%hi / addu $at,$at,idx / lw %lo($at)` form at both sites.
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
        p = *(u16 **)((u8 *)&D_800EFB78 + i * 8);
        if (p != 0 && (s32)D_800EFC38[*p] < 0) {
            voice = next;
            if ((s16)voice < 0x18) {
                off = i * 8;
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
