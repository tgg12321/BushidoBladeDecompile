"""Send a function's assembly to the local bb2-decomp model via Ollama API.

Usage: python tools/ask_local.py func_80040304
"""
import sys
import json
import urllib.request
import os

def main():
    if len(sys.argv) < 2:
        print("Usage: python tools/ask_local.py <func_name>")
        sys.exit(1)

    func = sys.argv[1]
    asm_path = os.path.join("asm", "funcs", f"{func}.s")

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
        "model": "bb2-decomp",
        "stream": True,
        "prompt": prompt,
    }).encode("utf-8")

    req = urllib.request.Request(
        "http://localhost:11434/api/generate",
        data=payload,
        headers={"Content-Type": "application/json"},
    )

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
