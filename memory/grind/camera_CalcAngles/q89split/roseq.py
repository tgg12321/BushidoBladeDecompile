"""roseq.py <cc1.s>: labels emitted while the current section is .rodata/.rdata, in order, with .align lines."""
import re, sys
sec = ".text"
for line in open(sys.argv[1]):
    s = line.strip()
    m = re.match(r"\.(text|data|rdata|sdata|bss|sbss)\b", s)
    if m:
        sec = "." + m.group(1); continue
    m = re.match(r"\.section\s+([.\w]+)", s)
    if m:
        sec = m.group(1); continue
    if sec in (".rodata", ".rdata"):
        if re.match(r"[A-Za-z_$.][\w$.]*:", s) or s.startswith(".align"):
            print(s)
