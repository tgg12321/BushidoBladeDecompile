/* REJECTED s3 (2026-08-11): genuinely-used pointer-array local as the 8-byte
 * frame producer. First honest spelling to reach the target frame size
 * (vars=80, frame 112) — but GCC 2.7.2 never scalarizes local arrays, so the
 * array stays memory-resident: emits sw $4,16($sp) / sw $2,20($sp) /
 * lw $4,16($sp), and callee-save count drops 4/0 -> 3/0 (s2's pointer moves
 * into the frame; the save block lands at the wrong offsets). Not
 * codegen-neutral; can never match the 71-insn target. Measured via
 * tmp/grind/func_8001E6E4/s3/probe_ps_array.py (asm: variant_ps_array.s). */
void func_8001E6E4(s32 arg0) {
    s32 *ps[2];
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
    ps[0] = (s32 *)&local;
    ps[1] = (s32 *)((u8 *)s2 + 0x20);
    func_80046BF4(ps[0], &local.rx, local.dist);
    func_8001A538(ps[0], ps[1]);
    func_80061064((s32 *)&local.rx, ps[1]);

    D_800A36B4 = (s32)s2;
}
