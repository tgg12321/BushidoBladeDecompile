#!/usr/bin/env python3
"""Build src/code6cac_tu2.c variants with the engine's exact per-file recipe and compare section bytes.

Variants: base (unchanged), no_100E0 (D_800100E0 definition removed), no_10428 (D_80010428 removed),
scalar_10428 (`const u32 D_80010428 = 0;`). Prints .rodata size + sha1 and .text sha1 per variant."""
import hashlib
import subprocess
import sys
from pathlib import Path

root = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(root))
from engine import pipeline  # noqa: E402

src = (root / "src/code6cac_tu2.c").read_bytes().decode("utf-8")
L_E0 = "const u32 D_800100E0[1] = { 0x00000000 };\n"
L_28 = "const u32 D_80010428[1] = { 0x00000000 };\n"
assert src.count(L_E0) == 1 and src.count(L_28) == 1
variants = {
    "base": src,
    "no_100E0": src.replace(L_E0, ""),
    "no_10428": src.replace(L_28, ""),
    "scalar_10428": src.replace(L_28, "const u32 D_80010428 = 0;\n"),
}
out = root / "tmp/func_80021424/tail"
out.mkdir(parents=True, exist_ok=True)
for name, text in variants.items():
    c = out / f"code6cac_tu2_{name}.c"
    c.write_bytes(text.encode("utf-8"))
    o = out / f"{name}.o"
    cmd = pipeline.c_pipeline_cmd("code6cac_tu2", str(o.relative_to(root)), {"src_override": str(c.relative_to(root))})
    r = subprocess.run(["bash", "-c", cmd], cwd=root, capture_output=True)
    if r.returncode != 0:
        print(name, "BUILD FAILED", r.stderr.decode("utf-8", "replace")[:1500])
        continue
    res = []
    for sec in (".rodata", ".text", ".data"):
        b = out / f"{name}{sec}.bin"
        subprocess.run(["mipsel-linux-gnu-objcopy", "-O", "binary", "-j", sec, str(o), str(b)], check=True)
        d = b.read_bytes()
        res.append(f"{sec} {len(d):#x} {hashlib.sha1(d).hexdigest()[:12]}")
    print(f"{name:14s}", " | ".join(res))
