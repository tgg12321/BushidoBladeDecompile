"""banners2.py FILE... : drop a one-line declaration-section banner whose section is empty: after
blank lines, the next line is another banner or not a declaration (extern / prototype / typedef)."""
import re, sys
BAN = re.compile(r"^/\* (Declarations from .*|Forward declarations|Externs for globals|Extern data declarations|Extern function declarations|Named globals|Functions|Data symbols) \*/$")
DECL = re.compile(r"^(extern\s|typedef\s|static\s[^(]*;|[A-Za-z_][\w \*]*\(.*\)\s*;)")
for p in sys.argv[1:]:
    lines = open(p, encoding="utf-8", newline="").read().split("\n")
    out, i, changed = [], 0, False
    while i < len(lines):
        l = lines[i]
        if BAN.match(l.strip()):
            k = i + 1
            while k < len(lines) and lines[k].strip() == "":
                k += 1
            nxt = lines[k] if k < len(lines) else ""
            if BAN.match(nxt.strip()) or not DECL.match(nxt):
                changed = True
                # drop the banner and the blank lines after it when a blank line precedes it
                if out and out[-1].strip() == "":
                    i = k
                else:
                    i += 1
                continue
            # a kept banner sits directly on its first declaration
            if k > i + 1:
                out.append(l)
                i = k
                changed = True
                continue
        out.append(l)
        i += 1
    if changed:
        open(p, "w", encoding="utf-8", newline="\n").write("\n".join(out))
        print("fixed", p)
