#!/usr/bin/env python3
"""Brute-force register asm combinations to find the best match.

Finds all `register ... asm("$reg")` declarations in a permuter base.c,
then tries all 2^N combinations of keeping/removing each asm constraint.
Compiles and scores each variant, reporting the best results.

Usage:
    python3 tools/regasm_explorer.py <func_name> [options]

Options:
    --top N       Show top N results (default: 10)
    --debug       Show colored diff for the best result
    --jobs N      Parallel compilation jobs (default: 4)
    --dry-run     Just show what would be tried, don't compile

Example:
    python3 tools/regasm_explorer.py func_80043C7C
    python3 tools/regasm_explorer.py func_80043C7C --top 5 --debug
"""
import argparse
import itertools
import os
import re
import subprocess
import sys
import tempfile
from concurrent.futures import ProcessPoolExecutor, as_completed


def find_project_root():
    """Walk up from script location to find Makefile."""
    d = os.path.dirname(os.path.abspath(__file__))
    while not os.path.exists(os.path.join(d, "Makefile")):
        parent = os.path.dirname(d)
        if parent == d:
            print("ERROR: Cannot find project root", file=sys.stderr)
            sys.exit(2)
        d = parent
    return d


# Pattern matches lines like:
#   register s32 sx asm("t0") = x;
#   register u32 mask_lo asm("a3") = 0x00FFFFFF;
#   register u8 *pp asm("a0") = (u8 *)...;
#   register s32 count asm("v1");
# Captures: (prefix before asm), (the asm("reg") part), (rest of line)
REGASM_PATTERN = re.compile(
    r'^(\s*register\s+\S[\w\s\*]*\w+)\s+(asm\s*\(\s*"[^"]*"\s*\))(.*)',
    re.MULTILINE,
)


def find_regasm_lines(source):
    """Find all register asm declarations. Returns list of (line_idx, match)."""
    results = []
    for i, line in enumerate(source.split("\n")):
        m = REGASM_PATTERN.match(line)
        if m:
            results.append((i, m))
    return results


def generate_variant(source_lines, regasm_info, combo):
    """Generate a source variant.

    combo is a tuple of bools — True means KEEP the asm constraint,
    False means REMOVE it.
    """
    lines = list(source_lines)
    for (line_idx, match), keep in zip(regasm_info, combo):
        if keep:
            continue  # Keep original line
        # Remove the asm("...") part
        prefix = match.group(1)  # e.g. "    register s32 sx"
        rest = match.group(3)    # e.g. " = x;" or ";"
        # Remove 'register' keyword too since it's meaningless without asm
        new_prefix = re.sub(r'\bregister\s+', '', prefix)
        lines[line_idx] = new_prefix + rest
    return "\n".join(lines)


def label_combo(regasm_info, combo):
    """Human-readable label for a combination."""
    parts = []
    for (line_idx, match), keep in zip(regasm_info, combo):
        # Extract variable name and register
        full = match.group(0)
        var_m = re.search(r'(\w+)\s+asm\s*\(\s*"([^"]*)"\s*\)', full)
        if var_m:
            var_name = var_m.group(1)
            reg_name = var_m.group(2)
            if keep:
                parts.append(f"+{var_name}({reg_name})")
            else:
                parts.append(f"-{var_name}({reg_name})")
    return " ".join(parts)


def compile_and_score(root, func_name, source_text, variant_idx):
    """Compile source text, score against target. Returns (score, variant_idx) or (None, variant_idx)."""
    permuter_dir = os.path.join(root, "permuter", func_name)
    target_o = os.path.join(permuter_dir, "target.o")
    compile_sh = os.path.join(permuter_dir, "compile.sh")

    # Write source to temp file
    with tempfile.NamedTemporaryFile(mode="w", suffix=".c", prefix="regasm_", delete=False) as f:
        f.write(source_text)
        src_path = f.name

    # Compile
    out_path = src_path.replace(".c", ".o")
    try:
        result = subprocess.run(
            ["bash", compile_sh, src_path, "-o", out_path],
            capture_output=True,
            text=True,
            timeout=30,
            cwd=root,
        )
        if result.returncode != 0:
            return (None, variant_idx)

        # Score
        score_result = subprocess.run(
            ["python3", os.path.join(root, "tools", "score_match.py"),
             target_o, out_path, "--quiet"],
            capture_output=True,
            text=True,
            timeout=30,
            cwd=root,
        )
        if score_result.returncode == 2:
            return (None, variant_idx)

        score = int(score_result.stdout.strip())
        return (score, variant_idx)
    except (subprocess.TimeoutExpired, ValueError):
        return (None, variant_idx)
    finally:
        for p in [src_path, out_path]:
            try:
                os.unlink(p)
            except OSError:
                pass


def main():
    parser = argparse.ArgumentParser(description="Brute-force register asm combinations")
    parser.add_argument("func_name", help="Function name (must have permuter dir)")
    parser.add_argument("--top", type=int, default=10, help="Show top N results")
    parser.add_argument("--debug", action="store_true", help="Show diff for best result")
    parser.add_argument("--jobs", "-j", type=int, default=4, help="Parallel jobs")
    parser.add_argument("--dry-run", action="store_true", help="Just show combinations")
    args = parser.parse_args()

    root = find_project_root()
    permuter_dir = os.path.join(root, "permuter", args.func_name)

    base_c = os.path.join(permuter_dir, "base.c")
    if not os.path.exists(base_c):
        print(f"ERROR: {base_c} not found", file=sys.stderr)
        sys.exit(2)
    if not os.path.exists(os.path.join(permuter_dir, "target.o")):
        print(f"ERROR: {permuter_dir}/target.o not found", file=sys.stderr)
        sys.exit(2)

    with open(base_c) as f:
        source = f.read()

    source_lines = source.split("\n")
    regasm_info = find_regasm_lines(source)

    if not regasm_info:
        print("No register asm declarations found in base.c")
        sys.exit(1)

    n = len(regasm_info)
    total = 2 ** n

    print(f"Found {n} register asm declarations:")
    for line_idx, match in regasm_info:
        print(f"  Line {line_idx+1}: {match.group(0).strip()}")
    print(f"\nWill try {total} combinations", end="")
    if total > 256:
        print(f" (WARNING: this may take a while)", end="")
    print()

    if args.dry_run:
        for combo in itertools.product([True, False], repeat=n):
            print(f"  {label_combo(regasm_info, combo)}")
        return

    # Generate all variants
    variants = []
    for i, combo in enumerate(itertools.product([True, False], repeat=n)):
        src = generate_variant(source_lines, regasm_info, combo)
        variants.append((src, combo))

    # Compile and score all variants
    print(f"\nCompiling with {args.jobs} parallel jobs...")
    results = []

    with ProcessPoolExecutor(max_workers=args.jobs) as executor:
        futures = {}
        for i, (src, combo) in enumerate(variants):
            f = executor.submit(compile_and_score, root, args.func_name, src, i)
            futures[f] = (i, combo)

        done = 0
        for future in as_completed(futures):
            done += 1
            i, combo = futures[future]
            score, _ = future.result()
            label = label_combo(regasm_info, combo)

            if score is not None:
                results.append((score, label, combo))
                status = f"score {score}"
            else:
                status = "COMPILE FAIL"

            # Progress
            print(f"\r  [{done}/{total}] {status:>20s}  {label}", end="", flush=True)

    print("\n")

    if not results:
        print("All variants failed to compile!")
        sys.exit(2)

    # Sort by score
    results.sort(key=lambda x: x[0])

    # Show top results
    print(f"Top {min(args.top, len(results))} results:")
    print(f"{'Score':>8s}  Combination")
    print(f"{'-----':>8s}  -----------")
    for score, label, combo in results[:args.top]:
        marker = " *** MATCH! ***" if score == 0 else ""
        print(f"{score:>8d}  {label}{marker}")

    # Show the best variant details
    best_score, best_label, best_combo = results[0]
    print(f"\nBest: score {best_score} — {best_label}")

    if best_score == 0:
        print("\n*** PERFECT MATCH FOUND! ***")
        # Write winning source
        winner_path = os.path.join(permuter_dir, "winner.c")
        winning_src = generate_variant(source_lines, regasm_info, best_combo)
        with open(winner_path, "w") as f:
            f.write(winning_src)
        print(f"Winning source saved to: {winner_path}")

    if args.debug:
        # Re-run with debug scoring for the best variant
        print(f"\n--- Debug diff for best variant (score {best_score}) ---\n")
        winning_src = generate_variant(source_lines, regasm_info, best_combo)
        with tempfile.NamedTemporaryFile(mode="w", suffix=".c", prefix="regasm_best_", delete=False) as f:
            f.write(winning_src)
            src_path = f.name
        out_path = src_path.replace(".c", ".o")
        try:
            subprocess.run(
                ["bash", os.path.join(permuter_dir, "compile.sh"), src_path, "-o", out_path],
                capture_output=True, cwd=root,
            )
            subprocess.run(
                ["python3", os.path.join(root, "tools", "score_match.py"),
                 os.path.join(permuter_dir, "target.o"), out_path, "--debug"],
                cwd=root,
            )
        finally:
            for p in [src_path, out_path]:
                try:
                    os.unlink(p)
                except OSError:
                    pass


if __name__ == "__main__":
    main()
