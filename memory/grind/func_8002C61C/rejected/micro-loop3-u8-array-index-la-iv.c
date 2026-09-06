#include "lhead.h"
#define SCR ((ProbeScr *)0x1F800078)
extern u8 g_chars[];
void probe(void) {
    s32 i;
    for (i = 0; i < 2; i++) {
        *(s32 *)&g_chars[i * 0x44C + 0x18C] = (SCR[i].j[5].x + SCR[i].j[6].x + SCR[i].j[7].x) / 3;
        *(s32 *)&g_chars[i * 0x44C + 0x190] = (SCR[i].j[5].y + SCR[i].j[6].y + SCR[i].j[7].y) / 3;
        *(s32 *)&g_chars[i * 0x44C + 0x194] = (SCR[i].j[5].z + SCR[i].j[6].z + SCR[i].j[7].z) / 3;
        *(s32 *)&g_chars[i * 0x44C + 0x174] = (SCR[i].j[8].x + SCR[i].j[9].x) / 2;
        *(s32 *)&g_chars[i * 0x44C + 0x178] = (SCR[i].j[8].y + SCR[i].j[9].y) / 2;
        *(s32 *)&g_chars[i * 0x44C + 0x17C] = (SCR[i].j[8].z + SCR[i].j[9].z) / 2;
    }
}
