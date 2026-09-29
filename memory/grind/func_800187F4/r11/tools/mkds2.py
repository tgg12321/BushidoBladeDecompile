"""Self-assign and chain-extender sweeps (same bases as mkds.py). Tags sa_* / ce_*."""
import importlib.util, re
spec = importlib.util.spec_from_file_location("mkds", "tmp/f187/mkds.py")
src = open("tmp/f187/mkds.py").read().split("tags = []")[0]
ns = {}
exec(src, ns)
D = ns["D"]; C6 = ns["C6"]; GENS = ns["GENS"]; insert = ns["insert"]; once = ns["once"]
CE = {
    "afterforce": ("        SCR->vel[0] = vx;\n", "        SCR->vel[0] = vx + {x} - {x};\n"),
    "cpos": ("            SCR->cpos[0] = SCR->pos[0] >> 5;\n", "            SCR->cpos[0] = (SCR->pos[0] >> 5) + {x} - {x};\n"),
    "sphtop": ("                r = SCR->rad[{i}];\n", "                r = SCR->rad[{i}] + {x} - {x};\n"),
    "beforetot": ("                tot = dist1 + dist2;\n", "                tot = dist1 + dist2 + {x} - {x};\n"),
    "nodeend": ("        node[3] = (SCR->vel[0] * 7) >> 3;\n", "        node[3] = ((SCR->vel[0] * 7) >> 3) + {x} - {x};\n"),
}
tags = []
for var, g in GENS.items():
    base, names = g(C6)
    i = "idx_sph" if var == "idx" else "idx"
    for n in names:
        for ak in ns["ANCHORS"]:
            t = f"sa_{var}_{n}_{ak}"
            open(D + t + ".c", "w", newline="\n").write(insert(base, ak, f"{n} = {n}; /* FAKE */"))
            tags.append(t)
        for ck, (a, b) in CE.items():
            t = f"ce_{var}_{n}_{ck}"
            open(D + t + ".c", "w", newline="\n").write(once(base, a.format(i=i), b.format(i=i, x=n)))
            tags.append(t)
print(" ".join(tags))
