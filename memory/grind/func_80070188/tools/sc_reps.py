import importlib.util, os
_s = importlib.util.spec_from_file_location('merge_reps', 'tmp/func_80070188/merge_reps.py')
_m = importlib.util.module_from_spec(_s); _s.loader.exec_module(_m)

SRC_REPS = [
    ('    u8 value;\n    u8 pad;\n} SelectEntryE534;', '    u8 value;\n    u8 unk1;\n} SelectEntryE534;', 1),
]
if os.environ.get('MERGE', '0') == '1':
    SRC_REPS += _m.reps(_m.REC_DECL)
elif os.environ.get('MERGE', '0') == 'union':
    SRC_REPS += _m.union_reps(_m.UNION_DECL)


def apply(txt):
    for old, new, n in SRC_REPS:
        c = txt.count(old)
        assert c == n, (c, n, old[:80])
        txt = txt.replace(old, new)
    return txt
