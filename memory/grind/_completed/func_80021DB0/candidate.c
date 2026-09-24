typedef struct { s32 x, y, z; } Vec3_21DB0;

void func_80021DB0(s32 arg0, Vec3_21DB0 *out, s32 *pos) {
    Vec3_21DB0 base;
    Vec3_21DB0 cur;
    Vec3_21DB0 probe;
    Vec3_21DB0 hit;
    s16 nrm[4];
    s16 *stage;
    s32 ofs;
    s32 phase;
    s32 i;
    s32 j;
    s32 angle;
    s32 dx;
    s32 dz;
    s32 best;
    s32 d;

    stage = (s16 *)stage_GetDataPtr();
    phase = rand();
    ofs = rand() & 7;
    base.x = pos[0];
    base.y = pos[1] - 500;
    base.z = pos[2];
    for (i = 0; i < 8; i++) {
        angle = (phase + ((ofs + i * 3) << 9)) & 0xFFF;
        dx = ((&Judge)[(angle + 0x400) & 0xFFF] * 4000) / 4096;
        dz = ((&Judge)[angle] * 4000) / 4096;
        probe.x = base.x + dx;
        probe.y = base.y;
        probe.z = base.z + dz;
        if (func_80053614(&base.x, &probe.x, &hit.x, (s32 *)nrm, 0x1F8002B8) != 0) {
            continue;
        }
        cur = base;
        for (j = 1; j < 41; j++) {
            probe.y = base.y;
            probe.x = base.x + (dx * j) / 40;
            probe.z = base.z + (dz * j) / 40;
            if (func_8005344C(&cur.x, &probe.x, &hit.x, (s32 *)nrm, 0x1F8002B8) != 0) {
                break;
            }
            *out = probe;
            cur = *out;
        }
        probe = *out;
        probe.y += 4000;
        if (func_80053614(&out->x, &probe.x, &hit.x, (s32 *)nrm, 0x1F8002B8) == 0) {
            continue;
        }
        if (nrm[1] != -0x1000) {
            continue;
        }
        cur = *out;
        for (j = 1; j < 41; j++) {
            probe.y = out->y + j * 100;
            if (func_8005344C(&cur.x, &probe.x, &hit.x, (s32 *)nrm, 0x1F8002B8) != 0 && nrm[1] == -0x1000) {
                out->y = hit.y;
                return;
            }
            cur.y = probe.y;
        }
    }
    if (D_800A38DC == 3) {
        j = D_800A38E0;
    } else {
        best = 0x7FFFFFFF;
        stage += D_800A36A4 * 24;
        for (i = 0, j = 0; i < 4; i++) {
            dx = stage[i * 6 + 3] - pos[0];
            dz = stage[i * 6 + 5] - pos[2];
            d = dx * dx + dz * dz;
            if (d < best) {
                best = d;
                j = i;
            }
        }
    }
    stage += j * 6 + 3;
    out->x = stage[0];
    out->y = stage[1];
    out->z = stage[2];
}
