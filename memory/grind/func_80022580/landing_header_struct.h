typedef struct { s32 x, y, z; } Vec3i32;
typedef struct { s32 vx, vy, vz, pad; } Vec4i32;
typedef struct { s16 vx, vy, vz, pad; } SVec4i16;

typedef struct PracticeMenuRec {
    struct PracticeMenuRec *unk_00; /* the other record: [1] for record 0, else [0] (func_80022580) */
    s16 unk_04;                    /* this record's own index (func_80022580) */
    s16 unk_06;                    /* != 0: func_8001BE20 hands pad input to func_80055B60 */
    s16 unk_08;
    s16 unk_0A;                    /* class idx: row of D_8008E5CC / D_8008E6A4, index of D_8008D9EC */
    s16 unk_0C;
    s16 unk_0E;                    /* column of D_8008E5CC / D_8008E6A4 */
    u8  unk_10[0x12 - 0x10];
    s16 unk_12;
    s16 unk_14;                    /* -1 == none, else index of D_8008EB80 */
    u8  unk_16[0x1A - 0x16];
    s16 unk_1A;
    s16 unk_1C;
    s16 unk_1E;
    s16 unk_20;
    u8  unk_22[0x3C - 0x22];
    s32 unk_3C;
    u8  unk_40[0x5E - 0x40];
    s16 unk_5E;                    /* 0/1, set alongside func_80021A98 */
    u8  unk_60[0x72 - 0x60];
    s16 unk_72;
    u8  unk_74[0x7C - 0x74];
    s32 unk_7C;
    u8  unk_80[0x84 - 0x80];
    s16 unk_84;
    u8  unk_86[0x88 - 0x86];
    s16 unk_88;
    s16 unk_8A;
    u8  unk_8C[0x8E - 0x8C];
    s16 unk_8E;
    s16 unk_90;
    u8  unk_92[0x96 - 0x92];
    s16 unk_96;
    u8  unk_98[0xA0 - 0x98];
    u8  unk_A0;
    u8  unk_A1[0xB1 - 0xA1];
    u8  unk_B1;
    u8  unk_B2;
    u8  unk_B3[0xB8 - 0xB3];
    Vec4i32 unk_B8;
    Vec4i32 unk_C8;
    Vec3i32 unk_D8;
    u8  unk_E4[0xE8 - 0xE4];
    Vec3i32 unk_E8;
    Vec3i32 unk_F4;
    u8  unk_100[0x104 - 0x100];
    Vec4i32 unk_104;
    Vec4i32 unk_114;
    Vec4i32 unk_124;
    Vec4i32 unk_134;
    s32 unk_144;
    s32 unk_148;
    s16 unk_14C;
    s16 unk_14E;
    s16 unk_150;
    s16 unk_152;
    u8  unk_154[0x156 - 0x154];
    s16 unk_156;
    s16 unk_158;
    s16 unk_15A;
    u8  unk_15C[0x15E - 0x15C];
    s16 unk_15E;
    s16 unk_160;
    s16 unk_162;
    u8  unk_164[0x168 - 0x164];
    Vec3i32 unk_168;
    Vec3i32 unk_174;
    Vec3i32 unk_180;
    Vec3i32 unk_18C;
    u8  unk_198[0x1C8 - 0x198];
    SVec4i16 unk_1C8;
    SVec4i16 unk_1D0;
    s16 unk_1D8;
    u8  unk_1DA[0x1DC - 0x1DA];
    s16 unk_1DC;
    u8  unk_1DE[0x1E6 - 0x1DE];
    s16 unk_1E6;
    s16 unk_1E8;
    s16 unk_1EA;
    u8  unk_1EC[0x1F8 - 0x1EC];
    Vec3i32 unk_1F8;
    u8  unk_204[0x24C - 0x204];
    Vec4i32 unk_24C;
    u8  unk_25C[0x268 - 0x25C];
    s32 unk_268;
    u8  unk_26C[0x274 - 0x26C];
    s16 unk_274;
    s16 unk_276[4];
    s16 unk_27E[4];
    s16 unk_286;
    u8  unk_288[0x31A - 0x288];
    s16 unk_31A;
    u8  unk_31C[0x330 - 0x31C];
    s16 unk_330;
    s16 unk_332;
    u8  unk_334[0x34A - 0x334];
    u8  unk_34A;
    u8  unk_34B;
    u8  unk_34C;
    u8  unk_34D;
    u8  unk_34E;                   /* written by func_8001BE20 for the OTHER record */
    u8  unk_34F[0x350 - 0x34F];
    s16 unk_350;
    u8  unk_352[0x44C - 0x352];
} PracticeMenuRec;                 /* sizeof == 0x44C */
