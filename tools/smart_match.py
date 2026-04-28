#!/usr/bin/env python3
"""
Smart-match v2: automated matching for GCC 2.7.2 PS1 decompilation.

Applies many known-working transformations to m2c output, trying each
independently and in combination, to find byte-identical matches.

Strategies (ordered cheapest → most expensive):
 0. Direct m2c output
 1. Pairwise statement/declaration swaps
 2. Cast variations (s16↔u16, s32↔u32, remove casts)
 3. Volatile on pointer and scalar declarations
 4. do-while(0) barriers between statements
 5. Temp variable inlining (replace single-use temps with expressions)
 6. Expression commutation (swap operands of + and *)
 7. Extern type swaps (s8↔u8 → lb↔lbu instruction change)
 8. goto loop conversion (while/do-while → goto)
 9. If/else restructuring (if/else → val=A; if(cond) val=B)
10. Pointer-based global access (anti-CSE pattern)
11. Dummy variable padding (unsigned int _pad = 0)
12. Temp variable introduction (split sub-expressions into temps)
13. Register asm() hints (force variable into specific register)
14. Statement order permutations (independent groups)
15. Combined decl + stmt permutations
16. Multi-strategy combinations (pairs of cheap transforms)

Usage:
  python3 tools/smart_match.py                          # all compilable stubs
  python3 tools/smart_match.py --func func_80043C7C     # single function
  python3 tools/smart_match.py --file text1a.c           # one file only
  python3 tools/smart_match.py --top 50                  # top 50 by score
  python3 tools/smart_match.py --max-score 5000          # only score <= 5000
  python3 tools/smart_match.py --max-insns 300           # only <= 300 instructions
  python3 tools/smart_match.py -j4                       # 4 parallel workers
  python3 tools/smart_match.py --score-only              # show scores, don't match
"""
import subprocess, os, re, sys, csv, tempfile, shutil, itertools, random, math, argparse
from pathlib import Path
from concurrent.futures import ProcessPoolExecutor, as_completed

ROOT = Path(__file__).resolve().parent.parent
M2C = ROOT / "tools" / "m2c" / "m2c.py"
M2C_CONTEXT = ROOT / "include" / "m2c_context.h"
CC1 = ROOT / "tools" / "gcc-2.7.2" / "build" / "cc1"
MASPSX = ROOT / "tools" / "maspsx" / "maspsx.py"

CC_FLAGS = "-O2 -G0 -funsigned-char -quiet -mcpu=3000 -mips1 -mno-abicalls -fno-builtin -w"

TYPEDEFS = """\
typedef unsigned char u8;
typedef signed char s8;
typedef unsigned short u16;
typedef signed short s16;
typedef unsigned int u32;
typedef signed int s32;
typedef unsigned long long u64;
typedef signed long long s64;

typedef struct GameObj {
    u8 field_00; u8 field_01; s16 field_02;
    s16 field_04; s16 field_06; s16 field_08; s16 field_0A;
    s16 field_0C; s16 field_0E; s16 field_10; s16 field_12;
    s16 field_14; s16 field_16; s32 field_18; s32 field_1C;
    s32 field_20; s32 field_24; s32 field_28; s32 field_2C;
    s16 field_30; s16 field_32; s16 field_34; s16 field_36;
    s16 field_38; s16 field_3A; s16 field_3C; s16 field_3E;
    s16 field_40; s16 field_42; s32 field_44; s32 field_48;
    s32 field_4C; s32 field_50; s16 field_54; s16 field_56;
    s32 field_58; s16 field_5C; s16 field_5E; s32 field_60;
    s32 field_64; s32 field_68; s32 field_6C; s32 field_70;
    s32 field_74; s32 field_78; s32 field_7C; s32 field_80;
    s16 field_84; s16 field_86; s16 field_88; s16 field_8A;
    s32 field_8C; s32 field_90; s32 field_94; s32 field_98;
    s32 field_9C; s32 field_A0; s32 field_A4; s32 field_A8;
    s32 field_AC; s32 field_B0; s32 field_B4; s32 field_B8;
    s32 field_BC; s32 field_C0; s32 field_C4; s32 field_C8;
    s32 field_CC; s32 field_D0; s32 field_D4; s32 field_D8;
    s32 field_DC; s32 field_E0; s32 field_E4; s32 field_E8;
    s32 field_EC; s32 field_F0; s32 field_F4; s16 field_F8;
    s16 field_FA; s32 field_FC;
} GameObj;

typedef struct Vec3s16 { s16 x; s16 y; s16 z; } Vec3s16;
typedef struct Vec3s32 { s32 x; s32 y; s32 z; } Vec3s32;
typedef struct Vec2s16 { s16 x; s16 y; } Vec2s16;

extern s16 D_800973FC[];
"""

# ---------------------------------------------------------------------------
# Build helpers
# ---------------------------------------------------------------------------

def run_m2c(func):
    asm_file = ROOT / "asm" / "funcs" / f"{func}.s"
    if not asm_file.exists():
        return None
    try:
        result = subprocess.run(
            [sys.executable, str(M2C), "--valid-syntax", "--target", "mips-gcc-c",
             "--context", str(M2C_CONTEXT), str(asm_file)],
            capture_output=True, text=True, timeout=30
        )
        if result.returncode == 0 and result.stdout.strip():
            return result.stdout
        return None
    except Exception:
        return None


def make_compilable(m2c_output):
    code = m2c_output
    code = re.sub(r'\bM2C_UNK\b', 's32', code)
    code = re.sub(
        r'M2C_FIELD\(([^,]+),\s*([^,]+),\s*([^)]+)\)',
        r'*(\2)(((u8*)\1)+\3)',
        code
    )
    return TYPEDEFS + code


def compile_to_obj(c_code, output_path):
    try:
        cpp_cmd = [
            "mipsel-linux-gnu-cpp",
            f"-I{ROOT}/include", "-undef", "-Wall", "-lang-c", "-fno-builtin",
            "-Dmips", "-D__GNUC__=2", "-D__OPTIMIZE__", "-D__mips__", "-D__mips",
            "-Dpsx", "-D__psx__", "-D__psx", "-D_PSYQ", "-D__EXTENSIONS__",
            "-D_MIPSEL", "-D_LANGUAGE_C", "-DLANGUAGE_C", "-DPERMUTER",
            "-"
        ]
        p_cpp = subprocess.Popen(cpp_cmd, stdin=subprocess.PIPE, stdout=subprocess.PIPE,
                                 stderr=subprocess.DEVNULL, cwd=str(ROOT))
        cc1_cmd = [str(CC1)] + CC_FLAGS.split()
        p_cc1 = subprocess.Popen(cc1_cmd, stdin=p_cpp.stdout, stdout=subprocess.PIPE,
                                 stderr=subprocess.DEVNULL, cwd=str(ROOT))
        p_cpp.stdout.close()
        maspsx_cmd = [
            sys.executable, str(MASPSX),
            "--expand-div", "--aspsx-version=2.34",
            f"--sdata-syms={ROOT}/sdata_syms.txt",
            f"--sdata-funcs={ROOT}/sdata_funcs.txt",
            f"--sdata-exclude={ROOT}/sdata_exclude.txt",
        ]
        p_maspsx = subprocess.Popen(maspsx_cmd, stdin=p_cc1.stdout, stdout=subprocess.PIPE,
                                    stderr=subprocess.DEVNULL, cwd=str(ROOT))
        p_cc1.stdout.close()
        as_cmd = [
            "mipsel-linux-gnu-as",
            f"-I{ROOT}/include", "-march=r3000", "-mtune=r3000",
            "-no-pad-sections", "-O1", "-G0", "-o", str(output_path)
        ]
        p_as = subprocess.Popen(as_cmd, stdin=p_maspsx.stdout, stdout=subprocess.DEVNULL,
                                stderr=subprocess.DEVNULL, cwd=str(ROOT))
        p_maspsx.stdout.close()
        p_cpp.stdin.write(c_code.encode())
        p_cpp.stdin.close()
        p_as.wait(timeout=30)
        return p_as.returncode == 0 and output_path.exists()
    except Exception:
        return False


def build_target_obj(func, output_path):
    asm_file = ROOT / "asm" / "funcs" / f"{func}.s"
    lines = [".set noat\n", ".set noreorder\n", ".section .text\n",
             f".global {func}\n", f"{func}:\n"]
    with open(asm_file) as f:
        for line in f:
            stripped = line.strip()
            if "glabel" in stripped or "endlabel" in stripped:
                continue
            m = re.search(r'/\*[^*]*\*/\s*(.*)', line)
            if m:
                instr = m.group(1).strip()
                if instr:
                    lines.append(f"  {instr}\n")
            elif stripped.startswith(".L"):
                lines.append(line)
            elif stripped.startswith("."):
                lines.append(f"  {stripped}\n")

    target_s = output_path.with_suffix('.s')
    with open(target_s, 'w') as f:
        f.writelines(lines)

    try:
        result = subprocess.run(
            ["mipsel-linux-gnu-as", "-march=r3000", "-mtune=r3000",
             "-no-pad-sections", "-O1", "-G0", "-o", str(output_path), str(target_s)],
            capture_output=True, timeout=10, cwd=str(ROOT)
        )
        return result.returncode == 0
    except Exception:
        return False


def get_obj_bytes(obj_path):
    try:
        result = subprocess.run(
            ["mipsel-linux-gnu-objcopy", "-O", "binary", "-j", ".text",
             str(obj_path), "/dev/stdout"],
            capture_output=True, timeout=10
        )
        if result.returncode == 0:
            return result.stdout
        return None
    except Exception:
        return None


def exact_match(compiled_o, target_o):
    compiled_bytes = get_obj_bytes(compiled_o)
    target_bytes = get_obj_bytes(target_o)
    if compiled_bytes is None or target_bytes is None:
        return False
    return compiled_bytes == target_bytes


def compute_score(compiled_o, target_o):
    """Compute a similarity score (0 = perfect match, higher = worse).

    Counts differing 4-byte words (MIPS instructions).
    Returns (score, total_instructions).
    """
    compiled_bytes = get_obj_bytes(compiled_o)
    target_bytes = get_obj_bytes(target_o)
    if compiled_bytes is None or target_bytes is None:
        return 99999, 0
    if compiled_bytes == target_bytes:
        return 0, len(target_bytes) // 4

    # Pad shorter to same length
    maxlen = max(len(compiled_bytes), len(target_bytes))
    cb = compiled_bytes.ljust(maxlen, b'\x00')
    tb = target_bytes.ljust(maxlen, b'\x00')

    total = maxlen // 4
    diffs = 0
    for i in range(0, maxlen, 4):
        if cb[i:i+4] != tb[i:i+4]:
            diffs += 1
    return diffs, total


def try_variant(func, c_code, tmpdir, target_o):
    compiled_o = Path(tmpdir) / "test.o"
    if compiled_o.exists():
        compiled_o.unlink()
    if not compile_to_obj(c_code, compiled_o):
        return False
    return exact_match(compiled_o, target_o)


def try_variant_scored(func, c_code, tmpdir, target_o):
    """Like try_variant but returns (matched, score, total_insns)."""
    compiled_o = Path(tmpdir) / "test.o"
    if compiled_o.exists():
        compiled_o.unlink()
    if not compile_to_obj(c_code, compiled_o):
        return False, 99999, 0
    score, total = compute_score(compiled_o, target_o)
    return score == 0, score, total


# ---------------------------------------------------------------------------
# Code parsing and manipulation
# ---------------------------------------------------------------------------

def extract_externs(m2c_code):
    lines = m2c_code.strip().split('\n')
    externs = []
    for line in lines:
        stripped = line.strip()
        if stripped.startswith("extern ") or (stripped.endswith(";") and "(" in stripped and not stripped.startswith("{")):
            externs.append(line)
    return "\n".join(externs) + "\n"


def extract_func_only(m2c_code):
    m = re.search(r'^(\w[\w\s\*]+\s+func_\w+\s*\([^)]*\)\s*\{)', m2c_code, re.MULTILINE)
    if not m:
        return m2c_code
    start = m.start()
    depth = 0
    for i in range(start, len(m2c_code)):
        if m2c_code[i] == '{':
            depth += 1
        elif m2c_code[i] == '}':
            depth -= 1
            if depth == 0:
                return m2c_code[start:i+1]
    return m2c_code[start:]


def parse_function(code):
    """Parse function into signature, declarations, and statements."""
    m = re.search(r'\{(.*)\}', code, re.DOTALL)
    if not m:
        return "", [], []

    body = m.group(1)
    lines = body.strip().split('\n')
    decls = []
    stmts = []
    in_decls = True

    TYPE_RE = re.compile(
        r'\s*(volatile\s+)?(s8|u8|s16|u16|s32|u32|void|'
        r's8\s*\*|u8\s*\*|s16\s*\*|u16\s*\*|s32\s*\*|u32\s*\*|void\s*\*)\s+\w+'
    )

    for line in lines:
        stripped = line.strip()
        if not stripped:
            continue
        if in_decls and TYPE_RE.match(stripped):
            decls.append(line)
        else:
            in_decls = False
            stmts.append(line)

    sig_m = re.match(r'(.*?\{)', code, re.DOTALL)
    sig = sig_m.group(1) if sig_m else ""

    return sig, decls, stmts


def rebuild_function(sig, decls, stmts):
    body_lines = decls + [""] + stmts
    return sig + "\n" + "\n".join(body_lines) + "\n}\n"


def get_variables_written(stmt):
    written = set()
    m = re.match(r'\s*(\w+)\s*=', stmt.strip())
    if m:
        written.add(m.group(1))
    m = re.match(r'\s*\*', stmt.strip())
    if m:
        written.add("__mem__")
    return written


def get_variables_read(stmt):
    stripped = stmt.strip()
    if '=' in stripped and not stripped.startswith('*') and not stripped.startswith('if') and not stripped.startswith('return'):
        idx = stripped.index('=')
        if idx > 0 and stripped[idx-1] not in '!<>':
            stripped = stripped[idx+1:]
    idents = set(re.findall(r'\b([a-zA-Z_]\w*)\b', stripped))
    idents -= {'u8', 'u16', 'u32', 's8', 's16', 's32', 'void', 'if', 'else',
               'while', 'for', 'return', 'do', 'switch', 'case', 'break',
               'continue', 'goto', 'sizeof', 'volatile'}
    if '*(' in stripped or '->(' in stripped:
        idents.add("__mem__")
    return idents


def are_independent(stmt_a, stmt_b):
    written_a = get_variables_written(stmt_a)
    written_b = get_variables_written(stmt_b)
    read_a = get_variables_read(stmt_a)
    read_b = get_variables_read(stmt_b)
    if read_a & written_b:
        return False
    if written_a & read_b:
        return False
    if written_a & written_b:
        return False
    return True


def find_swappable_groups(stmts):
    groups = []
    n = len(stmts)
    i = 0
    while i < n:
        group = [i]
        j = i + 1
        while j < n:
            can_add = True
            for k in group:
                if not are_independent(stmts[k], stmts[j]):
                    can_add = False
                    break
            if can_add:
                group.append(j)
                j += 1
            else:
                break
        if len(group) > 1:
            groups.append(group)
        i = j if j > i + 1 else i + 1
    return groups


# ---------------------------------------------------------------------------
# Individual strategies
# ---------------------------------------------------------------------------

def try_direct(func, c_code, tmpdir, target_o):
    """Strategy 0: Direct m2c output."""
    return try_variant(func, c_code, tmpdir, target_o)


def try_pairwise_swaps(func, func_code, externs, tmpdir, target_o):
    """Strategy 1: Try all pairwise swaps of adjacent statements and declarations."""
    sig, decls, stmts = parse_function(func_code)
    if len(stmts) < 2 and len(decls) < 2:
        return None

    for i in range(len(stmts) - 1):
        if are_independent(stmts[i], stmts[i+1]):
            new_stmts = list(stmts)
            new_stmts[i], new_stmts[i+1] = new_stmts[i+1], new_stmts[i]
            code = rebuild_function(sig, decls, new_stmts)
            full = TYPEDEFS + externs + code
            if try_variant(func, full, tmpdir, target_o):
                return full

    for i in range(len(decls) - 1):
        new_decls = list(decls)
        new_decls[i], new_decls[i+1] = new_decls[i+1], new_decls[i]
        code = rebuild_function(sig, new_decls, stmts)
        full = TYPEDEFS + externs + code
        if try_variant(func, full, tmpdir, target_o):
            return full

    return None


def try_cast_variations(func, func_code, externs, tmpdir, target_o):
    """Strategy 2: Try sign/unsign cast swaps and cast removal."""
    sig, decls, stmts = parse_function(func_code)

    cast_swaps = [
        ('(s16)', '(u16)'), ('(u16)', '(s16)'),
        ('(s32)', '(u32)'), ('(u32)', '(s32)'),
        ('(s8)', '(u8)'), ('(u8)', '(s8)'),
    ]

    for i, stmt in enumerate(stmts):
        for old_cast, new_cast in cast_swaps:
            if old_cast in stmt:
                new_stmt = stmt.replace(old_cast, new_cast, 1)
                new_stmts = list(stmts)
                new_stmts[i] = new_stmt
                code = rebuild_function(sig, decls, new_stmts)
                full = TYPEDEFS + externs + code
                if try_variant(func, full, tmpdir, target_o):
                    return full

    # Also try removing casts entirely
    for i, stmt in enumerate(stmts):
        for cast in ['(s16)', '(u16)', '(s32)', '(u32)', '(s8)', '(u8)']:
            if cast in stmt:
                new_stmt = stmt.replace(cast, '', 1)
                new_stmts = list(stmts)
                new_stmts[i] = new_stmt
                code = rebuild_function(sig, decls, new_stmts)
                full = TYPEDEFS + externs + code
                if try_variant(func, full, tmpdir, target_o):
                    return full

    return None


def try_volatile_variations(func, func_code, externs, tmpdir, target_o):
    """Strategy 3: Try adding volatile to declarations."""
    sig, decls, stmts = parse_function(func_code)

    for i, decl in enumerate(decls):
        stripped = decl.strip()
        if 'volatile' in stripped:
            continue
        # Try volatile on pointer declarations
        if '*' in stripped:
            new_decl = decl.replace('*', '* volatile ', 1)
            new_decls = list(decls)
            new_decls[i] = new_decl
            code = rebuild_function(sig, new_decls, stmts)
            full = TYPEDEFS + externs + code
            if try_variant(func, full, tmpdir, target_o):
                return full

    # Also try volatile on non-pointer scalar declarations
    for i, decl in enumerate(decls):
        stripped = decl.strip()
        if 'volatile' in stripped or '*' in stripped:
            continue
        # Insert volatile after the type
        m = re.match(r'(\s*)(s8|u8|s16|u16|s32|u32)\s+', stripped)
        if m:
            new_decl = f"{m.group(1)}volatile {stripped}"
            new_decls = list(decls)
            new_decls[i] = new_decl
            code = rebuild_function(sig, new_decls, stmts)
            full = TYPEDEFS + externs + code
            if try_variant(func, full, tmpdir, target_o):
                return full

    return None


def generate_stmt_permutations(stmts, max_perms=200):
    groups = find_swappable_groups(stmts)
    if not groups:
        return []

    permutations = []
    for group in groups:
        if len(group) < 2 or len(group) > 7:
            continue
        n_perms = math.factorial(len(group))
        if n_perms <= max_perms:
            perms = list(itertools.permutations(group))
        else:
            perms = set()
            perms.add(tuple(group))
            while len(perms) < max_perms:
                p = list(group)
                random.shuffle(p)
                perms.add(tuple(p))
            perms = list(perms)

        for perm in perms:
            if list(perm) == group:
                continue
            new_stmts = list(stmts)
            reordered = [stmts[i] for i in perm]
            for idx, orig_pos in enumerate(group):
                new_stmts[orig_pos] = reordered[idx]
            permutations.append(new_stmts)

    return permutations


def try_stmt_permutations(func, func_code, externs, tmpdir, target_o, max_tries=500):
    """Strategy 4: Permute statement order within swappable groups."""
    sig, decls, stmts = parse_function(func_code)
    if len(stmts) < 2:
        return None

    perm_stmts = generate_stmt_permutations(stmts, max_perms=200)
    tried = 0
    for new_stmts in perm_stmts:
        code = rebuild_function(sig, decls, new_stmts)
        full = TYPEDEFS + externs + code
        if try_variant(func, full, tmpdir, target_o):
            return full
        tried += 1
        if tried >= max_tries:
            break

    return None


def try_combined_permutations(func, func_code, externs, tmpdir, target_o, max_tries=500):
    """Strategy 5: Permute both declarations AND statements together."""
    sig, decls, stmts = parse_function(func_code)
    if len(decls) < 2 and len(stmts) < 2:
        return None

    stmt_perms = generate_stmt_permutations(stmts, max_perms=50)

    if 2 <= len(decls) <= 6:
        n_decl_perms = math.factorial(len(decls))
        if n_decl_perms <= 120:
            decl_perms = list(itertools.permutations(range(len(decls))))
        else:
            decl_perms = set()
            indices = list(range(len(decls)))
            decl_perms.add(tuple(indices))
            while len(decl_perms) < 60:
                p = list(indices)
                random.shuffle(p)
                decl_perms.add(tuple(p))
            decl_perms = list(decl_perms)
    else:
        decl_perms = [tuple(range(len(decls)))]

    tried = 0
    for dperm in decl_perms:
        new_decls = [decls[i] for i in dperm]
        code = rebuild_function(sig, new_decls, stmts)
        full = TYPEDEFS + externs + code
        if try_variant(func, full, tmpdir, target_o):
            return full
        tried += 1
        if tried >= max_tries:
            return None

    for new_stmts in stmt_perms:
        for dperm in decl_perms:
            new_decls = [decls[i] for i in dperm]
            code = rebuild_function(sig, new_decls, new_stmts)
            full = TYPEDEFS + externs + code
            if try_variant(func, full, tmpdir, target_o):
                return full
            tried += 1
            if tried >= max_tries:
                return None

    return None


def try_temp_inlining(func, func_code, externs, tmpdir, target_o):
    """Strategy 6: Inline single-use temporary variables.

    Replaces: s32 temp = expr; ... use(temp) ...
    With:     ... use(expr) ...

    Changes register pressure and allocation.
    """
    sig, decls, stmts = parse_function(func_code)
    if not decls:
        return None

    # Find declarations with initializers
    INIT_RE = re.compile(r'\s*(s8|u8|s16|u16|s32|u32|void\s*\*)\s+(\w+)\s*=\s*(.+);')

    for i, decl in enumerate(decls):
        m = INIT_RE.match(decl.strip())
        if not m:
            continue
        var_type, var_name, init_expr = m.group(1), m.group(2), m.group(3)

        # Count uses in statements
        use_count = 0
        for stmt in stmts:
            use_count += len(re.findall(r'\b' + re.escape(var_name) + r'\b', stmt))

        if use_count != 1:
            continue  # Only inline single-use temps

        # Check the var isn't written again in statements
        written_again = False
        for stmt in stmts:
            if re.search(r'\b' + re.escape(var_name) + r'\s*=', stmt):
                written_again = True
                break
        if written_again:
            continue

        # Try inlining: remove the decl and replace usage with init_expr
        new_decls = [d for j, d in enumerate(decls) if j != i]
        new_stmts = []
        for stmt in stmts:
            new_stmt = re.sub(r'\b' + re.escape(var_name) + r'\b', f'({init_expr})', stmt)
            new_stmts.append(new_stmt)

        code = rebuild_function(sig, new_decls, new_stmts)
        full = TYPEDEFS + externs + code
        if try_variant(func, full, tmpdir, target_o):
            return full

    return None


def try_temp_introduction(func, func_code, externs, tmpdir, target_o):
    """Strategy 7: Introduce temp variables for complex sub-expressions.

    Splits: result = A + B + C;
    Into:   s32 temp = A + B; result = temp + C;
    (and other splits)

    Changes instruction scheduling and register allocation.
    """
    sig, decls, stmts = parse_function(func_code)
    if not stmts:
        return None

    label_counter = [0]

    def fresh_temp():
        label_counter[0] += 1
        return f"_sm_t{label_counter[0]}"

    # Look for assignments with binary operations that could be split
    BINOP_RE = re.compile(r'^(\s*)(\w+)\s*=\s*(.+?)\s*([+\-*])\s*(.+?)\s*;$')

    for i, stmt in enumerate(stmts):
        m = BINOP_RE.match(stmt.strip())
        if not m:
            continue
        indent, lhs, expr_a, op, expr_b = m.groups()
        # Don't split trivial expressions
        if not re.search(r'[+\-*(/]', expr_a) and not re.search(r'[+\-*(/]', expr_b):
            continue

        temp = fresh_temp()

        # Try splitting left operand into temp
        new_decl = f"    s32 {temp};"
        new_stmt1 = f"    {temp} = {expr_a};"
        new_stmt2 = f"    {lhs} = {temp} {op} {expr_b};"
        new_decls = decls + [new_decl]
        new_stmts = list(stmts)
        new_stmts[i:i+1] = [new_stmt1, new_stmt2]
        code = rebuild_function(sig, new_decls, new_stmts)
        full = TYPEDEFS + externs + code
        if try_variant(func, full, tmpdir, target_o):
            return full

        # Try splitting right operand into temp
        temp = fresh_temp()
        new_decl = f"    s32 {temp};"
        new_stmt1 = f"    {temp} = {expr_b};"
        new_stmt2 = f"    {lhs} = {expr_a} {op} {temp};"
        new_decls = decls + [new_decl]
        new_stmts = list(stmts)
        new_stmts[i:i+1] = [new_stmt1, new_stmt2]
        code = rebuild_function(sig, new_decls, new_stmts)
        full = TYPEDEFS + externs + code
        if try_variant(func, full, tmpdir, target_o):
            return full

    return None


def try_dowhile_barriers(func, func_code, externs, tmpdir, target_o):
    """Strategy 8: Insert do-while(0) barriers between statements.

    do { } while(0); acts as a scheduling barrier for GCC 2.7.2,
    forcing different instruction ordering between statements.
    """
    sig, decls, stmts = parse_function(func_code)
    if len(stmts) < 2:
        return None

    barrier = "    do { } while(0);"

    # Try inserting barrier between each pair of adjacent statements
    for i in range(len(stmts) - 1):
        new_stmts = stmts[:i+1] + [barrier] + stmts[i+1:]
        code = rebuild_function(sig, decls, new_stmts)
        full = TYPEDEFS + externs + code
        if try_variant(func, full, tmpdir, target_o):
            return full

    # Try inserting barriers between ALL statements
    new_stmts = []
    for j, stmt in enumerate(stmts):
        new_stmts.append(stmt)
        if j < len(stmts) - 1:
            new_stmts.append(barrier)
    code = rebuild_function(sig, decls, new_stmts)
    full = TYPEDEFS + externs + code
    if try_variant(func, full, tmpdir, target_o):
        return full

    return None


def try_goto_loops(func, func_code, externs, tmpdir, target_o):
    """Strategy 9: Convert while/do-while loops to goto-based equivalents.

    GCC 2.7.2 has a loop optimizer bug where structured loops cause cc1
    to silently drop later __asm__ blocks. goto loops bypass this.
    Also changes register allocation/scheduling in the loop body.
    """
    # Try converting do-while to goto
    # Pattern: do { body } while (cond);
    DO_WHILE_RE = re.compile(
        r'(\s*)do\s*\{(.*?)\}\s*while\s*\((.+?)\)\s*;',
        re.DOTALL
    )

    func_str = func_code
    label_n = [0]

    def do_while_to_goto(m):
        indent = m.group(1)
        body = m.group(2)
        cond = m.group(3)
        label_n[0] += 1
        label = f"loop_{label_n[0]}"
        return f"{indent}{label}:\n{body}{indent}if ({cond}) goto {label};"

    converted = DO_WHILE_RE.sub(do_while_to_goto, func_str)
    if converted != func_str:
        full = TYPEDEFS + externs + converted
        if try_variant(func, full, tmpdir, target_o):
            return full

    # Try converting while loops to goto
    # Pattern: while (cond) { body }
    WHILE_RE = re.compile(
        r'(\s*)while\s*\((.+?)\)\s*\{(.*?)\}',
        re.DOTALL
    )

    def while_to_goto(m):
        indent = m.group(1)
        cond = m.group(2)
        body = m.group(3)
        label_n[0] += 1
        label = f"loop_{label_n[0]}"
        return f"{indent}if ({cond}) {{\n{indent}{label}:\n{body}{indent}if ({cond}) goto {label};\n{indent}}}"

    converted2 = WHILE_RE.sub(while_to_goto, func_str)
    if converted2 != func_str:
        full = TYPEDEFS + externs + converted2
        if try_variant(func, full, tmpdir, target_o):
            return full

    return None


def try_extern_type_swaps(func, func_code, externs, tmpdir, target_o):
    """Strategy 10: Swap s8↔u8 in extern declarations.

    s8 extern → lb instruction (sign-extending load byte)
    u8 extern → lbu instruction (zero-extending load byte)

    Similarly s16↔u16 → lh↔lhu
    """
    extern_lines = externs.strip().split('\n')
    if not extern_lines or extern_lines == ['']:
        return None

    type_swaps = [
        (r'\bs8\b', 'u8', 's8_to_u8'),
        (r'\bu8\b', 's8', 'u8_to_s8'),
        (r'\bs16\b', 'u16', 's16_to_u16'),
        (r'\bu16\b', 's16', 'u16_to_s16'),
    ]

    for i, ext_line in enumerate(extern_lines):
        stripped = ext_line.strip()
        if not stripped.startswith('extern'):
            continue

        for pattern, replacement, name in type_swaps:
            if re.search(pattern, stripped):
                new_line = re.sub(pattern, replacement, stripped, count=1)
                new_externs = list(extern_lines)
                new_externs[i] = new_line
                new_extern_str = '\n'.join(new_externs) + '\n'
                full = TYPEDEFS + new_extern_str + func_code
                if try_variant(func, full, tmpdir, target_o):
                    return full

    return None


def try_expr_commutation(func, func_code, externs, tmpdir, target_o):
    """Strategy 11: Swap operands of commutative operators (+ and *).

    (a + b) → (b + a) changes GCC 2.7.2's instruction ordering because
    the scheduler tiebreaks on source position (INSN_LUID).
    """
    sig, decls, stmts = parse_function(func_code)

    # Match simple binary expressions: expr OP expr
    ADD_RE = re.compile(r'(\([^()]+)\s*\+\s*([^()]+\))')
    MUL_RE = re.compile(r'(\([^()]+)\s*\*\s*([^()]+\))')

    for i, stmt in enumerate(stmts):
        # Try swapping addition operands
        for pattern in [ADD_RE, MUL_RE]:
            m = pattern.search(stmt)
            if m:
                swapped = stmt[:m.start()] + m.group(2).strip() + ' + ' + m.group(1).strip() + stmt[m.end():]
                new_stmts = list(stmts)
                new_stmts[i] = swapped
                code = rebuild_function(sig, decls, new_stmts)
                full = TYPEDEFS + externs + code
                if try_variant(func, full, tmpdir, target_o):
                    return full

        # Simpler approach: swap around + and * at the statement level
        # a + b → b + a for simple identifier expressions
        SIMPLE_ADD = re.compile(r'\b(\w+)\s*\+\s*(\w+)\b')
        for m in SIMPLE_ADD.finditer(stmt):
            if m.group(1) != m.group(2):  # Don't swap identical operands
                swapped = stmt[:m.start()] + m.group(2) + ' + ' + m.group(1) + stmt[m.end():]
                new_stmts = list(stmts)
                new_stmts[i] = swapped
                code = rebuild_function(sig, decls, new_stmts)
                full = TYPEDEFS + externs + code
                if try_variant(func, full, tmpdir, target_o):
                    return full

        SIMPLE_MUL = re.compile(r'\b(\w+)\s*\*\s*(\w+)\b')
        for m in SIMPLE_MUL.finditer(stmt):
            if m.group(1) != m.group(2):
                swapped = stmt[:m.start()] + m.group(2) + ' * ' + m.group(1) + stmt[m.end():]
                new_stmts = list(stmts)
                new_stmts[i] = swapped
                code = rebuild_function(sig, decls, new_stmts)
                full = TYPEDEFS + externs + code
                if try_variant(func, full, tmpdir, target_o):
                    return full

    return None


def try_ifelse_restructure(func, func_code, externs, tmpdir, target_o):
    """Strategy 12: Restructure if/else into assignment + conditional override.

    Converts: if (cond) { x = A; } else { x = B; }
    Into:     x = B; if (cond) { x = A; }

    This changes branch direction and register allocation.
    Also tries the reverse transformation.
    """
    sig, decls, stmts = parse_function(func_code)

    # Pattern: if (cond) { var = A; } else { var = B; }
    IFELSE_RE = re.compile(
        r'(\s*)if\s*\((.+?)\)\s*\{\s*(\w+)\s*=\s*(.+?)\s*;\s*\}\s*else\s*\{\s*\3\s*=\s*(.+?)\s*;\s*\}'
    )

    full_code = '\n'.join(stmts)
    m = IFELSE_RE.search(full_code)
    if m:
        indent, cond, var, val_true, val_false = m.groups()
        # Transform to: var = val_false; if (cond) { var = val_true; }
        replacement = f"{indent}{var} = {val_false};\n{indent}if ({cond}) {{ {var} = {val_true}; }}"
        new_code = full_code[:m.start()] + replacement + full_code[m.end():]
        new_stmts = new_code.split('\n')
        code = rebuild_function(sig, decls, new_stmts)
        full = TYPEDEFS + externs + code
        if try_variant(func, full, tmpdir, target_o):
            return full

        # Also try the swapped version: var = val_true; if (!cond) { var = val_false; }
        replacement2 = f"{indent}{var} = {val_true};\n{indent}if (!({cond})) {{ {var} = {val_false}; }}"
        new_code2 = full_code[:m.start()] + replacement2 + full_code[m.end():]
        new_stmts2 = new_code2.split('\n')
        code2 = rebuild_function(sig, decls, new_stmts2)
        full2 = TYPEDEFS + externs + code2
        if try_variant(func, full2, tmpdir, target_o):
            return full2

    return None


def try_pointer_global_access(func, func_code, externs, tmpdir, target_o):
    """Strategy 13: Convert direct global access to pointer-based access.

    Direct:  GLOBAL = val;
    Pointer: s32 *p = &GLOBAL; *p = val;

    This prevents GCC from caching the global's address, changing the
    instruction pattern (anti-CSE).
    """
    sig, decls, stmts = parse_function(func_code)

    # Find consecutive assignments to fields of the same global/extern
    # Pattern: D_XXXX = ...; or D_XXXX.field = ...;
    GLOBAL_ASSIGN_RE = re.compile(r'\s*(D_[0-9A-Fa-f]+)\s*=')

    # Collect globals that are assigned multiple times
    global_assigns = {}
    for i, stmt in enumerate(stmts):
        m = GLOBAL_ASSIGN_RE.match(stmt.strip())
        if m:
            gname = m.group(1)
            if gname not in global_assigns:
                global_assigns[gname] = []
            global_assigns[gname].append(i)

    for gname, indices in global_assigns.items():
        if len(indices) < 2:
            continue

        # Try pointer-based access for this global
        temp = f"_p_{gname}"
        new_decl = f"    s32 *{temp} = (s32 *)&{gname};"
        new_decls = decls + [new_decl]
        new_stmts = list(stmts)
        for idx in indices:
            stmt = new_stmts[idx]
            new_stmts[idx] = stmt.replace(gname, f'*{temp}', 1)

        code = rebuild_function(sig, new_decls, new_stmts)
        full = TYPEDEFS + externs + code
        if try_variant(func, full, tmpdir, target_o):
            return full

    return None


def try_register_hints(func, func_code, externs, tmpdir, target_o):
    """Strategy 14: Add register asm() hints to variable declarations.

    GCC 2.7.2 allows: register s32 var asm("v0");
    This forces a variable into a specific register, which can fix
    register allocation mismatches.

    Parses target assembly to determine which registers are used for
    function arguments and early stores, then tries hinting.
    """
    sig, decls, stmts = parse_function(func_code)
    if not decls:
        return None

    # Available MIPS registers for hinting
    hint_regs = ["v0", "v1", "a0", "a1", "a2", "a3",
                 "t0", "t1", "t2", "t3", "t4", "t5", "t6", "t7",
                 "s0", "s1", "s2", "s3", "s4", "s5", "s6", "s7",
                 "t8", "t9"]

    # Parse declarations to find hint-able variables
    DECL_RE = re.compile(r'(\s*)(s8|u8|s16|u16|s32|u32)\s+(\w+)\s*(=.*)?;')

    hintable = []
    for i, decl in enumerate(decls):
        m = DECL_RE.match(decl.strip())
        if m and 'register' not in decl and 'volatile' not in decl:
            hintable.append((i, m.group(1), m.group(2), m.group(3), m.group(4) or ''))

    if not hintable:
        return None

    # Try adding register hint to each declaration, one at a time
    for idx, indent, dtype, vname, init in hintable:
        for reg in hint_regs:
            init_part = f" {init}" if init else ""
            new_decl = f"{indent}register {dtype} {vname} asm(\"{reg}\"){init_part};"
            new_decls = list(decls)
            new_decls[idx] = new_decl
            code = rebuild_function(sig, new_decls, stmts)
            full = TYPEDEFS + externs + code
            if try_variant(func, full, tmpdir, target_o):
                return full

        # Limit: only try first 3 variables × all regs to keep it reasonable
        if idx >= 2:
            break

    return None


def try_unsigned_var_pad(func, func_code, externs, tmpdir, target_o):
    """Strategy 15: Insert dummy variable to perturb register state.

    Adding 'unsigned int _pad = 0;' between other declarations changes
    GCC's register allocation due to the additional live range.
    """
    sig, decls, stmts = parse_function(func_code)
    if not decls:
        return None

    pad_decl = "    unsigned int _sm_pad = 0;"

    # Try inserting at each position in declarations
    for i in range(len(decls) + 1):
        new_decls = decls[:i] + [pad_decl] + decls[i:]
        code = rebuild_function(sig, new_decls, stmts)
        full = TYPEDEFS + externs + code
        if try_variant(func, full, tmpdir, target_o):
            return full

    return None


# ---------------------------------------------------------------------------
# Combination strategies
# ---------------------------------------------------------------------------

def try_multi_strategy(func, func_code, externs, tmpdir, target_o):
    """Strategy 14: Try combining the cheapest transformations.

    Applies pairs of transformations that are individually cheap to compute.
    """
    sig, decls, stmts = parse_function(func_code)
    if len(stmts) < 2 and len(decls) < 2:
        return None

    tried = 0
    max_tries = 300

    # Combine: pairwise swap + volatile
    for i in range(len(stmts) - 1):
        if not are_independent(stmts[i], stmts[i+1]):
            continue
        new_stmts = list(stmts)
        new_stmts[i], new_stmts[i+1] = new_stmts[i+1], new_stmts[i]

        for j, decl in enumerate(decls):
            if 'volatile' in decl or '*' not in decl:
                continue
            new_decl = decl.replace('*', '* volatile ', 1)
            new_decls = list(decls)
            new_decls[j] = new_decl
            code = rebuild_function(sig, new_decls, new_stmts)
            full = TYPEDEFS + externs + code
            if try_variant(func, full, tmpdir, target_o):
                return full
            tried += 1
            if tried >= max_tries:
                return None

    # Combine: decl swap + pairwise stmt swap
    for di in range(len(decls) - 1):
        new_decls = list(decls)
        new_decls[di], new_decls[di+1] = new_decls[di+1], new_decls[di]
        for si in range(len(stmts) - 1):
            if not are_independent(stmts[si], stmts[si+1]):
                continue
            new_stmts = list(stmts)
            new_stmts[si], new_stmts[si+1] = new_stmts[si+1], new_stmts[si]
            code = rebuild_function(sig, new_decls, new_stmts)
            full = TYPEDEFS + externs + code
            if try_variant(func, full, tmpdir, target_o):
                return full
            tried += 1
            if tried >= max_tries:
                return None

    # Combine: cast swap + decl swap
    cast_swaps = [('(s16)', '(u16)'), ('(u16)', '(s16)'), ('(s8)', '(u8)'), ('(u8)', '(s8)')]
    for di in range(len(decls) - 1):
        new_decls = list(decls)
        new_decls[di], new_decls[di+1] = new_decls[di+1], new_decls[di]
        for si, stmt in enumerate(stmts):
            for old_cast, new_cast in cast_swaps:
                if old_cast in stmt:
                    new_stmts = list(stmts)
                    new_stmts[si] = stmt.replace(old_cast, new_cast, 1)
                    code = rebuild_function(sig, new_decls, new_stmts)
                    full = TYPEDEFS + externs + code
                    if try_variant(func, full, tmpdir, target_o):
                        return full
                    tried += 1
                    if tried >= max_tries:
                        return None

    return None


# ---------------------------------------------------------------------------
# Main orchestration
# ---------------------------------------------------------------------------

ALL_STRATEGIES = [
    ("direct",              try_direct),
    # Cheap single-transform strategies (fast, try first)
    ("pairwise_swap",       try_pairwise_swaps),
    ("cast_variation",      try_cast_variations),
    ("volatile",            try_volatile_variations),
    ("dowhile_barrier",     try_dowhile_barriers),
    ("temp_inline",         try_temp_inlining),
    ("expr_commutation",    try_expr_commutation),
    ("extern_type_swap",    try_extern_type_swaps),
    ("goto_loop",           try_goto_loops),
    ("ifelse_restructure",  try_ifelse_restructure),
    ("ptr_global_access",   try_pointer_global_access),
    ("unsigned_pad",        try_unsigned_var_pad),
    # Medium-cost strategies
    ("temp_introduction",   try_temp_introduction),
    ("register_hint",       try_register_hints),
    # Expensive permutation strategies (many variants, try last)
    ("stmt_permutation",    try_stmt_permutations),
    ("combined_permutation", try_combined_permutations),
    ("multi_strategy",      try_multi_strategy),
]


def process_function(func, score_only=False):
    """Try all strategies to match a function. Returns (strategy_name, code) or None."""
    m2c_out = run_m2c(func)
    if not m2c_out:
        return None

    c_code = make_compilable(m2c_out)
    externs = extract_externs(m2c_out)
    func_only = extract_func_only(c_code)

    tmpdir = tempfile.mkdtemp(prefix=f"smartmatch_{func}_")
    try:
        target_o = Path(tmpdir) / "target.o"
        if not build_target_obj(func, target_o):
            return None

        if score_only:
            matched, score, total = try_variant_scored(func, c_code, tmpdir, target_o)
            return ("score", score, total)

        # Strategy 0: Direct m2c output
        if try_direct(func, c_code, tmpdir, target_o):
            return ("direct", c_code)

        # Iterate through remaining strategies (cheapest first)
        for name, strategy_fn in ALL_STRATEGIES[1:]:
            # Strategies that take (func, func_code, externs, tmpdir, target_o)
            try:
                result = strategy_fn(func, func_only, externs, tmpdir, target_o)
                if result:
                    return (name, result)
            except Exception:
                continue

        return None
    finally:
        shutil.rmtree(tmpdir, ignore_errors=True)


def process_function_wrapper(args):
    """Wrapper for multiprocessing."""
    func, file_name, asm_lines, score, score_only = args
    try:
        result = process_function(func, score_only=score_only)
        return func, file_name, asm_lines, score, result
    except Exception as e:
        return func, file_name, asm_lines, score, None


def find_candidates(args):
    # Single-function fast-path: bypass the (stale) triage CSV entirely.
    # When --func is given, fabricate the candidate row from disk truth
    # so smart_match works without a full triage run.
    if args.func:
        asm_path = ROOT / "asm" / "funcs" / f"{args.func}.s"
        if not asm_path.exists():
            print(f"ERROR: asm/funcs/{args.func}.s not found", file=sys.stderr)
            sys.exit(1)
        src_name = ""
        for src in sorted((ROOT / "src").glob("*.c")):
            txt = src.read_text(encoding="utf-8", errors="ignore")
            if (f'INCLUDE_ASM("asm/funcs", {args.func})' in txt
                    or f"glabel {args.func}" in txt):
                src_name = src.name
                break
        with open(asm_path) as f:
            asm_lines = sum(
                1 for ln in f
                if ln.strip() and not ln.lstrip().startswith(("/*", "glabel", "endlabel"))
            )
        return [{
            "func": args.func,
            "file": src_name,
            "asm_lines": str(asm_lines),
            "score": "0",
            "compile_ok": "1",
        }]

    csvpath = ROOT / "triage_results.csv"
    if not csvpath.exists():
        print("ERROR: triage_results.csv not found. Run: python3 tools/triage_all.py",
              file=sys.stderr)
        sys.exit(1)

    candidates = []
    with open(csvpath) as f:
        reader = csv.DictReader(f)
        for row in reader:
            if row["compile_ok"] != "1":
                continue
            score = int(row["score"])
            asm_lines = int(row["asm_lines"])

            if args.func and row["func"] != args.func:
                continue
            if args.file and row["file"] != args.file:
                continue
            if args.max_score and score > args.max_score:
                continue
            if args.max_insns and asm_lines > args.max_insns:
                continue

            candidates.append(row)

    candidates.sort(key=lambda r: (int(r["score"]), int(r["asm_lines"])))

    if args.top:
        candidates = candidates[:args.top]

    return candidates


def main():
    parser = argparse.ArgumentParser(description="Smart-match v2: automated matching for GCC 2.7.2 PS1 decomp")
    parser.add_argument("--func", help="Match a single function (e.g., func_80043C7C)")
    parser.add_argument("--file", help="Only match functions from this source file (e.g., text1a.c)")
    parser.add_argument("--top", type=int, help="Only try the top N candidates by score")
    parser.add_argument("--max-score", type=int, help="Only try candidates with score <= N")
    parser.add_argument("--max-insns", type=int, default=200, help="Skip functions with > N instructions (default: 200)")
    parser.add_argument("-j", "--jobs", type=int, default=1, help="Number of parallel workers")
    parser.add_argument("--score-only", action="store_true", help="Just show scores, don't try to match")
    args = parser.parse_args()

    candidates = find_candidates(args)
    if not candidates:
        print("No candidates found matching criteria.")
        return

    strategies = [name for name, _ in ALL_STRATEGIES]
    print(f"Smart-match v2: {len(candidates)} candidates")
    print(f"Strategies: {' | '.join(strategies)}")
    print()

    matches = []
    errors = 0

    if args.jobs > 1:
        # Parallel mode
        work_items = [
            (row["func"], row["file"], int(row["asm_lines"]), int(row["score"]), args.score_only)
            for row in candidates
        ]
        with ProcessPoolExecutor(max_workers=args.jobs) as executor:
            futures = {executor.submit(process_function_wrapper, item): item for item in work_items}
            for i, future in enumerate(as_completed(futures)):
                func, file_name, asm_lines, score, result = future.result()
                prefix = f"[{i+1}/{len(candidates)}] {func} ({file_name}, {asm_lines} insns, score={score})"
                if result:
                    if args.score_only:
                        _, new_score, total = result
                        print(f"{prefix} → score={new_score}/{total}")
                    else:
                        strategy, code = result
                        print(f"{prefix} → MATCH via {strategy}!")
                        matches.append((func, file_name, asm_lines, strategy, code))
                        match_dir = ROOT / "auto_matches"
                        match_dir.mkdir(exist_ok=True)
                        with open(match_dir / f"{func}.c", 'w') as f:
                            f.write(code)
                else:
                    print(f"{prefix} → no match")
    else:
        # Sequential mode
        for i, row in enumerate(candidates):
            func = row["func"]
            score = int(row["score"])
            asm_lines = int(row["asm_lines"])

            prefix = f"[{i+1}/{len(candidates)}] {func} ({row['file']}, {asm_lines} insns, score={score})"
            print(f"{prefix}...", end=" ", flush=True)

            try:
                result = process_function(func, score_only=args.score_only)
                if result:
                    if args.score_only:
                        _, new_score, total = result
                        print(f"score={new_score}/{total}")
                    else:
                        strategy, code = result
                        print(f"MATCH via {strategy}!")
                        matches.append((func, row["file"], asm_lines, strategy, code))
                        match_dir = ROOT / "auto_matches"
                        match_dir.mkdir(exist_ok=True)
                        with open(match_dir / f"{func}.c", 'w') as f:
                            f.write(code)
                else:
                    print("no match")
            except Exception as e:
                print(f"error: {e}")
                errors += 1

    # Summary
    print(f"\n{'='*60}")
    if args.score_only:
        print(f"Scored {len(candidates)} functions.")
    else:
        print(f"RESULTS: {len(matches)} functions matched!")
        if errors:
            print(f"  ({errors} errors)")
        print(f"{'='*60}")
        for func, fname, insns, strategy, _ in matches:
            print(f"  {func:20s}  {fname:16s}  {insns:4d} insns  via {strategy}")

        if matches:
            print(f"\nMatching code saved to {ROOT / 'auto_matches'}/")
            print("Review and integrate into source files.")


if __name__ == "__main__":
    main()
