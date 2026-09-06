#include "lhead.h"
#define SCR ((ProbeScr *)0x1F800078)
#define CH ((ProbeRec *)&D_80101EC8)
void probe(void) {
    s32 i;
    for (i = 0; i < 2; i++) {
        CH[i].cen.x = (SCR[i].j[5].x + SCR[i].j[6].x + SCR[i].j[7].x) / 3;
        CH[i].cen.y = (SCR[i].j[5].y + SCR[i].j[6].y + SCR[i].j[7].y) / 3;
        CH[i].cen.z = (SCR[i].j[5].z + SCR[i].j[6].z + SCR[i].j[7].z) / 3;
        CH[i].mid.x = (SCR[i].j[8].x + SCR[i].j[9].x) / 2;
        CH[i].mid.y = (SCR[i].j[8].y + SCR[i].j[9].y) / 2;
        CH[i].mid.z = (SCR[i].j[8].z + SCR[i].j[9].z) / 2;
    }
}
