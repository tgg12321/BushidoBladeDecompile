/* REJECTED (s1, 2026-09-15): distance 75: `(1 << i)` written twice (test + `bits | (1 << i)`); each shift pseudo is single-use so combine folds the test into a zero_extract (srav/andi) -- target keeps sllv+and. Also loop.c hoisted the D_800A344C[idx] count load (target reloads it every iteration) because `bits = D_800A3454[idx]` outside the loop left no in-loop store on the count alias class. */
extern u32 D_800A344C[];
extern s32 D_800A3454[];
extern s32 D_800A3478;
extern s32 D_800A347C;
extern SVECTOR D_800F1000[][10];
extern s32 D_800F0EC8[][10][3];
u8 func_80063BD0(s32 idx) {
    s32 bits;
    s32 i;
    s32 *src;

    if (D_800A344C[idx] < 10) {
        D_800A344C[idx]++;
        bits = D_800A3454[idx];
        for (i = 0; i < D_800A344C[idx]; i++) {
            if (!(bits & (1 << i))) {
                D_800A3454[idx] = bits | (1 << i);
                D_800F1000[idx][i].vy = ((u16 *)D_800A3478)[1];
                D_800F1000[idx][i].vx = D_800F1000[idx][i].vz = 0;
                src = (s32 *)D_800A347C;
                D_800F0EC8[idx][i][0] = src[0];
                D_800F0EC8[idx][i][1] = src[1];
                D_800F0EC8[idx][i][2] = src[2];
                return 1;
            }
        }
        return 1;
    } else {
        D_800A344C[idx] = (D_800A344C[idx] + 1) % 10 + 10;
        D_800F1000[idx][D_800A344C[idx] - 10].vy = ((u16 *)D_800A3478)[1];
        D_800F1000[idx][D_800A344C[idx] - 10].vx = D_800F1000[idx][D_800A344C[idx] - 10].vz = 0;
        src = (s32 *)D_800A347C;
        D_800F0EC8[idx][D_800A344C[idx] - 10][0] = src[0];
        D_800F0EC8[idx][D_800A344C[idx] - 10][1] = src[1];
        D_800F0EC8[idx][D_800A344C[idx] - 10][2] = src[2];
        return 1;
    }
}
