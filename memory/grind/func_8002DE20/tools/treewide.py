"""Tree-wide before/after of the sandbox's stripped input for the Q11 gtemacro change.

For every src/*.c and every banked .c under memory/grind/ (candidates, rejected
bodies, variants), compute inlineasm.strip_cheat_asm_file(text,
keep_gte_macro_units=True) with the CURRENT engine/gtemacro.py (before) and with
tmp/func_8002DE20/eng/engine/gtemacro.py (after). Report every file whose
stripped text differs, and for each changed file the macro units that differ.
Also runs the DEFAULT strip (keep_gte_macro_units=False) both ways as a check that
it is untouched.
"""
import glob
import importlib.util
import json
import sys

sys.path.insert(0, ".")
import engine  # noqa: E402
from engine import gtemacro as g_tree  # noqa: E402
from engine import inlineasm  # noqa: E402

def _load(name, path):
    spec = importlib.util.spec_from_file_location(name, path)
    m = importlib.util.module_from_spec(spec)
    m.__package__ = "engine"
    spec.loader.exec_module(m)
    return m


# before = HEAD engine/gtemacro.py (git show HEAD:... > tmp copy); after = the Q11 version
g_old = _load("engine.gtemacro_old", "tmp/func_8002DE20/gtemacro_head.py")
g_new = _load("engine.gtemacro_new", "tmp/func_8002DE20/eng/engine/gtemacro.py")


def strip_with(g, text, keep=True):
    sys.modules["engine.gtemacro"] = g
    engine.gtemacro = g
    try:
        return inlineasm.strip_cheat_asm_file(text, keep_gte_macro_units=keep)
    finally:
        sys.modules["engine.gtemacro"] = g_tree
        engine.gtemacro = g_tree


files = sorted(glob.glob("src/*.c")) + sorted(glob.glob("memory/grind/**/*.c", recursive=True))
changed, default_changed, errors = [], [], []
for f in files:
    try:
        text = open(f, encoding="utf-8", errors="replace").read()
        a, b = strip_with(g_old, text), strip_with(g_new, text)
        if a != b:
            ua = [m for _s, _e, m in g_old.unit_spans(text)]
            ub = [m for _s, _e, m in g_new.unit_spans(text)]
            changed.append({"file": f, "stripped_before": a[1], "stripped_after": b[1],
                            "units_before": len(ua), "units_after": len(ub),
                            "new_macros": sorted(set(ub) - set(ua)) or sorted(set(ub))})
        if strip_with(g_old, text, False) != strip_with(g_new, text, False):
            default_changed.append(f)
    except Exception as exc:  # noqa: BLE001
        errors.append((f, repr(exc)[:120]))
res = {"files_scanned": len(files), "src_files": len(glob.glob("src/*.c")),
       "changed": changed, "default_strip_changed": default_changed, "errors": errors}
json.dump(res, open(sys.argv[1] if len(sys.argv) > 1 else "tmp/func_8002DE20/treewide.json", "w"), indent=1)
print(f"scanned {len(files)} files ({res['src_files']} src); keep-mode stripped input changed in {len(changed)}; "
      f"default strip changed in {len(default_changed)}; errors {len(errors)}")
for c in changed:
    print(f"  {c['file']}: stripped {c['stripped_before']}->{c['stripped_after']} units {c['units_before']}->{c['units_after']} {c['new_macros']}")
for e in errors[:10]:
    print("  ERR", e)
