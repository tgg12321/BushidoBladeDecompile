void func_8007526C(void) {
    SelWork *base;
    s32 i;

    base = SELWORK;
    i = 0;
    do {
        switch ((u8)base->f10.half[i]) {
        case 1:
            base->f08[i] += 0xA;
            base->f0C[i] += 0xA;
            if (base->f0C[i] >= 0xC8) {
                if (((u16)base->f10.half[i] >> 8) == 0) {
                    base->f10.half[i] += 1;
                }
                base->f08[i] = 0;
                base->f0C[i] = 0xC8;
                base->f14.half[i] = base->f18[i];
                base->f3C.half[i] = base->f38[i];
            }
            break;
        case 3:
            base->f0C[i] += 0xA;
            if (base->f0C[i] >= 0xC8) {
                base->f08[i] = 0xC8;
                base->f0C[i] = 0xC8;
                base->f14.half[i] = base->f18[i];
                base->f3C.half[i] = base->f38[i];
                if (((u16)base->f10.half[i] >> 8) == 0) {
                    base->f10.half[i] += 1;
                }
            }
            break;
        case 2:
            base->f0C[i] -= 0xA;
            if (base->f0C[i] <= 0) {
                base->f08[i] = 0;
                base->f0C[i] = 0;
                base->f10.half[i] = 0;
            }
            break;
        case 4:
            base->f08[i] -= 0xA;
            base->f0C[i] -= 0xA;
            if (base->f0C[i] <= 0) {
                base->f08[i] = 0;
                base->f0C[i] = 0;
                base->f10.half[i] = 0;
            }
            break;
        }
        i++;
    } while (i < 2);
}
