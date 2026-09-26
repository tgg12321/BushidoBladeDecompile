"""Per-value twin + sanctioned-family extenders (brief point 10 / dead-store-fake-exception).

For every test `if ((P ^ Q) >= 0) {` of r11/final_pv.c, add at the top of the
block the test guards (a DIFFERENT basic block from P/Q's write and xor) one of:
  dead   : `P = 0; Q = 0; /* FAKE */`            dead stores (values never read)
  self   : `P = P; Q = Q; /* FAKE */`            self-assignments
  chain  : `P = P + 1 - 1; Q = Q + 1 - 1; /* FAKE */`  (live-looking detour, dead)
  use    : the block's first cross write gets `+ (P - P)`: a LIVE use of P
           routed through a detour that folds to +0 (combine-foldable chain-
           extender shape); tests whose block is `return 1` get nothing.
  usevar : like use, but through a volatile-free copy: `+ (P ^ P)`.
usage: python3 gen_ext.py <in.c> <out.c> <mode>
"""
import re
import sys

inp, out, mode = sys.argv[1:4]
lines = open(inp, newline="").read().split("\n")
res = []
pending = None
deferred = None
for ln in lines:
    if deferred and not re.match(r"^ *s32 \w+;$", ln):
        res.append(deferred[0] + deferred[1])
        deferred = None
    res.append(ln)
    m = re.match(r"^( *)if \(\((\w+) \^ (\w+)\) >= 0\) \{$", ln)
    if m:
        ind, p, q = m.group(1) + "    ", m.group(2), m.group(3)
        if mode == "dead":
            ext = (ind, f"{p} = 0; {q} = 0; /* FAKE */")
        elif mode == "self":
            ext = (ind, f"{p} = {p}; {q} = {q}; /* FAKE */")
        elif mode == "chain":
            ext = (ind, f"{p} = {p} + 1 - 1; {q} = {q} + 1 - 1; /* FAKE */")
        if mode in ("dead", "self", "chain"):
            deferred = ext
        elif mode in ("use", "usevar"):
            pending = (p, q)
        continue
    if pending and re.match(r"^ *cross_\w+ = .*;$", ln) and not ln.strip().startswith("s32"):
        p, q = pending
        op = "-" if mode == "use" else "^"
        res[-1] = ln[:-1] + f" + ({p} {op} {p}) + ({q} {op} {q}); /* FAKE */"
        pending = None
    elif pending and "return 1" in ln:
        pending = None
open(out, "w", newline="\n").write("\n".join(res))
print(out, mode)
