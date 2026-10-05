import sys
NL = "\n"
def sub1(t, a, b):
    assert t.count(a) == 1, a[:60]; return t.replace(a, b)
p = "include/game.h"
g = open(p, encoding="utf-8").read()
g = sub1(g, "/* Unk1F8002B8Rec.unk00 is per-function scratch: each function that keeps data in these 0x60 bytes\n"
            " * has its own layout of them, one member of Unk1F8002B8Unk00 each, and uses only its own. */\n",
            "/* Unk1F8002B8Rec.unk00 is per-function scratch: the functions that keep data in these 0x60 bytes\n"
            " * lay them out differently. func_8002A458 and func_80031B24 have a member of Unk1F8002B8Unk00\n"
            " * each (func_8002AB08 writes func_8002A458's two points); func_8002AB08 / func_80030D7C /\n"
            " * func_800321E8 have none yet (below). */\n")
g = sub1(g, "/* unk00 (bytes) sizes the union to 0x60. func_8002AB08 still uses these bytes through its byte\n"
            " * pointer; func_80030D7C",
            "/* unk00 (bytes) sizes the union to 0x60. func_8002AB08 still uses these bytes through its u8 *\n"
            " * pointer; func_80030D7C")
g = sub1(g, " * func_80031B24 use their members; func_8002AB08 / func_80030D7C / func_800321E8 still use\n"
            " * it through a byte pointer. */\n",
            " * func_80031B24 use their members; func_8002AB08 / func_80030D7C (u8 *) and func_800321E8\n"
            " * (s32 *) still use it through raw pointers. */\n")
open(p, "w", encoding="utf-8", newline=NL).write(g)
m = "tmp/p2/msg_f02b3.txt"
t = open(m, encoding="utf-8").read()
t = sub1(t, "after its function. The comment says each function uses only its own member.\n",
            "after its function. The comment says the functions that keep data in these bytes lay them\n"
            "    out differently: func_8002A458 / func_80031B24 have a member each (func_8002AB08 writes\n"
            "    func_8002A458's two points), func_8002AB08 / func_80030D7C / func_800321E8 none yet.\n")
t = sub1(t, "    - `u8 unk00[0x60]` sizes the union. Its comment says func_8002AB08 still uses the bytes\n"
            "      through its byte pointer,",
            "    - `u8 unk00[0x60]` sizes the union. Its comment says func_8002AB08 still uses the bytes\n"
            "      through its u8 * pointer,")
t = sub1(t, "    func_8002AB08 / func_80030D7C / func_800321E8 still use it through a byte pointer.\n",
            "    func_8002AB08 / func_80030D7C (u8 *) and func_800321E8 (s32 *) still use it through raw\n"
            "    pointers.\n")
t = sub1(t, "  batch (93 record sites).\n", "  batch.\n")
open(m, "w", encoding="utf-8", newline=NL).write(t)
print("ok")
