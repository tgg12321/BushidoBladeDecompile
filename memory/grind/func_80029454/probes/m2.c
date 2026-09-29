void h1(void) {
    Vec3i32 *ws = (Vec3i32 *)0x1F8000A8;
    s32 i, j;
    s32 *p;
    for (i = 0; i < 2; i++) {
        for (j = 0; j < 4; j++) {
            p = (s32 *)((u8 *)ws + i * 0x60 + j * 0x18);
            p[0] >>= 1; p[1] >>= 1; p[2] >>= 1; p[3] >>= 1; p[4] >>= 1; p[5] >>= 1;
        }
    }
}
void h2(void) {
    s32 (*ws)[4][6] = (s32 (*)[4][6])0x1F8000A8;
    s32 i, j;
    for (i = 0; i < 2; i++) {
        for (j = 0; j < 4; j++) {
            ws[i][j][0] >>= 1; ws[i][j][1] >>= 1; ws[i][j][2] >>= 1;
            ws[i][j][3] >>= 1; ws[i][j][4] >>= 1; ws[i][j][5] >>= 1;
        }
    }
}
void h3(void) {
    Vec3i32 *ws = (Vec3i32 *)0x1F8000A8;
    s32 i, j;
    for (i = 0; i < 2; i++) {
        for (j = 0; j < 4; j++) {
            Vec3i32 *v = &ws[i * 8 + j * 2];
            v[0].x >>= 1; v[0].y >>= 1; v[0].z >>= 1;
            v[1].x >>= 1; v[1].y >>= 1; v[1].z >>= 1;
        }
    }
}
void h4(void) {
    Vec3i32 *ws = (Vec3i32 *)0x1F8000A8;
    s32 i, j;
    for (i = 0; i < 2; i++) {
        for (j = 0; j < 4; j++) {
            ws[i * 8 + j * 2].x >>= 1; ws[i * 8 + j * 2].y >>= 1; ws[i * 8 + j * 2].z >>= 1;
            ws[i * 8 + j * 2 + 1].x >>= 1; ws[i * 8 + j * 2 + 1].y >>= 1; ws[i * 8 + j * 2 + 1].z >>= 1;
        }
    }
}
