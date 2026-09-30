# usage: score.py <func> <dir> -> scores every *.c in dir via engine sandbox --disable all
import sys, os, glob, json, subprocess
func, d = sys.argv[1], sys.argv[2]
ROOT = "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
lines = []
d = os.path.abspath(d)
for f in sorted(glob.glob(os.path.join(d, "*.c"))):
    r = subprocess.run(["python3", "-m", "engine.cli", "sandbox", func, "--disable", "all", "--candidate", f],
                       cwd=ROOT, capture_output=True, text=True)
    try:
        j = json.loads(r.stdout[r.stdout.index("{"):])
        line = f"{os.path.basename(f)[:-2]:28s} score={j.get('score')} build={j.get('build_insns')} target={j.get('target_insns')}"
    except Exception as e:
        line = f"{os.path.basename(f)[:-2]:28s} ERROR rc={r.returncode} {r.stderr.strip()[-400:]} {r.stdout.strip()[-400:]}"
    print(line, flush=True)
    lines.append(line)
open(os.path.join(d, "scores.txt"), "w").write("\n".join(lines) + "\n")
