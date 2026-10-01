void func_80030D04(void) {
    Obj80106A78 *p = D_80106A78;
    s32 i = 0;
    do {
        if ((u32)((u16)p->unk_02 - 0x12) < 12u) {
            p->unk_02 = -1;
        }
        p++;
    } while (++i < 12);
}
