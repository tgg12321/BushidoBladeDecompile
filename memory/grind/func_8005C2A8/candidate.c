/* func_8005C2A8 (text1b.c) - MATCHED, sandbox score 0, full-build SHA1 == oracle (s1, 2026-09-14).
 * Pure C, no FAKE constructs, no sanctioned-family claims.
 * Two load-bearing spellings found this session:
 *   (1) snd_VabOpen declared to return s32 (not s16) so `s16 id = snd_VabOpen(...)`
 *       sinks the sign-extension INTO the assignment (sll/sra write back into id's
 *       pseudo -> $s0), instead of extending at each use into a scratch ($v1).
 *   (2) the tail is the THEN-arm of `if (id != -1) { ...; return ...; }` with the
 *       printf/return-0 arm trailing at the end of the body. The mirror spelling
 *       (`if (id == -1) { printf; return 0; } <tail>`) emits the two blocks in the
 *       opposite physical order (+1 `j` to the shared epilogue, -1 nop).
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
    if (id != -1) {
        D_800EFC38[id] = hdr;
        D_800A3408 += hdr[3];
        D_800A3404 = D_800A340C + D_800A3408;
        D_800EFB38[vab] = SsUtGetVBaddrInSB(vab);
        return hdr[2] - (s32) hdr;
    }
    printf(D_800158CC, vab);
    return 0;
}
