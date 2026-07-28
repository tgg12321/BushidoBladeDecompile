/* func_8003D330 (code6cac_c2.c) — SANDBOX DISTANCE 0 (s1, 2026-07-28).
 * PsyQ addPrim/DR_TPAGE shape spelled with the OTag bitfield struct
 * (gpu.h, user-sanctioned field order), matching the matched sibling
 * ot_Link (src/gpu.c). The bitfield insertion/extraction makes GCC
 * materialize + CSE the 0xFFFFFF / 0xFF000000 masks itself, which lands
 * the whole allocation (code $a1, mask_lo $a2, mask_hi $a3, tag $a0)
 * with zero pins. len is stored as an explicit byte (((u8*)p)[3]) —
 * the fork's byte-store offset for the high-bits len:8 bitfield lands
 * on byte 0 instead of 3 (see rejected/bitfield-len-store-wrong-byte.c).
 */
void func_8003D330(void) {
    OTag *p = (OTag *)((u8 *)&D_800A3D30 + (D_800A3218 << 3));
    OTag *ot;
    ((u8 *)p)[3] = 1;
    *((u32 *)p + 1) = 0xE100001F;
    ot = (OTag *)D_800A374C;
    p->addr = ot->addr;
    ot->addr = (u32)p;
}
