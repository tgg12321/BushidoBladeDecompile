/* REJECTED (s1, 2026-09-15): distance 4: for-loop, `goto found` with the arm placed after the loop (no label between the loop head and the exit test). cse.c cse_around_loop then scans the exit test too, and cse_set_around_loop emits the `r96 = r75` base copy right after the `la D_800A344C` in block 0 (cse.c:7909-7960), so the base dies in block 0, is local-allocated to t2 and the copy coalesces away -- target keeps the base in a0 and copies to t2 in the preheader. The arm INSIDE the loop with `break` (final form) supplies the skip label that stops the scan. */
extern u32 D_800A344C[];
extern s32 D_800A3454[];
extern s32 D_800A3478;
extern s32 D_800A347C;
extern SVECTOR D_800F1000[][10];
extern s32 D_800F0EC8[][10][3];
u8 func_80063BD0(s32 idx) {
    s32 mask;
    s32 i;

    if (D_800A344C[idx] < 10) {
        D_800A344C[idx]++;
        for (i = 0; i < D_800A344C[idx]; i++) {
            if (!(D_800A3454[idx] & (mask = 1 << i))) goto found;
        }
        return 1;
    found:
        D_800A3454[idx] |= mask;
        D_800F1000[idx][i].vy = ((u16 *)D_800A3478)[1];
        D_800F1000[idx][i].vx = D_800F1000[idx][i].vz = 0;
        D_800F0EC8[idx][i][0] = ((s32 *)D_800A347C)[0];
        D_800F0EC8[idx][i][1] = ((s32 *)D_800A347C)[1];
        D_800F0EC8[idx][i][2] = ((s32 *)D_800A347C)[2];
    } else {
        D_800A344C[idx] = (D_800A344C[idx] + 1) % 10 + 10;
        D_800F1000[idx][D_800A344C[idx] - 10].vy = ((u16 *)D_800A3478)[1];
        D_800F1000[idx][D_800A344C[idx] - 10].vx = D_800F1000[idx][D_800A344C[idx] - 10].vz = 0;
        D_800F0EC8[idx][D_800A344C[idx] - 10][0] = ((s32 *)D_800A347C)[0];
        D_800F0EC8[idx][D_800A344C[idx] - 10][1] = ((s32 *)D_800A347C)[1];
        D_800F0EC8[idx][D_800A344C[idx] - 10][2] = ((s32 *)D_800A347C)[2];
    }
    return 1;
}
