#!/usr/bin/env python3
"""s15 -- apply the aggregate-merge candidate to the tree (LF-safe), or restore.

usage: apply.py apply | apply.py restore
"""
import sys, pathlib

R = pathlib.Path("/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile")
BK = R / "tmp/grind/func_80062020/s15/backup"
FILES = ["src/text1b.c", "src/text1b_b.c", "include/game.h"]

HEADER_BLOCK = """
/* 3-word record table at 0x800F1198, terminated by an all-zero record.
 * Object model evidence (independent of and predating any byte-chasing):
 * the original binary walks this table with a 12-byte-stride induction
 * register (asm/funcs/func_80062020.s:.L80062038, `addiu $v1, $v1, 0xC`)
 * and addresses the record members through one base register at
 * displacements 0/4/8 (`sw $zero, 0x8($v0)` / `sw $zero, 0x4($v0)`), i.e.
 * record stride and base+offset addressing, not mere symbol adjacency.
 * Replaces the splat per-word scalars D_800F1198 / D_800F119C / D_800F11A0. */
typedef struct {
    s32 unk0;
    s32 unk4;
    s32 unk8;
} Unk800F1198Record;

extern Unk800F1198Record D_800F1198[];
"""

BODY = """void func_80062020(s32 *arg0) {
    s32 i;
    s32 ofs;
    s32 t;
    t = *(s32 *)((u8 *)arg0 + 0);
    D_800A32B8 = 0;
    i = 0;
    if ((t & 1) == 0) goto end;
    ofs = 0;
    do {
        D_800F1198[i].unk0 = *(s32 *)((u8 *)arg0 + ofs + 0);
        D_800F1198[i].unk4 = *(s32 *)((u8 *)arg0 + ofs + 4);
        D_800F1198[i].unk8 = *(s32 *)((u8 *)arg0 + ofs + 8);
        i = i + 1;
        ofs = ofs + 12;
        t = *(s32 *)((u8 *)arg0 + ofs + 0);
    } while ((t & 1) != 0);
end:
    D_800F1198[i].unk0 = D_800F1198[i].unk4 = D_800F1198[i].unk8 = 0;
}
"""

SCALARS = ["extern s32 D_800F1198;\n", "extern s32 D_800F119C;\n", "extern s32 D_800F11A0;\n"]


def backup():
    BK.mkdir(parents=True, exist_ok=True)
    for f in FILES:
        (BK / pathlib.Path(f).name).write_bytes((R / f).read_bytes())


def restore():
    for f in FILES:
        p = BK / pathlib.Path(f).name
        if p.exists():
            (R / f).write_bytes(p.read_bytes())
    print("restored")


def apply():
    backup()
    # 1. header: append the aggregate declaration before the closing #endif
    gh = (R / "include/game.h").read_text()
    assert "#endif /* GAME_H */" in gh
    gh = gh.replace("#endif /* GAME_H */", HEADER_BLOCK + "\n#endif /* GAME_H */")
    (R / "include/game.h").write_bytes(gh.encode().replace(b"\r\n", b"\n"))

    # 2. remove every per-word scalar extern (merge completeness)
    for f in ["src/text1b.c", "src/text1b_b.c"]:
        txt = (R / f).read_text()
        for s in SCALARS:
            n = txt.count(s)
            txt = txt.replace(s, "")
            print("%s: removed %d x %s" % (f, n, s.strip()))
        (R / f).write_bytes(txt.encode().replace(b"\r\n", b"\n"))

    # 3. replace the INCLUDE_ASM with the C body
    txt = (R / "src/text1b.c").read_text()
    tag = 'INCLUDE_ASM("asm/funcs", func_80062020);\n'
    assert txt.count(tag) == 1, txt.count(tag)
    txt = txt.replace(tag, BODY)
    (R / "src/text1b.c").write_bytes(txt.encode().replace(b"\r\n", b"\n"))
    print("applied")


if __name__ == "__main__":
    {"apply": apply, "restore": restore}[sys.argv[1]]()
