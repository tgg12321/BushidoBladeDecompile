"""Send a function's assembly to a local Ollama model for decompilation.

Usage:
    python tools/ask_local.py func_80040304
    python tools/ask_local.py func_80040304 --model bb2-deepseek
    python tools/ask_local.py --list-models
"""
import sys
import json
import urllib.request
import os
import argparse

MODELS = {
    "bb2-decomp":    "Qwen 2.5 Coder 32B (original)",
    "bb2-deepseek":  "DeepSeek Coder V2 16B",
    "bb2-codestral": "Codestral 22B",
}
DEFAULT_MODEL = "bb2-decomp"
OLLAMA_URL = "http://localhost:11434/api/generate"


def main():
    parser = argparse.ArgumentParser(description="Query local model for decompilation")
    parser.add_argument("func", nargs="?", help="Function name (e.g. func_80040304)")
    parser.add_argument("--model", default=DEFAULT_MODEL,
                        help=f"Model to use (choices: {', '.join(MODELS.keys())})")
    parser.add_argument("--list-models", action="store_true", help="List available models")
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

    prompt = (
        "Decompile this MIPS assembly to matching C for GCC 2.7.2 -O2. "
        "Produce ONLY the C function, no explanation.\n\n" + asm
    )

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

    print(f"[{args.model} — {MODELS.get(args.model, 'unknown')}]\n")

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
