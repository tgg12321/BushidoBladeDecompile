"""mk_game.py: split the game headers (system.h, gpu.h, game.h, code6cac.h) into
include/game.h (types: typedef/struct/union definitions and type macros) and include/bb2.h
(declarations: extern objects, prototypes, macros naming objects). Items keep their comment blocks
and their order; header banners and file-top descriptions are dropped. Game-side declarations of
Sony library functions (whose definition is in src/main/psxsdk/) go to a labelled last section."""
import glob, json, re, sys
sys.path.insert(0, "tmp/s5")
from citems import items, strip_cs

SRC = ["include/system.h", "include/gpu.h", "include/game.h", "include/code6cac.h"]
BANNERS = {
    "/* Named globals */", "/* Functions */", "/* Data symbols */",
    "/* Game state - gameplay, stage, pad, camera, animation */",
    "/* Shared declarations for the code6cac module family */",
    "/* System - IRQ, file I/O, memory card */",
    "/* GPU subsystem - display, drawing, math LUTs */",
}
# Sony functions: defined in src/main/psxsdk/ (C definitions or INCLUDE_ASM / BIOS stubs)
sony = set()
for f in glob.glob("src/main/psxsdk/**/*.c", recursive=True):
    t = open(f, encoding="utf-8").read()
    sony |= set(re.findall(r'INCLUDE_ASM\("asm/funcs",\s*(\w+)\)', t))
    sony |= set(re.findall(r"BIOS_[ABC]_FUNCTION\((\w+)", t))
    sony |= set(re.findall(r"^[A-Za-z_][\w \*]*?\b(\w+)\s*\([^;]*\)\s*\{", t, re.M))

types, decls, sonyd = [], [], []
for h in SRC:
    text = open(h, encoding="utf-8").read()
    its, lines = items(text)
    for it in its:
        own = "\n".join(lines[it["start"] - 1:it["end"]])
        lead = lines[it["lead"] - 1:it["start"] - 1]
        # drop banner lines from the lead; keep real comments
        lead = [l for l in lead if l.strip() not in BANNERS]
        while lead and lead[0].strip() == "":
            lead.pop(0)
        block = ("\n".join(lead) + "\n" if lead else "") + own
        c = " ".join(strip_cs(own).split())
        if it["kind"] == "pp":
            if re.match(r"#\s*(ifndef|endif|include)", c) or re.match(r"#\s*define\s+\w+_H\b\s*$", c):
                continue
            if c.startswith("#define SELWORK"):
                decls.append(block)
            else:
                types.append(block)
            continue
        if c.startswith("typedef") or re.match(r"(struct|union|enum)\s+\w+\s*\{", c):
            types.append(block)
        elif c.startswith("struct ") and c.endswith(";") and "{" not in c and "(" not in c and " extern" not in c:
            continue  # bare forward declaration of a struct tag
        else:
            m = re.match(r"(?:extern\s+)?[\w\s\*]*?\b(\w+)\s*\(", c)
            if m and "(*" not in c.split(m.group(1))[0] and m.group(1) in sony:
                sonyd.append(block)
            else:
                decls.append(block)


def join(blocks):
    out = []
    for b in blocks:
        b = b.rstrip("\n")
        if out and ("\n" in b or b.lstrip().startswith("/*") or "\n" in out[-1] or "{" in b):
            out.append("")
        out.append(b)
    return "\n".join(out) + "\n"


game = """#ifndef GAME_H
#define GAME_H

/* Bushido Blade 2's shared game types: the records, tables and object layouts several
 * translation units use. The shared objects and functions themselves are declared in bb2.h,
 * which includes this file; Sony's library types come from include/psxsdk/. */

#include "common.h"
#include <psxsdk/kernel.h>
#include <psxsdk/libapi.h>
#include <psxsdk/libc.h>
#include <psxsdk/libcard.h>
#include <psxsdk/libcd.h>
#include <psxsdk/libcomb.h>
#include <psxsdk/libetc.h>
#include <psxsdk/libgpu.h>
#include <psxsdk/libgte.h>
#include <psxsdk/libsn.h>
#include <psxsdk/libsnd.h>
#include <psxsdk/libspu.h>

""" + join(types) + """
#endif /* GAME_H */
"""
bb2 = """#ifndef BB2_H
#define BB2_H

/* Shared declarations of the game code in SLUS_006.63 (src/main/*.c): the objects and functions
 * more than one translation unit uses. Their types are in game.h. */

#include "game.h"

""" + join(decls) + """
/* Game-side declarations of Sony library functions whose spelling differs from the library's
 * definition (src/main/psxsdk/). Reconciling them is Phase 2 work; until then they stay here,
 * where the game code has always seen them. */
""" + join(sonyd) + """
#endif /* BB2_H */
"""
game = re.sub(r"\n{3,}", "\n\n", game)
bb2 = re.sub(r"\n{3,}", "\n\n", bb2)
open("include/game.h", "w", encoding="utf-8", newline="\n").write(game)
open("include/bb2.h", "w", encoding="utf-8", newline="\n").write(bb2)
print(len(types), "type items;", len(decls), "declarations;", len(sonyd), "Sony-function declarations:")
for b in sonyd:
    print("   ", b.strip().splitlines()[-1])
