//DECL: struct FileFlagBits { u8 rest : 5; u8 b2 : 1; u8 b1 : 1; u8 b0 : 1; }; extern struct FileFlagBits D_80106A73;
void func_80034F88(void) {
    s32 *p;
    s32 i;

    p = func_80077D00();
    D_80106A73.b0 = 0;
    D_80106A73.b1 = 0;
    D_80106A73.b2 = 0;
    D_80106A73.b0 = (p[8] & 1) ? 1 : D_80106A73.b0;
    D_80106A73.b1 = (p[8] & 2) ? 1 : D_80106A73.b1;
    D_80106A73.b2 = (p[8] & 4) ? 1 : D_80106A73.b2;

    for (i = 0; i < 3; i++) {
        *(&D_80106A70 + i) = *((u8 *)p + 0x17 + i);
    }
}
