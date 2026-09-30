#!/usr/bin/env python3
"""sitewin.py <funcA> <funcB> <ro_lo> <ro_hi>: evidence window for one phase-change site.

Lists the text functions from funcA to funcB (the candidate cut positions), then every rodata item
in [ro_lo, ro_hi) and every .data item referenced by a function in the window, each with its
referencing functions tagged by position (window index, or '<' / '>' when outside the window)."""
import json, sys
R = "/home/user/BushidoBladeDecompile/"
d = json.load(open(R + "tmp/owners.json"))
F = d["funcs"]
idx = {f["name"]: i for i, f in enumerate(F)}
a, b = idx[sys.argv[1]], idx[sys.argv[2]]
lo, hi = int(sys.argv[3], 16), int(sys.argv[4], 16)
print("text window:")
for i in range(a, b + 1):
    print("  w%-3d %08x %-28s %s" % (i - a, F[i]["addr"], F[i]["name"], F[i]["obj"]))
def tag(r):
    if r < a:
        return "<" + F[r]["name"]
    if r > b:
        return ">" + F[r]["name"]
    return "w%d" % (r - a)
print("rodata %08x..%08x:" % (lo, hi))
for it in d["items"]:
    if it["sec"] == "rodata" and lo <= it["addr"] < hi:
        print("  %08x %-26s %-22s %s" % (it["addr"], ",".join(it["names"])[:26], it["obj"],
                                         " ".join(tag(r) for r in it["refs"])))
print("data referenced from the window:")
for it in d["items"]:
    if it["sec"] != "rodata" and any(a <= r <= b for r in it["refs"]):
        print("  %08x %-4s %-26s %-18s %s" % (it["addr"], it["sec"], ",".join(it["names"])[:26], it["obj"],
                                              " ".join(tag(r) for r in it["refs"])[:120]))
