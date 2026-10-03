p = "include/psxsdk/libcd.h"
t = open(p, encoding="utf-8").read()
t = t.replace("#ifndef LIBCD_H\n#define LIBCD_H\n\n#include \"common.h\"\n",
"""#ifndef PSXSDK_LIBCD_H
#define PSXSDK_LIBCD_H

/* PsyQ LIBCD types and entry points (Sony's libcd.h; SOTN include/psxsdk/libcd.h), spelled as
 * BB2's code uses them (the module definitions in src/main/psxsdk/libcd/). Library-internal
 * state shared by the modules: src/main/psxsdk/libcd/libcd_internal.h. */

#include "common.h"
""", 1)
old_tail = """/* libcd control entry points (src/main/psxsdk/libcd/sys.c): com, param bytes, result bytes. */
s32 CdControl(u8 com, u8 *param, u8 *result);
s32 CdControlB(u8 com, u8 *param, u8 *result);
s32 CdControlF(u8 com, u8 *param);

#endif /* LIBCD_H */
"""
assert old_tail in t
new_tail = """/* libcd control entry points (src/main/psxsdk/libcd/sys.c): com, param bytes, result bytes. */
s32 CdControl(u8 com, u8 *param, u8 *result);
s32 CdControlB(u8 com, u8 *param, u8 *result);
s32 CdControlF(u8 com, u8 *param);

extern s32 CdReset(s32);
extern void CdFlush(void);
extern u32 CdStatus(void);
extern u32 CdMode(void);
extern u32 CdLastCom(void);
extern void *CdLastPos(void);
extern void *CdComstr(u8);
extern void *CdIntstr(u8);
extern s32 CdSync(s32, u8 *);
extern s32 CdReady(s32, u8 *);
extern s32 CdSyncCallback(s32);
extern s32 CdDataCallback(s32);
extern void CdDataSync(s32);
extern s32 CdGetSector2(s32, s32);
extern s32 CdRead(s32, s32, s32);
extern s32 CdReadSync(s32, s32);
extern s32 CdReadCallback(s32);
extern s32 CdReadMode(s32);
extern void CdReadBreak(void);

#endif /* PSXSDK_LIBCD_H */
"""
t = t.replace(old_tail, new_tail)
t = t.replace(" * addressing of one symbol. All its users are in src/main/psxsdk/libcd/cdread.c. */",
              " * addressing of one symbol. All its users are in src/main/psxsdk/libcd/cdread.c (the Q99\n * admission in .claude/rules/aggregate-merge-family.md names this declaration). */")
open(p, "w", encoding="utf-8", newline="\n").write(t)
