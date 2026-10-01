"""build.py [edits flags...]: copy src/text1b.c + include/code6cac.h to dm/s, apply edits.py, build
text1b.o with the faithful pipeline (src_override; the header beside it wins the quote-include), and
compare against build/src/text1b.o: raw section bytes + per-function scores. Run from repo root."""
import shutil
import subprocess
import sys
from pathlib import Path

sys.path.insert(0, ".")
from engine import pipeline, score  # noqa: E402

D = Path("tmp/func_80057E84/dm")  # scratch outputs; edits.py + bodies from memory/grind/func_80057E84/dm
S = D / "s"
S.mkdir(parents=True, exist_ok=True)
shutil.copy("src/text1b.c", S / "text1b.c")
shutil.copy("include/code6cac.h", S / "code6cac.h")
r = subprocess.run([sys.executable, "memory/grind/func_80057E84/dm/edits.py", str(S / "text1b.c"), str(S / "code6cac.h")] + sys.argv[1:],
                   capture_output=True, text=True)
print(r.stdout.strip(), r.stderr.strip()[-2000:])
if r.returncode:
    sys.exit(1)
out = str(D / "text1b.o")
try:
    pipeline.build_c_object("text1b", out, cheat_overrides={"src_override": str(S / "text1b.c")})
except RuntimeError as e:
    print(str(e)[-3000:])
    sys.exit(1)
ref = "build/src/text1b.o"
for sec in (".text", ".data", ".rodata", ".sdata", ".bss"):
    a = subprocess.run(["mipsel-linux-gnu-objcopy", "-O", "binary", "-j", sec, out, "/tmp/e84a.bin"], capture_output=True)
    b = subprocess.run(["mipsel-linux-gnu-objcopy", "-O", "binary", "-j", sec, ref, "/tmp/e84b.bin"], capture_output=True)
    x, y = Path("/tmp/e84a.bin").read_bytes(), Path("/tmp/e84b.bin").read_bytes()
    print(f"{sec}: {'IDENTICAL' if x == y else 'DIFFER'} ({len(x)} vs {len(y)} bytes)")
for f in ("func_800571C0", "func_80057ACC", "func_80057CC8", "func_80057E84", "func_80058580", "func_80055B60"):
    try:
        res = score.score_func(out, ref, f)
        print(f"{f}: score {res.get('score')} insns {res.get('build_insns')}/{res.get('target_insns')}")
    except Exception as e:  # noqa: BLE001
        print(f"{f}: {e}")
