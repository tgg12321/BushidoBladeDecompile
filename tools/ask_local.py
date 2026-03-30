"""Send a function's assembly to a local Ollama model for decompilation.

Feeds existing context (named symbols, permuter results, prior drafts) into the prompt.

Usage:
    python tools/ask_local.py func_80040304
    python tools/ask_local.py func_80040304 --model bb2-codestral
    python tools/ask_local.py func_80040304 --no-context
    python tools/ask_local.py --list-models
"""
import sys
import json
import urllib.request
import os
import re
import argparse

MODELS = {
    "bb2-deepseek":  "DeepSeek Coder V2 16B",
    "bb2-codestral": "Codestral 22B",
}
DEFAULT_MODEL = "bb2-deepseek"
OLLAMA_URL = "http://localhost:11434/api/generate"
OUT_DIR = "local_drafts"
PERMUTER_DIR = "permuter"
SYMBOL_ADDRS = "symbol_addrs.txt"


def load_symbol_table():
    globals_by_addr = {}
    funcs_by_addr = {}
    if not os.path.exists(SYMBOL_ADDRS):
        return globals_by_addr, funcs_by_addr
    with open(SYMBOL_ADDRS) as f:
        for line in f:
            line = line.strip()
            if not line or line.startswith("//"):
                continue
            m = re.match(r'(\w+)\s*=\s*(0x[0-9A-Fa-f]+)\s*;', line)
            if not m:
                continue
            name, addr = m.group(1), m.group(2).lower()
            if name.startswith("g_") or name.startswith("D_"):
                globals_by_addr[addr] = name
            else:
                funcs_by_addr[addr] = name
    return globals_by_addr, funcs_by_addr


def resolve_symbols_in_asm(asm, globals_by_addr, funcs_by_addr):
    found = {}
    for m in re.finditer(r'\bD_([0-9A-Fa-f]{8})\b', asm):
        addr = "0x" + m.group(1).lower()
        if addr in globals_by_addr:
            found[m.group(0)] = globals_by_addr[addr]
    for m in re.finditer(r'\bfunc_([0-9A-Fa-f]{8})\b', asm):
        addr = "0x" + m.group(1).lower()
        if addr in funcs_by_addr:
            found[m.group(0)] = funcs_by_addr[addr]
    return found


def get_existing_context(func_name, asm, globals_by_addr, funcs_by_addr):
    context_parts = []

    # Named symbols
    known_syms = resolve_symbols_in_asm(asm, globals_by_addr, funcs_by_addr)
    if known_syms:
        sym_lines = "\n".join(f"  {k} => {v}" for k, v in sorted(known_syms.items()))
        context_parts.append(
            "## Known symbol names (symbol_addrs.txt / Kengo cross-reference):\n"
            "Use these names instead of D_XXXXXXXX / func_XXXXXXXX where shown.\n"
            + sym_lines
        )

    # Best permuter result
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
                f"## Best permuter result (score {best_score}, 0=exact match):\n"
                f"```c\n{perm_code}\n```"
            )

    # Prior local draft
    draft_path = os.path.join(OUT_DIR, f"{func_name}.c")
    if os.path.exists(draft_path):
        with open(draft_path) as f:
            draft = f.read().strip()
        if draft and "ERROR" not in draft:
            context_parts.append(
                "## Previous draft (reference only):\n"
                f"```c\n{draft}\n```"
            )

    return "\n\n".join(context_parts)


def main():
    parser = argparse.ArgumentParser(description="Query local model for decompilation")
    parser.add_argument("func", nargs="?", help="Function name (e.g. func_80040304)")
    parser.add_argument("--model", default=DEFAULT_MODEL,
                        help=f"Model to use (choices: {', '.join(MODELS.keys())})")
    parser.add_argument("--list-models", action="store_true", help="List available models")
    parser.add_argument("--no-context", action="store_true", help="Don't feed prior work into prompt")
    args = parser.parse_args()

    if args.list_models:
        print("Available models:")
        for name, desc in MODELS.items():
            print(f"  {name:<20} {desc}")
        return

    if not args.func:
        parser.print_help()
        sys.exit(1)

    asm_path = os.path.join("asm", "funcs", f"{args.func}.s")
    if not os.path.exists(asm_path):
        print(f"Error: {asm_path} not found")
        sys.exit(1)

    with open(asm_path, "r") as f:
        asm = f.read()

    globals_by_addr, funcs_by_addr = load_symbol_table()

    prompt = (
        "Decompile this MIPS assembly to matching C for GCC 2.7.2 -O2. "
        "Produce ONLY the C function, no explanation.\n\n"
    )

    if not args.no_context:
        context = get_existing_context(args.func, asm, globals_by_addr, funcs_by_addr)
        if context:
            prompt += (
                "I have prior work on this function. Use it to produce a better result.\n\n"
                f"{context}\n\n"
                "## Target assembly:\n"
            )
            tags = []
            syms = resolve_symbols_in_asm(asm, globals_by_addr, funcs_by_addr)
            if syms: tags.append(f"{len(syms)} named syms")
            if os.path.isdir(os.path.join(PERMUTER_DIR, args.func)): tags.append("permuter")
            if os.path.exists(os.path.join(OUT_DIR, f"{args.func}.c")): tags.append("draft")
            print(f"[context: {', '.join(tags)}]")

    prompt += asm

    payload = json.dumps({
        "model": args.model,
        "stream": True,
        "prompt": prompt,
    }).encode("utf-8")

    req = urllib.request.Request(
        OLLAMA_URL,
        data=payload,
        headers={"Content-Type": "application/json"},
    )

    print(f"[{args.model} -- {MODELS.get(args.model, 'unknown')}]\n")

    with urllib.request.urlopen(req, timeout=300) as resp:
        for line in resp:
            line = line.decode("utf-8").strip()
            if not line:
                continue
            try:
                data = json.loads(line)
                token = data.get("response", "")
                sys.stdout.write(token)
                sys.stdout.flush()
            except json.JSONDecodeError:
                pass
    print()


if __name__ == "__main__":
    main()
