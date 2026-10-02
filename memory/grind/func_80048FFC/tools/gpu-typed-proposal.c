/* Copy packet: tag followed by five GPU command words. */
typedef struct { OTag tag; u32 code[5]; } DR_MOVE;
void SetDrawMove(DR_MOVE *a0, RECT *a1, u32 a2, u32 a3) {
    s32 size = 5;
    if (a1->w == 0) {
        size = 0;
    } else if (a1->h == 0) {
        size = 0;
    }
    a0->code[0] = OT_TERMINATOR;
    a0->code[1] = OT_TAG_BASE;
    a0->tag.len = size;
    /* SOTN: src/main/psxsdk/libgpu/sys.c:276 @db41b28eee52969244a52cc269c8163d1ed8826a */
    a0->code[2] = *(s32 *)&a1->x;
    a0->code[3] = (a3 << 16) | (a2 & 0xFFFF);
    /* SOTN: src/main/psxsdk/libgpu/sys.c:278 @db41b28eee52969244a52cc269c8163d1ed8826a */
    a0->code[4] = *(s32 *)&a1->w;
}
