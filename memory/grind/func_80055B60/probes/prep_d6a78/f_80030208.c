void func_80030208(void) {
    Obj80106A78 *obj;
    s32 i;
    s16 kind;
    s32 lookup;

    for (obj = D_80106A78, i = 0; i < 12; i++, obj++) {
        kind = obj->unk_02;
        if (kind == -1) {
            continue;
        }
        if (obj->unk_08 != 0) {
            func_800300B4((u8 *)obj);
            continue;
        }
        if (obj->unk_00 < 2) {
            continue;
        }
        lookup = D_8008EB80[kind];
        if ((u16)(kind - 0x12) < 0xC) {
            lookup = obj->unk_0B;
        } else if (kind == 0xE && obj->unk_05 == 2) {
            lookup += 3;
        }
        func_80049718(lookup, 1, &obj->unk_2C, obj->unk_54);
        func_800393C8(obj->unk_0A, lookup, &obj->unk_2C, obj->unk_54);
    }
}
