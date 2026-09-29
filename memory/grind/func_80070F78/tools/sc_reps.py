"""Landing-time source replacements for func_80070F78 scoring (applied to a COPY)."""
SRC_REPS = [
    ('extern void func_80070F78(s32 a0, s32 *prim);', 'extern void func_80070F78(s32 a0, DescF97C *prim);', 1),
    ('    func_80070F78(arg0, (s32 *)&prim);', '    func_80070F78(arg0, (DescF97C *)&prim);', 1),
]


def apply(txt):
    for old, new, n in SRC_REPS:
        c = txt.count(old)
        assert c == n, (c, n, old[:80])
        txt = txt.replace(old, new)
    return txt
