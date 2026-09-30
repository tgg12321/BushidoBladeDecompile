"""CD_ready landing (+ CD_sync / CD_datasync in SOTN's shape). DRY by default -> tmp/CD_ready/dry/;
pass --apply ONLY while holding the landing lock."""
import sys
from pathlib import Path

sys.path.insert(0, ".")
from engine import inlineasm  # noqa: E402

DRY = "--apply" not in sys.argv


def rd(p):
    return Path(p).read_bytes().decode("utf-8")


def wr(p, t):
    assert "\r" not in t, p
    out = Path("tmp/CD_ready/dry") / p if DRY else Path(p)
    out.parent.mkdir(parents=True, exist_ok=True)
    out.write_bytes(t.encode("utf-8"))


def must(t, old, new, count=1, what=""):
    n = t.count(old)
    if n != count:
        sys.exit(f"ABORT {what}: expected {count} of {old[:70]!r}, found {n}")
    return t.replace(old, new)


L = "memory/grind/CD_ready/"
s = rd("src/system.c")
# 1. bios.c helpers move above CD_sync (Sony's order); CD_ready's name string
HSTART = "/* bios.c's alarm helpers, as in Sony's source"
HEND = "s32 CD_cw(u8 com, u8 *param, u8 *result, s32 async)\n"
assert s.count(HSTART) == 1 and s.count(HEND) == 1
i0, i1 = s.index(HSTART), s.index(HEND)
helpers = s[i0:i1]
s = s[:i0] + s[i1:]
s = must(s, "s32 CD_sync(s32 a0, u8 *a1)\n",
         'extern char D_80016248[]; /* "CD_ready" */\n\n' + helpers + "s32 CD_sync(s32 a0, u8 *a1)\n", what="CD_sync anchor")
# 2. bodies
s = inlineasm.substitute_body(s, "CD_sync", rd(L + "probes-0930/CD_sync_sotn.c"))
s = inlineasm.substitute_body(s, "CD_ready", rd(L + "candidate.c"))
s = inlineasm.substitute_body(s, "CD_datasync", rd(L + "probes-0930/CD_datasync_sotn.c"))
# 3. the second C handle to the "CD timeout: " string is now unused
s = must(s, "extern s32 g_str_cd_timeout;\n", "", what="g_str_cd_timeout extern")
for bad in ("g_str_cd_timeout", "tbl_125c", "idx_1495", "INCLUDE_ASM(\"asm/funcs\", CD_ready)"):
    if bad in s:
        sys.exit(f"ABORT leftover {bad}")
wr("src/system.c", s)

# 4. symbol rows that retire with CD_ready (its asm was their last built reference)
us = rd("undefined_syms_auto.txt")
us = must(us, "D_800F19B8 = 0x800F19B8;  /* alias of Alarm+0x0; retire with CD_ready */\n", "", what="u D_800F19B8")
us = must(us, "Alarm_plus_0x4 = 0x800F19BC;  /* data-wave 2026-09-25: was D_800F19BC */  /* alias of Alarm+0x4; retire with CD_ready */\n", "", what="u +4")
us = must(us, "Alarm_plus_0x8 = 0x800F19C0;  /* data-wave 2026-09-25: was D_800F19C0 */  /* alias of Alarm+0x8; retire with CD_ready */\n", "", what="u +8")
us = must(us, "(static volatile CD_intr, src/system.c); retire with CD_ready */  /* also named by",
          "(static volatile CD_intr, src/system.c) */  /* named by", what="u D_800A1494")
wr("undefined_syms_auto.txt", us)
ns = rd("named_syms.txt")
ns = must(ns, "Alarm_plus_0x8          = 0x800F19C0;  /* cached pointer to panic format string (&D_80016240) */  /* data-wave 2026-09-25: was g_panic_msg_format_ptr */  /* alias of Alarm+0x8; retire with CD_ready */\n", "", what="n +8")
ns = must(ns, "Alarm_plus_0x4                                  = 0x800F19BC;  /* IRQ dispatch sequence counter (already noted as g_cdrom_init_counter elsewhere in IRQ subsystem) */  /* data-wave 2026-09-25: was g_irq_dispatch_counter */  /* alias of Alarm+0x4; retire with CD_ready */\n", "", what="n +4")
wr("named_syms.txt", ns)
print("DRY" if DRY else "APPLIED")
