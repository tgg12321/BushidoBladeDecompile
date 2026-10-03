def rd(p): return open(p, encoding="utf-8", newline="").read()
def wr(p, t): open(p, "w", encoding="utf-8", newline="\n").write(t)
s = rd("include/system.h")
alarm = """/* PsyQ libcd bios.c's command-timeout alarm (Sony's Alarm_t {int, int, char *};
 * SOTN: src/main/psxsdk/libcd/bios.c:24 @aa53500; object map:
 * pre-slim-2026-10-01:memory/closer/libcd-identity.md): armed and polled by libcd's command
 * wait loops in src/main/psxsdk/libcd/bios.c. */
typedef struct {
    s32 time;   /* 0x800F19B8: VSync(-1) deadline */
    s32 count;  /* 0x800F19BC: poll count */
    char *name; /* 0x800F19C0: caller name for the timeout report */
} Alarm_t;
extern Alarm_t Alarm;

"""
assert alarm in s
s = s.replace(alarm, "")
old = """/* Functions */
extern void ResetCallback(void);

/* PsyQ libapi struct EXEC: the PS-EXE header body (0x3C bytes, 0x10 into the image). */
typedef struct EXEC {
    u32 pc0, gp0;
    u32 t_addr, t_size;
    u32 d_addr, d_size;
    u32 b_addr, b_size;
    u32 s_addr, s_size;
    u32 sp, fp, gp, ret, base;
} EXEC;
"""
assert old in s
s = s.replace(old, "")
s = s.replace('#include "common.h"\n', '#include "common.h"\n#include <psxsdk/kernel.h>\n#include <psxsdk/libetc.h>\n', 1)
wr("include/system.h", s)
b = rd("src/main/psxsdk/libcd/bios.c")
anchor = "/* bios.c's alarm helpers, as in Sony's source (SOTN: src/main/psxsdk/libcd/bios.c:95 @aa53500).\n * SOTN reaches its `volatile Alarm_t Alarm` only through the non-volatile view\n * `((Alarm_t *)&Alarm)->`; include/system.h declares Alarm non-volatile, which is\n * that view without the cast. */"
assert anchor in b
newc = "/* bios.c's alarm helpers, as in Sony's source (SOTN: src/main/psxsdk/libcd/bios.c:95 @aa53500).\n * SOTN reaches its `volatile Alarm_t Alarm` only through the non-volatile view\n * `((Alarm_t *)&Alarm)->`; Alarm is declared non-volatile above, which is\n * that view without the cast. */"
b = b.replace(anchor, alarm + newc)
b = b.replace('#include "system.h"\n', "", 1)
wr("src/main/psxsdk/libcd/bios.c", b)
print(s)
