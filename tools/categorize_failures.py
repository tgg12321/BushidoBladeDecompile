#!/usr/bin/env python3
"""Categorize remaining CC1 compile failures from m2c output."""
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

errors = {}
tested = 0
compiled = 0

for func in stubs:
    asm_path = str(PROJECT / "asm/funcs" / (func + ".s"))
    if not os.path.exists(asm_path):
        continue
    cmd = [sys.executable, M2C, "-t", "mipsel-gcc-c", "--context", CTX,
           "--valid-syntax", asm_path]
    try:
        r = subprocess.run(cmd, capture_output=True, text=True, cwd=str(PROJECT), timeout=60)
    except subprocess.TimeoutExpired:
        continue
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
        errors.setdefault("CPP_FAIL", []).append(func)
        tested += 1
        continue

    cc1 = subprocess.run([CC1] + CC_FLAGS, input=cpp.stdout, capture_output=True, cwd=str(PROJECT))
    os.unlink(tmp.name)
    tested += 1

    if cc1.returncode == 0:
        compiled += 1
        continue

    err = cc1.stderr.decode()
    for line in err.split("\n"):
        if any(k in line for k in ["error", "invalid", "parse error", "undeclared"]):
            line = re.sub(r"/tmp/[^:]+:\d+: ", "", line)
            line = re.sub(r"In function [^:]+: *", "", line)
            line = line.strip()[:80]
            errors.setdefault(line, []).append(func)
            break

print("Tested: %d, Compiled: %d, Failed: %d" % (tested, compiled, tested - compiled))
print("\nFailure categories:")
for err, funcs in sorted(errors.items(), key=lambda x: -len(x[1])):
    print("  [%3d] %s" % (len(funcs), err))
