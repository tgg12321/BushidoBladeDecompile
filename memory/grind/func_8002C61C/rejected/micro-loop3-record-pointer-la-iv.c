#include "lhead.h"
#define SCR ((ProbeScr *)0x1F800078)
void probe(void) {
    s32 i, off;
    ProbeRec *ch;
    for (i = 0; i < 2; i++) {
        off = i * sizeof(ProbeRec);
        ch = (ProbeRec *)((u8 *)&D_80101EC8 + off);
        ch->cen.x = (SCR[i].j[5].x + SCR[i].j[6].x + SCR[i].j[7].x) / 3;
        ch->cen.y = (SCR[i].j[5].y + SCR[i].j[6].y + SCR[i].j[7].y) / 3;
        ch->cen.z = (SCR[i].j[5].z + SCR[i].j[6].z + SCR[i].j[7].z) / 3;
        ch->mid.x = (SCR[i].j[8].x + SCR[i].j[9].x) / 2;
        ch->mid.y = (SCR[i].j[8].y + SCR[i].j[9].y) / 2;
        ch->mid.z = (SCR[i].j[8].z + SCR[i].j[9].z) / 2;
    }
}
