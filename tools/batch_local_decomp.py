"""Batch decompile all remaining INCLUDE_ASM stubs using the local bb2-decomp model.

Usage:
    python tools/batch_local_decomp.py                    # all remaining stubs
    python tools/batch_local_decomp.py --file text1a.c    # one file only
    python tools/batch_local_decomp.py --func func_80040304  # one function only
    python tools/batch_local_decomp.py --dry-run           # list stubs without running

Output goes to local_drafts/<func_name>.c
"""
import argparse
import json
import os
import re
import sys
import time
import urllib.request

OLLAMA_URL = "http://localhost:11434/api/generate"
MODEL = "bb2-decomp"
SRC_DIR = "src"
ASM_DIR = os.path.join("asm", "funcs")
OUT_DIR = "local_drafts"


def find_stubs(src_filter=None):
    """Find all INCLUDE_ASM stubs across src/ files."""
    stubs = []
    for fname in sorted(os.listdir(SRC_DIR)):
        if not fname.endswith(".c"):
            continue
        if src_filter and fname != src_filter:
            continue
        fpath = os.path.join(SRC_DIR, fname)
        with open(fpath) as f:
            for line in f:
                m = re.search(r'INCLUDE_ASM\("asm/funcs",\s*(func_[0-9A-Fa-f]+)\)', line)
                if m:
                    stubs.append((fname, m.group(1)))
    return stubs


def get_asm_size(func_name):
    """Return line count of assembly file."""
    path = os.path.join(ASM_DIR, f"{func_name}.s")
    if not os.path.exists(path):
        return 0
    with open(path) as f:
        return sum(1 for _ in f)


def query_model(func_name):
    """Send assembly to local model, return generated C code."""
    asm_path = os.path.join(ASM_DIR, f"{func_name}.s")
    if not os.path.exists(asm_path):
        return None

    with open(asm_path) as f:
        asm = f.read()

    prompt = (
        "Decompile this MIPS assembly to matching C for GCC 2.7.2 -O2. "
        "Output ONLY the C function, no explanation, no markdown.\n\n" + asm
    )

    payload = json.dumps({
        "model": MODEL,
        "stream": False,
        "prompt": prompt,
    }).encode("utf-8")

    req = urllib.request.Request(
        OLLAMA_URL,
        data=payload,
        headers={"Content-Type": "application/json"},
    )

    try:
        with urllib.request.urlopen(req, timeout=600) as resp:
            data = json.loads(resp.read().decode("utf-8"))
            return data.get("response", "")
    except Exception as e:
        return f"/* ERROR: {e} */"


def clean_response(text):
    """Strip markdown fences and leading/trailing whitespace."""
    text = text.strip()
    # Remove ```c ... ``` wrappers
    text = re.sub(r'^```[a-zA-Z]*\n', '', text)
    text = re.sub(r'\n```\s*$', '', text)
    return text.strip()


def main():
    parser = argparse.ArgumentParser(description="Batch local model decompilation")
    parser.add_argument("--file", help="Only process stubs from this .c file")
    parser.add_argument("--func", help="Only process this one function")
    parser.add_argument("--dry-run", action="store_true", help="List stubs without querying model")
    parser.add_argument("--min-lines", type=int, default=25, help="Skip functions shorter than this (default: 25)")
    parser.add_argument("--max-lines", type=int, default=200, help="Skip functions longer than this (default: 200)")
    args = parser.parse_args()

    os.makedirs(OUT_DIR, exist_ok=True)

    if args.func:
        stubs = [("manual", args.func)]
    else:
        stubs = find_stubs(args.file)

    # Filter by asm size and skip already-done
    filtered = []
    skipped_size = 0
    skipped_done = 0
    for src_file, func_name in stubs:
        out_path = os.path.join(OUT_DIR, f"{func_name}.c")
        if os.path.exists(out_path):
            skipped_done += 1
            continue
        size = get_asm_size(func_name)
        if size < args.min_lines or size > args.max_lines:
            skipped_size += 1
            continue
        filtered.append((src_file, func_name, size))

    print(f"Found {len(stubs)} total stubs")
    print(f"  Skipped {skipped_done} already done")
    print(f"  Skipped {skipped_size} outside {args.min_lines}-{args.max_lines} line range")
    print(f"  Processing {len(filtered)} stubs")
    print()

    if args.dry_run:
        for src_file, func_name, size in filtered:
            print(f"  {func_name} ({size} lines) from {src_file}")
        return

    total = len(filtered)
    for i, (src_file, func_name, size) in enumerate(filtered):
        out_path = os.path.join(OUT_DIR, f"{func_name}.c")
        print(f"[{i+1}/{total}] {func_name} ({size} lines, {src_file})...", end=" ", flush=True)

        t0 = time.time()
        response = query_model(func_name)
        elapsed = time.time() - t0

        if response:
            cleaned = clean_response(response)
            with open(out_path, "w") as f:
                f.write(cleaned + "\n")
            print(f"done ({elapsed:.1f}s, {len(cleaned)} chars)")
        else:
            print(f"FAILED ({elapsed:.1f}s)")

    print(f"\nDone! Drafts saved to {OUT_DIR}/")


if __name__ == "__main__":
    main()
