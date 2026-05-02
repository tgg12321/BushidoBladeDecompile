"""Batch decompile all remaining INCLUDE_ASM stubs using local Ollama models.

Supports A/B testing across multiple models. Feeds existing context (local drafts,
best permuter results, named symbols from symbol_addrs.txt + Kengo cross-ref) into
the prompt so models can build on prior work.

Usage:
    python tools/batch_local_decomp.py                              # default model (bb2-deepseek)
    python tools/batch_local_decomp.py --model bb2-codestral        # specific model
    python tools/batch_local_decomp.py --ab-test                    # run both models, compare
    python tools/batch_local_decomp.py --file text1a.c              # one file only
    python tools/batch_local_decomp.py --func func_80040304         # one function
    python tools/batch_local_decomp.py --func f1 f2 f3              # multiple functions
    python tools/batch_local_decomp.py --dry-run                    # list stubs without running

Output:
    Single model:  local_drafts/<func>.c
    A/B test mode: local_drafts/<model>/<func>.c
"""
import argparse
import json
import os
import re
import time
import urllib.request

OLLAMA_URL = "http://localhost:11434/api/generate"

MODELS = {
    "bb2-deepseek":  "DeepSeek Coder V2 16B",
    "bb2-codestral": "Codestral 22B",
}
DEFAULT_MODEL = "bb2-deepseek"

SRC_DIR = "src"
ASM_DIR = os.path.join("asm", "funcs")
OUT_DIR = "local_drafts"
PERMUTER_DIR = "permuter"
SYMBOL_ADDRS = "symbol_addrs.txt"


# ---------------------------------------------------------------------------
# Symbol table — parsed once at startup
# ---------------------------------------------------------------------------

def load_symbol_table():
    """Parse symbol_addrs.txt into two dicts: addr->name for globals and funcs."""
    globals_by_addr = {}  # "0x800A3218" -> "g_anim_select"
    funcs_by_addr = {}    # "0x80040304" -> "saSe_Init"

    if not os.path.exists(SYMBOL_ADDRS):
        return globals_by_addr, funcs_by_addr

    with open(SYMBOL_ADDRS) as f:
        for line in f:
            line = line.strip()
            if not line or line.startswith("//"):
                continue
            # Format: name = 0xADDRESS; // comment
            m = re.match(r'(\w+)\s*=\s*(0x[0-9A-Fa-f]+)\s*;', line)
            if not m:
                continue
            name, addr = m.group(1), m.group(2).upper().replace("0X", "0x")
            addr_norm = addr.lower()
            if name.startswith("func_") or name[0].islower() and "_" in name and not name.startswith("g_") and not name.startswith("D_"):
                # Could be a function — check if it looks like a code address
                funcs_by_addr[addr_norm] = name
            else:
                globals_by_addr[addr_norm] = name

    return globals_by_addr, funcs_by_addr


# Loaded once
_GLOBALS_BY_ADDR, _FUNCS_BY_ADDR = None, None

def get_symbol_tables():
    global _GLOBALS_BY_ADDR, _FUNCS_BY_ADDR
    if _GLOBALS_BY_ADDR is None:
        _GLOBALS_BY_ADDR, _FUNCS_BY_ADDR = load_symbol_table()
    return _GLOBALS_BY_ADDR, _FUNCS_BY_ADDR


def resolve_symbols_in_asm(asm):
    """Scan assembly for D_XXXXXXXX and func_XXXXXXXX references, return known names."""
    globals_by_addr, funcs_by_addr = get_symbol_tables()
    found = {}

    # Match D_800XXXXX style global references
    for m in re.finditer(r'\bD_([0-9A-Fa-f]{8})\b', asm):
        addr = "0x" + m.group(1).lower()
        if addr in globals_by_addr:
            found[m.group(0)] = globals_by_addr[addr]

    # Match func_800XXXXX call targets (jal targets)
    for m in re.finditer(r'\bfunc_([0-9A-Fa-f]{8})\b', asm):
        addr = "0x" + m.group(1).lower()
        if addr in funcs_by_addr:
            found[m.group(0)] = funcs_by_addr[addr]

    return found


# ---------------------------------------------------------------------------
# Context gathering
# ---------------------------------------------------------------------------

def get_existing_context(func_name, asm=None):
    """Gather all existing work: local draft, best permuter result, named symbols."""
    context_parts = []

    # 1. Named symbols referenced in this function's assembly
    if asm:
        known_syms = resolve_symbols_in_asm(asm)
        if known_syms:
            sym_lines = "\n".join(f"  {k} => {v}" for k, v in sorted(known_syms.items()))
            context_parts.append(
                "## Known symbol names (from symbol_addrs.txt / Kengo cross-reference):\n"
                "Use these names instead of D_XXXXXXXX / func_XXXXXXXX where shown.\n"
                + sym_lines
            )

    # 2. Best permuter result (highest priority — it's the closest to matching)
    perm_dir = os.path.join(PERMUTER_DIR, func_name)
    if os.path.isdir(perm_dir):
        best_score = None
        best_source = None
        for entry in os.listdir(perm_dir):
            m = re.match(r'output-(\d+)-\d+', entry)
            if m:
                score = int(m.group(1))
                source_path = os.path.join(perm_dir, entry, "source.c")
                if os.path.exists(source_path) and (best_score is None or score < best_score):
                    best_score = score
                    best_source = source_path
        if best_source:
            with open(best_source) as f:
                perm_code = f.read().strip()
            context_parts.append(
                f"## Best permuter result (score {best_score}, lower=better, 0=exact match):\n"
                f"This version is very close. Identify what's still wrong and fix it.\n"
                f"```c\n{perm_code}\n```"
            )

    # 3. Existing local draft (reference)
    draft_path = os.path.join(OUT_DIR, f"{func_name}.c")
    if os.path.exists(draft_path):
        with open(draft_path) as f:
            draft = f.read().strip()
        if draft and "ERROR" not in draft:
            context_parts.append(
                "## Previous decompilation draft (may have issues, use as reference):\n"
                f"```c\n{draft}\n```"
            )

    return "\n\n".join(context_parts)


# ---------------------------------------------------------------------------
# Model querying
# ---------------------------------------------------------------------------

def build_prompt(func_name, asm):
    """Build the full prompt including assembly and any existing context."""
    context = get_existing_context(func_name, asm=asm)

    if context:
        prompt = (
            "Decompile this MIPS assembly to matching C for GCC 2.7.2 -O2. "
            "Output ONLY the C function, no explanation, no markdown.\n\n"
            "I have prior work on this function. Use it to produce a better result — "
            "fix issues, apply named symbols, resolve any remaining differences.\n\n"
            f"{context}\n\n"
            "## Target assembly to match:\n"
            f"{asm}"
        )
    else:
        prompt = (
            "Decompile this MIPS assembly to matching C for GCC 2.7.2 -O2. "
            "Output ONLY the C function, no explanation, no markdown.\n\n"
            f"{asm}"
        )

    return prompt


def query_model(func_name, model):
    """Send assembly + context to local model, return generated C code."""
    asm_path = os.path.join(ASM_DIR, f"{func_name}.s")
    if not os.path.exists(asm_path):
        return None

    with open(asm_path) as f:
        asm = f.read()

    prompt = build_prompt(func_name, asm)

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


# ---------------------------------------------------------------------------
# Stub discovery
# ---------------------------------------------------------------------------

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


# ---------------------------------------------------------------------------
# Batch runner
# ---------------------------------------------------------------------------

def get_out_dir(model, ab_test):
    return os.path.join(OUT_DIR, model) if ab_test else OUT_DIR


def run_model(model, filtered, ab_test):
    """Run a single model across all filtered stubs. Returns (model, results_dict)."""
    out_dir = get_out_dir(model, ab_test)
    os.makedirs(out_dir, exist_ok=True)

    results = {}
    total = len(filtered)

    print(f"\n{'='*60}")
    print(f"  Model: {model} -- {MODELS.get(model, 'unknown')}")
    print(f"  Output: {out_dir}/")
    print(f"  Functions: {total}")
    print(f"{'='*60}\n")

    for i, (src_file, func_name, size) in enumerate(filtered):
        out_path = os.path.join(out_dir, f"{func_name}.c")

        if os.path.exists(out_path):
            print(f"[{i+1}/{total}] {func_name} -- already done, skipping")
            with open(out_path) as f:
                chars = len(f.read())
            results[func_name] = {"time": 0, "chars": chars, "path": out_path, "skipped": True}
            continue

        # Check what context we have
        asm_path = os.path.join(ASM_DIR, f"{func_name}.s")
        asm = open(asm_path).read() if os.path.exists(asm_path) else ""
        ctx_tags = []
        if resolve_symbols_in_asm(asm):
            ctx_tags.append("syms")
        if os.path.isdir(os.path.join(PERMUTER_DIR, func_name)):
            ctx_tags.append("perm")
        if os.path.exists(os.path.join(OUT_DIR, f"{func_name}.c")):
            ctx_tags.append("draft")
        ctx_str = f" [{','.join(ctx_tags)}]" if ctx_tags else ""

        print(f"[{i+1}/{total}] {func_name} ({size} lines){ctx_str}...", end=" ", flush=True)

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
    models = list(all_results.keys())
    funcs = sorted(set(f for r in all_results.values() for f in r))

    print(f"\n{'='*60}")
    print("  A/B TEST SUMMARY")
    print(f"{'='*60}\n")

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
    print(f"\nComparison saved to {csv_path}")
    print(f"Next: python tools/compare_drafts.py  to compile-test all drafts")


# ---------------------------------------------------------------------------
# Main
# ---------------------------------------------------------------------------

def main():
    parser = argparse.ArgumentParser(description="Batch local model decompilation")
    parser.add_argument("--model", default=DEFAULT_MODEL,
                        help=f"Model to use (choices: {', '.join(MODELS.keys())})")
    parser.add_argument("--ab-test", action="store_true",
                        help="Run both models and compare")
    parser.add_argument("--models", nargs="+", metavar="MODEL",
                        help="Specific subset of models for A/B test")
    parser.add_argument("--file", help="Only process stubs from this .c file")
    parser.add_argument("--func", nargs="+", help="Only process these function(s)")
    parser.add_argument("--dry-run", action="store_true", help="List stubs without querying")
    parser.add_argument("--min-lines", type=int, default=25)
    parser.add_argument("--max-lines", type=int, default=200)
    parser.add_argument("--no-context", action="store_true",
                        help="Don't feed prior work / symbols into prompts")
    args = parser.parse_args()

    if args.no_context:
        global get_existing_context
        get_existing_context = lambda func_name, asm=None: ""

    os.makedirs(OUT_DIR, exist_ok=True)

    models_to_run = (args.models or list(MODELS.keys())) if args.ab_test else [args.model]

    for m in models_to_run:
        if m not in MODELS:
            print(f"Warning: '{m}' not in known models list. Will try anyway.")

    stubs = [("manual", f) for f in args.func] if args.func else find_stubs(args.file)

    filtered = []
    skipped_size = 0
    for src_file, func_name in stubs:
        size = get_asm_size(func_name)
        if size < args.min_lines or size > args.max_lines:
            skipped_size += 1
            continue
        filtered.append((src_file, func_name, size))

    # Preload symbol table and report
    globals_by_addr, funcs_by_addr = get_symbol_tables()
    print(f"Loaded {len(globals_by_addr)} globals + {len(funcs_by_addr)} named funcs from symbol_addrs.txt")
    print(f"Found {len(stubs)} total stubs")
    print(f"  Skipped {skipped_size} outside {args.min_lines}-{args.max_lines} line range")
    print(f"  Candidates: {len(filtered)}")
    print(f"  Models: {', '.join(models_to_run)}")
    print(f"  Context: {'disabled' if args.no_context else 'symbols + permuter + drafts'}")

    if args.dry_run:
        print()
        for src_file, func_name, size in filtered:
            asm_path = os.path.join(ASM_DIR, f"{func_name}.s")
            asm = open(asm_path).read() if os.path.exists(asm_path) else ""
            syms = resolve_symbols_in_asm(asm)
            has_perm = os.path.isdir(os.path.join(PERMUTER_DIR, func_name))
            has_draft = os.path.exists(os.path.join(OUT_DIR, f"{func_name}.c"))
            tags = []
            if syms: tags.append(f"{len(syms)} syms")
            if has_perm: tags.append("perm")
            if has_draft: tags.append("draft")
            tag_str = f" [{', '.join(tags)}]" if tags else ""
            print(f"  {func_name} ({size} lines){tag_str}")
        return

    all_results = {}
    for model in models_to_run:
        model_name, results = run_model(model, filtered, args.ab_test)
        all_results[model_name] = results

    if args.ab_test and len(models_to_run) > 1:
        print_ab_summary(all_results)
    else:
        print(f"\nDone! Drafts saved to {get_out_dir(models_to_run[0], args.ab_test)}/")


if __name__ == "__main__":
    main()
