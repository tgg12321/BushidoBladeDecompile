/* Per-frame sound-request flush: walk the 24-entry pending-sound pool, and for
 * every entry whose VAB is loaded, find the first free SPU voice at or after the
 * running `next` cursor and key the note on with the entry's stored volumes.
 * Each pool slot is cleared as it is visited.
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
                /* FAKE: second name for the pool byte offset i*8, feeding only the
                 * two volume-byte reads (named-intermediate family, .claude/rules/
                 * no-new-park-categories.md SOTN-accepted list as amended by
                 * .claude/rules/ordinary-c-judge-decidable.md Ruling 1);
                 * mechanism: GCC 2.7.2 local-alloc/global.c gives one C name one
                 * pseudo, so a single name can never produce the target's second,
                 * callee-saved copy of the offset that survives the SpuGetKeyStatus
                 * call (`addu $s2,$v1,$zero`, asm/funcs/func_8005C6D0.s:41,
                 * 0x8005C768); loop.c LICM hoists this copy into the scan preheader
                 * exactly where the target emits it;
                 * lever-exhaustion: memory/grind/func_8005C6D0/hypotheses.md H9 +
                 * evidence.md s2 - nine single-name spellings (8..39, all short of
                 * 118 insns), fifteen guard-free arrangements, and a 6,562-iteration
                 * decomp-permuter campaign that converged independently on this form. */
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
