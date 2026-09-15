/* REJECTED (s1, 2026-09-15): distance 58: `if (!(D_800A3454[idx] & (1 << i)))` + `|= 1 << i`. cse does not unify the two shift pseudos, so the test shift is single-use and combine emits srav/andi (3 hits). A `mask` local read twice is required. */
extern u32 D_800A344C[];
extern s32 D_800A3454[];
extern s32 D_800A3478;
extern s32 D_800A347C;
extern SVECTOR D_800F1000[][10];
extern s32 D_800F0EC8[][10][3];
u8 func_80063BD0(s32 idx) {
    s32 i;

    if (D_800A344C[idx] < 10) {
        D_800A344C[idx]++;
        for (i = 0; i < D_800A344C[idx]; i++) {
            if (!(D_800A3454[idx] & (1 << i))) {
                D_800A3454[idx] |= 1 << i;
                D_800F1000[idx][i].vy = ((u16 *)D_800A3478)[1];
                D_800F1000[idx][i].vx = D_800F1000[idx][i].vz = 0;
                D_800F0EC8[idx][i][0] = ((s32 *)D_800A347C)[0];
                D_800F0EC8[idx][i][1] = ((s32 *)D_800A347C)[1];
                D_800F0EC8[idx][i][2] = ((s32 *)D_800A347C)[2];
                return 1;
            }
        }
        return 1;
    } else {
        D_800A344C[idx] = (D_800A344C[idx] + 1) % 10 + 10;
        D_800F1000[idx][D_800A344C[idx] - 10].vy = ((u16 *)D_800A3478)[1];
        D_800F1000[idx][D_800A344C[idx] - 10].vx = D_800F1000[idx][D_800A344C[idx] - 10].vz = 0;
        D_800F0EC8[idx][D_800A344C[idx] - 10][0] = ((s32 *)D_800A347C)[0];
        D_800F0EC8[idx][D_800A344C[idx] - 10][1] = ((s32 *)D_800A347C)[1];
        D_800F0EC8[idx][D_800A344C[idx] - 10][2] = ((s32 *)D_800A347C)[2];
        return 1;
    }
}
