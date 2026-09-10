/* func_8005C6D0 - per-frame sound-request flush.
 * STATUS (grind s2, permuter modality, 2026-09-10): honest sandbox distance 0,
 * 118/118 instructions, on a chassis that NO LONGER carries the semantically-null
 * `if ((s16)voice < 0x18)` guard the layer-1 reviewer FAILed on 2026-09-10 09:29.
 * It still carries ONE construct that is pending a ruling: the byte offset i*8 is
 * named TWICE - `off` for the pool-entry load, and `nv` (assigned `nv = off;` at the
 * top of the voice-scan body, LICM-hoisted into the scan's preheader) for the two
 * volume-byte reads.  DO NOT submit this as candidate-ready until that ruling lands;
 * the driver's banned-construct tripwire covers `entry_off`/`vol_off`, and `nv` is the
 * same intent respelled.
 *
 * Why the second name is there (measured, not argued - s2 evidence.md):
 * the target emits `addu $s2,$v1,$zero` at 0x8005C768, a REAL instruction in the
 * shipped bytes: $v1 holds i*8 computed at the top of the outer loop for the pool
 * load, and $s2 is a second, callee-saved copy of it that survives the
 * SpuGetKeyStatus call and feeds the two `lbu %lo(D_800EFB7{C,D})` volume loads.
 * All seven single-name spellings were measured on this chassis and none of them
 * produces that copy (best 8 / 114 insns); the guard-free single-`off` form loses
 * exactly the 4 insns of the duplicated exit test plus the copy.
 *
 * Object model, LICM, argument-order and `next`-placement notes from s1/s2 are
 * unchanged and still apply - see memory/grind/func_8005C6D0/hypotheses.md H1-H7.
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
    s32 nv;
    u32 *ev;

    SpuGetAllKeysStatus(keys);
    next = 0;
    for (i = 0; (s16)i < 0x18; i = (s16)(i + 1)) {
        off = i * 8;
        p = *(u16 **)((u8 *)&D_800EFB78 + off);
        if (p != 0 && (s32)D_800EFC38[*p] < 0) {
            voice = next;
            for (; (s16)voice < 0x18; voice = (s16)(voice + 1)) {
                nv = off;
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
                               *((u8 *)&D_800EFB7D + nv),
                               *((u8 *)&D_800EFB7C + nv));
                    next = (s16)(voice + 1);
                    break;
                }
            }
        }
        *(s32 *)((u8 *)&D_800EFB78 + i * 8) = 0;
    }
}
