import re, subprocess, json, sys
from pathlib import Path
t = Path('src/text1b_tu1d.c').read_text()
a = t.index('void func_80070F78(s32 arg0, DescF97C *s) {'); b = t.index('\n}\n', a) + 3
body = t[a:b]
def rep(s, old, new, count=1):
    assert s.count(old) == count, (old, s.count(old))
    return s.replace(old, new)
def cells_a(s):
    s = rep(s, "            } else {\n                if ((D_800A354C & (0x10 << (port * 16))) && D_800A3578 == 0 && D_800A35BC != 3) {",
               "            } else {\n                s32 cells_a;\n\n                if ((D_800A354C & (0x10 << (port * 16))) && D_800A3578 == 0 && D_800A35BC != 3) {")
    s = rep(s, "                cells = s->header + 0xC;\n                s->table = cells;\n                s->x = (D_800A3590[i] << 6) + 0x85;",
               "                cells_a = s->header + 0xC;\n                s->table = cells_a;\n                s->x = (D_800A3590[i] << 6) + 0x85;")
    return s
def cells_b(s):
    s = rep(s, "            if (D_800A3578 != 3) {\n                s->scale_x = 0x100;",
               "            if (D_800A3578 != 3) {\n                s32 cells_b;\n\n                s->scale_x = 0x100;")
    s = rep(s, "                cells = s->header + 0xC;\n                s->table = cells;\n                c = ",
               "                cells_b = s->header + 0xC;\n                s->table = cells_b;\n                c = ")
    return s
def cells_c(s):
    s = rep(s, "    s16 i;\n", "    s32 cells_c;\n    s16 i;\n")
    s = rep(s, "    cells = s->header + 0x24;", "    cells_c = s->header + 0x24;")
    s = rep(s, "            s->table = cells;\n            s->x = (D_800A3590[i] << 6) + 0x80;",
               "            s->table = cells_c;\n            s->x = (D_800A3590[i] << 6) + 0x80;")
    return s
def sheets2(s):
    s = rep(s, "    s32 flag;\n", "    s32 *sheets2;\n    s32 flag;\n")
    s = rep(s, "    sheets = *(s32 **)(D_800A35A8 + 0x60);\n    s->header = sheets[0];",
               "    sheets2 = *(s32 **)(D_800A35A8 + 0x60);\n    s->header = sheets2[0];")
    return s
def vram2(s):
    old = ("                    s32 *tim; /* FAKE: image pointer address, mechanism at the ==3 `tim` */\n\n"
           "                    func_8005C650(1, 0x7F, 0x7F);\n                    flag = 2;\n"
           "                    vram = *(u8 **)(D_800A35A8 + 0x7C);\n                    vram += i << 6;\n")
    new = ("                    s32 *tim; /* FAKE: image pointer address, mechanism at the ==3 `tim` */\n                    u8 *vram2;\n\n"
           "                    func_8005C650(1, 0x7F, 0x7F);\n                    flag = 2;\n"
           "                    vram2 = *(u8 **)(D_800A35A8 + 0x7C);\n                    vram2 += i << 6;\n")
    s = rep(s, old, new)
    i = s.index("vram2 += i << 6;")
    j = s.index("LoadImage(vram + id * 8, *tim);", i)
    return s[:j] + "LoadImage(vram2 + id * 8, *tim);" + s[j+len("LoadImage(vram + id * 8, *tim);"):]
V = {
 'same': lambda s: s,
 'cells_a': cells_a, 'cells_b': cells_b, 'cells_c': cells_c,
 'cells_all': lambda s: cells_c(cells_b(cells_a(s))),
 'sheets2': sheets2, 'vram2': vram2,
 'r11_all': lambda s: vram2(sheets2(cells_c(cells_b(cells_a(s))))),
}
import os; os.makedirs('/tmp/r11l', exist_ok=True)
for k, f in V.items():
    p = f'/tmp/r11l/{k}.c'; Path(p).write_text(f(body))
    r = subprocess.run(['python3','-m','engine.cli','sandbox','func_80070F78','--disable','all','--candidate',p,'--workdir','/tmp/r11l/ws'] if False else ['python3','-m','engine.cli','sandbox','func_80070F78','--disable','all','--candidate',p], capture_output=True, text=True)
    try:
        d = json.loads(r.stdout); print(f'{k}: {d["score"]} ({d["build_insns"]}/{d["target_insns"]})', flush=True)
    except Exception:
        print(k, 'ERR', r.stdout[-300:], r.stderr[-500:], flush=True)
