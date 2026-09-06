
typedef unsigned char u8;
typedef signed char s8;
typedef unsigned short u16;
typedef signed short s16;
typedef unsigned int u32;
typedef signed int s32;
typedef unsigned long long u64;
typedef signed long long s64;
typedef volatile u8 vu8;
typedef volatile s8 vs8;
typedef volatile u16 vu16;
typedef volatile s16 vs16;
typedef volatile u32 vu32;
typedef volatile s32 vs32;
typedef struct 
{
  s16 unk0;
  s16 unk2;
  s16 unk4;
  s16 unk6;
  s32 unk8;
  s32 unkC;
  s32 unk10;
  s32 unk14;
} Unk800A9CF8Header;
extern Unk800A9CF8Header D_800A9CF8;
extern void *game_GetCharData(void);
typedef struct 
{
  s8 unk0;
  s8 unk1;
  s16 unk2;
  u16 unk4;
  s16 unk6;
  s16 unk8;
  s16 unkA;
  s32 unkC;
  s16 unk10;
  s16 unk12;
  s16 unk14;
  s16 unk16;
  s8 pad18[0x14];
  s32 unk2C;
  s32 unk30;
  s32 unk34;
  s8 pad38[0x14];
  s32 unk4C;
  s32 unk50;
  s32 unk54;
  s32 unk58;
  s32 unk5C;
  s8 pad60[8];
} Rec4473C;
void func_8004473C(void)
{
  Rec4473C *src;
  Rec4473C *dst;
  s32 i;
  D_800A9CF8.unk10 = (s32) game_GetCharData();
  src = (Rec4473C *) ((double) D_800A9CF8.unk10);
  dst = (Rec4473C *) D_800A9CF8.unkC;
  for (i = 0; i < D_800A9CF8.unk6; i++, dst++, src++)
  {
    dst->unk0 = 0;
    dst->unk1 = 0;
    dst->unk2 = 0;
    dst->unk4 = D_800A9CF8.unk0;
    dst->unk6 = 0;
    dst->unk8 = 0;
    dst->unkA = 4;
    dst->unkC = 0;
    dst->unk10 = 0;
    dst->unk12 = 0;
    dst->unk14 = 0;
    dst->unk4C = src->unk2C;
    dst->unk50 = src->unk30;
    dst->unk54 = src->unk34;
    dst->unk58 = -1;
  }

}
