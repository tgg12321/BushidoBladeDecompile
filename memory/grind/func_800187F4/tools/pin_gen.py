"""Emit PINNED entries (name, (first,last), sha256, text) for the macros func_800187F4 needs,
cut byte-exact from the pinned inline_o.h 4.3 copy. Run from the repo root."""
import hashlib

HDR = "tmp/f187/hdr/sh_inline_o.h"
raw = open(HDR, "rb").read()
assert hashlib.sha256(raw).hexdigest() == "76f28032e381a78a4c96347eeee753150cfb55b9f0f0be414fd5040bf4c6e47d"
lines = raw.decode("ascii").split("\n")
WANT = [("gte_ldlvl", 104, 109), ("gte_lddp", 144, 147), ("gte_rtv0tr", 451, 455),
        ("gte_sqr0", 646, 650), ("gte_gpf0", 721, 725), ("gte_gpl12", 726, 730),
        ("gte_stlvl", 898, 903)]
for name, a, b in WANT:
    text = "\n".join(lines[a - 1:b]) + "\n"
    assert text.startswith(f"#define {name}("), name
    assert text.rstrip("\n").endswith("}"), name
    sha = hashlib.sha256(text.encode("ascii")).hexdigest()
    print(f'            ("{name}", ({a}, {b}),')
    print(f'             "{sha}",')
    print("             r'''" + text + "'''),")
