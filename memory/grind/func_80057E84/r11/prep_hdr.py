"""prep_hdr.py [outdir]: copy include/code6cac.h into outdir (default: the engine sandbox's src
dir for func_80057E84) with PracticeMenuRec's 0x360/0x361 bytes split out of unk_352 -- the
header edit of this landing. A quote-include finds it before -Iinclude. Run from the repo root."""
import sys
from pathlib import Path

src = Path("include/code6cac.h").read_text(encoding="utf-8")
old = "    u8  unk_352[0x362 - 0x352];\n"
new = ("    u8  unk_352[0x360 - 0x352];\n"
       "    u8  unk_360;                   /* polygon index (func_80057ACC) */\n"
       "    u8  unk_361;                   /* vertex index in that polygon (func_80057ACC) */\n")
out = Path(sys.argv[1] if len(sys.argv) > 1 else "tmp/sandbox/func_80057E84/src")
out.mkdir(parents=True, exist_ok=True)
if "cpu_route" in src:  # the data-model landing (A) is on main: nothing to stage
    pass
elif src.count(old) == 1:
    src = src.replace(old, new)
else:
    assert "u8  unk_360;" in src, "anchor missing"  # header edit already applied
(out / "code6cac.h").write_bytes(src.encode("utf-8"))
print("ok")
