typedef struct {
    Vec3i32 pts[2][3];
    Vec3i32 pts2[2][2];
    u8 pad78[0xA8 - 0x78];
    Vec3i32 limb[2][22];
} ScrPad;
#define SPAD ((ScrPad *)0x1F800000)

void t1(void) {
    Vec3i32 saved[16];
    s32 k;
    for (k = 0; k < 16; k++) {
        saved[k] = ((Vec3i32 *)0x1F800000)[k + 14];
    }
    func_8002DAD0(saved);
}
void t2(void) {
    Vec3i32 saved[16];
    s32 k;
    for (k = 0; k < 16; k++) {
        saved[k] = SPAD->limb[0][k];
    }
    func_8002DAD0(saved);
}
void t3(void) {
    Vec3i32 saved[16];
    s32 k;
    for (k = 0; k < 16; k++) {
        saved[k] = (&SPAD->limb[0][0])[k];
    }
    func_8002DAD0(saved);
}
