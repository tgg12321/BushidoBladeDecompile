/* Rodata sub-TU split out for the 101C.rodata_text1a_b_pre cluster
 * (rodata-cleanup project, docs/rodata-cleanup-project.md, 2026-06-09).
 * MULTI-FILE cluster: 23 symbols (12 jtbls + 5 strings + 6 data words)
 * spanning text1a.c and text1b.c. Sub-TU packs all bytes into one file
 * at the asm/data slot (between text1a_b.o and text1b_b.o). */
#include "common.h"

/* Auto-extracted from asm/data/101C.rodata_text1a_b_pre.s */

/* D_800153F0: 11 words (44B) @ 0x800153F0 */
const u32 D_800153F0[11] = {
    0x0E000E00,
    0x00000E00,
    0x00000000,
    0x00000000,
    0x0E000000,
    0x00010A00,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00300030,
    0x10000030,
};

/* jtbl_8001541C: 16 words (64B) @ 0x8001541C */
const u32 jtbl_8001541C[16] = {
    0x8004A990,
    0x8004ABD8,
    0x8004A5D0,
    0x8004AE0C,
    0x8004A5D0,
    0x8004A5D0,
    0x8004A5D0,
    0x8004A5D0,
    0x8004A5D0,
    0x8004A5D0,
    0x8004AF84,
    0x8004A5D0,
    0x8004B0C8,
    0x8004B0E4,
    0x8004B4D8,
    0x8004B27C,
};

/* jtbl_8001545C: 5 words (20B) @ 0x8001545C */
const u32 jtbl_8001545C[5] = {
    0x8004A614,
    0x8004A644,
    0x8004A694,
    0x8004A6F0,
    0x8004A730,
};

/* D_80015470: 4 words (16B) @ 0x80015470 */
const u32 D_80015470[4] = {
    0x8004B6E4,
    0x8004BCC0,  /* saTan2LineDraw */
    0x80052C10,  /* InitFadePanel */
    0x80052C10,  /* InitFadePanel */
};

/* D_80015480: 8 words (32B) @ 0x80015480 — function-pointer table */
const u32 D_80015480[8] = {
    0x8004C994,  /* func_8004C994 */
    0x8004CB8C,  /* func_8004CB8C */
    0x8004CDB0,  /* func_8004CDB0 */
    0x8004CFE0,  /* func_8004CFE0 */
    0x80051208,  /* func_80051208 */
    0x800513B0,  /* func_800513B0 */
    0x800515AC,  /* func_800515AC */
    0x80051754,  /* func_80051754 */
};

/* D_800154A0: 96 words (384B) @ 0x800154A0 */
const u32 D_800154A0[96] = {
    0x0FE01000,
    0x0FA30FC1,
    0x0F680F85,
    0x0F300F4C,
    0x0EFB0F15,
    0x0EC70EE1,
    0x0E960EAE,
    0x0E660E7E,
    0x0E380E4F,
    0x0E0C0E22,
    0x0DE20DF7,
    0x0DB90DCD,
    0x0D910DA5,
    0x0D6B0D7E,
    0x0D450D58,
    0x0D210D33,
    0x0CFF0D10,
    0x0CDD0CEE,
    0x0CBC0CCC,
    0x0C9C0CAC,
    0x0C7D0C8D,
    0x0C5F0C6E,
    0x0C420C51,
    0x0C260C34,
    0x0C0A0C18,
    0x0BEF0BFD,
    0x0BD50BE2,
    0x0BBB0BC8,
    0x0BA20BAF,
    0x0B8A0B96,
    0x0B720B7E,
    0x0B5B0B67,
    0x0B450B50,
    0x0B2E0B39,
    0x0B190B24,
    0x0B040B0E,
    0x0AEF0AF9,
    0x0ADB0AE5,
    0x0AC70AD1,
    0x0AB40ABD,
    0x0AA10AAA,
    0x0A8E0A97,
    0x0A7C0A85,
    0x0A6A0A73,
    0x0A590A61,
    0x0A470A50,
    0x0A370A3F,
    0x0A260A2E,
    0x0A160A1E,
    0x0A060A0E,
    0x09F609FE,
    0x09E709EF,
    0x09D809E0,
    0x09C909D1,
    0x09BB09C2,
    0x09AD09B4,
    0x099E09A5,
    0x09910998,
    0x0983098A,
    0x0976097C,
    0x0969096F,
    0x095C0962,
    0x094F0955,
    0x09430949,
    0x0936093C,
    0x092A0930,
    0x091E0924,
    0x09120918,
    0x0907090D,
    0x08FB0901,
    0x08F008F6,
    0x08E508EB,
    0x08DA08E0,
    0x08CF08D5,
    0x08C508CA,
    0x08BA08BF,
    0x08B008B5,
    0x08A608AB,
    0x089C08A1,
    0x08920897,
    0x0888088D,
    0x087E0883,
    0x0875087A,
    0x086B0870,
    0x08620867,
    0x0859085E,
    0x08500855,
    0x0847084C,
    0x083E0843,
    0x0836083A,
    0x082D0831,
    0x08240829,
    0x081C0820,
    0x08140818,
    0x080C0810,
    0x08040808,
};

/* D_80015620: 128 words (512B) @ 0x80015620 */
const u32 D_80015620[128] = {
    0x02000000,
    0x037602D4,
    0x04780400,
    0x054A04E6,
    0x060005A8,
    0x06A20653,
    0x073606ED,
    0x07BE077B,
    0x083F0800,
    0x08B7087C,
    0x092A08F1,
    0x09970961,
    0x0A0009CC,
    0x0A640A32,
    0x0AC50A95,
    0x0B220AF4,
    0x0B7D0B50,
    0x0BD50BA9,
    0x0C2A0C00,
    0x0C7D0C54,
    0x0CCE0CA6,
    0x0D1D0CF6,
    0x0D6A0D44,
    0x0DB60D90,
    0x0E000DDB,
    0x0E480E24,
    0x0E8F0E6C,
    0x0ED50EB2,
    0x0F190EF7,
    0x0F5C0F3B,
    0x0F9E0F7D,
    0x0FDF0FBF,
    0x101F1000,
    0x105E103F,
    0x109C107E,
    0x10DA10BB,
    0x111610F8,
    0x11521134,
    0x118C116F,
    0x11C611A9,
    0x120011E3,
    0x1238121C,
    0x12701254,
    0x12A7128C,
    0x12DE12C2,
    0x131412F9,
    0x1349132E,
    0x137E1364,
    0x13B21398,
    0x13E613CC,
    0x14191400,
    0x144C1432,
    0x147E1465,
    0x14B01497,
    0x14E114C8,
    0x151214F9,
    0x1542152A,
    0x1572155A,
    0x15A2158A,
    0x15D115B9,
    0x160015E8,
    0x162E1617,
    0x165C1645,
    0x16891673,
    0x16B716A0,
    0x16E416CD,
    0x171016FA,
    0x173C1726,
    0x17681752,
    0x1794177E,
    0x17BF17AA,
    0x17EA17D5,
    0x18151800,
    0x183F182A,
    0x18691854,
    0x1893187E,
    0x18BD18A8,
    0x18E618D1,
    0x190F18FA,
    0x19381923,
    0x1960194C,
    0x19881974,
    0x19B0199C,
    0x19D819C4,
    0x1A0019EC,
    0x1A271A13,
    0x1A4E1A3A,
    0x1A751A61,
    0x1A9B1A88,
    0x1AC21AAE,
    0x1AE81AD5,
    0x1B0E1AFB,
    0x1B331B21,
    0x1B591B46,
    0x1B7E1B6C,
    0x1BA31B91,
    0x1BC81BB6,
    0x1BED1BDB,
    0x1C121C00,
    0x1C361C24,
    0x1C5A1C48,
    0x1C7E1C6C,
    0x1CA21C90,
    0x1CC61CB4,
    0x1CE91CD8,
    0x1D0D1CFB,
    0x1D301D1E,
    0x1D531D41,
    0x1D761D64,
    0x1D981D87,
    0x1DBB1DAA,
    0x1DDD1DCC,
    0x1E001DEE,
    0x1E221E11,
    0x1E431E33,
    0x1E651E54,
    0x1E871E76,
    0x1EA81E98,
    0x1ECA1EB9,
    0x1EEB1EDA,
    0x1F0C1EFB,
    0x1F2D1F1C,
    0x1F4E1F3D,
    0x1F6E1F5E,
    0x1F8F1F7E,
    0x1FAF1F9F,
    0x1FCF1FBF,
    0x1FEF1FDF,
};

/* D_80015820: 8 words (32B) @ 0x80015820 */
const u32 D_80015820[8] = {
    0x80050C0C,
    0x80050DAC,
    0x80050FB4,
    0x80051154,
    0x80051358,
    0x800514FC,
    0x800516FC,
    0x80051894,
};

/* D_80015840: 1 string, 28B @ 0x80015840 */
const char D_80015840[28] = "Destruction tiny model.\n";

/* jtbl_8001585C: 10 words (40B) @ 0x8001585C */
const u32 jtbl_8001585C[10] = {
    0x80059DD0,
    0x80059DE8,
    0x80059E00,
    0x80059E18,
    0x80059E30,
    0x80059E7C,
    0x80059EC4,
    0x80059F10,
    0x80059F5C,
    0x00000000,
};

/* jtbl_80015884: 6 words (24B) @ 0x80015884 */
const u32 jtbl_80015884[6] = {
    0x8005A190,
    0x8005A238,
    0x8005A190,
    0x8005A238,
    0x8005A2C0,
    0x00000000,
};

/* jtbl_8001589C: 6 words (24B) @ 0x8001589C */
const u32 jtbl_8001589C[6] = {
    0x8005A418,
    0x8005A430,
    0x8005A458,
    0x8005A480,
    0x8005A4A8,
    0x8005A4B4,
};

/* D_800158B4: 1 string, 24B @ 0x800158B4 */
const char D_800158B4[24] = "common_vab start:%08x\n";

/* D_800158CC: 1 string, 20B @ 0x800158CC */
const char D_800158CC[20] = "vab id:%d mistake\n";

/* D_800158E0: 24B @ 0x800158E0 — "eff prim over :%d \n" + alignment + empty trailing string */
const char D_800158E0[24] = "eff prim over :%d \n";

/* jtbl_800158F8: 18 words (72B) @ 0x800158F8 */
const u32 jtbl_800158F8[18] = {
    0x80065AC8,
    0x800659E4,
    0x800659E4,
    0x80065A34,
    0x80065A34,
    0x80065AA0,
    0x80065BCC,
    0x80065BCC,
    0x80065D1C,
    0x80065D1C,
    0x80065AEC,
    0x80065AEC,
    0x80065DC8,
    0x80065DC8,
    0x80065DC8,
    0x80065DC8,
    0x80065EF0,
    0x80065EF0,
};

/* jtbl_80015940: 18 words (72B) @ 0x80015940 */
const u32 jtbl_80015940[18] = {
    0x80066168,
    0x80066324,
    0x80066324,
    0x8006663C,
    0x8006663C,
    0x80066784,
    0x8006692C,
    0x80066968,
    0x800665D0,
    0x800665D0,
    0x8006692C,
    0x80066968,
    0x80066550,
    0x80066550,
    0x80066550,
    0x80066550,
    0x80066324,
    0x80066324,
};

/* jtbl_80015988: 6 words (24B) @ 0x80015988 */
const u32 jtbl_80015988[6] = {
    0x8006B6B8,
    0x8006B6E8,
    0x8006B720,
    0x8006B7B4,
    0x8006B7FC,
    0x8006B828,
};

/* D_800159A0: 16B @ 0x800159A0 — "warning\n" + alignment + empty trailing string */
const char D_800159A0[16] = "warning\n";

/* jtbl_800159B0: 8 words (32B) @ 0x800159B0 */
const u32 jtbl_800159B0[8] = {
    0x8006E5D8,
    0x8006E618,
    0x8006E5F0,
    0x8006E618,
    0x8006E5E4,
    0x8006E628,
    0x8006E5E4,
    0x00000000,
};

/* jtbl_800159D0: 15 words (60B) @ 0x800159D0 */
const u32 jtbl_800159D0[15] = {
    0x8006EE74,
    0x8006EF10,
    0x8006EF10,
    0x8006EE88,
    0x8006EF10,
    0x8006EF10,
    0x8006EF10,
    0x8006EF10,
    0x8006EF10,
    0x8006EF10,
    0x8006EF10,
    0x8006EF10,
    0x8006EE38,
    0x8006EE4C,
    0x8006EE60,
};

/* jtbl_80015A0C: 6 words (24B) @ 0x80015A0C */
const u32 jtbl_80015A0C[6] = {
    0x800748F0,
    0x80074984,
    0x800749D8,
    0x80074A58,
    0x80074AB0,
    0x00000000,
};

/* jtbl_80015A24: 6 words (24B) @ 0x80015A24 */
const u32 jtbl_80015A24[6] = {
    0x80077438,
    0x80077460,
    0x800774B0,
    0x80077540,
    0x800775D0,
    0x80077670,
};

