Obj80106A78 *func_80030580(PracticeMenuRec *arg0, s32 arg1) {
    /* FAKE: unwritten leading pad (phantom-frame-slot volatile pad local family, owner ruling 2026-08-18; row granted by owner ruling 2026-09-02, docs/grind/decisions.md "foreclosed-bucket disposition"): reserves the 16 untouched locals bytes the target frame holds beyond our single combine-orphan slot (target vars=24, ours 8; zero ($sp) references in asm/funcs/func_80030580.s). Mechanism: reload alter_reg / get_frame_size counts the never-accessed volatile object and emits no instruction. Lever exhaustion: pre-slim-2026-10-01:memory/grind/func_80030580/hypotheses.md (s1-s9, 44 structural respellings + 31 frame-producer shapes, all measured inert). SOTN-master precedent: volatile u32 pad[4]; // FAKE at st/sel/stream.c:80. */
    volatile u32 pre_pad[4]; /* !FAKE */
    Obj80106A78 *obj;
    Tbl8008E194 *tbl;
    s32 i;

    obj = D_80106A78;
    for (i = 0; i < 12; i++, obj++) {
        if (obj->unk_02 == -1 && obj->unk_0A == 0xFF) break;
    }
    obj->unk_0A = i;
    obj->unk_02 = arg1;
    obj->unk_07 = 0;
    obj->unk_08 = 0;
    obj->unk_04 = 1;
    obj->unk_06 = arg0->unk_04;
    obj->unk_2C.x = arg0->unk_F4.x;
    obj->unk_2C.y = arg0->unk_F4.y - arg0->unk_1A / 32;
    obj->unk_2C.z = arg0->unk_F4.z;
    tbl = &D_8008E194[arg1];
    obj->unk_44.x = (Judge[arg0->unk_1C8.vy & 0xFFF] * tbl->unk4) >> 12;
    obj->unk_44.y = tbl->unk6;
    obj->unk_44.z = (Judge[(arg0->unk_1C8.vy + 0x400) & 0xFFF] * tbl->unk4) >> 12;
    obj->unk_2C.x += obj->unk_44.x;
    obj->unk_2C.y += obj->unk_44.y;
    obj->unk_2C.z += obj->unk_44.z;
    obj->unk_2C.x += obj->unk_44.x / 2;
    obj->unk_2C.y += obj->unk_44.y / 2;
    obj->unk_2C.z += obj->unk_44.z / 2;
    obj->unk_38 = obj->unk_2C;
    obj->unk_54[0] = 0;
    obj->unk_54[1] = arg0->unk_1C8.vy;
    obj->unk_54[2] = 0;
    if (tbl->unk0 == 1) {
        obj->unk_5C[0] = 0;
        obj->unk_5C[1] = tbl->unk8;
        obj->unk_5C[2] = 0;
    } else if (tbl->unk0 == 2) {
        obj->unk_5C[0] = tbl->unk8;
        obj->unk_5C[1] = 0;
        obj->unk_5C[2] = 0;
    } else if (tbl->unk0 == 3) {
        obj->unk_5C[0] = 0;
        obj->unk_5C[1] = tbl->unk8;
        obj->unk_5C[2] = 0;
    } else {
        obj->unk_5C[0] = 0;
        obj->unk_5C[1] = 0;
        obj->unk_5C[2] = 0;
    }
    obj->unk_50 = 1;
    obj->unk_05 = 0;
    obj->unk_00 = 0;
    return obj;
}
