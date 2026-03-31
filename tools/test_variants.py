#!/usr/bin/env python3
"""Quick test: compile multiple C variants for a function and compare against target."""
import subprocess, sys, re, tempfile
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
CC1 = ROOT / "tools" / "gcc-2.7.2" / "build" / "cc1"
MASPSX = ROOT / "tools" / "maspsx" / "maspsx.py"
CC_FLAGS = "-O2 -G0 -funsigned-char -quiet -mcpu=3000 -mips1 -mno-abicalls -fno-builtin -w"

TYPEDEFS = "typedef unsigned char u8;\ntypedef signed char s8;\ntypedef unsigned short u16;\ntypedef signed short s16;\ntypedef unsigned int u32;\ntypedef signed int s32;\n"

def compile_to_obj(c_code, output_path):
    try:
        p_cpp = subprocess.Popen(["mipsel-linux-gnu-cpp",
            f"-I{ROOT}/include", "-undef", "-Wall", "-lang-c", "-fno-builtin",
            "-Dmips", "-D__GNUC__=2", "-D__OPTIMIZE__", "-D__mips__", "-D__mips",
            "-Dpsx", "-D__psx__", "-D__psx", "-D_PSYQ", "-D__EXTENSIONS__",
            "-D_MIPSEL", "-D_LANGUAGE_C", "-DLANGUAGE_C", "-DPERMUTER", "-"],
            stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.DEVNULL, cwd=str(ROOT))
        p_cc1 = subprocess.Popen([str(CC1)] + CC_FLAGS.split(),
            stdin=p_cpp.stdout, stdout=subprocess.PIPE, stderr=subprocess.DEVNULL, cwd=str(ROOT))
        p_cpp.stdout.close()
        p_maspsx = subprocess.Popen([sys.executable, str(MASPSX), "--expand-div", "--aspsx-version=2.34",
            f"--sdata-syms={ROOT}/sdata_syms.txt", f"--sdata-funcs={ROOT}/sdata_funcs.txt",
            f"--sdata-exclude={ROOT}/sdata_exclude.txt"],
            stdin=p_cc1.stdout, stdout=subprocess.PIPE, stderr=subprocess.DEVNULL, cwd=str(ROOT))
        p_cc1.stdout.close()
        p_as = subprocess.Popen(["mipsel-linux-gnu-as", f"-I{ROOT}/include", "-march=r3000", "-mtune=r3000",
            "-no-pad-sections", "-O1", "-G0", "-o", str(output_path)],
            stdin=p_maspsx.stdout, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL, cwd=str(ROOT))
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
            capture_output=True, timeout=10, cwd=str(ROOT))
        return result.returncode == 0
    except Exception:
        return False


def get_insns(obj):
    r = subprocess.run(["mipsel-linux-gnu-objdump", "-d", "-M", "no-aliases", str(obj)],
        capture_output=True, text=True)
    insns = []
    for line in r.stdout.splitlines():
        m = re.match(r'\s+[0-9a-f]+:\s+[0-9a-f]+\s+(.*)', line)
        if m:
            insns.append(m.group(1).strip())
    return insns


def get_bytes(obj):
    r = subprocess.run(["mipsel-linux-gnu-objcopy", "-O", "binary", "-j", ".text",
        str(obj), "/dev/stdout"], capture_output=True, timeout=10)
    return r.stdout if r.returncode == 0 else None


def test_variant(code, label, target_o):
    tmpdir = tempfile.mkdtemp()
    compiled_o = Path(tmpdir) / "compiled.o"
    if not compile_to_obj(code, compiled_o):
        print(f"{label}: COMPILE FAILED")
        return

    tb = get_bytes(target_o)
    cb = get_bytes(compiled_o)
    if tb and cb and tb == cb:
        print(f"{label}: *** EXACT MATCH! ***")
        return

    target = get_insns(target_o)
    compiled = get_insns(compiled_o)
    diffs = sum(1 for i in range(min(len(target), len(compiled))) if target[i] != compiled[i])
    diffs += abs(len(target) - len(compiled))
    print(f"{label}: {len(compiled)} insns, {diffs} diffs")
    shown = 0
    for i in range(min(len(target), len(compiled))):
        if target[i] != compiled[i]:
            print(f"  {i:3d}  {target[i]:35s} | {compiled[i]}")
            shown += 1
            if shown >= 3:
                break


func = "func_8003032C"
tmpdir = tempfile.mkdtemp()
target_o = Path(tmpdir) / "target.o"
build_target_obj(func, target_o)

# Variant A: original m2c order with fixed ptr scaling
codeA = TYPEDEFS + """
s32 func_8007FD5C(s16, s16);
extern u8 D_800973FC;

void func_8003032C(s32 *arg0, s16 *arg1) {
    s16 temp_a0;
    s16 temp_a1;
    s32 temp_v0;
    s32 temp_v1;
    s32 temp_v0_2;
    s32 temp_v1_2;
    s32 temp_v0_3;
    s32 var_v1;

    temp_v0 = func_8007FD5C(arg1[0], arg1[2]);
    temp_a1 = *(s16 *)(&D_800973FC + ((temp_v0 + 0x400) & 0xFFF) * 2);
    temp_v1 = *(s32 *)((u8 *)arg0 + 0x44);
    temp_a0 = *(s16 *)(&D_800973FC + (temp_v0 & 0xFFF) * 2);
    temp_v0_2 = *(s32 *)((u8 *)arg0 + 0x4C);
    temp_v1_2 = (temp_v1 * temp_a1 + temp_v0_2 * temp_a0) >> 0xC;
    temp_v0_3 = -((-temp_v1 * temp_a0 + temp_v0_2 * temp_a1) >> 0xC);
    var_v1 = *(s32 *)((u8 *)arg0 + 0x48);
    *(s32 *)((u8 *)arg0 + 0x44) = (temp_v1_2 * temp_a1 - temp_v0_3 * temp_a0) >> 0xF;
    *(s32 *)((u8 *)arg0 + 0x4C) = (temp_v1_2 * temp_a0 + temp_v0_3 * temp_a1) >> 0xF;
    if (var_v1 < 0) {
        var_v1 += 3;
    }
    *(s32 *)((u8 *)arg0 + 0x48) = var_v1 >> 2;
}
"""

# Variant B: swap cos/sin names to try different reg alloc
codeB = TYPEDEFS + """
s32 func_8007FD5C(s16, s16);
extern u8 D_800973FC;

void func_8003032C(s32 *arg0, s16 *arg1) {
    s32 angle;
    s16 cos;
    s16 sin;
    s32 x;
    s32 z;
    s32 rx;
    s32 rz;
    s32 vy;

    angle = func_8007FD5C(arg1[0], arg1[2]);
    cos = *(s16 *)(&D_800973FC + ((angle + 0x400) & 0xFFF) * 2);
    x = *(s32 *)((u8 *)arg0 + 0x44);
    sin = *(s16 *)(&D_800973FC + (angle & 0xFFF) * 2);
    z = *(s32 *)((u8 *)arg0 + 0x4C);
    rx = (x * cos + z * sin) >> 0xC;
    rz = -((-x * sin + z * cos) >> 0xC);
    vy = *(s32 *)((u8 *)arg0 + 0x48);
    *(s32 *)((u8 *)arg0 + 0x44) = (rx * cos - rz * sin) >> 0xF;
    *(s32 *)((u8 *)arg0 + 0x4C) = (rx * sin + rz * cos) >> 0xF;
    if (vy < 0) {
        vy += 3;
    }
    *(s32 *)((u8 *)arg0 + 0x48) = vy >> 2;
}
"""

# Variant C: swap x/z load order (load z before x)
codeC = TYPEDEFS + """
s32 func_8007FD5C(s16, s16);
extern u8 D_800973FC;

void func_8003032C(s32 *arg0, s16 *arg1) {
    s32 angle;
    s16 cos;
    s16 sin;
    s32 x;
    s32 z;
    s32 rx;
    s32 rz;
    s32 vy;

    angle = func_8007FD5C(arg1[0], arg1[2]);
    cos = *(s16 *)(&D_800973FC + ((angle + 0x400) & 0xFFF) * 2);
    sin = *(s16 *)(&D_800973FC + (angle & 0xFFF) * 2);
    x = *(s32 *)((u8 *)arg0 + 0x44);
    z = *(s32 *)((u8 *)arg0 + 0x4C);
    rx = (x * cos + z * sin) >> 0xC;
    rz = -((-x * sin + z * cos) >> 0xC);
    vy = *(s32 *)((u8 *)arg0 + 0x48);
    *(s32 *)((u8 *)arg0 + 0x44) = (rx * cos - rz * sin) >> 0xF;
    *(s32 *)((u8 *)arg0 + 0x4C) = (rx * sin + rz * cos) >> 0xF;
    if (vy < 0) {
        vy += 3;
    }
    *(s32 *)((u8 *)arg0 + 0x48) = vy >> 2;
}
"""

# Variant D: use volatile s16* for table access
codeD = TYPEDEFS + """
s32 func_8007FD5C(s16, s16);
extern u8 D_800973FC;

void func_8003032C(s32 *arg0, s16 *arg1) {
    s32 angle;
    s16 cos;
    s32 x;
    s16 sin;
    s32 z;
    s32 rx;
    s32 rz;
    s32 vy;

    angle = func_8007FD5C(arg1[0], arg1[2]);
    cos = *(volatile s16 *)(&D_800973FC + ((angle + 0x400) & 0xFFF) * 2);
    x = *(s32 *)((u8 *)arg0 + 0x44);
    sin = *(volatile s16 *)(&D_800973FC + (angle & 0xFFF) * 2);
    z = *(s32 *)((u8 *)arg0 + 0x4C);
    rx = (x * cos + z * sin) >> 0xC;
    rz = -((-x * sin + z * cos) >> 0xC);
    vy = *(s32 *)((u8 *)arg0 + 0x48);
    *(s32 *)((u8 *)arg0 + 0x44) = (rx * cos - rz * sin) >> 0xF;
    *(s32 *)((u8 *)arg0 + 0x4C) = (rx * sin + rz * cos) >> 0xF;
    if (vy < 0) {
        vy += 3;
    }
    *(s32 *)((u8 *)arg0 + 0x48) = vy >> 2;
}
"""

# Variant E: all (s32) casts
codeE = TYPEDEFS + """
s32 func_8007FD5C(s16, s16);
extern u8 D_800973FC;

void func_8003032C(s32 *arg0, s16 *arg1) {
    s32 angle;
    s16 cos;
    s32 x;
    s16 sin;
    s32 z;
    s32 rx;
    s32 rz;
    s32 vy;

    angle = func_8007FD5C(arg1[0], arg1[2]);
    cos = *(s16 *)(&D_800973FC + ((angle + 0x400) & 0xFFF) * 2);
    x = *(s32 *)((u8 *)arg0 + 0x44);
    sin = *(s16 *)(&D_800973FC + (angle & 0xFFF) * 2);
    z = *(s32 *)((u8 *)arg0 + 0x4C);
    rx = (s32)(x * cos + z * sin) >> 0xC;
    rz = -((s32)(-x * sin + z * cos) >> 0xC);
    vy = *(s32 *)((u8 *)arg0 + 0x48);
    *(s32 *)((u8 *)arg0 + 0x44) = (s32)(rx * cos - rz * sin) >> 0xF;
    *(s32 *)((u8 *)arg0 + 0x4C) = (s32)(rx * sin + rz * cos) >> 0xF;
    if (vy < 0) {
        vy += 3;
    }
    *(s32 *)((u8 *)arg0 + 0x48) = vy >> 2;
}
"""

# Variant F: fewer temps - inline the table lookups directly into mult expressions
codeF = TYPEDEFS + """
s32 func_8007FD5C(s16, s16);
extern u8 D_800973FC;

void func_8003032C(s32 *arg0, s16 *arg1) {
    s32 angle;
    s16 cos;
    s16 sin;
    s32 x;
    s32 z;
    s32 vy;

    angle = func_8007FD5C(arg1[0], arg1[2]);
    cos = *(s16 *)(&D_800973FC + ((angle + 0x400) & 0xFFF) * 2);
    x = *(s32 *)((u8 *)arg0 + 0x44);
    sin = *(s16 *)(&D_800973FC + (angle & 0xFFF) * 2);
    z = *(s32 *)((u8 *)arg0 + 0x4C);
    vy = *(s32 *)((u8 *)arg0 + 0x48);
    *(s32 *)((u8 *)arg0 + 0x44) = ((x * cos + z * sin) >> 0xC * cos - (-((-x * sin + z * cos) >> 0xC)) * sin) >> 0xF;
    *(s32 *)((u8 *)arg0 + 0x4C) = ((x * cos + z * sin) >> 0xC * sin + (-((-x * sin + z * cos) >> 0xC)) * cos) >> 0xF;
    if (vy < 0) {
        vy += 3;
    }
    *(s32 *)((u8 *)arg0 + 0x48) = vy >> 2;
}
"""

test_variant(codeA, "A: m2c+fix", target_o)
test_variant(codeB, "B: clean names", target_o)
test_variant(codeC, "C: both lookups first", target_o)
test_variant(codeD, "D: volatile table", target_o)
test_variant(codeE, "E: s32 casts", target_o)
test_variant(codeF, "F: fewer temps", target_o)
