void func_80030D04(void) {
    Obj80106A78 *p;
    s32 i;
    for (i = 0, p = D_80106A78; i < 12; i++, p++) {
        if ((u32)((u16)p->unk_02 - 0x12) < 12u) {
            p->unk_02 = -1;
        }
    }
}
