OLDBODY = """        s32 nx = x2 + x2f;
        s32 ny = y2 + y2f;
        s32 cy = y + yf;
        s32 dy = end - phase;
        s32 old;
        rect.x = x + xf;
        rect.y = cy;
        rect.w = h;
        rect.h = dy;
        SetDrawMove(p, &rect, nx, ny + phase);
        old = phase;
        phase >>= 1;
        xf >>= 1;
        yf >>= 1;
        x2f >>= 1;
        y2f >>= 1;
        h >>= 1;
        end >>= 1;
        i++;
        p->tag.addr = ((OTag *)D_800A378C)[0xFFF].addr;
        ((OTag *)D_800A378C)[0xFFF].addr = (u32)p;
        p++;
        rect.y = cy + dy;
        rect.h = old;
        SetDrawMove(p, &rect, nx, ny);
        p->tag.addr = ((OTag *)D_800A378C)[0xFFF].addr;
        ((OTag *)D_800A378C)[0xFFF].addr = (u32)p;
        p++;
"""
S = """nx = x2 + x2f;
ny = y2 + y2f;
cy = y + yf;
dy = end - phase;
rect.x = x + xf;
rect.y = cy;
rect.w = h;
rect.h = dy;
SetDrawMove(p, &rect, nx, ny + phase);
old = phase;
phase >>= 1;
xf >>= 1;
yf >>= 1;
x2f >>= 1;
y2f >>= 1;
h >>= 1;
end >>= 1;
i++;
p->tag.addr = ((OTag *)D_800A378C)[0xFFF].addr;
((OTag *)D_800A378C)[0xFFF].addr = (u32)p;
p++;
rect.y = cy + dy;
rect.h = old;
SetDrawMove(p, &rect, nx, ny);
p->tag.addr = ((OTag *)D_800A378C)[0xFFF].addr;
((OTag *)D_800A378C)[0xFFF].addr = (u32)p;
p++;""".split("\n")
import os
lo, hi = int(os.environ.get("WLO", "0")), int(os.environ.get("WHI", "999"))
VARIANTS = []
n = len(S)
k = 0
for a in range(n):
    for b in range(a + 1, n + 1):
        k += 1
        if not (lo <= k < hi):
            continue
        body = "        s32 nx, ny, cy, dy, old;\n"
        for j, st in enumerate(S):
            if j == a:
                body += "        do {\n"
            ind = "            " if a <= j < b else "        "
            body += ind + st + "\n"
            if j == b - 1:
                body += "        } while (0);\n"
        VARIANTS.append((f"w{a}_{b}", [(OLDBODY, body)]))
