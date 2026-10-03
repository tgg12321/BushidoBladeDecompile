"""reinc_game.py FILE... : replace the game-header include set with one #include "bb2.h"."""
import sys
GAME = {'"gpu.h"', '<psxsdk/libsnd.h>', '"game.h"', '"system.h"', '"code6cac.h"', '<psxsdk/libspu.h>'}
for p in sys.argv[1:]:
    t = open(p, encoding="utf-8", newline="").read()
    out, done = [], False
    for l in t.split("\n"):
        s = l.strip()
        if s.startswith("#include ") and s[len("#include "):] in GAME:
            if not done:
                out.append('#include "bb2.h"')
                done = True
            continue
        out.append(l)
    open(p, "w", encoding="utf-8", newline="\n").write("\n".join(out))
