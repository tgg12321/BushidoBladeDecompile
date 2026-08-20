void _spu_gcSPU(void) {
    s32 i;
    s32 j;
    Entry *base;
    Entry *cur;
    Entry *p;
    Entry *outer;
    Entry *inner;
    Entry *e;
    s32 ival;
    s32 ov0;
    s32 in1;
    s32 ou1;
    s32 v;
    s32 cnt;
    s32 icnt;

    /* Phase 1: merge each flagged entry with the next matching flagged one */
    if (D_800A2D3C >= 0) {
        i = 0;
        base = (Entry *)D_800A2D40;
        cur = base;
        do {
            if (cur->w0 & 0x80000000) {
                j = i + 1;
                p = base + j;
            p1_scan:
                if (p->w0 == 0x2FFFFFFF) {
                    p++;
                    j++;
                    goto p1_scan;
                }
                p = base + j;
                if (p->w0 & 0x80000000) {
                    if ((p->w0 & 0x0FFFFFFF) == ((cur->w0 & 0x0FFFFFFF) + cur->w1)) {
                        p->w0 = 0x2FFFFFFF;
                        cur->w1 = cur->w1 + p->w1;
                        goto p1_cont;
                    }
                }
            }
            cur++;
            i++;
        p1_cont:
            ;
        } while (D_800A2D3C >= i);
    }

    /* Phase 2: mark zero-value entries as sentinels */
    cnt = D_800A2D3C;
    if (cnt >= 0) {
        i = 0;
        p = (Entry *)D_800A2D40;
        do {
            if (p->w1 == 0) {
                p->w0 = 0x2FFFFFFF;
            }
            i++;
            p++;
        } while (cnt >= i);
    }

    /* Phase 3: selection sort by (w0 & 0x0FFFFFFF), skipping flagged */
    cnt = D_800A2D3C;
    if (cnt >= 0) {
        i = 0;
        base = (Entry *)D_800A2D40;
        outer = base;
        do {
            if (outer->w0 & 0x40000000) {
                goto p3_done;
            }
            j = i + 1;
            if (cnt >= j) {
                icnt = D_800A2D3C;
                inner = base + j;
                do {
                    ival = inner->w0;
                    if (ival & 0x40000000) {
                        goto p3_next;
                    }
                    ov0 = outer->w0;
                    if ((u32)(ival & 0x0FFFFFFF) < (u32)(ov0 & 0x0FFFFFFF)) {
                        outer->w0 = ival;
                        in1 = inner->w1;
                        ou1 = outer->w1;
                        outer->w1 = in1;
                        inner->w0 = ov0;
                        inner->w1 = ou1;
                    }
                    j++;
                    inner++;
                } while (icnt >= j);
            }
        p3_next:
            i++;
            outer++;
        } while (D_800A2D3C >= i);
    }
p3_done:
    ;

    /* Phase 4: compact - replace first sentinel with base[count], shrink */
    cnt = D_800A2D3C;
    if (cnt >= 0) {
        i = 0;
        base = (Entry *)D_800A2D40;
        p = base;
        do {
            if (p->w0 & 0x40000000) {
                goto p4_done;
            }
            if (p->w0 == 0x2FFFFFFF) {
                e = base + cnt;
                p->w0 = e->w0;
                D_800A2D3C = i;
                p->w1 = e->w1;
                goto p4_done;
            }
            i++;
            p++;
            cnt = D_800A2D3C;
        } while (cnt >= i);
    }
p4_done:
    ;

    /* Phase 5: finalize backward - re-flag entries, fold in counts */
    i = D_800A2D3C - 1;
    if (i >= 0) {
        base = (Entry *)D_800A2D40;
        p = base + i;
        do {
            v = p->w0;
            if (!(v & 0x80000000)) {
                goto p5_done;
            }
            cnt = D_800A2D3C;
            p->w0 = (v & 0x0FFFFFFF) | 0x40000000;
            D_800A2D3C = i;
            p->w1 = p->w1 + base[cnt].w1;
            i--;
            p--;
        } while (i >= 0);
    }
p5_done:
    ;
}
