/* REJECTED (s1): p->len = 1 through the OTag bitfield emits
 * `sb v0,0(v1)` — our fork computes the byte-store offset for the
 * high-bits len:8 field as byte 0; target (and cc1psx layout) needs
 * `sb v0,3(v1)`. Everything else in this form already matched
 * (score 1). Fix: explicit byte store ((u8*)p)[3] = 1; → score 0.
 */
void func_8003D330(void) {
    OTag *p = (OTag *)((u8 *)&D_800A3D30 + (D_800A3218 << 3));
    OTag *ot;
    p->len = 1; /* <-- sb 0(v1), wrong byte */
    *((u32 *)p + 1) = 0xE100001F;
    ot = (OTag *)D_800A374C;
    p->addr = ot->addr;
    ot->addr = (u32)p;
}
