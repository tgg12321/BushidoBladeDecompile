"""Generate full-file system.c variants for the Alarm merge study (writes tmp/CD_cw/full_*.c)."""
import re
from pathlib import Path

SRC = Path("src/system.c").read_text(encoding="utf-8")
CW_BODY = Path("tmp/CD_cw/body_struct.c").read_text(encoding="utf-8")


def must(text, old, new, count=1):
    n = text.count(old)
    if n != count:
        raise SystemExit(f"expected {count} of {old!r}, found {n}")
    return text.replace(old, new)


def base_merge(t):
    t = must(t, "extern void D_80016240;\n", "extern char D_80016240[]; /* \"CD_sync\" */\n")
    t = must(t, "extern void D_800162C0;\n", "extern char D_800162C0[]; /* \"CD_datasync\" */\n")
    decl = ("extern s32 D_800F19B8;\n"
            "extern s32 Alarm_plus_0x4;\n"
            "extern void *Alarm_plus_0x8;\n")
    assert t.count(decl) == 2, t.count(decl)
    first = t.index(decl)
    t = t[:first] + ALARM_DECL + t[first + len(decl):]
    t = must(t, decl, "")
    t = t.replace("D_800F19B8 = VSync(-1) + 0x3C0;", "Alarm.time = VSync(-1) + 0x3C0;")
    t = t.replace("Alarm_plus_0x4 = 0;", "Alarm.count = 0;")
    t = must(t, "Alarm_plus_0x8 = &D_80016240;", "Alarm.name = D_80016240;")
    t = must(t, "Alarm_plus_0x8 = &D_800162C0;", "Alarm.name = D_800162C0;")
    t = t.replace("if (D_800F19B8 < v0)", "if (Alarm.time < v0)")
    t = t.replace("cnt = Alarm_plus_0x4;", "cnt = Alarm.count;")
    t = t.replace("Alarm_plus_0x4 = cnt + 1;", "Alarm.count = cnt + 1;")
    t = t.replace("void **pp;", "char **pp;", 2)
    t = t.replace("pp = &Alarm_plus_0x8;", "pp = &Alarm.name;")
    assert "Alarm_plus_0x" not in t and "D_800F19B8" not in t, "leftover per-word"
    t = must(t, 'INCLUDE_ASM("asm/funcs", CD_cw);\n', CW_BODY)
    return t


ALARM_DECL = ("typedef struct {\n"
              "    s32 time;   /* 0x800F19B8: VSync deadline */\n"
              "    s32 count;  /* 0x800F19BC: poll counter */\n"
              "    char *name; /* 0x800F19C0: caller name for the timeout report */\n"
              "} Alarm_t;\n"
              "extern Alarm_t Alarm;\n")

b0 = base_merge(SRC)
Path("tmp/CD_cw/full_b0.c").write_text(b0, encoding="utf-8", newline="\n")

# B1: pp alias dropped in CD_sync / CD_datasync (direct Alarm.name read)
b1 = re.sub(r"\n\s*pp = &Alarm\.name;[^\n]*", "", b0)
b1 = b1.replace("printf(&D_800161C8, *pp,", "printf(&D_800161C8, Alarm.name,")
Path("tmp/CD_cw/full_b1.c").write_text(b1, encoding="utf-8", newline="\n")
print("ok")

# B2: B1 + unused pp decls removed + CD_pos typed as Sony's 4-byte array
b2 = must(b1, "    char **pp;\n", "", 2)
b2 = must(b2, "extern u8 CD_pos;\n", "extern u8 CD_pos[4];\n")
b2 = must(b2, "return &CD_pos;", "return CD_pos;")
b2 = must(b2, "(&CD_pos)[i] = param[i];", "CD_pos[i] = param[i];")
Path("tmp/CD_cw/full_b2.c").write_text(b2, encoding="utf-8", newline="\n")
# B3: loop bound read straight from the nparam table
b3 = must(b2, "i < D_800A12FC[com + 0x40]", "i < D_800A13FC[com]")
Path("tmp/CD_cw/full_b3.c").write_text(b3, encoding="utf-8", newline="\n")
print("ok2")

# FINAL: b2 with the annotated body
bs = CW_BODY.replace("(&CD_pos)[i] = param[i];", "CD_pos[i] = param[i];")
fin = must(b2, bs, Path("tmp/CD_cw/body_final.c").read_text(encoding="utf-8"))
Path("tmp/CD_cw/full_final.c").write_text(fin, encoding="utf-8", newline="\n")
print("ok3")
