#!/usr/bin/env python3
"""
integrate_match.py - Integrate a matched permuter function into its source file.

Usage:
    python3 tools/integrate_match.py <func_name> [--dry-run] [--no-verify]

Reads permuter/<func_name>/base.c, strips permuter boilerplate (typedefs, includes),
replaces the INCLUDE_ASM stub in the source file with the function body, and verifies
the build still matches.
"""

import argparse
import os
import re
import subprocess
import sys

PROJ = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
os.chdir(PROJ)

# Standard typedefs that are already in common.h
STRIP_TYPEDEFS = {
    "typedef unsigned char u8;",
    "typedef signed char s8;",
    "typedef unsigned short u16;",
    "typedef signed short s16;",
    "typedef unsigned int u32;",
    "typedef signed int s32;",
}


def find_source_file(func_name):
    """Find which .c file has the INCLUDE_ASM stub for this function."""
    for fname in sorted(os.listdir("src")):
        if not fname.endswith(".c"):
            continue
        path = os.path.join("src", fname)
        with open(path) as f:
            content = f.read()
        pattern = rf'INCLUDE_ASM\("asm/funcs",\s*{re.escape(func_name)}\);'
        if re.search(pattern, content):
            return path
    return None


def extract_function_body(base_c_path):
    """Extract the function body from base.c, stripping permuter boilerplate."""
    with open(base_c_path) as f:
        content = f.read()

    # Handle files with literal \n instead of actual newlines (single-line files)
    if "\n" not in content and "\\n" in content:
        content = content.replace("\\n", "\n")

    lines = content.splitlines()

    result = []
    in_body = False
    for line in lines:
        stripped = line.strip()

        # Skip blank lines before body starts
        if not in_body and not stripped:
            continue

        # Skip typedefs
        if stripped in STRIP_TYPEDEFS:
            continue

        # Skip #include lines
        if stripped.startswith("#include"):
            continue

        # Skip #define lines (like PERMUTER)
        if stripped.startswith("#define"):
            continue

        # Skip extern declarations
        if stripped.startswith("extern "):
            continue

        # Once we find something that's not boilerplate, we're in the body
        if stripped:
            in_body = True

        if in_body:
            result.append(line)

    # Trim trailing blank lines
    while result and not result[-1].strip():
        result.pop()

    return "\n".join(result)


def extract_externs(base_c_path):
    """Extract extern declarations from base.c that may need adding to source."""
    with open(base_c_path) as f:
        content = f.read()

    if "\n" not in content and "\\n" in content:
        content = content.replace("\\n", "\n")

    lines = content.splitlines()

    externs = []
    for line in lines:
        stripped = line.strip()
        if stripped.startswith("extern "):
            externs.append(stripped)
    return externs


def check_externs_needed(source_path, externs):
    """Check which externs from base.c are missing from the source file.
    Skips externs whose symbol already appears (avoids type conflicts)."""
    with open(source_path) as f:
        source = f.read()

    missing = []
    for ext in externs:
        # Extract the symbol name
        tokens = ext.rstrip(";").split()
        if tokens:
            sym = tokens[-1].rstrip("(").rstrip("[").rstrip("*")
            # Skip if symbol already appears anywhere in source (even with different type)
            # This prevents conflicting type declarations
            if sym and not re.search(r'\b' + re.escape(sym) + r'\b', source):
                missing.append(ext)

    return missing


def check_integrability(base_c_path):
    """Check if a base.c file can be integrated into real source.
    Returns (ok, issues) where issues is a list of blockers."""
    with open(base_c_path) as f:
        content = f.read()
    if "\n" not in content and "\\n" in content:
        content = content.replace("\\n", "\n")

    issues = []

    # M2C_ERROR drops GTE instructions — score is unreliable
    if "M2C_ERROR" in content:
        issues.append("M2C_ERROR: GTE/coprocessor instructions dropped (score unreliable)")

    # .L goto labels are assembler syntax, not valid C in real source context
    if re.search(r'goto\s+\.L', content):
        issues.append(".L goto labels: assembler-style labels won't work in source")

    # sp as a variable — m2c translates stack pointer ops literally
    if re.search(r'(?<![a-zA-Z_])sp(?:\s*[+\-=;)]|\s*\[)', content):
        issues.append("sp variable: raw stack pointer manipulation")

    # M2C_FIELD is technically valid (defined in m2c_macros.h) but
    # the include may not be in every source file
    if "M2C_FIELD" in content:
        issues.append("M2C_FIELD: needs m2c_macros.h (may not be included in source)")

    return (len(issues) == 0, issues)


def integrate(func_name, dry_run=False, no_verify=False):
    """Main integration logic."""
    # 1. Check permuter dir exists and has base.c
    base_c = os.path.join("permuter", func_name, "base.c")
    if not os.path.isfile(base_c):
        print(f"ERROR: {base_c} not found")
        return False

    # 1b. Check integrability (reject false positives)
    ok, issues = check_integrability(base_c)
    if not ok:
        print(f"ERROR: {base_c} is NOT integrable:")
        for issue in issues:
            print(f"  - {issue}")
        return False

    # 2. Verify it's actually score 0
    if not no_verify:
        print(f"[1/4] Verifying score 0 for {func_name}...")
        try:
            r = subprocess.run(["bash", "tools/try_match.sh", func_name],
                              capture_output=True, text=True, timeout=60)
            output = r.stdout + r.stderr
            m = re.search(r"Score:\s*(\d+)", output)
            if not m:
                print(f"ERROR: Could not get score for {func_name}")
                print(output[-500:])
                return False
            score = int(m.group(1))
            if score != 0:
                print(f"ERROR: {func_name} has score {score}, not 0. Skipping.")
                return False
            print(f"  Score: 0 (verified)")
        except subprocess.TimeoutExpired:
            print(f"ERROR: Scoring timed out for {func_name}")
            return False
    else:
        print(f"[1/4] Score verification skipped")

    # 3. Find source file
    print(f"[2/4] Finding source file...")
    source_path = find_source_file(func_name)
    if not source_path:
        print(f"ERROR: No INCLUDE_ASM stub found for {func_name} in src/*.c")
        return False
    print(f"  Source: {source_path}")

    # 4. Extract function body
    print(f"[3/4] Extracting function body from {base_c}...")
    body = extract_function_body(base_c)
    if not body:
        print(f"ERROR: Could not extract function body from {base_c}")
        return False

    externs = extract_externs(base_c)
    missing_externs = check_externs_needed(source_path, externs)

    # 5. Replace INCLUDE_ASM stub with function body
    with open(source_path) as f:
        source = f.read()

    pattern = rf'INCLUDE_ASM\("asm/funcs",\s*{re.escape(func_name)}\);'
    if not re.search(pattern, source):
        print(f"ERROR: INCLUDE_ASM pattern not found in {source_path}")
        return False

    new_source = re.sub(pattern, body, source)

    # Add missing externs near top of file (after last existing extern before first function)
    if missing_externs:
        print(f"  Adding {len(missing_externs)} missing extern(s):")
        for ext in missing_externs:
            print(f"    {ext}")

        lines = new_source.splitlines()
        insert_idx = 0
        for i, line in enumerate(lines):
            if line.strip().startswith("extern "):
                insert_idx = i + 1
            # Stop searching at first function definition
            if re.match(r'^(void|s32|u32|s16|u16|s8|u8|int|unsigned)\s+\w+\s*\(', line):
                break

        for ext in reversed(missing_externs):
            lines.insert(insert_idx, ext)
        new_source = "\n".join(lines)

    if dry_run:
        print(f"\n--- DRY RUN: Would replace INCLUDE_ASM with ---")
        for line in body.splitlines()[:20]:
            print(f"  {line}")
        if body.count("\n") > 20:
            print(f"  ... ({body.count(chr(10))+1} lines total)")
        if missing_externs:
            print(f"  Would add externs: {missing_externs}")
        return True

    # Write the modified source (LF line endings!)
    with open(source_path, "w", newline="\n") as f:
        f.write(new_source)

    print(f"  Replaced INCLUDE_ASM stub in {source_path}")

    # 6. Build and verify
    if not no_verify:
        print(f"[4/4] Building and verifying...")
        try:
            r = subprocess.run(["make"], capture_output=True, text=True, timeout=120)
            output = r.stdout + r.stderr
            if "OK: bb2 matches!" in output:
                print(f"  BUILD OK - matches!")
                return True
            else:
                print(f"  BUILD FAILED!")
                for line in output.splitlines()[-15:]:
                    print(f"    {line}")
                return False
        except subprocess.TimeoutExpired:
            print(f"  BUILD TIMED OUT!")
            return False
    else:
        print(f"[4/4] Build verification skipped")
        return True


def main():
    parser = argparse.ArgumentParser(description="Integrate matched permuter function into source")
    parser.add_argument("func_name", nargs="+", help="Function name(s) to integrate")
    parser.add_argument("--dry-run", action="store_true",
                        help="Show what would be done without modifying files")
    parser.add_argument("--no-verify", action="store_true",
                        help="Skip score and build verification")
    args = parser.parse_args()

    results = {}
    for func in args.func_name:
        print(f"\n{'='*60}")
        print(f"Integrating {func}")
        print(f"{'='*60}")
        ok = integrate(func, dry_run=args.dry_run, no_verify=args.no_verify)
        results[func] = ok
        if not ok and not args.dry_run:
            print(f"\nStopping due to failure on {func}")
            break

    print(f"\n{'='*60}")
    print("Results:")
    for func, ok in results.items():
        status = "OK" if ok else "FAILED"
        print(f"  {func}: {status}")
    print(f"{'='*60}")

    return all(results.values())


if __name__ == "__main__":
    sys.exit(0 if main() else 1)
