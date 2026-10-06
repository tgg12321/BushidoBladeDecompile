#include "common.h"
#include "bb2.h"
typedef char c1[sizeof(TexRec) == 8 ? 1 : -1];
typedef char c2[(int)&((TexRec *)0)->v == 6 ? 1 : -1];
typedef char c3[sizeof(Unk1F800000Rec) == 0x400 ? 1 : -1];
typedef char c4[(int)&((Unk1F800000Rec *)0)->unk50 == 0x50 ? 1 : -1];
typedef char c5[(int)&((Unk1F800000Rec *)0)->unk58 == 0x58 ? 1 : -1];
typedef char c6[(int)&((Unk1F800000Rec *)0)->unkA0 == 0xA0 ? 1 : -1];
typedef char c7[(int)&((Unk1F800000Rec *)0)->unkA4 == 0xA4 ? 1 : -1];
typedef char c8[(int)&((Unk1F800000Rec *)0)->unkA8 == 0xA8 ? 1 : -1];
typedef char c9[(int)&((Unk1F800000Rec *)0)->unkB0 == 0xB0 ? 1 : -1];
typedef char c10[(int)&((Unk1F800000Rec *)0)->unkB8 == 0xB8 ? 1 : -1];
int x;
