#!/usr/bin/env python3
"""Self-correcting decompilation agent using local Ollama model.

Iteratively decompiles functions by:
1. Running m2c for an initial skeleton
2. Asking the local model to improve it with file context
3. Compiling and checking for errors -> feeding errors back to model
4. Comparing bytes against target -> feeding diff back to model
5. Optionally running the permuter as a last resort

Run from Git Bash or Windows cmd (NOT WSL) so Ollama at localhost is reachable.
Compilation is delegated to WSL via subprocess.

Usage:
    python tools/local_agent.py --file config.c
    python tools/local_agent.py --func func_8003F274
    python tools/local_agent.py --file config.c --apply --permuter
    python tools/local_agent.py --file config.c --max-attempts 6
"""
import argparse
import json
import os
import re
import subprocess
import sys
import time
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
OLLAMA_URL = os.environ.get("OLLAMA_URL", "http://localhost:11434/api/generate")
MODEL = os.environ.get("OLLAMA_MODEL", "bb2-decomp")
SRC_DIR = ROOT / "src"
ASM_DIR = ROOT / "asm" / "funcs"

DRAFT_DIRS = ["local_drafts", "local_agent_output"]  # checked in order

TYPEDEFS = """\
typedef unsigned char u8;
typedef signed char s8;
typedef unsigned short u16;
typedef signed short s16;
typedef unsigned int u32;
typedef signed int s32;
#define NULL ((void *)0)
"""


def wsl_run(cmd, timeout=30):
    """Run a command in WSL, with the project dir and venv activated.
    Returns (stdout, stderr, returncode)."""
    # Wrap the command in a script that cd's and activates, quoting the path
    shell = (
        'cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile" '
        '&& source .venv/bin/activate '
        f'&& {cmd}'
    )
    try:
        r = subprocess.run(
            ["wsl", "bash", "-c", shell],
            capture_output=True, timeout=timeout
        )
        return r.stdout, r.stderr, r.returncode
    except subprocess.TimeoutExpired:
        return b"", b"Timed out", 1
    except Exception as e:
        return b"", str(e).encode(), 1


def find_stubs(src_file):
    """Find all INCLUDE_ASM stubs in a source file."""
    stubs = []
    fpath = SRC_DIR / src_file
    if not fpath.exists():
        print(f"Error: {fpath} not found")
        return stubs
    with open(fpath) as f:
        for line in f:
            m = re.search(r'INCLUDE_ASM\("asm/funcs",\s*(func_[0-9A-Fa-f]+)\)', line)
            if m:
                stubs.append(m.group(1))
    return stubs


def get_asm(func_name):
    """Read assembly for a function."""
    path = ASM_DIR / f"{func_name}.s"
    if not path.exists():
        return None
    with open(path) as f:
        return f.read()


def get_asm_lines(func_name):
    path = ASM_DIR / f"{func_name}.s"
    if not path.exists():
        return 0
    with open(path) as f:
        return sum(1 for _ in f)


def run_m2c(func_name):
    """Run m2c via WSL."""
    cmd = f'python3 tools/m2c/m2c.py --valid-syntax --target mips-gcc-c asm/funcs/{func_name}.s'
    stdout, stderr, rc = wsl_run(cmd)
    if rc == 0 and stdout.strip():
        return stdout.decode("utf-8", errors="replace").strip()
    return None


def get_file_context(src_file, func_name):
    """Extract externs and nearby decompiled functions from the source file."""
    fpath = SRC_DIR / src_file
    if not fpath.exists():
        return ""
    with open(fpath) as f:
        content = f.read()

    lines = content.split('\n')
    context_lines = []
    for line in lines:
        stripped = line.strip()
        if (stripped.startswith('extern ') or stripped.startswith('#include') or
                stripped.startswith('typedef') or stripped.startswith('#define')):
            context_lines.append(line)

    func_pattern = re.compile(
        r'^(?:void|s32|u32|s16|u16|s8|u8|s16\s*\*|void\s*\*|u8\s*\*|u32\s*\*)\s+'
        r'(func_[0-9A-Fa-f]+)\s*\(',
        re.MULTILINE
    )
    all_funcs = [(m.group(1), m.start()) for m in func_pattern.finditer(content)]

    stub_pattern = re.compile(r'INCLUDE_ASM\("asm/funcs",\s*(func_[0-9A-Fa-f]+)\)')
    stub_positions = {m.group(1): m.start() for m in stub_pattern.finditer(content)}

    target_pos = stub_positions.get(func_name, -1)
    if target_pos < 0:
        return '\n'.join(context_lines)

    nearby = []
    for fname, pos in all_funcs:
        if fname in stub_positions:
            continue
        if abs(pos - target_pos) < 3000:
            end = len(content)
            for fname2, pos2 in all_funcs:
                if pos2 > pos:
                    end = pos2
                    break
            for sname, spos in stub_positions.items():
                if spos > pos and spos < end:
                    end = spos
                    break
            func_text = content[pos:end].strip()
            if len(func_text) < 2000:
                nearby.append(func_text)

    context = '\n'.join(context_lines)
    if nearby:
        context += '\n\n/* Nearby decompiled functions for reference: */\n'
        context += '\n\n'.join(nearby[:3])
    return context


def query_model(prompt, timeout=1800):
    """Send prompt to local Ollama model. Default 30min timeout for large prompts."""
    import urllib.request

    payload = json.dumps({
        "model": MODEL,
        "stream": False,
        "prompt": prompt,
        "options": {"temperature": 0.3, "num_ctx": 8192},
    }).encode("utf-8")

    req = urllib.request.Request(
        OLLAMA_URL,
        data=payload,
        headers={"Content-Type": "application/json"},
    )
    try:
        with urllib.request.urlopen(req, timeout=timeout) as resp:
            data = json.loads(resp.read().decode("utf-8"))
            return clean_response(data.get("response", ""))
    except Exception as e:
        return f"/* ERROR: {e} */"


def clean_response(text):
    """Strip markdown fences and extract C code."""
    text = text.strip()
    text = re.sub(r'^```[a-zA-Z]*\n', '', text)
    text = re.sub(r'\n```\s*$', '', text)
    lines = text.split('\n')
    start = 0
    for i, line in enumerate(lines):
        stripped = line.strip()
        if (re.match(r'^(void|s32|u32|s16|u16|s8|u8|extern|typedef|#)', stripped) or
                stripped.startswith('/*')):
            start = i
            break
    return '\n'.join(lines[start:]).strip()


def compile_check(c_code, func_name):
    """Compile C code and compare against target assembly.

    Writes a temp shell script to avoid quoting/variable-expansion issues,
    then executes it in WSL.

    Returns: (compiled_ok, score, errors, diff_text)
        compiled_ok: True if compilation succeeded
        score: byte-level score (0=match), or -1 if compile failed
        errors: compiler error text if failed
        diff_text: instruction diff if compiled but mismatched
    """
    # Write C code to temp file (LF line endings!)
    # Prepend common.h if the code doesn't already include types
    tmp_c = ROOT / f"_tmp_{func_name}.c"
    with open(tmp_c, 'w', newline='\n') as f:
        if '#include' not in c_code[:200] and 'typedef' not in c_code[:200]:
            f.write('#include "common.h"\n\n')
        f.write(c_code)

    # Write target assembly
    asm_file = ASM_DIR / f"{func_name}.s"
    tmp_target_s = ROOT / f"_tmp_target_{func_name}.s"
    with open(asm_file) as f:
        asm_content = f.read()
    with open(tmp_target_s, 'w', newline='\n') as f:
        f.write(".set noat\n.set noreorder\n.section .text\n")
        f.write(f".global {func_name}\n{func_name}:\n")
        for line in asm_content.split('\n'):
            stripped = line.strip()
            if 'glabel' in stripped or 'endlabel' in stripped:
                continue
            m = re.search(r'/\*[^*]*\*/\s*(.*)', line)
            if m:
                instr = m.group(1).strip()
                if instr:
                    f.write(f"  {instr}\n")
            elif stripped.startswith('.L'):
                f.write(f"{stripped}\n")
            elif stripped.startswith('.'):
                f.write(f"  {stripped}\n")

    # Check for --expand-lb
    expand_lb_flag = ""
    expand_lb_file = ROOT / "expand_lb_funcs.txt"
    if expand_lb_file.exists():
        with open(expand_lb_file) as f:
            if func_name in f.read():
                expand_lb_flag = "--expand-lb"

    # Write a self-contained shell script to avoid quoting hell
    # Key: compile from /tmp/ so .file directive has no spaces (maspsx bug)
    script_path = ROOT / f"_tmp_check_{func_name}.sh"
    wsl_root = "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
    with open(script_path, 'w', newline='\n') as f:
        f.write(f"""#!/bin/bash
# NOTE: no pipefail — maspsx prints warnings to stderr that are non-fatal
ROOT="{wsl_root}"
TMP="/tmp/bb2_agent"
mkdir -p "$TMP"
cd "$ROOT"
source .venv/bin/activate 2>/dev/null

# Copy C file to /tmp (no spaces in path — maspsx .file directive bug)
cp "$ROOT"/_tmp_{func_name}.c "$TMP"/{func_name}.c

# Compile C code through BB2 pipeline
mipsel-linux-gnu-cpp \\
  -I"$ROOT"/include -undef -Wall -lang-c -fno-builtin \\
  -Dmips -D__GNUC__=2 -D__OPTIMIZE__ -D__mips__ -D__mips \\
  -Dpsx -D__psx__ -D__psx -D_PSYQ -D__EXTENSIONS__ \\
  -D_MIPSEL -D_LANGUAGE_C -DLANGUAGE_C -DPERMUTER \\
  "$TMP"/{func_name}.c \\
  | "$ROOT"/tools/gcc-2.7.2/build/cc1 \\
    -O2 -G0 -funsigned-char -quiet -mcpu=3000 -mips1 -mno-abicalls -fno-builtin -w \\
  | python3 "$ROOT"/tools/maspsx/maspsx.py \\
    --expand-div --aspsx-version=2.34 \\
    --sdata-syms="$ROOT"/sdata_syms.txt \\
    --sdata-funcs="$ROOT"/sdata_funcs.txt \\
    --sdata-exclude="$ROOT"/sdata_exclude.txt \\
    {expand_lb_flag} \\
  | mipsel-linux-gnu-as \\
    -I"$ROOT"/include -march=r3000 -mtune=r3000 \\
    -no-pad-sections -O1 -G0 -o "$TMP"/{func_name}.o

if [ ! -f "$TMP"/{func_name}.o ]; then
  echo "COMPILE_FAILED"
  exit 1
fi

# Assemble target
mipsel-linux-gnu-as -march=r3000 -mtune=r3000 \\
  -no-pad-sections -O1 -G0 \\
  -o "$TMP"/target_{func_name}.o \\
  "$ROOT"/_tmp_target_{func_name}.s 2>/dev/null

if [ $? -ne 0 ]; then
  echo "TARGET_ASM_FAILED"
  exit 1
fi

# Compare bytes
COMPILED=$(mipsel-linux-gnu-objcopy -O binary -j .text "$TMP"/{func_name}.o /dev/stdout | xxd -p | tr -d '\\n')
TARGET=$(mipsel-linux-gnu-objcopy -O binary -j .text "$TMP"/target_{func_name}.o /dev/stdout | xxd -p | tr -d '\\n')

if [ "$COMPILED" = "$TARGET" ]; then
  echo "MATCH"
else
  echo "MISMATCH"
  echo "---COMPILED---"
  mipsel-linux-gnu-objdump -d "$TMP"/{func_name}.o
  echo "---TARGET---"
  mipsel-linux-gnu-objdump -d "$TMP"/target_{func_name}.o
fi
""")

    # Execute the script in WSL
    # MSYS_NO_PATHCONV prevents Git Bash from mangling /mnt/c paths
    wsl_script = f"{wsl_root}/_tmp_check_{func_name}.sh"
    env = {**os.environ, "MSYS_NO_PATHCONV": "1", "MSYS2_ARG_CONV_EXCL": "*"}
    try:
        r = subprocess.run(
            ["wsl", "bash", "-c", f"bash '{wsl_script}'"],
            capture_output=True, timeout=120, env=env
        )
        stdout_text = r.stdout.decode("utf-8", errors="replace")
        stderr_text = r.stderr.decode("utf-8", errors="replace")
        rc = r.returncode
    except subprocess.TimeoutExpired:
        stdout_text = ""
        stderr_text = "Compilation timed out"
        rc = 1
    except Exception as e:
        stdout_text = ""
        stderr_text = str(e)
        rc = 1

    # Clean up local temp files
    for suffix in ['.c', '.o', '.s', '.sh']:
        for prefix in [f'_tmp_{func_name}', f'_tmp_target_{func_name}', f'_tmp_check_{func_name}']:
            p = ROOT / f"{prefix}{suffix}"
            if p.exists():
                try:
                    p.unlink()
                except Exception:
                    pass
    # Clean up WSL /tmp files
    try:
        subprocess.run(
            ["wsl", "bash", "-c", f"rm -f /tmp/bb2_agent/{func_name}.* /tmp/bb2_agent/target_{func_name}.*"],
            capture_output=True, timeout=5
        )
    except Exception:
        pass

    if rc != 0 or "COMPILE_FAILED" in stdout_text:
        errors = stderr_text[:2000]
        if not errors.strip():
            errors = stdout_text[:2000]
        return False, -1, errors, ""

    if "MATCH" in stdout_text and "MISMATCH" not in stdout_text:
        return True, 0, "", ""

    if "MISMATCH" in stdout_text:
        diff = parse_objdump_diff(stdout_text)
        score = count_mismatches(stdout_text)
        return True, score, "", diff

    return False, -1, stderr_text[:1000], ""


def parse_objdump_diff(output):
    """Parse the COMPILED vs TARGET objdump sections and produce a diff."""
    compiled_section = ""
    target_section = ""

    if "---COMPILED---" in output and "---TARGET---" in output:
        parts = output.split("---TARGET---")
        compiled_part = parts[0].split("---COMPILED---")[-1] if "---COMPILED---" in parts[0] else ""
        target_part = parts[1] if len(parts) > 1 else ""

        compiled_insns = extract_insns(compiled_part)
        target_insns = extract_insns(target_part)

        diff_lines = []
        max_len = max(len(compiled_insns), len(target_insns))
        mismatches = 0

        for i in range(min(max_len, 80)):
            c = compiled_insns[i] if i < len(compiled_insns) else "<missing>"
            t = target_insns[i] if i < len(target_insns) else "<missing>"
            if c != t:
                diff_lines.append(f"  {i:3d}: GOT  {c}")
                diff_lines.append(f"       WANT {t}")
                mismatches += 1

        header = (f"Size: compiled={len(compiled_insns)} target={len(target_insns)}\n"
                  f"Mismatches: {mismatches}")
        return header + '\n' + '\n'.join(diff_lines[:50])

    return output[:1500]


def extract_insns(dump):
    """Extract instruction lines from objdump output.
    Format: '   0:\\t27bdffb8 \\taddiu\\tsp,sp,-72'
    We want: 'addiu sp,sp,-72' (opcode + operands joined)
    """
    lines = []
    for line in dump.split('\n'):
        line = line.strip()
        if ':' in line and '\t' in line:
            parts = line.split('\t')
            if len(parts) >= 3:
                # parts[0]=addr, parts[1]=hex, parts[2:]=opcode+operands
                insn = ' '.join(p.strip() for p in parts[2:] if p.strip())
                lines.append(insn)
            elif len(parts) >= 2:
                lines.append(parts[1].strip())
    return lines


def count_mismatches(output):
    """Count byte-level mismatches from the objdump diff."""
    if "---COMPILED---" in output and "---TARGET---" in output:
        compiled_insns = extract_insns(output.split("---TARGET---")[0].split("---COMPILED---")[-1])
        target_insns = extract_insns(output.split("---TARGET---")[1])
        score = abs(len(compiled_insns) - len(target_insns)) * 100
        for i in range(min(len(compiled_insns), len(target_insns))):
            if i < len(compiled_insns) and i < len(target_insns):
                if compiled_insns[i] != target_insns[i]:
                    score += 4  # 4 bytes per instruction
        return score
    return 9999


def get_existing_drafts(func_name):
    """Load existing first drafts. Prefers local_agent_output (higher quality) over local_drafts."""
    drafts = []
    for dirname in DRAFT_DIRS:
        path = ROOT / dirname / f"{func_name}.c"
        if path.exists():
            with open(path) as f:
                content = f.read().strip()
            if content and len(content) > 20:
                drafts.append((dirname, content))
    # If we have multiple, keep only the best one (agent output > raw draft)
    # to avoid bloating the prompt
    if len(drafts) > 1:
        drafts = [d for d in drafts if d[0] == "local_agent_output"] or [drafts[0]]
    return drafts


def make_initial_prompt(func_name, asm_text, m2c_output, file_context):
    # Load existing drafts
    drafts = get_existing_drafts(func_name)
    draft_section = ""
    if drafts:
        draft_section = "\n\nHere are previous decompilation drafts for reference. " \
            "They may have useful structural insights but are NOT compilable as-is:\n"
        for source, content in drafts:
            # Trim very long drafts
            trimmed = content[:2500] if len(content) > 2500 else content
            draft_section += f"\n--- Draft from {source} ---\n```c\n{trimmed}\n```\n"

    return f"""Decompile this MIPS assembly to matching C for GCC 2.7.2 -O2.

RULES:
- Output ONLY the C function with extern declarations. No markdown, no explanation.
- Use types: s32, u32, s16, u16, s8, u8 (never int/short/char)
- C89 only: all variable declarations at top of function/block
- NEVER use goto unless absolutely necessary (prefer loops and if/else)
- do {{ }} while() is preferred over while() or for() - GCC 2.7.2 inverts loops
- For globals: use extern declarations (e.g., extern s32 D_XXXXXXXX)
- For function calls: use extern forward declarations
- NEVER use __attribute__, register asm(), or inline assembly
- Variable declaration order affects register allocation - declare in order of first use

Here is the m2c decompiler output as a starting point (it compiles but may not match):
```c
{m2c_output}
```

Here is context from the source file (types, externs, nearby functions):
```c
{file_context[:3000]}
```
{draft_section}
Here is the assembly to decompile:
```
{asm_text}
```

Produce the complete C function with all needed extern declarations. Output ONLY code."""


def make_compile_fix_prompt(func_name, asm_text, bad_code, errors):
    return f"""This C code for {func_name} has compilation errors. Fix them.

ERRORS:
{errors[:1500]}

CURRENT CODE:
```c
{bad_code}
```

ASSEMBLY (for reference):
```
{asm_text[:2000]}
```

Output ONLY the fixed C code with extern declarations. No explanation."""


def make_match_fix_prompt(func_name, asm_text, current_code, diff_text, score):
    return f"""This C code for {func_name} compiles but doesn't match the target assembly.
Score: {score} (0 = perfect match, lower = better)

INSTRUCTION DIFF (GOT vs WANT):
{diff_text[:2000]}

CURRENT CODE:
```c
{current_code}
```

TARGET ASSEMBLY:
```
{asm_text[:2000]}
```

Common fixes for GCC 2.7.2 matching:
- Variable declaration order affects register allocation
- do {{ }} while() vs while() vs for() produce different code
- Unsigned comparisons: (u32)x < N instead of x >= 0 && x < N
- Pointer cast: (type *)((u8 *)ptr + offset) for struct field access
- do {{ }} while(0); at function start is a register allocation trick
- Temp variable for sub-expressions can change register assignment
- sll+addu patterns = array indexing, match the shift width to element size

Adjust the C code to match. Output ONLY the corrected C code. No explanation."""


def make_m2c_compilable(m2c_output):
    code = m2c_output
    code = re.sub(r'\bM2C_UNK\b', 's32', code)
    code = re.sub(
        r'M2C_FIELD\(([^,]+),\s*([^,]+),\s*([^)]+)\)',
        r'*(\2)(((u8*)\1)+\3)',
        code
    )
    return TYPEDEFS + code


def run_permuter(func_name, c_code, src_file, max_iters=2000):
    """Run decomp-permuter as last resort."""
    permuter_dir = ROOT / "permuter" / func_name
    permuter_dir.mkdir(parents=True, exist_ok=True)

    # Set up via WSL
    wsl_run(f'bash tools/permuter_setup.sh {func_name} src/{src_file}', timeout=30)

    # Write base.c
    with open(permuter_dir / "base.c", 'w', newline='\n') as f:
        f.write(c_code)

    # Ensure compile.sh + settings
    compile_sh = permuter_dir / "compile.sh"
    if not compile_sh.exists():
        import shutil
        shutil.copy(ROOT / "tools" / "permuter_compile.sh", compile_sh)
    settings = permuter_dir / "settings.toml"
    if not settings.exists():
        with open(settings, 'w', newline='\n') as f:
            f.write('compiler = "gcc"\n')

    print(f"    Running permuter ({max_iters} iters)...", end=" ", flush=True)
    cmd = (f'python3 tools/decomp-permuter/permuter.py permuter/{func_name} '
           f'-j2 --iterations={max_iters} --stop-on-zero')
    stdout, stderr, rc = wsl_run(cmd, timeout=600)
    output = stdout.decode("utf-8", errors="replace")

    if "score 0" in output.lower() or "found a zero" in output.lower():
        best_file = permuter_dir / "best.c"
        if best_file.exists():
            with open(best_file) as f:
                print("MATCH via permuter!")
                return f.read()

    score_m = re.search(r'best score[:\s]+(\d+)', output, re.IGNORECASE)
    if score_m:
        print(f"best score: {score_m.group(1)}")
    else:
        print("done")
    return None


def process_function(func_name, src_file, max_attempts=4, use_permuter=False):
    """Process a single function. Returns (status, code, score)."""
    asm_text = get_asm(func_name)
    if not asm_text:
        return 'error', None, -1

    asm_lines = get_asm_lines(func_name)
    print(f"\n{'='*60}")
    print(f"  {func_name} ({asm_lines} asm lines, {src_file})")
    print(f"{'='*60}")

    # Step 1: m2c
    print("  [1] Running m2c...", end=" ", flush=True)
    m2c_output = run_m2c(func_name)
    if m2c_output:
        print(f"OK ({len(m2c_output)} chars)")
    else:
        print("failed")
        m2c_output = "/* m2c failed */"

    # Step 2: File context + existing drafts
    file_context = get_file_context(src_file, func_name)
    drafts = get_existing_drafts(func_name)
    if drafts:
        print(f"  [+] Found {len(drafts)} existing draft(s): {', '.join(d[0] for d in drafts)}")

    # Step 3: Query local model
    print("  [2] Querying local model...", end=" ", flush=True)
    t0 = time.time()
    prompt = make_initial_prompt(func_name, asm_text, m2c_output, file_context)
    current_code = query_model(prompt)
    elapsed = time.time() - t0
    print(f"done ({elapsed:.0f}s)")

    if "ERROR" in current_code[:20]:
        print(f"  Model error: {current_code[:100]}")
        print("  Falling back to m2c")
        current_code = make_m2c_compilable(m2c_output)

    best_code = current_code
    best_score = 999999

    # Step 4: Compile -> check -> fix loop
    for attempt in range(max_attempts):
        print(f"  [attempt {attempt+1}/{max_attempts}] ", end="", flush=True)

        compiled_ok, score, errors, diff_text = compile_check(current_code, func_name)

        if not compiled_ok:
            print(f"Compile FAIL")
            for el in errors.strip().split('\n')[:3]:
                if el.strip():
                    print(f"    {el.strip()}")
            if attempt < max_attempts - 1:
                print(f"    Asking model to fix...", end=" ", flush=True)
                t0 = time.time()
                current_code = query_model(
                    make_compile_fix_prompt(func_name, asm_text, current_code, errors))
                print(f"done ({time.time()-t0:.0f}s)")
            continue

        print(f"Compiled OK, score={score}")

        if score < best_score:
            best_score = score
            best_code = current_code

        if score == 0:
            print(f"  *** MATCH! ***")
            return 'match', current_code, 0

        # Show diff preview and ask model to fix
        if attempt < max_attempts - 1 and diff_text:
            for dl in diff_text.split('\n')[:5]:
                print(f"    {dl}")
            print(f"    Asking model to fix...", end=" ", flush=True)
            t0 = time.time()
            current_code = query_model(
                make_match_fix_prompt(func_name, asm_text, current_code, diff_text, score))
            print(f"done ({time.time()-t0:.0f}s)")

    # Step 5: Try raw m2c
    if best_score > 0 and m2c_output and "m2c failed" not in m2c_output:
        print(f"  [fallback] Trying raw m2c...", end=" ", flush=True)
        m2c_compilable = make_m2c_compilable(m2c_output)
        compiled_ok, m2c_score, _, _ = compile_check(m2c_compilable, func_name)
        if compiled_ok:
            print(f"score={m2c_score}")
            if m2c_score == 0:
                print(f"  *** MATCH (m2c direct)! ***")
                return 'match', m2c_compilable, 0
            if m2c_score < best_score:
                best_score = m2c_score
                best_code = m2c_compilable
        else:
            print("compile fail")

    # Step 6: Permuter
    if use_permuter and best_score > 0 and best_score < 10000:
        result = run_permuter(func_name, best_code, src_file)
        if result:
            return 'match', result, 0

    if best_score == 999999 or best_score < 0:
        return 'compile_fail', best_code, best_score
    elif best_score < 500:
        return 'close', best_code, best_score
    else:
        return 'far', best_code, best_score


def apply_match(func_name, src_file, matched_code):
    """Replace INCLUDE_ASM stub with matched C code."""
    fpath = SRC_DIR / src_file
    with open(fpath) as f:
        content = f.read()

    stub = f'INCLUDE_ASM("asm/funcs", {func_name});'
    if stub not in content:
        print(f"  WARNING: stub not found for {func_name}")
        return False

    # Extract function definition
    func_match = re.search(
        r'^((?:void|s32|u32|s16|u16|s8|u8|void\s*\*|s16\s*\*|u32\s*\*|u8\s*\*)\s+'
        + re.escape(func_name) + r'\s*\(.*?\{.*?\n\})',
        matched_code, re.MULTILINE | re.DOTALL
    )
    func_body = func_match.group(1) if func_match else matched_code

    # Add new externs
    new_externs = []
    for line in matched_code.split('\n'):
        stripped = line.strip()
        if stripped.startswith('extern ') and stripped not in content:
            new_externs.append(stripped)

    if new_externs:
        lines = content.split('\n')
        last_extern = 0
        for i, line in enumerate(lines):
            if line.strip().startswith('extern '):
                last_extern = i
        for ext in new_externs:
            lines.insert(last_extern + 1, ext)
            last_extern += 1
        content = '\n'.join(lines)

    content = content.replace(stub, func_body)
    with open(fpath, 'w', newline='\n') as f:
        f.write(content)
    return True


def main():
    parser = argparse.ArgumentParser(description="Local model decompilation agent")
    parser.add_argument("--file", required=True, help="Source file (e.g., config.c)")
    parser.add_argument("--func", help="Single function to process")
    parser.add_argument("--max-attempts", type=int, default=4)
    parser.add_argument("--permuter", action="store_true", help="Permuter fallback")
    parser.add_argument("--apply", action="store_true", help="Auto-apply matches")
    parser.add_argument("--dry-run", action="store_true")
    args = parser.parse_args()

    stubs = [args.func] if args.func else find_stubs(args.file)
    if not stubs:
        print(f"No stubs found in {args.file}")
        return

    print(f"Local Agent v2 -- {args.file}")
    print(f"  Model: {MODEL} @ {OLLAMA_URL}")
    print(f"  Stubs: {len(stubs)}")
    print(f"  Max attempts: {args.max_attempts}, Permuter: {args.permuter}")

    if args.dry_run:
        for func in stubs:
            print(f"  {func} ({get_asm_lines(func)} asm lines)")
        return

    results = []
    for func in stubs:
        status, code, score = process_function(
            func, args.file,
            max_attempts=args.max_attempts,
            use_permuter=args.permuter
        )
        results.append((func, status, score, code))

        out_dir = ROOT / "local_agent_output"
        out_dir.mkdir(exist_ok=True)
        if code:
            with open(out_dir / f"{func}.c", 'w', newline='\n') as f:
                f.write(code + '\n')

        if args.apply and status == 'match' and code:
            print(f"  Applying to {args.file}...", end=" ", flush=True)
            print("done" if apply_match(func, args.file, code) else "FAILED")

    # Summary
    print(f"\n{'='*60}")
    print(f"  RESULTS -- {args.file}")
    print(f"{'='*60}")
    icons = {"match": "+++", "close": " ~ ", "far": " X ", "compile_fail": "!!!", "error": "ERR"}
    for func, status, score, _ in results:
        print(f"  [{icons.get(status, '???')}] {func}: {status} (score={score})")

    matches = sum(1 for _, s, _, _ in results if s == 'match')
    close = sum(1 for _, s, _, _ in results if s == 'close')
    print(f"\n  Matched: {matches}/{len(results)}, Close: {close}, "
          f"Failed: {len(results)-matches-close}")
    print(f"  Output: {ROOT / 'local_agent_output'}/")


if __name__ == "__main__":
    main()
