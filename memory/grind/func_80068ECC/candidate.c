/* func_80068ECC — sandbox distance 0 (s1, 2026-07-27). Applied in src/text1b.c. */
void func_80068ECC(s32 arg0) {
    s32 *p = &D_8009BC04;
    s32 v = *p;
    v &= ~0x1; v |= arg0 & 0x1;
    v &= ~0x2; v |= arg0 & 0x2;
    v &= ~0x4; v |= arg0 & 0x4;
    v &= ~0x8; v |= ((u32)arg0 >> 1) & 0x8;
    v &= ~0x10; v |= ((u16)arg0 >> 1) & 0x10;
    v &= ~0x20; v |= ((u8)arg0 >> 1) & 0x20;
    v &= ~0x40; v |= (arg0 << 3) & 0x40;
    v &= ~0x80; v |= arg0 & 0x80;
    *p = v;
}
