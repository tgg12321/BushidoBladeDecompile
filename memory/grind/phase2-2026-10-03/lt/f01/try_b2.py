#!/usr/bin/env python3
# try_b2.py : measure candidate bodies for func_80064F68 / func_80064FB4 on top of f01b2.py's output
# (tmp/p2/f01b2/51268.c), compiled in tmp/p2/wk against the base snapshot's object.
import os, re, shutil, subprocess, sys
NL = chr(10)
S = open("tmp/p2/f01b2/51268.c", encoding="utf-8").read()
G = open("tmp/p2/f01b2/game.h", encoding="utf-8").read()
H = open("tmp/p2/f01b2/bb2.h", encoding="utf-8").read()

def span(s, f):
    m = re.search(r"\n[a-z0-9_]+ %s\(" % f, s)
    i = m.start() + 1
    return i, s.index("\n}\n", i) + 3

def body(k, n, val, flag):
    return V[k].format(n=n, val=val, flag=flag)

V = {
"dq2": """s32 func_{fn}(void) {{
    s32 *q = &D_800A347C[2];
    s32 last;
    D_800F0CA0[{n}].unk0 = D_800A347C[0];
    D_800F0CA0[{n}].unk4 = D_800A347C[1];
    last = *q;
    {flag} = 1;
    D_800F0BA8[{n}] = {val};
    D_800F0CA0[{n}].unk8 = last;
    return 1;
}}
""",
"dq2b": """s32 func_{fn}(void) {{
    s32 last;
    s32 *q;
    D_800F0CA0[{n}].unk0 = D_800A347C[0];
    D_800F0CA0[{n}].unk4 = D_800A347C[1];
    q = &D_800A347C[2];
    last = *q;
    {flag} = 1;
    D_800F0BA8[{n}] = {val};
    D_800F0CA0[{n}].unk8 = last;
    return 1;
}}
""",
"q2nolast": """s32 func_{fn}(void) {{
    s32 *p = D_800A347C;
    s32 *q = &p[2];
    D_800F0CA0[{n}].unk0 = p[0];
    D_800F0CA0[{n}].unk4 = p[1];
    {flag} = 1;
    D_800F0BA8[{n}] = {val};
    D_800F0CA0[{n}].unk8 = *q;
    return 1;
}}
""",
"pk": """s32 func_{fn}(void) {{
    s32 *p = D_800A347C;
    s32 last;
    D_800F0CA0[{n}].unk0 = p[0];
    D_800F0CA0[{n}].unk4 = p[1];
    last = p[2];
    {flag} = 1;
    D_800F0BA8[{n}] = {val};
    D_800F0CA0[{n}].unk8 = last;
    return 1;
}}
""",
"step2": """s32 func_{fn}(void) {{
    s32 *p = D_800A347C;
    s32 last;
    D_800F0CA0[{n}].unk0 = p[0];
    D_800F0CA0[{n}].unk4 = p[1];
    p += 2;
    last = *p;
    {flag} = 1;
    D_800F0BA8[{n}] = {val};
    D_800F0CA0[{n}].unk8 = last;
    return 1;
}}
""",
"q2": """s32 func_{fn}(void) {{
    s32 *p = D_800A347C;
    s32 *q = &p[2];
    s32 last;
    D_800F0CA0[{n}].unk0 = p[0];
    D_800F0CA0[{n}].unk4 = p[1];
    last = *q;
    {flag} = 1;
    D_800F0BA8[{n}] = {val};
    D_800F0CA0[{n}].unk8 = last;
    return 1;
}}
""",
"walk1": """s32 func_{fn}(void) {{
    s32 *p = D_800A347C;
    s32 last;
    D_800F0CA0[{n}].unk0 = *p;
    D_800F0CA0[{n}].unk4 = p[1];
    last = p[2];
    {flag} = 1;
    D_800F0BA8[{n}] = {val};
    D_800F0CA0[{n}].unk8 = last;
    return 1;
}}
""",
"nolast": """s32 func_{fn}(void) {{
    s32 *p = D_800A347C;
    D_800F0CA0[{n}].unk0 = p[0];
    D_800F0CA0[{n}].unk4 = p[1];
    {flag} = 1;
    D_800F0BA8[{n}] = {val};
    D_800F0CA0[{n}].unk8 = p[2];
    return 1;
}}
""",
"vec": """s32 func_{fn}(void) {{
    s32 *p = D_800A347C;
    s32 last;
    D_800F0CA0[{n}].unk0 = *p++;
    D_800F0CA0[{n}].unk4 = *p;
    last = p[1];
    {flag} = 1;
    D_800F0BA8[{n}] = {val};
    D_800F0CA0[{n}].unk8 = last;
    return 1;
}}
""",
"q12": """s32 func_{fn}(void) {{
    s32 *p = D_800A347C;
    s32 *q;
    s32 last;
    D_800F0CA0[{n}].unk0 = p[0];
    q = &p[1];
    D_800F0CA0[{n}].unk4 = *q++;
    last = *q;
    {flag} = 1;
    D_800F0BA8[{n}] = {val};
    D_800F0CA0[{n}].unk8 = last;
    return 1;
}}
""",
}
FN = (("80064F68", 3, "0x40", "D_800F10F4"), ("80064FB4", 4, "0x40", "D_800F10F8"))
ks = sys.argv[1:] or list(V)
for k in ks:
    s = S
    for fnid, n, val, flag in FN:
        i, j = span(s, "func_" + fnid)
        s = s[:i] + V[k].format(fn=fnid, n=n, val=val, flag=flag) + s[j:]
    shutil.rmtree("tmp/p2/wk", ignore_errors=True)
    os.makedirs("tmp/p2/wk/include"); os.makedirs("tmp/p2/wk/src/main")
    for p, t in (("include/game.h", G), ("include/bb2.h", H), ("src/main/51268.c", s)):
        open("tmp/p2/wk/" + p, "w", encoding="utf-8", newline=NL).write(t)
    os.makedirs("tmp/p2/f01b2/try", exist_ok=True)
    open("tmp/p2/f01b2/try/%s.c" % k, "w", encoding="utf-8", newline=NL).write(s)
    r = subprocess.run(["wsl", "bash", "tmp/p2/item3/abl2.sh", "main/51268", "func_80064F68", "func_80064FB4"], capture_output=True, text=True)
    print(k, r.stdout.strip().replace(NL, " || "))
