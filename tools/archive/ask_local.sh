#!/bin/bash
# Usage: bash tools/ask_local.sh func_80040304
# Sends a function's assembly to the local bb2-decomp model via Ollama API

FUNC="$1"
if [ -z "$FUNC" ]; then
    echo "Usage: bash tools/ask_local.sh <func_name>"
    exit 1
fi

ASM_FILE="asm/funcs/${FUNC}.s"
if [ ! -f "$ASM_FILE" ]; then
    echo "Error: $ASM_FILE not found"
    exit 1
fi

ASM=$(cat "$ASM_FILE")

# Escape for JSON
ASM_JSON=$(python -c "import sys,json; print(json.dumps(sys.stdin.read()))" <<< "$ASM")
# Strip outer quotes since we embed it in a larger string
ASM_JSON=${ASM_JSON:1:-1}

PROMPT="Decompile this MIPS assembly to matching C for GCC 2.7.2 -O2. Produce ONLY the C function, no explanation.\\n\\n${ASM_JSON}"

curl -sN http://localhost:11434/api/generate \
  -d "{\"model\":\"bb2-decomp\",\"stream\":true,\"prompt\":\"${PROMPT}\"}" \
  | python -c "import sys,json
for line in sys.stdin:
    line=line.strip()
    if not line: continue
    try:
        data=json.loads(line)
        print(data.get('response',''),end='',flush=True)
    except: pass
print()"
