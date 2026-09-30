import os
D = os.path.dirname(os.path.abspath(__file__))
b = open(D + "/v/angle4.c").read()
old = "    s32 dist;\n"
assert b.count(old) == 1
new = """    /* FAKE: dist holds each magnitude in turn (|n| for the 0x4000 guard, then |a.xz| or |n.xz|
     * for the pitch ratan2), each value dead before the next write; reuse admitted on SOTN
     * precedent (Q51, Q53); mechanism: one pseudo, so the ratan2 argument's $a1 preference also
     * seats the site-1 value in $a1 as the target does; split per site 3 (site-1 value in $v0),
     * in every declaration order and as u32; memory/grind/func_8002CD58/evidence.md */
    s32 dist; /* SOTN: src/dra/4B758.c:71 @db41b28 */
"""
open(D + "/final.c", "w").write(b.replace(old, new))
