"""xbuild: compile a whole-TU copy with an include-override dir (faithful pipeline) and
compare every function against build/src/<stem>.o.

usage (WSL, repo root, venv):
  python3 tmp/func_80023F08/xbuild.py <stem> <src_copy.c> <inc_override_dir> [focus_func]
Prints functions whose exact byte signature differs, and the masked score for focus_func.
"""
import sys
from pathlib import Path

sys.path.insert(0, ".")
from engine import pipeline, score  # noqa: E402
from engine import buildconfig as cfg  # noqa: E402

stem, src, inc = sys.argv[1:4]
focus = sys.argv[4] if len(sys.argv) > 4 else None
out = f"tmp/func_80023F08/xb/{stem}.o"
Path(out).parent.mkdir(parents=True, exist_ok=True)
cmd = pipeline.c_pipeline_cmd(stem, out, {"src_override": src})
cmd = cmd.replace(cfg.CPP_FLAGS, f"-I{inc} " + cfg.CPP_FLAGS, 1)
r = pipeline.sh(cmd, capture_output=True, text=True)
if r.returncode != 0:
    print("BUILD FAILED\n" + r.stderr[-4000:])
    sys.exit(1)
ref = f"build/src/{stem}.o"
rt = score._o_func_table(ref)
bt = score._o_func_table(out)
bad = []
for fn in sorted(rt):
    if fn not in bt:
        bad.append((fn, "missing"))
        continue
    try:
        if score.func_byte_signature(ref, fn) != score.func_byte_signature(out, fn):
            s = score.score_func(out, ref, fn)
            bad.append((fn, s))
    except KeyError as e:
        bad.append((fn, str(e)))
print(f"{stem}: {len(rt)} funcs, {len(bad)} differ")
for fn, s in bad:
    print("  DIFF", fn, s)
if focus and focus in rt:
    print("FOCUS", focus, score.score_func(out, ref, focus))
