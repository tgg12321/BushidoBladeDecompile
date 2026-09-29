import importlib.util
_s = importlib.util.spec_from_file_location('merge_reps', 'tmp/func_80070188/merge_reps.py')
_m = importlib.util.module_from_spec(_s); _s.loader.exec_module(_m)

SRC_REPS = [
    ('    u8 value;\n    u8 pad;\n} SelectEntryE534;', '    u8 value;\n    u8 unk1;\n} SelectEntryE534;', 1),
] + _m.reps(_m.REC_DECL)


def apply(txt):
    for old, new, n in SRC_REPS:
        c = txt.count(old)
        assert c == n, (c, n, old[:80])
        txt = txt.replace(old, new)
    return txt
