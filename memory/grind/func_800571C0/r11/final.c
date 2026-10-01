s32 func_800571C0(s32 obj) {
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
     * Proof: memory/grind/func_800571C0/r11/proof.md */
    s8 temp;
    u8 goL;
    u8 goR;
    s32 ang;
    s32 rad;
    s32 a;
    s32 p;
    s32 dx;
    s32 dz;
    s32 x;
    s32 e;
    s32 z;

    temp = 0;
    nl = 0;
    goR = 1;
    goL = 1;
    ret = 0;
    left.x = *(s32 *)(obj + 0xB8);
    left.y = *(s32 *)(obj + 0xBC) - 5;
    rad = D_800A387C + 800;
    left.z = *(s32 *)(obj + 0xC0);
    right = left;
    for (ang = 0x200; ang <= 0x800; ang += 0x200) {
        if (goL) {
            p = *(s32 *)obj;
            a = *(s16 *)(p + 0x1D8) + ang;
            goL = 0;
            dx = rad * Judge[a & 0xFFF];
            dz = rad * Judge[(a + 0x400) & 0xFFF];
            x = *(s32 *)(p + 0xB8) + (dx >> 12);
            z = *(s32 *)(p + 0xC0) + (dz >> 12);
            probe.x = x;
            probe.y = *(s32 *)(obj + 0xBC) - 5;
            probe.z = z;
            top.x = x;
            top.y = *(s32 *)(obj + 0xBC) + 5;
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
            p = *(s32 *)obj;
            a = *(s16 *)(p + 0x1D8) - ang;
            goR = 0;
            dx = rad * Judge[a & 0xFFF];
            dz = rad * Judge[(a + 0x400) & 0xFFF];
            x = *(s32 *)(p + 0xB8) + (dx >> 12);
            z = *(s32 *)(p + 0xC0) + (dz >> 12);
            probe.x = x;
            probe.y = *(s32 *)(obj + 0xBC) - 5;
            probe.z = z;
            top.x = x;
            top.y = *(s32 *)(obj + 0xBC) + 5;
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
            s32 base = *(s16 *)(*(s32 *)obj + 0x1D8);
            if (temp != 0) {
                a = base + ang;
            } else {
                a = base - ang;
            }
            e = obj + nl * 6;
            *(s16 *)(e + 0x364) = *(s32 *)(*(s32 *)obj + 0xB8) + ((D_800A387C * Judge[a & 0xFFF]) >> 12);
            *(s16 *)(e + 0x366) = *(s32 *)(*(s32 *)obj + 0xC0) + ((D_800A387C * Judge[(a + 0x400) & 0xFFF]) >> 12);
            *(u8 *)(e + 0x368) = 2;
        }
        *(s16 *)(obj + 0x398) = 0;
        *(s16 *)(obj + 0x3A0) = *(s16 *)(obj + 0x364);
        *(s16 *)(obj + 0x3A2) = *(s16 *)(obj + 0x366);
        *(s16 *)(obj + 0x39E) = *(u8 *)(obj + 0x368);
    }
    return ret;
}
