import sys
hdr = None; buf = []
def flush():
    if hdr and buf: print(hdr); print("\n".join(buf))
for l in sys.stdin:
    l = l.rstrip("\n")
    if l.startswith("== "):
        flush(); hdr = l; buf = []
    elif l.startswith(">"):
        buf.append(l)
    elif "DIFF" in l or "FAIL" in l:
        print(l)
flush()
