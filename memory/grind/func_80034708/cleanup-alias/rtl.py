# rtl.py <func> <stem> <candidate> <tag>: sandbox-substitute candidate, cpp the copy, dump all RTL passes
import sys, subprocess
from pathlib import Path
ROOT = Path("/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile")
sys.path.insert(0, str(ROOT))
from engine import buildconfig as cfg
func, stem, cand, tag = sys.argv[1:5]
subprocess.run(["python3", "-m", "engine.cli", "sandbox", func, "--disable", "all", "--candidate", cand], cwd=ROOT, check=True, capture_output=True)
src = ROOT / "tmp/sandbox" / func / "src" / (stem + ".c")
print("GP" if stem in cfg.GP_FILES else "G0", "NO_SR" if stem in cfg.NO_SR_FILES else "")
r = subprocess.run(["bash", "-c", "%s %s %s '%s'" % (cfg.CPP, cfg.CPP_FLAGS, cfg.CPP_DEFS, src)], cwd=ROOT, capture_output=True, text=True)
out = ROOT / "tmp/ff-b/rtl_in" ; out.mkdir(exist_ok=True)
(out / (tag + ".i")).write_text(r.stdout)
print(subprocess.run(["python3", "tools/rtl_track/dump.py", tag, "--input", str(out / (tag + ".i"))], cwd=ROOT, capture_output=True, text=True).stdout[:300])
