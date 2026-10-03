"""Every census src_location must sit on a definition: an INCLUDE_ASM / BIOS_x_FUNCTION / glabel line,
or a line naming the function that (comments removed) does not end in ';' and whose own or next
non-blank line opens the body '{'."""
import csv, re
bad = 0; n = 0
for r in csv.DictReader(open("docs/naming/function-names.csv", encoding="utf-8")):
    loc = r["src_location"]
    if not loc: continue
    f, ln = loc.split("(")[0].rsplit(":", 1)
    L = open(f, encoding="utf-8", errors="replace").read().split("\n")
    i = int(ln) - 1
    line = L[i]
    n += 1
    if "INCLUDE_ASM" in line or re.match(r"^BIOS_[ABC]_FUNCTION\(", line) or "glabel" in line:
        continue
    code = re.sub(r"/\*.*?\*/", "", line).split("//")[0].rstrip()
    j = i
    while "{" not in re.sub(r"/\*.*?\*/", "", L[j]) and j < i + 12 and not L[j].rstrip().endswith(";"):
        j += 1
    if code.endswith(";") or "{" not in L[j]:
        bad += 1; print(r["address"], r["current_name"], loc, line.strip()[:100])
print(n, "rows checked;", bad, "not on a definition")
