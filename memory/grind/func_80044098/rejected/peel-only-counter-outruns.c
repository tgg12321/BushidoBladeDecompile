/* s2 KILL — peel WITHOUT the hdr split cannot flip: the peel lifts counter
 * refs proportionally (counter 17@22 pri 30909 > pointer 16@23 pri 27826).
 * The hdr load/test split is REQUIRED alongside the peel (it strips 3 refs
 * off the counter: 14@18 pri 23333 < 27826 -> flip). NB the hdr split ALONE
 * is an ANTI-lever: it shortens counter livelen 16->12 raising counter pri
 * to 27500 (corrects the s1 ledger's ~24400 estimate) while staying
 * byte-identical to base. */
void func_80044098(s16 a0) {
    s32 *v1;
    s32 a4;
    s32 *a6;

    v1 = D_80103608[a0];
    a4 = *(v1 - 1);
    a6 = v1 - 1;
    if (a4 & 0x8000) {
        a4 = a4 & 0x7FFF;
        *(v1 - 1) = a4;
        a4 = a4 - 1;
        if (a4 != -1) {
            *v1 -= (s32)a6;
            v1++;
            a4--;
            while (a4 != -1) {
                *v1 -= (s32)a6;
                v1++;
                a4--;
            }
        }
    }
}
