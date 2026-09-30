"""Apply the CD_cw landing edits to the tree (run ONLY while holding the landing lock).

src/system.c, include/system.h, named_syms.txt, undefined_syms_auto.txt. LF preserved.
Every edit asserts its exact match count so a drifted tree aborts instead of mis-editing.
"""
import re
import sys
from pathlib import Path


def must(text, old, new, count=1, what=""):
    n = text.count(old)
    if n != count:
        sys.exit(f"ABORT {what}: expected {count} of {old[:80]!r}, found {n}")
    return text.replace(old, new)


def rd(p):
    return Path(p).read_bytes().decode("utf-8")


DRY = "--apply" not in sys.argv   # default is a DRY run into tmp/CD_cw/dry/


def wr(p, t):
    assert "\r" not in t, p
    out = Path("tmp/CD_cw/dry") / p if DRY else Path(p)
    out.parent.mkdir(parents=True, exist_ok=True)
    out.write_bytes(t.encode("utf-8"))


# ---------------------------------------------------------------- src/system.c
s = rd("src/system.c")
s = must(s, "extern u8 CD_pos;\n",
         "extern u8 CD_pos[4]; /* Sony's u_char CD_pos[4] (SOTN: src/main/psxsdk/libcd/bios.c:42 @aa53500) */\n",
         what="CD_pos decl")
s = must(s, "    return &CD_pos;\n", "    return CD_pos;\n", what="CdLastPos")
s = must(s, "extern void D_80016240;\n", "extern char D_80016240[]; /* \"CD_sync\" */\n", what="D_80016240")
s = must(s, "extern void D_800162C0;\n", "extern char D_800162C0[]; /* \"CD_datasync\" */\n", what="D_800162C0")
s = must(s, "extern s32 D_800F19B8;\nextern s32 Alarm_plus_0x4;\nextern void *Alarm_plus_0x8;\n", "",
         count=2, what="per-word Alarm externs")
# CD_sync + CD_datasync: member-for-word respelling
s = must(s, "D_800F19B8 = VSync(-1) + 0x3C0;", "Alarm.time = VSync(-1) + 0x3C0;", count=2, what="time store")
s = must(s, "Alarm_plus_0x4 = 0;", "Alarm.count = 0;", count=2, what="count clear")
s = must(s, "Alarm_plus_0x8 = &D_80016240;", "Alarm.name = D_80016240;", what="CD_sync name")
s = must(s, "Alarm_plus_0x8 = &D_800162C0;", "Alarm.name = D_800162C0;", what="CD_datasync name")
s = must(s, "if (D_800F19B8 < v0)", "if (Alarm.time < v0)", count=2, what="deadline test")
s = must(s, "cnt = Alarm_plus_0x4;", "cnt = Alarm.count;", count=2, what="count read")
s = must(s, "Alarm_plus_0x4 = cnt + 1;", "Alarm.count = cnt + 1;", count=2, what="count write")
# the pp alias existed only for the per-word model (evidence.md 2026-09-30, B1)
s, n = re.subn(r"\n[ \t]*pp = &Alarm_plus_0x8;[^\n]*", "", s)
if n != 2:
    sys.exit(f"ABORT pp alias lines: {n}")
s = must(s, "    void **pp;\n", "", count=2, what="pp decls")
s = must(s, "printf(&D_800161C8, *pp,", "printf(&D_800161C8, Alarm.name,", count=2, what="pp reads")
if re.search(r"\bpp\b", s[s.index("s32 CD_sync(s32 a0"):s.index('INCLUDE_ASM("asm/funcs", CD_ready);')]):
    sys.exit("ABORT pp left in CD_sync")
for sym in ("Alarm_plus_0x", "D_800F19B8"):
    if sym in s:
        sys.exit(f"ABORT leftover {sym}")
body = rd("memory/grind/CD_cw/candidate.c")
s = must(s, 'INCLUDE_ASM("asm/funcs", CD_cw);\n', body, what="CD_cw INCLUDE_ASM")
wr("src/system.c", s)

# ------------------------------------------------------------- include/system.h
h = rd("include/system.h")
h = must(h, "extern s16 D_800A3710;\n",
         "extern s16 D_800A3710;\n"
         "\n"
         "/* PsyQ libcd bios.c's command-timeout alarm (Sony's Alarm_t {int, int, char *};\n"
         " * SOTN: src/main/psxsdk/libcd/bios.c:24 @aa53500; object map:\n"
         " * memory/closer/libcd-identity.md): armed and polled by libcd's command\n"
         " * wait loops in src/system.c. */\n"
         "typedef struct {\n"
         "    s32 time;   /* 0x800F19B8: VSync(-1) deadline */\n"
         "    s32 count;  /* 0x800F19BC: poll count */\n"
         "    char *name; /* 0x800F19C0: caller name for the timeout report */\n"
         "} Alarm_t;\n"
         "extern Alarm_t Alarm;\n", what="system.h")
wr("include/system.h", h)

# ---------------------------------------------------------------- named_syms.txt
ns = rd("named_syms.txt")
old8 = ("Alarm_plus_0x8          = 0x800F19C0;  /* cached pointer to panic format string (&D_80016240) */"
        "  /* data-wave 2026-09-25: was g_panic_msg_format_ptr */\n")
ns = must(ns, old8,
          "Alarm = 0x800F19B8;  /* libcd bios.c alarm {time, count, name} (SOTN name; memory/closer/libcd-identity.md) */\n"
          + old8[:-1] + "  /* alias of Alarm+0x8; retire with CD_ready */\n", what="named Alarm_plus_0x8")
old4 = ("Alarm_plus_0x4                                  = 0x800F19BC;  /* IRQ dispatch sequence counter (already noted as"
        " g_cdrom_init_counter elsewhere in IRQ subsystem) */  /* data-wave 2026-09-25: was g_irq_dispatch_counter */\n")
ns = must(ns, old4, old4[:-1] + "  /* alias of Alarm+0x4; retire with CD_ready */\n", what="named Alarm_plus_0x4")
wr("named_syms.txt", ns)

# ------------------------------------------------------- undefined_syms_auto.txt
us = rd("undefined_syms_auto.txt")
us = must(us, "D_800F19B8 = 0x800F19B8;\n",
          "D_800F19B8 = 0x800F19B8;  /* alias of Alarm+0x0; retire with CD_ready */\n", what="D_800F19B8 row")
us = must(us, "Alarm_plus_0x4 = 0x800F19BC;  /* data-wave 2026-09-25: was D_800F19BC */\n",
          "Alarm_plus_0x4 = 0x800F19BC;  /* data-wave 2026-09-25: was D_800F19BC */  /* alias of Alarm+0x4; retire with CD_ready */\n",
          what="Alarm_plus_0x4 row")
us = must(us, "Alarm_plus_0x8 = 0x800F19C0;  /* data-wave 2026-09-25: was D_800F19C0 */\n",
          "Alarm_plus_0x8 = 0x800F19C0;  /* data-wave 2026-09-25: was D_800F19C0 */  /* alias of Alarm+0x8; retire with CD_ready */\n",
          what="Alarm_plus_0x8 row")
us = must(us, "D_800A1495 = 0x800A1495;  /* alias of Intr+0x1 (static volatile CD_intr, src/system.c); retire with CD_cw */\n",
          "", what="D_800A1495 row (retires with CD_cw)")
us = must(us, "retire with CD_cw, CD_ready */", "retire with CD_ready */", what="D_800A1494 row")
wr("undefined_syms_auto.txt", us)
print("landing edits applied")
