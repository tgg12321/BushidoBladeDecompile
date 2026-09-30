#!/usr/bin/env python3
"""jtmark.py: stdin->stdout filter. Replaces each `.align 3` with `.align 2` (the normal build's
effect) followed by a local marker label JTMARK_<n>, so the object records every site's offset.
Also notes the most recent function label for attribution in the marker name."""
import re, sys
n = 0
fn = "x"
for line in sys.stdin:
    m = re.match(r"^([A-Za-z_][A-Za-z0-9_]*):\s*$", line)
    if m and not m.group(1).startswith("JTMARK"):
        fn = m.group(1)
    if re.match(r"^\s*\.align\s+3\s*$", line):
        keep = len(sys.argv) > 1 and sys.argv[1] == "keep"
        sys.stdout.write("\t.align\t%d\nJTMARK_%d__%s:\n" % (3 if keep else 2, n, fn))
        n += 1
    else:
        sys.stdout.write(line)
