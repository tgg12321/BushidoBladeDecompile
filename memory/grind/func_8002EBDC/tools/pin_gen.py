"""Emit PINNED entries (name, (first,last), sha256, text) for gte_ldlv0 / gte_SetRotMatrix (func_8002EBDC cluster;
adapted from memory/grind/func_800187F4/tools/pin_gen.py). HDR = curl of
raw.githubusercontent.com/shdecompilations/silent-hill-decomp/a1f407cb1ed0992997ace33a024e52b47001fdac/include/psyq/inline_o.h,
cut byte-exact from the pinned inline_o.h 4.3 copy. Run from the repo root."""
import hashlib

HDR = "tmp/pinned/inline_o.h"
raw = open(HDR, "rb").read()
assert hashlib.sha256(raw).hexdigest() == "76f28032e381a78a4c96347eeee753150cfb55b9f0f0be414fd5040bf4c6e47d"
lines = raw.decode("ascii").split("\n")
WANT = [("gte_ldlv0", 95, 103), ("gte_SetRotMatrix", 272, 284)]
for name, a, b in WANT:
    text = "\n".join(lines[a - 1:b]) + "\n"
    assert text.startswith(f"#define {name}("), name
    assert text.rstrip("\n").endswith("}"), name
    sha = hashlib.sha256(text.encode("ascii")).hexdigest()
    print(f'            ("{name}", ({a}, {b}),')
    print(f'             "{sha}",')
    print("             r'''" + text + "'''),")
