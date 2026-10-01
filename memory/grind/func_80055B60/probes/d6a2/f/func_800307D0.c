s32 func_800307D0(PracticeMenuRec *a0) {
    s32 count;
    s32 idx;
    s32 kind;
    s32 id;
    Obj80106A78 *obj;
    s32 i;

    count = a0->unk_330;
    if (count == 0) {
        return -1;
    }
    idx = 0;
    if (count >= 2 && a0->unk_88 != -1) {
        idx = a0->unk_332[0] == a0->unk_14;
    }
    id = a0->unk_332[idx];
    obj = func_80030580(a0, id);
    for (i = idx; i < a0->unk_330 - 1; i++) {
        a0->unk_332[i] = a0->unk_332[i + 1];
    }

    a0->unk_330--;
    kind = obj->unk_02;
    if (kind == 0xE) {
        func_80032854((D_800A36F2[0] ^ 0xE) != 0, 0x2F, (u8 *)&obj->unk_2C, 0);
    } else {
        func_80032854((kind ^ D_800A36F2[0]) != 0, 0x2A, (u8 *)&obj->unk_2C, 0);
    }
    return id;
}
