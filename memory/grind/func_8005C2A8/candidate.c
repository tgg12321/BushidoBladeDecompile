/* func_8005C2A8 (text1b.c) - MATCHED: sandbox --disable all = 0 and full-build
 * SHA1 == oracle (s2/recon, 2026-09-15). Ordinary C; no FAKE, no volatile, no
 * asm, no pin, no dead store, no pad, no alias local, no sanctioned-family
 * exception claimed or needed.
 *
 * This body supersedes the 2026-09-14 form that layer-1 FAILed. Both banned
 * constructs are GONE and neither is respelled:
 *   - the second local bound to the unmodified parameter is deleted; every use
 *     site reads the parameter directly (measured: still score 0).
 *   - the forward prototype no longer contradicts anything: the in-TU callee's
 *     DEFINITION (src/text1b.c, the VAB-open wrapper at 0x8005C5A8) is changed
 *     in the same diff from `s16` to `s32` return, keeping its body's explicit
 *     `(s16)` cast on the SsVabTransBody result. That callee's own bytes are
 *     unchanged (measured: sandbox snd_VabOpen --disable all = 0 before and
 *     after), because the sll/sra at 0x8005C5F4 is emitted by the cast in its
 *     body, not by its return type. The return type is therefore not decidable
 *     from that function's own bytes; it IS decidable from this call site's
 *     bytes, and they say s32. Prototype and definition agree.
 *
 * Apply: replace `INCLUDE_ASM("asm/funcs", func_8005C2A8);` (src/text1b.c:2662)
 * with everything below, AND change the callee definition's return type as
 * described above.
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

    if ((func_80077D00()[5] & 0xF) == 3 && vabid == 5) {
        return 0;
    }
    func_800858D0(0);
    if (D_800EFC38[vabid] != 0) {
        SsVabClose(vabid);
        D_800EFC38[vabid] = 0;
        D_800EFB38[vabid] = 0;
    }
    if (vabid != 0) {
        D_800A3404 = D_800EFB38[0];
        for (i = 0; i < 16; i++) {
            if (D_800EFC38[i] != 0) {
                D_800A3404 += D_800EFC38[i][3];
            }
        }
    }
    D_800A3408 = D_800A3404 - D_800A340C;
    if (vabid != 0) {
        func_8005C074(vabid, arg2);
    }
    hdr[0] += (s32) hdr;
    hdr[1] += (s32) hdr;
    hdr[2] += (s32) hdr;
    id = snd_VabOpen(hdr, vabid);
    SsVabTransCompleted(1);
    if (id != -1) {
        D_800EFC38[id] = hdr;
        D_800A3408 += hdr[3];
        D_800A3404 = D_800A340C + D_800A3408;
        D_800EFB38[vabid] = SsUtGetVBaddrInSB(vabid);
        return hdr[2] - (s32) hdr;
    }
    printf(D_800158CC, vabid);
    return 0;
}
