typedef struct {
    s32 vx, vy, vz;
    s32 pad0;
    s16 rx, ry, rz;
    s16 pad1;
    s32 dist;
    s32 pad2[11];
} CamWork;

void func_8001E6E4(s32 arg0) {
    s32 pre_pad[2];
    CamWork local;
    s32 *s2;

    s2 = (s32 *)&D_800F5328;
    if ((u32)(arg0 - 0x555) >= 0x556U) {
        s2 = (s32 *)&D_800F6608;
    }

    local.vx = s2[0] + D_800FF5C8;
    local.vy = s2[1] + D_800FF5CC;
    local.vz = s2[2] + D_800FF5D0;
    local.rx = *(u16 *)((u8 *)s2 + 0x10) + (u16)D_800FF5D8;
    local.ry = *(u16 *)((u8 *)s2 + 0x12) + (u16)D_800FF5DA;
    local.rz = *(u16 *)((u8 *)s2 + 0x14) + (u16)D_800FF5DC;

    local.dist = *(s32 *)((u8 *)s2 + 0x18) + D_800FF5E0;
    {
        s32 *p0 = (s32 *)&local;
        s32 *p20 = (s32 *)((u8 *)s2 + 0x20);
        func_80046BF4(p0, &local.rx, local.dist);
        func_8001A538(p0, p20);
        func_80061064((s32 *)&local.rx, p20);
    }

    D_800A36B4 = (s32)s2;
}
