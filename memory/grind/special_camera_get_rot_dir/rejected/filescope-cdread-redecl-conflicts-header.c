/* s7 (2026-08-26, structural). File-scope `extern s32 CdRead(s32, s32, s32);` placed in
 * src/code6cac_b2_post.c immediately after #include "code6cac.h", instead of the
 * block-scope spelling that layer-1 FAILed on 2026-08-26 01:42.
 *
 * MEASURED: it compiles under GCC 2.7.2 and scores 0 (72/72) -- i.e. it is a
 * BYTE-VALID, IN-SCOPE form. It is banked as REJECTED anyway, on semantics:
 * it is a conflicting redeclaration of a symbol the very header included two
 * lines above declares as `extern void CdRead(s32);` (include/code6cac.h:510).
 * No human programmer writes a contradicting prototype directly beneath the
 * header that supplies the wrong one -- they fix the header. It is therefore
 * the SAME scope-gate workaround the 01:42 layer-1 review already FAILed,
 * relocated one scope level outward, and re-proposing it would be respelling
 * a FAILed construct (T2 human-programmer FAIL).
 *
 * Its real value is as EVIDENCE: taken together with rejected/
 * unread-params-arity-inert (v2, also 0/72), it proves the CdRead arity
 * contributes ZERO bytes. The prototype question is purely one of source
 * fidelity, which is exactly why it belongs in an owner decision packet and
 * not in a coercion search.
 */
extern s32 CdRead(s32, s32, s32);
s32 func_800372F4(s32 nbytes, s32 buf, s32 mode) {
    s32 v = nbytes;
    nbytes += 0x7FF;
    if (nbytes < 0) {
        nbytes = v + 0xFFE;
    }
    CdRead(nbytes >> 11, buf, mode);
    do {
        v = CdReadSync(1, 0);
        if (v > 0) {
            VSync(0);
        }
    } while (v > 0);
    return v;
}
