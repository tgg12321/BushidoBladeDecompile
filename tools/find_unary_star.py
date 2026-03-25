#!/usr/bin/env python3
"""Find specific lines causing 'invalid type argument of unary *' errors."""
import subprocess, sys, os, tempfile, re
from pathlib import Path

PROJECT = Path(__file__).resolve().parent.parent
CC1 = str(PROJECT / "tools/gcc-2.7.2/build/cc1")
M2C = str(PROJECT / "tools/m2c/m2c.py")
CTX = str(PROJECT / "include/m2c_context.h")
CC_FLAGS = "-O2 -G0 -funsigned-char -quiet -mcpu=3000 -mips1 -mno-abicalls -fno-builtin -w".split()
CPP_FLAGS = "-Iinclude -undef -Wall -lang-c -fno-builtin".split()
CPP_DEFS = "-Dmips -D__GNUC__=2 -D__OPTIMIZE__ -D__mips__ -D__mips -Dpsx -D__psx__ -D__psx -D_PSYQ -D__EXTENSIONS__ -D_MIPSEL -D_LANGUAGE_C -DLANGUAGE_C".split()

stubs = []
for path in sorted((PROJECT / "src").glob("*.c")):
    with open(path) as f:
        for line in f:
            m = re.match(r'\s*INCLUDE_ASM\s*\(\s*"[^"]+"\s*,\s*(\w+)\s*\)', line)
            if m:
                stubs.append(m.group(1))

found = 0
patterns = {}
for func in stubs:
    asm_path = PROJECT / "asm/funcs" / (func + ".s")
    if not asm_path.exists():
        continue
    r = subprocess.run([sys.executable, M2C, "-t", "mipsel-gcc-c", "--context", CTX,
           "--valid-syntax", str(asm_path)], capture_output=True, text=True,
           cwd=str(PROJECT), timeout=60)
    if r.returncode != 0:
        continue

    tmp = tempfile.NamedTemporaryFile(mode="w", suffix=".c", delete=False, dir="/tmp")
    tmp.write('#include "common.h"\n#include "m2c_macros.h"\n\n')
    tmp.write(r.stdout)
    tmp.close()

    cpp = subprocess.run(["mipsel-linux-gnu-cpp"] + CPP_FLAGS + CPP_DEFS + [tmp.name],
                       capture_output=True, cwd=str(PROJECT))
    if cpp.returncode != 0:
        os.unlink(tmp.name)
        continue
    cc1 = subprocess.run([CC1] + CC_FLAGS, input=cpp.stdout, capture_output=True, cwd=str(PROJECT))
    os.unlink(tmp.name)

    if cc1.returncode != 0:
        err = cc1.stderr.decode()
        if "invalid type argument" in err:
            for eline in err.split("\n"):
                if "invalid type" in eline:
                    lm = re.search(r":(\d+):", eline)
                    if lm:
                        lineno = int(lm.group(1))
                        mlines = r.stdout.split("\n")
                        idx = lineno - 4  # offset for headers
                        if 0 <= idx < len(mlines):
                            code = mlines[idx].strip()
                            # Normalize to find patterns
                            norm = re.sub(r'\b\w+_\w+\b', 'VAR', code)
                            norm = re.sub(r'0x[0-9A-Fa-f]+', 'HEX', norm)
                            norm = re.sub(r'\d+', 'N', norm)
                            patterns.setdefault(norm[:60], []).append((func, code))
                            found += 1
                    break
    if found >= 40:
        break

print("Patterns causing 'invalid type argument of unary *':\n")
for pat, examples in sorted(patterns.items(), key=lambda x: -len(x[1])):
    print("[%d] %s" % (len(examples), pat))
    for func, code in examples[:3]:
        print("     %s: %s" % (func, code))
    print()
