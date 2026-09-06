/*BEGIN func_800644FC*/
void func_800644FC(s32 *count, MATRIX *m, s32 idx) {
    s32 i;
    for (i = 0; i < *count; i++, m++) {
        if (D_800A3454[idx] & (1 << i)) {
            RotMatrix(&D_800F1000[idx][i], m);
        }
    }
}
/*END func_800644FC*/
