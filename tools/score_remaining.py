#!/usr/bin/env python3
"""Score remaining INCLUDE_ASM stubs with m2c to find best Wave 13 targets."""
import subprocess, os, re

target_files = [
    "src/main.c", "src/text1a.c", "src/code6cac.c", "src/code6cac_c.c",
    "src/code6cac_c2.c", "src/system.c", "src/sound.c", "src/config.c"
]

funcs_by_file = {}
for src in target_files:
    fname = os.path.basename(src)
    funcs_by_file[fname] = []
    with open(src) as f:
        for line in f:
            m = re.search(r'INCLUDE_ASM\("asm/funcs",\s*(func_[0-9A-Fa-f]+)\)', line)
            if m:
                funcs_by_file[fname].append(m.group(1))

results = []
for src_file, funcs in funcs_by_file.items():
    for func in funcs:
        asm_file = f"asm/funcs/{func}.s"
        if not os.path.exists(asm_file):
            continue
        with open(asm_file) as af:
            asm_lines = [l for l in af.readlines()
                        if l.strip() and not l.strip().startswith("#")
                        and not l.strip().startswith(".")
                        and "glabel" not in l and "nop" not in l]
        size = len(asm_lines)

        try:
            r = subprocess.run(
                ["m2c", "--valid-syntax", "-t", "mips-gcc-c", asm_file],
                capture_output=True, text=True, timeout=30
            )
            compiles = r.returncode == 0 and "INCLUDE_ASM" not in r.stdout and len(r.stdout) > 20
            results.append((src_file, func, size, compiles))
        except Exception:
            results.append((src_file, func, size, False))

compilable = [(f, fn, s) for f, fn, s, c in results if c]

print(f"Total stubs: {len(results)}")
print(f"m2c compilable: {len(compilable)}")
print(f"Non-compilable: {len(results) - len(compilable)}")

print("\n=== Compilable by size (20-80 lines — best targets) ===")
medium = sorted([(f, fn, s) for f, fn, s in compilable if 20 <= s <= 80], key=lambda x: x[2])
for f, fn, s in medium:
    print(f"  {f}: {fn} ({s} lines)")
print(f"({len(medium)} medium-sized compilable)")

print("\n=== All compilable sorted by size ===")
all_sorted = sorted(compilable, key=lambda x: x[2])
for f, fn, s in all_sorted:
    print(f"  {f}: {fn} ({s} lines)")
