"""Batch decompile all remaining INCLUDE_ASM stubs using local Ollama models.

Supports A/B testing across multiple models.

Usage:
    python tools/batch_local_decomp.py                         # default model (bb2-decomp)
    python tools/batch_local_decomp.py --model bb2-deepseek    # specific model
    python tools/batch_local_decomp.py --ab-test               # run all 3 models, compare
    python tools/batch_local_decomp.py --file text1a.c         # one file only
    python tools/batch_local_decomp.py --func func_80040304    # one function only
    python tools/batch_local_decomp.py --dry-run               # list stubs without running

Output:
    Single model:  local_drafts/<func>.c
    A/B test mode: local_drafts/<model>/<func>.c
"""
import argparse
import json
import os
import re
import sys
import time
import urllib.request

OLLAMA_URL = "http://localhost:11434/api/generate"

# Available models — add new ones here
MODELS = {
    "bb2-decomp":    "Qwen 2.5 Coder 32B (original)",
    "bb2-deepseek":  "DeepSeek Coder V2 16B",
    "bb2-codestral": "Codestral 22B",
}
DEFAULT_MODEL = "bb2-decomp"

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


def query_model(func_name, model):
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
        "model": model,
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
    text = re.sub(r'^```[a-zA-Z]*\n', '', text)
    text = re.sub(r'\n```\s*$', '', text)
    return text.strip()


def get_out_dir(model, ab_test):
    """Return output directory for a model."""
    if ab_test:
        return os.path.join(OUT_DIR, model)
    return OUT_DIR


def run_model(model, filtered, ab_test):
    """Run a single model across all filtered stubs. Returns (model, results_dict)."""
    out_dir = get_out_dir(model, ab_test)
    os.makedirs(out_dir, exist_ok=True)

    results = {}  # func_name -> {time, chars, path}
    total = len(filtered)

    print(f"\n{'='*60}")
    print(f"  Model: {model} — {MODELS.get(model, 'unknown')}")
    print(f"  Output: {out_dir}/")
    print(f"  Functions: {total}")
    print(f"{'='*60}\n")

    for i, (src_file, func_name, size) in enumerate(filtered):
        out_path = os.path.join(out_dir, f"{func_name}.c")

        # Skip if already done (per-model)
        if os.path.exists(out_path):
            print(f"[{i+1}/{total}] {func_name} — already done, skipping")
            # Still record it for comparison
            with open(out_path) as f:
                chars = len(f.read())
            results[func_name] = {"time": 0, "chars": chars, "path": out_path, "skipped": True}
            continue

        print(f"[{i+1}/{total}] {func_name} ({size} lines, {src_file})...", end=" ", flush=True)

        t0 = time.time()
        response = query_model(func_name, model)
        elapsed = time.time() - t0

        if response:
            cleaned = clean_response(response)
            with open(out_path, "w") as f:
                f.write(cleaned + "\n")
            print(f"done ({elapsed:.1f}s, {len(cleaned)} chars)")
            results[func_name] = {"time": elapsed, "chars": len(cleaned), "path": out_path, "skipped": False}
        else:
            print(f"FAILED ({elapsed:.1f}s)")
            results[func_name] = {"time": elapsed, "chars": 0, "path": None, "skipped": False}

    return model, results


def print_ab_summary(all_results):
    """Print comparison summary across models."""
    models = list(all_results.keys())
    funcs = set()
    for results in all_results.values():
        funcs.update(results.keys())
    funcs = sorted(funcs)

    print(f"\n{'='*60}")
    print("  A/B TEST SUMMARY")
    print(f"{'='*60}\n")

    # Per-model totals
    print(f"{'Model':<20} {'Functions':>10} {'Avg Time':>10} {'Avg Chars':>10}")
    print("-" * 52)
    for model in models:
        results = all_results[model]
        active = {k: v for k, v in results.items() if not v.get("skipped")}
        if active:
            avg_time = sum(v["time"] for v in active.values()) / len(active)
            avg_chars = sum(v["chars"] for v in active.values()) / len(active)
        else:
            avg_time = avg_chars = 0
        print(f"{model:<20} {len(results):>10} {avg_time:>9.1f}s {avg_chars:>10.0f}")

    # Save comparison CSV
    csv_path = os.path.join(OUT_DIR, "ab_comparison.csv")
    with open(csv_path, "w") as f:
        f.write("function," + ",".join(f"{m}_chars,{m}_time" for m in models) + "\n")
        for func in funcs:
            row = [func]
            for model in models:
                r = all_results[model].get(func, {})
                row.append(str(r.get("chars", "")))
                row.append(f"{r.get('time', 0):.1f}")
            f.write(",".join(row) + "\n")
    print(f"\nDetailed comparison saved to {csv_path}")
    print(f"\nNext step: run  python tools/compare_drafts.py  to compile-test all drafts")


def main():
    parser = argparse.ArgumentParser(description="Batch local model decompilation")
    parser.add_argument("--model", default=DEFAULT_MODEL,
                        help=f"Model to use (choices: {', '.join(MODELS.keys())})")
    parser.add_argument("--ab-test", action="store_true",
                        help="Run ALL models and compare (A/B test)")
    parser.add_argument("--models", nargs="+", metavar="MODEL",
                        help="Specific models for A/B test (default: all)")
    parser.add_argument("--file", help="Only process stubs from this .c file")
    parser.add_argument("--func", help="Only process this one function")
    parser.add_argument("--dry-run", action="store_true", help="List stubs without querying model")
    parser.add_argument("--min-lines", type=int, default=25, help="Skip functions shorter than this (default: 25)")
    parser.add_argument("--max-lines", type=int, default=200, help="Skip functions longer than this (default: 200)")
    args = parser.parse_args()

    os.makedirs(OUT_DIR, exist_ok=True)

    # Determine which models to run
    if args.ab_test:
        models_to_run = args.models if args.models else list(MODELS.keys())
    else:
        models_to_run = [args.model]

    # Validate model names
    for m in models_to_run:
        if m not in MODELS:
            print(f"Warning: '{m}' not in known models list. Will try anyway.")

    # Find stubs
    if args.func:
        stubs = [("manual", args.func)]
    else:
        stubs = find_stubs(args.file)

    # Filter by asm size (don't filter by already-done in ab mode — each model checks independently)
    filtered = []
    skipped_size = 0
    for src_file, func_name in stubs:
        size = get_asm_size(func_name)
        if size < args.min_lines or size > args.max_lines:
            skipped_size += 1
            continue
        filtered.append((src_file, func_name, size))

    print(f"Found {len(stubs)} total stubs")
    print(f"  Skipped {skipped_size} outside {args.min_lines}-{args.max_lines} line range")
    print(f"  Candidates: {len(filtered)}")
    print(f"  Models: {', '.join(models_to_run)}")

    if args.dry_run:
        print()
        for src_file, func_name, size in filtered:
            print(f"  {func_name} ({size} lines) from {src_file}")
        return

    # Run each model
    all_results = {}
    for model in models_to_run:
        model_name, results = run_model(model, filtered, args.ab_test)
        all_results[model_name] = results

    # Print summary
    if args.ab_test and len(models_to_run) > 1:
        print_ab_summary(all_results)
    else:
        model = models_to_run[0]
        out_dir = get_out_dir(model, args.ab_test)
        print(f"\nDone! Drafts saved to {out_dir}/")


if __name__ == "__main__":
    main()
