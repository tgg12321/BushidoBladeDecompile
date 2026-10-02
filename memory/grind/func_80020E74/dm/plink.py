"""Private full link: build/ objects with the stems in <dir>/obj/ substituted, linked
with a rewritten copy of bb2.ld; prints the EXE SHA1. The tree and build/ are untouched.

usage: python3 tmp/func_80020E74/plink.py <dir> [extra .ld symbol file]   (WSL, repo root)"""
import hashlib, subprocess, sys
from pathlib import Path
sys.path.insert(0, '.')
from engine import buildconfig as cfg

d = Path(sys.argv[1])
objs = sorted((d / "obj").glob("*.o"))
ld = Path("bb2.ld").read_text()
for o in objs:
    key = f"build/src/{o.name}("
    assert key in ld, key
    ld = ld.replace(key, f"{o}(")
(d / "bb2.ld").write_text(ld)
syms = list(cfg.LD_SYM_FILES)
if len(sys.argv) > 2:
    syms = [sys.argv[2] if Path(s).name == Path(sys.argv[2]).name else s for s in syms]
cmd = (f"{cfg.LD} {cfg.LD_FLAGS} -Map {d}/bb2.map -T {d}/bb2.ld "
       + " ".join(f"-T {s}" for s in syms) + f" -o {d}/bb2.elf")
subprocess.run(cmd, shell=True, check=True)
subprocess.run(f"{cfg.OBJCOPY} -O binary -j .main {d}/bb2.elf {d}/bb2.bin", shell=True, check=True)
subprocess.run(f"python3 tools/make_psexe.py {cfg.TARGET_EXE} {d}/bb2.bin {d}/bb2.exe", shell=True, check=True)
h = hashlib.sha1((d / "bb2.exe").read_bytes()).hexdigest()
print("SHA1", h, "MATCH" if h == "62efab4f73f992798c43e8c730aa43baa10bb4fa" else "MISMATCH")
if h != "62efab4f73f992798c43e8c730aa43baa10bb4fa":
    a = Path(cfg.TARGET_EXE).read_bytes(); b = (d / "bb2.exe").read_bytes()
    diffs = [i for i in range(0, min(len(a), len(b)), 4) if a[i:i+4] != b[i:i+4]]
    print("differing words:", len(diffs), "first:", [hex(0x80010000 + i - 0x800) for i in diffs[:8]])
