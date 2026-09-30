"""do-while(0) wrap ablations for F2D0/F770 v1 -> <dir>/w_*.c"""
import sys
from pathlib import Path

d = sys.argv[1]
t = Path(f"{d}/v1.c").read_bytes().decode()
W1 = "    do {\n        i2 = c2 / det;\n    } while (0);\n"
assert W1 in t
w2s = t.index("    do {\n        /* inline_o.h: gte_SetRotMatrix")
w2e = t.index("    } while (0);\n", w2s) + len("    } while (0);\n")
w2 = t[w2s:w2e]
inner = w2[len("    do {\n"):-len("    } while (0);\n")]
inner = "".join(l[4:] + "\n" for l in inner.splitlines())
V = {
    "no1": t.replace(W1, "    i2 = c2 / det;\n"),
    "no2": t[:w2s] + inner + t[w2e:],
}
V["none"] = V["no1"].replace(w2, inner) if w2 in V["no1"] else None
t2 = V["no1"]
s = t2.index("    do {\n        /* inline_o.h: gte_SetRotMatrix")
e = t2.index("    } while (0);\n", s) + len("    } while (0);\n")
V["none"] = t2[:s] + inner + t2[e:]
for k, v in V.items():
    Path(f"{d}/w_{k}.c").write_bytes(v.encode())
print("ok")
