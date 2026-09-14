/* REJECTED (s1, 2026-09-14) - scored 13, NOT 0. Identical to the matched
 * candidate except the final error check is spelled with the printf/return-0
 * path as the THEN-arm and the success tail trailing:
 *     if (id == -1) { printf(D_800158CC, vab); return 0; }
 *     <success tail>  return hdr[2] - (s32) hdr;
 * GCC 2.7.2 emits the two exit blocks in source-arm order, so this spelling
 * puts the printf block last: the branch comes out `beq` into a far block and
 * the success tail needs a trailing `j` to the shared epilogue (133 insns vs
 * the target 134). The matched form inverts the test and moves the tail into
 * the arm - see memory/grind/func_8005C2A8/candidate.c.
 * NOTE this file already carries the s32 snd_VabOpen prototype fix (that fix
 * alone took the floor 16 -> 13); the arm orientation is the remaining 13.
 */
extern s32 *func_80077D00(void);
extern void func_800858D0(s32);
extern void func_8005C074(s16, s32);
extern void SsVabClose(s16);
extern s16 SsVabTransCompleted(s16);
extern s32 SsUtGetVBaddrInSB(s16);
extern s32 snd_VabOpen(s32 *, s16);
extern s32 printf(const char *, s32);
extern const char D_800158CC[];
extern s32 *D_800EFC38[];
extern s32 D_800EFB38[];
extern s32 D_800A3404;
extern s32 D_800A3408;
extern s32 D_800A340C;

s32 func_8005C2A8(s32 *hdr, s16 vabid, s32 arg2) {
    s16 i;
    s16 id;
    s16 vab;

    if ((func_80077D00()[5] & 0xF) == 3 && vabid == 5) {
        return 0;
    }
    func_800858D0(0);
    id = vabid;
    if (D_800EFC38[id] != 0) {
        SsVabClose(id);
        D_800EFC38[id] = 0;
        D_800EFB38[id] = 0;
    }
    if (id != 0) {
        D_800A3404 = D_800EFB38[0];
        for (i = 0; i < 16; i++) {
            if (D_800EFC38[i] != 0) {
                D_800A3404 += D_800EFC38[i][3];
            }
        }
    }
    D_800A3408 = D_800A3404 - D_800A340C;
    vab = vabid;
    if (vab != 0) {
        func_8005C074(vab, arg2);
    }
    hdr[0] += (s32) hdr;
    hdr[1] += (s32) hdr;
    hdr[2] += (s32) hdr;
    id = snd_VabOpen(hdr, vab);
    SsVabTransCompleted(1);
    if (id == -1) {
        printf(D_800158CC, vab);
        return 0;
    }
    D_800EFC38[id] = hdr;
    D_800A3408 += hdr[3];
    D_800A3404 = D_800A340C + D_800A3408;
    D_800EFB38[vab] = SsUtGetVBaddrInSB(vab);
    return hdr[2] - (s32) hdr;
}
