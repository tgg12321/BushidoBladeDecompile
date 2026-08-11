/* BEST ADAPTED BODY - src/main.c - _spu_gcSPU (LIBSPU/S_M_INT)
 * Reference: sotn-decomp src/main/psxsdk/libspu/s_m_m.c:88-153 - transplanted
 * essentially verbatim; only the symbols change:
 *   _spu_memList[]  ->  ((Entry *)D_800A2D40)[]   (BB2 holds the list via a pointer)
 *   D_80033560      ->  D_800A2D3C                (the high-water index)
 *   .addr/.size     ->  Entry.w0 / Entry.w1       (BB2's existing typedef, main.c:2040)
 *
 * MEASURED: 57   (baseline 121, with 104 regfix rules).  build 187 / target 194.
 * The reference more than halves the honest distance and would retire 104 rules.
 *
 * RESIDUAL: 7 insns short. BB2's previous body was a pointer-walking rewrite (cur++,
 * p = base + j); the reference's index form (ml[i], ml[j]) is closer, but the target
 * re-derives the ml[i]/ml[j] element addresses more often than GCC does from the
 * indexed form. This is the same CSE-of-a-recomputed-base residual seen in
 * func_8007D6D8, not an algorithm difference - the algorithm is confirmed identical.
 */

void _spu_gcSPU(void) {
    s32 i, j;
    Entry *ml = (Entry *)D_800A2D40;

    for (i = 0; i <= D_800A2D3C;) {
        if (ml[i].w0 & 0x80000000) {
            for (j = i + 1;; j++) {
                if (ml[j].w0 != 0x2FFFFFFF) {
                    break;
                }
            }
            if ((ml[j].w0 & 0x80000000) &&
                ((ml[j].w0 & 0x0FFFFFFF) ==
                 ((ml[i].w0 & 0x0FFFFFFF) + ml[i].w1))) {
                ml[j].w0 = 0x2FFFFFFF;
                ml[i].w1 += ml[j].w1;
                continue;
            }
        }
        i++;
    }
    for (i = 0; i <= D_800A2D3C; i++) {
        if (ml[i].w1 == 0) {
            ml[i].w0 = 0x2FFFFFFF;
        }
    }
    for (i = 0; i <= D_800A2D3C; i++) {
        if (ml[i].w0 & 0x40000000) {
            break;
        }
        for (j = i + 1; j <= D_800A2D3C; j++) {
            if (ml[j].w0 & 0x40000000) {
                break;
            }
            if ((ml[j].w0 & 0x0FFFFFFF) < (ml[i].w0 & 0x0FFFFFFF)) {
                s32 swapAddr = ml[i].w0;
                s32 swapSize = ml[i].w1;
                ml[i].w0 = ml[j].w0;
                ml[i].w1 = ml[j].w1;
                ml[j].w0 = swapAddr;
                ml[j].w1 = swapSize;
            }
        }
    }
    for (i = 0; i <= D_800A2D3C; i++) {
        if (ml[i].w0 & 0x40000000) {
            break;
        }
        if (ml[i].w0 == 0x2FFFFFFF) {
            ml[i].w0 = ml[D_800A2D3C].w0;
            ml[i].w1 = ml[D_800A2D3C].w1;
            D_800A2D3C = i;
            break;
        }
    }
    for (i = D_800A2D3C - 1; i >= 0; i--) {
        if (!(ml[i].w0 & 0x80000000)) {
            break;
        }
        ml[i].w0 &= 0x0FFFFFFF;
        ml[i].w0 |= 0x40000000;
        ml[i].w1 += ml[D_800A2D3C].w1;
        D_800A2D3C = i;
    }
}
