s32 func_80057ACC(PracticeMenuRec *arg0, NavPolySet *arg1, s32 arg2, s32 arg3) {
    s32 sp28;
    s32 sp2C;
    s32 best;
    s16 i;
    s16 j;
    s16 k;
    s16 n;
    NavPoly *poly;
    s32 dx;
    s32 dy;
    s32 d;

    best = 100000;
    for (i = 0; i < arg1->npolys; i++) {
        poly = &arg1->polys[i];
        n = poly->nvtx;
        if (poly->flags & 0x80) {
            n = poly->nvtx - 1;
        }
        for (j = 0; j < n; j++) {
            k = j + 1;
            if (!(k < poly->nvtx)) {
                k = 0;
            }
            if (func_8005763C(arg0->unk_F4.x, arg0->unk_F4.z, arg2, arg3,
                              poly->vtx[j][0],
                              poly->vtx[j][1],
                              poly->vtx[k][0],
                              poly->vtx[k][1],
                              &sp28, &sp2C) != 0) {
                dx = sp28 - arg0->unk_F4.x;
                dy = sp2C - arg0->unk_F4.z;
                d = SquareRoot0(dx * dx + dy * dy);
                if (d < best) {
                    best = d;
                    arg0->cpu_route.poly = i;
                    arg0->cpu_route.vtx = j;
                }
            }
        }
    }
    return best;
}
