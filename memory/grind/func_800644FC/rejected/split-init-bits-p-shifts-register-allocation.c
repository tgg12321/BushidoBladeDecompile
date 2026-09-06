/*BEGIN func_800644FC*/
void func_800644FC(s32 *count, MATRIX *m, s32 idx) {
    s32 i;
    MATRIX *ptr;
    s32 *bits_p;
    s32 mul50;
    i = 0;
    if (i < *count) {
        bits_p = D_800A3454;
        bits_p += idx;
        mul50 = idx * 0x50;
        ptr = m;
    top:
        {
            s32 mask = 1 << i;
            if (*bits_p & mask) {
                RotMatrix((SVECTOR *)((u8 *)D_800F1000 + mul50 + (i << 3)), ptr);
            }
        }
        i++;
        ptr++;
        if (i < *count) goto top;
    }
}
/*END func_800644FC*/
