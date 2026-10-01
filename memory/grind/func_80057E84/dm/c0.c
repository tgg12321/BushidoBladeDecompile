s32 func_800571C0(PracticeMenuRec *obj) {
    Vec4_571C0 probe;
    Vec4_571C0 top;
    Vec4_571C0 left;
    Vec4_571C0 right;
    s32 hit[4];
    s16 work[4];
    s32 ret;
    s8 nl;
    /* Ruling 11 (ordinary-c-judge-decidable.md): holds two values -- the count of clear probe steps on
     * the right-hand side, then which side was chosen (0 right, 1 left; per-branch constants, Q20).
     * Proof: pre-slim-2026-10-01:memory/grind/func_800571C0/r11/proof.md */
    s8 temp;
    u8 goL;
    u8 goR;
    s32 ang;
    s32 rad;
    s32 a;
    PracticeMenuRec *p;
    s32 dx;
    s32 dz;
    s32 x;
    s32 z;

    temp = 0;
    nl = 0;
    goR = 1;
    goL = 1;
    ret = 0;
    left.x = obj->unk_B8.vx;
    left.y = obj->unk_B8.vy - 5;
    rad = D_800A387C + 800;
    left.z = obj->unk_B8.vz;
    right = left;
    for (ang = 0x200; ang <= 0x800; ang += 0x200) {
        if (goL) {
            p = obj->unk_00;
            a = p->unk_1D8 + ang;
            goL = 0;
            dx = rad * Judge[a & 0xFFF];
            dz = rad * Judge[(a + 0x400) & 0xFFF];
            x = p->unk_B8.vx + (dx >> 12);
            z = p->unk_B8.vz + (dz >> 12);
            probe.x = x;
            probe.y = obj->unk_B8.vy - 5;
            probe.z = z;
            top.x = x;
            top.y = obj->unk_B8.vy + 5;
            top.z = z;
            if (func_80053614(&probe.x, &top.x, (s32)hit, (s32)work, 0x1F8002B8) != 0) {
                goL = func_80053614(&left.x, &probe.x, (s32)hit, (s32)work, 0x1F8002B8) == 0;
            }
            if (goL) {
                left = probe;
                nl++;
            }
        }
        if (goR) {
            p = obj->unk_00;
            a = p->unk_1D8 - ang;
            goR = 0;
            dx = rad * Judge[a & 0xFFF];
            dz = rad * Judge[(a + 0x400) & 0xFFF];
            x = p->unk_B8.vx + (dx >> 12);
            z = p->unk_B8.vz + (dz >> 12);
            probe.x = x;
            probe.y = obj->unk_B8.vy - 5;
            probe.z = z;
            top.x = x;
            top.y = obj->unk_B8.vy + 5;
            top.z = z;
            if (func_80053614(&probe.x, &top.x, (s32)hit, (s32)work, 0x1F8002B8) != 0) {
                goR = func_80053614(&right.x, &probe.x, (s32)hit, (s32)work, 0x1F8002B8) == 0;
            }
            if (goR) {
                right = probe;
                temp++;
            }
        }
    }
    if (nl != 0 || temp != 0) {
        if (nl == temp) {
            if (rand() & 1) {
                nl = 0;
            } else {
                temp = 0;
            }
        }
        if (nl < temp) {
            nl = temp;
            temp = 0;
        } else {
            temp = 1;
        }
        ret = nl--;
        for (ang = 0x200; nl >= 0; nl--, ang += 0x200) {
            s32 base = obj->unk_00->unk_1D8;
            if (temp != 0) {
                a = base + ang;
            } else {
                a = base - ang;
            }
            obj->cpu_route.node[nl].x = obj->unk_00->unk_B8.vx + ((D_800A387C * Judge[a & 0xFFF]) >> 12);
            obj->cpu_route.node[nl].z = obj->unk_00->unk_B8.vz + ((D_800A387C * Judge[(a + 0x400) & 0xFFF]) >> 12);
            obj->cpu_route.node[nl].kind = 2;
        }
        obj->unk_398 = 0;
        obj->unk_3A0 = obj->cpu_route.node[0].x;
        obj->unk_3A2 = obj->cpu_route.node[0].z;
        obj->unk_39E = obj->cpu_route.node[0].kind;
    }
    return ret;
}
