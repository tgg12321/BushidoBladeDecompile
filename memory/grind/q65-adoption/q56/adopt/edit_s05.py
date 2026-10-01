#!/usr/bin/env python3
"""one-off edits: scope relocate_records to the files a step moved; keep metrics out of step commits."""
def rep(p, a, b):
    s = open(p, encoding="utf-8").read()
    assert s.count(a) == 1, (p, a[:60])
    open(p, "w", newline="\n", encoding="utf-8").write(s.replace(a, b))

p = "tmp/q56/adopt/relocate_records.py"
rep(p, '"""relocate_records.py <tree>: a record', '"""relocate_records.py <tree> <old-stem>...: a record')
rep(p, "function. Touches:", "function AND it names one of the <old-stem> files (the files a step split, merged or cut;\nunrelated pre-existing drift is left alone). Touches:")
rep(p, "R = sys.argv[1]\nos.chdir(R)", "R = sys.argv[1]\nOLD = set(sys.argv[2:])\nos.chdir(R)")
rep(p, "    if not fs or old in fs or len(fs) != 1:", "    if old not in OLD or not fs or old in fs or len(fs) != 1:")
rep(p, "                if m and m.group(1) not in where[tk[0]] and len(where[tk[0]]) == 1 and not os.path.exists(x):",
    "                if m and m.group(1) in OLD and m.group(1) not in where[tk[0]] and len(where[tk[0]]) == 1:")
p = "tmp/q56/adopt/s05_apply.py"
rep(p, 'r = subprocess.run([sys.executable, f"{H}/relocate_records.py", R], capture_output=True, text=True)',
    '# the files step 2 split / merged / cut\nOLD = ["text1a_c", "code6cac_b_tu2", "code6cac_b2_pre", "replay_camera_rob_back_loose2", "config"]\n'
    'r = subprocess.run([sys.executable, f"{H}/relocate_records.py", R] + OLD, capture_output=True, text=True)')
p = "tmp/q56/adopt/step.sh"
rep(p, "git add -A . >/dev/null\ngit -c core.hooksPath=/dev/null commit",
    "git checkout -q -- metrics/events.jsonl 2>/dev/null   # engine commands append metrics; not part of a step\n"
    "git add -A . >/dev/null\ngit -c core.hooksPath=/dev/null commit")
print("ok")
