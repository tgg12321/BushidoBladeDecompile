void func_80030900(PracticeMenuRec *a0, Vec3i32 *a1) {
    Obj80106A78 *p;
    s32 rnd;
    s32 i;

    p = func_80030580(a0, a0->unk_332[0]);
    p->unk_04 = 0;
    p->unk_2C = *a1;
    p->unk_44.x = (rng_Next() & 0xFF) - 0x80;
    p->unk_44.y = -(rng_Next() & 0x3F) - 0x80;
    p->unk_44.z = (rng_Next() & 0xFF) - 0x80;
    rnd = rng_Next();
    if (rnd & 0x1000) {
        p->unk_5C[0] = (rnd & 0x3FF) + 0x200;
    } else {
        p->unk_5C[0] = -(rnd & 0x3FF) - 0x200;
    }
    p->unk_5C[1] = (rng_Next() & 0x7FF) - 0x400;
    p->unk_5C[2] = 0;
    p->unk_07 = 1;
    for (i = 0; i < a0->unk_330 - 1; i++) {
        a0->unk_332[i] = a0->unk_332[i + 1];
    }
    a0->unk_330--;
}
