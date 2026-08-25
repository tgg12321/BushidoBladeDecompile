
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
asm(".include \"include/macro.inc\"\n");;
extern s32 g_snd_bgm_id;
extern s32 g_snd_se_id;
extern s32 g_snd_volume;
extern void SsSetSerialAttr(s32, s32, s32);
extern u8 SpecialCam;
extern u8 g_disp_enable;
extern u8 g_disp_fade;
extern s16 g_game_mirror_mode;
extern s16 g_game_p2_ctrl;
extern s32 g_game_timer;
extern s16 g_stage_id;
extern s16 g_stage_variant;
extern s16 StatusUpBuf;
extern u8 cpu_practice_honmokuroku_data_tbl;
extern u8 g_module_type_tbl;
extern s32 menuDat;
extern s16 single_dojo_yaburi_char_id_tbl;
typedef struct PracticeMenuRec
{
  u8 unk_00[0x12];
  s16 unk_12;
  u8 unk_14[0x438];
} PracticeMenuRec;
extern PracticeMenuRec g_practice_menu_table[];
extern s32 D_800100A4;
extern s32 D_800109C8;
extern u8 D_8008D518;
extern u8 D_8008D538;
extern u8 D_8008D55C;
extern u8 D_8008D578;
extern u16 D_8008D59E;
extern u8 D_8008D864;
extern s32 D_8008D86C;
extern s32 D_8008D88C;
extern u8 D_8008D9EC;
extern u8 D_8008DA08;
extern s16 D_8008DA50;
extern s16 D_8008DA94;
extern s16 D_8008DAD8;
extern u8 D_8008DB1C;
extern s16 D_8008E194;
extern s16 D_8008E19E;
extern u8 D_8008E1A1;
extern u8 D_8008E338;
extern u16 D_8008E3C0;
extern u8 D_8008E5A8;
extern u8 D_8008E5CC;
extern u8 D_8008E748;
extern u8 D_8008E75C;
extern u8 D_8008EA44;
extern u8 D_8008EA45;
extern s16 D_8008EAC0;
extern s16 D_8008EB04;
extern s16 D_8008EB06;
extern s16 D_8008EB08;
extern s16 D_8008EB0A;
extern s16 D_8008EB0C;
extern s32 D_8008EB10;
extern s32 D_8008EB14;
extern s32 D_8008EB18;
extern u8 D_8008EB1C;
extern u8 D_8008EB38;
extern s16 D_8008EB54;
extern u8 D_8008EB6C;
extern u8 D_8008EB80;
extern u8 D_8008EB8E;
extern u16 D_8008EBA0;
extern s32 D_8008EBCC;
extern s32 D_8008EBE0;
extern u8 D_8008EBF4;
extern u8 D_8008EBFC;
extern u8 D_8008EBFD;
extern u8 D_8008EC30;
extern u32 D_8008EC38;
extern s16 D_8008F12C;
extern u8 D_8008F13C;
extern u8 D_8008F19C[];
extern u8 D_8008F1A8[];
extern u8 D_8008F204[];
extern u8 D_800900EC;
extern u8 D_8009016C;
extern u32 D_80090178;
extern u32 D_800905F8;
extern s32 D_80090600;
extern s32 D_80090604;
extern s16 D_80090608;
extern s16 D_800906A4;
extern s32 D_800948C0;
extern u16 D_80094C68;
extern s16 D_800A310C;
extern s32 D_800A3134;
extern s32 D_800A3140;
extern u8 D_800A31DA;
extern u32 D_800A3220;
extern u8 D_800A3670;
extern u8 D_800A3671;
extern s16 D_800A367A;
extern s16 D_800A367C;
extern u8 D_800A3680;
extern s32 D_800A368C;
extern u8 D_800A3690;
extern s32 D_800A369C;
extern s16 D_800A36A4;
extern s32 D_800A36AC;
extern s32 D_800A36B4;
extern u8 D_800A36B9;
extern u8 D_800A36BA;
extern u8 D_800A36BB;
extern s16 D_800A36C2;
extern s32 D_800A36C4;
extern s16 D_800A36C6;
extern u8 D_800A36C8;
extern u16 D_800A36CA;
extern u8 D_800A36CC;
extern s16 D_800A36D2;
extern s32 D_800A36D4;
extern s32 D_800A36D8;
extern u8 D_800A36E8;
extern u8 D_800A36F0;
extern u8 D_800A36F1;
extern u8 D_800A36F2;
extern u8 D_800A36F4;
extern s16 D_800A36F6;
extern u8 D_800A36F9;
extern u8 D_800A36FA;
extern s16 D_800A36FC;
extern u8 D_800A3712;
extern u8 D_800A3713;
extern u8 D_800A3719;
extern u8 D_800A371A;
extern u8 D_800A371B;
extern s32 D_800A371C;
extern u8 D_800A3728;
extern s8 D_800A3748;
extern s16 D_800A3756;
extern u8 D_800A3758;
extern u8 D_800A3764;
extern u8 D_800A3769;
extern u8 D_800A376A;
extern u8 D_800A376B;
extern u8 D_800A376C;
extern s16 D_800A376E;
extern s32 D_800A3778;
extern u8 D_800A377B;
extern u8 D_800A3781;
extern u8 D_800A3783;
extern s32 D_800A3784;
extern u8 D_800A3788;
extern s32 D_800A3794;
extern u8 D_800A37A0;
extern s32 D_800A37A4;
extern u8 D_800A37B0;
extern u8 D_800A37B4;
extern u8 D_800A37B5;
extern u8 D_800A37B6;
extern s32 D_800A37B8;
extern u8 D_800A37BC;
extern s32 D_800A37C0;
extern u8 D_800A37C6;
extern u8 D_800A37D2;
extern u8 D_800A37D3;
extern u8 D_800A37E0;
extern u8 D_800A37E1;
extern s16 D_800A37E8;
extern s16 D_800A37EA;
extern s16 D_800A37EC;
extern u8 D_800A37F8;
extern u8 D_800A3804;
extern s32 D_800A380C;
extern u8 D_800A3816;
extern u8 D_800A3817;
extern s16 D_800A381C;
extern u8 D_800A381E;
extern s32 D_800A3820;
extern s16 D_800A3824;
extern u8 D_800A382D;
extern s16 D_800A382E;
extern s32 D_800A3830;
extern s16 D_800A3834;
extern u8 D_800A3836;
extern s32 D_800A3844;
extern s32 D_800A3858;
extern u8 *D_800A385C;
extern s32 D_800A3860;
extern s32 D_800A3864;
extern u8 D_800A3874;
extern s16 D_800A3876;
extern s32 D_800A3878;
extern s32 D_800A387C;
extern u8 D_800A3880;
extern s32 D_800A3888;
extern s32 D_800A388C;
extern u8 D_800A3898;
extern u8 D_800A3899;
extern u8 D_800A389A;
extern u8 D_800A389B;
extern u16 D_800A389C;
extern s32 D_800A38A0;
extern u8 D_800A38A4;
extern s16 D_800A38A8;
extern u8 D_800A38AA;
extern u8 D_800A38AB;
extern s16 D_800A38AE;
extern u8 D_800A38B0;
extern u8 D_800A38B8;
extern s16 D_800A38BA;
extern u8 D_800A38C0;
extern u8 D_800A38C1;
extern u16 D_800A38C6;
extern u8 D_800A38D4;
extern s16 D_800A38DC;
extern u8 D_800A38DE;
extern u8 D_800A38DF;
extern u8 D_800A38E0;
extern u8 D_800A38E1;
extern u8 D_800A38E2;
extern s32 D_800A38E4;
extern u8 D_800A38E8;
extern u8 D_800A38E9;
extern u8 D_800A38EC;
extern u8 D_800A38ED;
extern u8 D_800A38EE;
extern s32 D_800A38F0;
extern s32 D_800A38F4;
extern u8 D_800A38F8;
extern u8 D_800A3906;
extern u8 D_800A3907;
extern u8 D_800A390C;
extern u8 D_800A390D;
extern s8 D_800A390E;
extern u8 D_800A390F;
extern s16 D_800A3910;
extern u8 D_800A3912;
extern u8 D_800A3913;
extern u8 D_800A3914;
extern u8 D_800A3915;
extern u8 D_800A3918;
extern u8 D_800A391E;
extern u8 D_800A391F;
extern u8 D_800A3920;
extern u8 D_800A3928;
extern u8 D_800A3929;
extern s32 D_800A3D40;
typedef struct Rec44
{
  s32 w0;
  s32 w4;
  s32 w8;
  s32 wC;
  s16 h10;
  s16 h12;
  s16 h14;
  s16 h16;
  s32 w18;
  s16 h1C;
  u8 b1E;
  u8 b1F;
  s32 w20;
  s32 w24;
  s32 w28;
  s32 w2C;
  s16 h30;
  s16 h32;
  s16 h34;
  s16 h36;
  s16 h38;
  s16 h3A;
  s16 h3C;
  s16 h3E;
  u8 b40;
  u8 b41;
  u8 b42;
  u8 b43;
} Rec44;
typedef struct Rec1C
{
  s16 h0;
  s16 h2;
  s16 h4;
  s16 h6;
  s16 h8;
  s16 hA;
  s16 hC;
  s16 hE;
  s32 w10;
  s32 w14;
  s32 w18;
} Rec1C;
extern Rec44 D_800F5328;
extern Rec44 D_800F6608;
extern u8 D_800F65F9;
extern s16 D_800F68E0[];
extern s32 D_800FF584;
extern s32 D_800FF5A4;
extern s32 D_800FF5A8;
extern s32 D_800FF5C8;
extern s32 D_800FF5CC;
extern s32 D_800FF5D0;
extern s16 D_800FF5D8;
extern s16 D_800FF5DA;
extern s16 D_800FF5DC;
extern s32 D_800FF5E0;
extern u8 D_80101BF0;
extern s16 D_80101E00;
extern u8 D_80101E59;
extern s32 D_80101E5C;
typedef struct 
{
  s32 a;
  s32 b;
} CamPair;
typedef struct 
{
  s16 unk00;
  s16 unk02;
  s16 unk04;
  s16 unk06;
  s16 unk08;
  s16 unk0A;
  CamPair pair;
  s32 unk14;
} ReplayCamRec;
extern ReplayCamRec D_80101E60;
extern s32 D_80101E78;
extern s32 D_80101E7C;
extern s32 D_80101E80;
extern s32 D_80101E84;
extern s32 D_80101E88;
extern s32 D_80101E8C;
extern u8 D_80101E90;
extern s32 D_80101E94;
extern s16 D_80101E98;
extern s16 D_80101E9A;
extern s16 D_80101E9C;
extern u16 D_80101E9E;
extern s32 D_80101EA0;
extern s32 D_80101EA4;
extern u8 D_80101EC8;
extern s16 D_80101ECE;
extern s16 D_80101ED2;
extern s16 D_80101EDC;
extern s16 D_80101EE8;
extern s32 D_80101F04;
extern s16 D_80101F08;
extern s16 D_80101F10;
extern s16 D_80101F12;
extern s16 D_80101F14;
extern s16 D_80101F26;
extern s16 D_80101F42;
extern s16 D_80101F4C;
extern s16 D_80101F4E;
extern s16 D_80101F5E;
extern u8 D_80101F75;
extern u8 D_80101F79;
extern u8 D_80101F7A;
extern u8 D_80101F7B;
extern s32 D_80101F80;
extern s32 D_80101F84;
extern s32 D_80101F88;
extern s32 D_80101FA0;
extern s32 D_80101FA4;
extern s32 D_80101FA8;
extern s32 D_80101FB0;
extern s32 D_80101FB4;
extern s32 D_80101FB8;
extern s32 D_80101FBC;
extern s32 D_80101FC0;
extern s32 D_80101FC4;
extern s32 D_80101FCC;
extern s32 D_80101FD0;
extern s32 D_80101FD4;
extern s32 D_80101FDC;
extern s32 D_80101FE0;
extern s32 D_80101FE4;
extern s32 D_80101FEC;
extern s32 D_80101FF0;
extern s32 D_80101FF4;
extern s32 D_80101FFC;
extern s32 D_80102000;
extern s32 D_80102004;
extern s32 D_8010200C;
extern s32 D_80102010;
extern s16 D_80102014;
extern s16 D_80102016;
extern s16 D_80102018;
extern s16 D_8010201A;
extern s32 D_8010203C;
extern s32 D_80102040;
extern s32 D_80102044;
extern s32 D_80102054;
extern s32 D_80102058;
extern s32 D_8010205C;
extern s32 D_801020D8;
extern s32 D_801020DC;
extern s32 D_801020E0;
extern s32 D_801020E4;
extern s32 D_801020E8;
extern s32 D_801020EC;
extern s32 D_801020FC;
extern s32 D_80102100;
extern s32 D_80102104;
extern s32 D_80102108;
extern s32 D_8010210C;
extern s32 D_80102110;
extern s16 D_8010214E;
extern s32 D_80102154;
extern s16 D_801021E2;
extern u8 D_80102216;
extern s16 D_8010231A;
extern u16 D_80102322;
extern s16 D_80102334;
extern s32 D_80102350;
extern s16 D_8010235C;
extern s16 D_80102372;
extern u16 D_8010237E;
extern s16 D_8010238E;
extern s16 D_801023AA;
extern u8 D_801023C1;
extern u8 D_801023C5;
extern s32 D_801023EC;
extern s32 D_801023F4;
extern s32 D_80102408;
extern s32 D_80102410;
extern s32 D_80102448;
extern s32 D_80102450;
extern s16 D_80102462;
extern s16 D_801024DE;
extern s16 D_8010259A;
extern s32 D_801025A0;
extern s16 D_8010262E;
extern s32 D_80102760;
extern s32 D_80102764;
extern s32 D_80102768;
extern s32 D_80102770;
extern s16 D_80102778;
extern s16 D_8010277A;
extern u8 D_8010277C;
extern u8 D_8010277E;
extern u8 D_80102780;
extern u8 D_80102782;
extern u8 D_80102783;
extern u8 D_80102784;
extern u8 D_80102785;
extern u8 D_80102786;
extern u8 D_80102787;
extern s16 D_80102788;
extern s16 D_8010278A;
extern s16 D_8010278C;
extern s16 D_8010278E;
extern s32 D_80102790;
extern u8 D_80102795;
extern s32 D_80102798;
extern s32 D_8010279C;
extern s32 D_801027B0;
extern s32 D_801027B4;
extern s32 D_801027B8;
extern s32 D_801027BC;
extern s32 D_801027C0;
extern s32 D_801027D4;
extern u8 D_80104E88;
extern s32 MotDataBaseAddress;
extern u8 D_80106A70;
extern u8 D_80106A71;
extern u8 D_80106A72;
extern s16 D_80106A7A;
extern u8 D_80106A80;
extern u8 D_80106A82;
extern u8 D_801077AF;
extern u8 D_801077B0;
extern u8 D_801077BA;
extern s32 D_80107850;
extern s32 D_80107854;
extern s32 D_80107858;
extern void func_8001B748(Rec44 *, Rec1C *, Rec1C *, s32, s32, s32);
extern void func_8003D52C(s32, s32, s32, s32);
extern void func_80021A98(s32, u8 *, s32);
extern void func_80022580(s32, s32, s32, s32, s32);
extern s32 func_80036EA8(s32, s32);
extern void func_8003A728(s32);
extern void func_8003AE5C(s32);
extern void func_8003DE14(s32 *, s32);
extern void game_SetControllerPorts(s32);
extern void func_80041688(s32, s32);
extern void func_80048BA4(s32, s32, s32);
extern void func_800493E4(s32);
extern void func_800494D4(s32, s32);
extern void func_80049584(s32);
extern s32 func_8005B8B8(s32);
extern void func_8005C6D0(void);
extern void Exec(s32 *, s32, s32 *);
extern s32 format(s32 *);
extern void sprintf(s32 *, s32, s32, s32);
extern void ResetGraph(s32);
extern void SetDispMask(s32);
extern void DrawSync(s32);
extern void CdInit(void);
extern void CdFlush(void);
extern void CdSetDebug(s32);
extern void CdReadyCallback(s32);
extern void CdControlF(s32, s32);
extern void CdRead(s32);
extern s32 CdReadSync(s32, s32);
extern void SsSetSerialVol(s32, s32, s32);
extern s32 _comb_control(s32, s32, s32);
extern s32 func_80038C70(void);
extern void func_8003E2D8(s32, s32, s32, s32);
extern void func_80036940(void);
extern s16 D_80099478;
extern s16 D_8009947A;
extern s32 D_800A33B0;
extern s32 D_800A33B4;
extern s32 *func_800457A0(s32);
extern s32 *func_800455AC(s32);
extern void func_80044F30(s32, s32);
extern void func_80045824(s32, s32, s32);
extern void func_80045230(s32);
extern void func_80044010(s32, s32);
extern void func_8003EDC0(s32, s32);
extern s32 func_80044670(s32, s32, s32);
extern void snd_SetVolume(s32);
extern s32 snd_GetMaxFade(void);
extern void func_800481E8(s32, s32);
extern void func_80054410(s32);
extern void func_80045600(s32, s32);
extern void func_80045694(s32, s32);
extern void stage_ExecInitFunc(void);
extern void func_8004659C(s32);
extern void func_800466C0(void);
extern void func_80045510(s32, s32);
extern void func_80044098(s32);
extern void func_800453E0(s32);
void func_800460E4(s32 stage_id, s32 arg1)
{
  s32 *s0;
  s32 s7;
  s32 *s6;
  s32 *s4;
  s32 *s2;
  s32 s3;
  s32 *s1;
  s32 *fp_ptr;
  s32 *sp10;
  s32 *sp18;
  u8 *new_var;
  s32 *sp20;
  s0 = func_800457A0(7);
  if (s0 != ((void *) 0))
  {
    if (D_80099478 == stage_id)
    {
      s7 = 1;
      switch (stage_id)
      {
        case 3:
          break;

        case 4:

        case 7:

        case 18:
          s3 = s0[0];
        {
          s32 off = (((u32) s0[s3 - 1]) >> 2) << 2;
          func_8003EDC0((s32) (((u8 *) s0) + off), 7);
        }
          break;

        case 34:
          s7 = 0;
          break;

      }

      stage_ExecInitFunc();
      if (s7 != 0)
      {
        return;
      }
    }
  }
  D_80099478 = (s16) stage_id;
  s7 = 7;
  s0 = func_800455AC(7);
  if (arg1 != 0)
  {
    func_80044F30(stage_id, arg1);
  }
  else
  {
    func_80044F30(stage_id, (s32) s0);
  }
  if (arg1 != 0)
  {
    s3 = *((s32 *) arg1);
    func_80045824(arg1, (s32) s0, ((s32 *) arg1)[s3]);
  }
  {
    s32 off1_raw = s0[1];
    s3 = s0[0];
    s6 = (s32 *) (((u8 *) s0) + ((((u32) off1_raw) >> 2) << 2));
    {
      s32 *a0_ptr = (s32 *) ((s3 << 2) + ((s32) s0));
      s4 = (s32 *) (((u8 *) s0) + ((((u32) a0_ptr[-1]) >> 2) << 2));
      sp10 = (s32 *) (((u8 *) s0) + ((((u32) s0[2]) >> 2) << 2));
      sp18 = (s32 *) (((u8 *) s0) + ((((u32) s0[3]) >> 2) << 2));
      sp20 = (s32 *) (((u8 *) s0) + ((((u32) s0[4]) >> 2) << 2));
      {
        s32 off = (((u32) a0_ptr[0]) >> 2) << 2;
        s2 = (s32 *) (((u8 *) s0) + off);
      }
      if (arg1 != 0)
      {
        fp_ptr = (s32 *) (((u8 *) arg1) + ((((u32) ((s32 *) arg1)[s3]) >> 2) << 2));
      }
      else
      {
        fp_ptr = s2;
        {
          s32 off3 = (((u32) a0_ptr[1]) >> 2) << 2;
          func_80045230((s32) (((u8 *) s0) + off3));
        }
      }
    }
  }
  D_8009947A = 0;
  s1 = (s32 *) (((s32) s4) - ((s32) s0));
  s1 = (s32 *) (((s32) s1) + ((s32) s0));
  switch (stage_id)
  {
    case 3:
      s1 = s2;
      s4 = (s32 *) (((u8 *) s0) + ((((u32) s0[s3 - 1]) >> 2) << 2));
      s3 = 2;
      s6 = (s32 *) (((u8 *) s0) + ((((u32) s0[s3 - s3]) >> 2) << 2));
      D_8009947A = 1;
      break;

    case 4:

    case 7:

    case 18:
      s1 = s2;
      func_80044010((s32) (((u8 *) s0) + ((((u32) s0[5]) >> 2) << 2)), 8);
      s1 = (s32 *) func_80044670((s32) (((u8 *) s0) + ((((u32) s0[6]) >> 2) << 2)), 8, (s32) s1);
      break;

    case 11:
      snd_SetVolume((s32) s1);
      s1 = (s32 *) (((s32) s1) + snd_GetMaxFade());
      break;

    case 13:
      s1 = s2;
      s6 = (s32 *) (((u8 *) s0) + ((((u32) s0[s3 - 2]) >> 2) << 2));
      s4 = (s32 *) (((u8 *) s0) + ((((u32) s0[s3 - 1]) >> 2) << 2));
      func_80044010((s32) (((u8 *) s0) + ((((u32) s0[5]) >> 2) << 2)), 8);
      D_8009947A = 1;
      break;

    case 34:
      new_var = (u8 *) s0;
      s1 = s2;
      D_8009947A = 1;
      s4 = (s32 *) (new_var + ((((u32) s0[5]) >> 2) << 2));
      break;

  }

  func_80044010((s32) s6, 7);
  func_800481E8((s32) fp_ptr, 0);
  func_8003EDC0((s32) s4, 7);
  func_80054410((s32) sp10);
  D_800A33B0 = (s32) sp18;
  D_800A33B4 = (s32) sp20;
  DrawSync(0);
  func_80045600(s7, (s32) s1);
  func_80045694(s7, (s32) func_800466C0);
  stage_ExecInitFunc();
  if (D_800A38DC != 0)
  {
    if (stage_id != 0x22)
    {
      func_8004659C(-1);
    }
  }
}

void func_800464C4(void);
void func_8004659C(s32 a0);
extern void func_800453E0(s32);
void func_8004668C(void);
