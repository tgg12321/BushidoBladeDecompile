
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
extern s32 game_GetPlayerData(s32);
extern void MulMatrix0(s32 *, s32 *, s32 *);
extern void func_8002F2D0(s32 *a0, s32 *a1);
extern void func_80049718(s32, s32, s32 *, s32 *);
extern void func_800393C8(s32, s32, s32 *, s32 *);
extern u8 D_8008EB80;
void func_800300B4(u8 *arg0)
{
  s32 mac[3];
  unsigned long new_var;
  s32 dir[2];
  s32 mtx[8];
  s32 *playerData;
  s32 *mat;
  s32 lookup;
  s32 *lv;
  u32 packed;
  playerData = (s32 *) game_GetPlayerData(arg0[6] < 1);
  mat = (s32 *) playerData[arg0[9]];
  asm volatile("move   $12, %0\nlw     $13, 0($12)\nlw     $14, 4($12)\nctc2   $13, $0\nctc2   $14, $1\nlw     $13, 8($12)\nlw     $14, 12($12)\nlw     $15, 16($12)\nctc2   $13, $2\nctc2   $14, $3\nctc2   $15, $4\n" : : "r"(mat) : "$12", "$13", "$14", "$15");
  lv = arg0 + 0x2C;
  packed = (*((u16 *) lv)) | ((*((u16 *) (arg0 + 0x30))) << 16);
  lv = (s32 *) lv;
  asm volatile("move   $12, %0\nmtc2   %1, $0\nlwc2   $1, 8($12)\nnop\nnop\n.word  0x4A486012\n" : : "r"(lv), "r"(packed) : "$12");
  asm volatile("move   $12, %0\nswc2   $25, 0($12)\nswc2   $26, 4($12)\nswc2   $27, 8($12)\n" : : "r"(mac) : "$12", "memory");
  mac[0] += mat[5];
  mac[1] += mat[6];
  mac[2] += mat[7];
  MulMatrix0(mat, (s32 *) (arg0 + 0xC), mtx);
  func_8002F2D0(mtx, dir);
  lookup = (&D_8008EB80)[*((s16 *) (arg0 + 2))];
  func_80049718(lookup, 1, mac, dir);
  func_800393C8(new_var = arg0[10], lookup, mac, dir);
}
