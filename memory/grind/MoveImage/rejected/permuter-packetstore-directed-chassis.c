typedef signed int s32;
typedef signed short s16;
typedef unsigned int u32;
typedef unsigned char u8;

extern s32 g_gpu_dev_table;
extern void func_8007B3A8(u8 *, s16 *);
extern u8 D_80015F74;
extern s32 D_8009BF24;
extern s32 D_8009BF28;
extern s32 D_8009BF2C;

s32 MoveImage(s32 *arg0, s16 arg1, s16 arg2) {
    s32 *p;
    s32 *q;
    s32 (*fn)();
    s32 packed;
    s32 *bf24;
    s32 *rect;
    s32 src;
    s32 pkt;

    func_8007B3A8(&D_80015F74, (s16 *)arg0);
    if (((s16 *)arg0)[2] == 0 || ((s16 *)arg0)[3] == 0) {
        return -1;
    }
    PERM_LINESWAP(
    packed = ((s32)arg2 << 16) | ((u32)arg1 & 0xFFFF);
    PERM_GENERAL(bf24 = &D_8009BF24;, bf24 = (s32 *)&D_8009BF24;, bf24 = &D_8009BF28 - 1;)
    rect = arg0;
    src = *rect++;
    D_8009BF28 = packed;
    PERM_GENERAL(*bf24 = src;, bf24[0] = src;, D_8009BF24 = src;)
    D_8009BF2C = *rect;
    p = (s32 *)g_gpu_dev_table;
    fn = (s32 (*)())p[2];
    q = p + 6;
    PERM_GENERAL(pkt = (s32)bf24 - 8;, pkt = (s32)&D_8009BF24 - 8;, pkt = (s32)(bf24 - 2);, pkt = (s32)(&D_8009BF24 - 2);)
    )
    return fn(*q, pkt, 0x14, 0);
}
