/* PsyQ 4.0 LIBCARD INIT: InitCARD, StartCARD and StopCARD. .text 0x8007A370..0x8007A428, a verbatim
 * LIBSCAN module span (docs/naming/libscan/matches.json), Q106 D3. */
#include "common.h"
#include <psxsdk/libapi.h>
#include <psxsdk/libcard.h>

/* Declarations from the file this module was split from (gpu.c). */
extern void StopCARD2(void);
extern void _ExitCard(void);

void InitCARD(s32 a0) {
    ChangeClearPAD(0);
    EnterCriticalSection();
    if (ReadInitPadFlag() == 0) {
        a0 = 0;
    }
    InitCARD2(a0);
    ExitCriticalSection();
}
void StartCARD(void) {
    EnterCriticalSection();
    StartCARD2();
    ChangeClearPAD(0);
    ExitCriticalSection();
}

void StopCARD(void) {
    StopCARD2();
    _ExitCard();
}
