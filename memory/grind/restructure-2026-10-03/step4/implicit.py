#!/usr/bin/env python3
"""implicit.py FILE.c... : every implicit function declaration cc1 reports, as sorted
"function<TAB>callee" lines (cpp | cc1 -Wimplicit, the build's flags with -w replaced). The
restructure check that a split keeps each call's declaration state (implicit stays implicit,
prototyped stays prototyped). Run in WSL from the repo root."""
import re
import subprocess
import sys

CPP = ["mipsel-linux-gnu-cpp", "-Iinclude", "-undef", "-Wall", "-lang-c", "-fno-builtin", "-Dmips",
       "-D__GNUC__=2", "-D__OPTIMIZE__", "-D__mips__", "-D__mips", "-Dpsx", "-D__psx__", "-D__psx", "-D_PSYQ",
       "-D__EXTENSIONS__", "-D_MIPSEL", "-D_LANGUAGE_C", "-DLANGUAGE_C"]
CC1 = ["tools/gcc-2.7.2/build/cc1", "-O2", "-G0", "-funsigned-char", "-quiet", "-mcpu=3000", "-mips1",
       "-mno-abicalls", "-fno-builtin", "-Wimplicit", "-mel", "-msoft-float", "-o", "/dev/null"]
out = set()
for f in sys.argv[1:]:
    pre = subprocess.run(CPP + [f], capture_output=True).stdout
    err = subprocess.run(CC1, input=pre, capture_output=True).stderr.decode("utf-8", "replace")
    fn = "<file scope>"
    for line in err.splitlines():
        m = re.search(r"In function `([^']*)'", line)
        if m:
            fn = m.group(1)
            continue
        if "At top level" in line:
            fn = "<file scope>"
        m = re.search(r"implicit declaration of function `([^']*)'", line)
        if m:
            out.add(f"{fn}\t{m.group(1)}")
print("\n".join(sorted(out)))
