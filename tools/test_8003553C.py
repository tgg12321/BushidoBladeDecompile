#!/usr/bin/env python3
"""Test variants for func_8003553C."""
import subprocess, sys, re, tempfile
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
CC1 = ROOT / "tools" / "gcc-2.7.2" / "build" / "cc1"
MASPSX = ROOT / "tools" / "maspsx" / "maspsx.py"
CC_FLAGS = "-O2 -G0 -funsigned-char -quiet -mcpu=3000 -mips1 -mno-abicalls -fno-builtin -w"
TYPEDEFS = "typedef unsigned char u8;\ntypedef signed char s8;\ntypedef unsigned short u16;\ntypedef signed short s16;\ntypedef unsigned int u32;\ntypedef signed int s32;\n"

def compile_to_obj(c_code, output_path):
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

def build_target(func, output_path):
    asm_file = ROOT / "asm" / "funcs" / f"{func}.s"
    lines = [".set noat\n", ".set noreorder\n", ".section .text\n",
             f".global {func}\n", f"{func}:\n"]
    with open(asm_file) as f:
        for line in f:
            s = line.strip()
            if "glabel" in s or "endlabel" in s: continue
            m = re.search(r'/\*[^*]*\*/\s*(.*)', line)
            if m:
                i = m.group(1).strip()
                if i: lines.append(f"  {i}\n")
            elif s.startswith(".L"): lines.append(line)
            elif s.startswith("."): lines.append(f"  {s}\n")
    ts = output_path.with_suffix(".s")
    with open(ts, "w") as f: f.writelines(lines)
    subprocess.run(["mipsel-linux-gnu-as", "-march=r3000", "-mtune=r3000",
        "-no-pad-sections", "-O1", "-G0", "-o", str(output_path), str(ts)],
        capture_output=True, cwd=str(ROOT))

def get_insns(obj):
    r = subprocess.run(["mipsel-linux-gnu-objdump", "-d", "-M", "no-aliases", str(obj)],
        capture_output=True, text=True)
    insns = []
    for line in r.stdout.splitlines():
        m = re.match(r'\s+[0-9a-f]+:\s+[0-9a-f]+\s+(.*)', line)
        if m: insns.append(m.group(1).strip())
    return insns

def get_bytes(obj):
    r = subprocess.run(["mipsel-linux-gnu-objcopy", "-O", "binary", "-j", ".text",
        str(obj), "/dev/stdout"], capture_output=True, timeout=10)
    return r.stdout if r.returncode == 0 else None

def test(code, label, target_o):
    tmpdir = tempfile.mkdtemp()
    co = Path(tmpdir) / "c.o"
    if not compile_to_obj(code, co):
        print(f"{label}: COMPILE FAILED")
        return False
    tb = get_bytes(target_o)
    cb = get_bytes(co)
    if tb and cb and tb == cb:
        print(f"{label}: *** EXACT MATCH! ***")
        return True
    target = get_insns(target_o)
    compiled = get_insns(co)
    diffs = sum(1 for i in range(min(len(target),len(compiled))) if target[i] != compiled[i])
    diffs += abs(len(target) - len(compiled))
    print(f"{label}: {len(compiled)} insns, {diffs} diffs")
    for i in range(max(len(target), len(compiled))):
        t = target[i] if i < len(target) else "---"
        c = compiled[i] if i < len(compiled) else "---"
        if t != c:
            print(f"  {i:3d}  {t:35s} | {c}")
    return False

tmpdir = tempfile.mkdtemp()
target_o = Path(tmpdir) / "target.o"
build_target("func_8003553C", target_o)

# volatile stores to force ordering
code_vol = TYPEDEFS + """
void func_8007AA30(u8 *);
void func_8007A8B4(s32, u8 *);
extern s32 D_800A374C;
extern u8 *D_800A38B4;

void func_8003553C(void) {
    volatile u8 *s0;
    s32 ot;

    s0 = (volatile u8 *)D_800A38B4;
    func_8007AA30((u8 *)s0);
    *(volatile s16 *)(s0 + 0x1A) = 0xF0;
    *(volatile s16 *)(s0 + 0x22) = 0xF0;
    s0[4] = 0;
    s0[5] = 0;
    s0[6] = 0x80;
    s0[0xC] = 0;
    s0[0xD] = 0;
    s0[0xE] = 0x80;
    s0[0x14] = 0;
    s0[0x15] = 0;
    s0[0x16] = 0;
    s0[0x1C] = 0;
    s0[0x1D] = 0;
    s0[0x1E] = 0;
    ot = D_800A374C;
    *(volatile s16 *)(s0 + 8) = 0;
    *(volatile s16 *)(s0 + 0xA) = 0;
    *(volatile s16 *)(s0 + 0x10) = 0x280;
    *(volatile s16 *)(s0 + 0x12) = 0;
    *(volatile s16 *)(s0 + 0x18) = 0;
    *(volatile s16 *)(s0 + 0x20) = 0x280;
    func_8007A8B4(ot + 0x401C, (u8 *)s0);
    D_800A38B4 = (u8 *)s0 + 0x24;
}
"""

# struct approach
code_struct = TYPEDEFS + """
typedef struct {
    u8 pad0[4];
    u8 r1; u8 g1; u8 b1; u8 pad1;
    s16 x1; s16 y1;
    u8 r2; u8 g2; u8 b2; u8 pad2;
    s16 w1; s16 h1;
    u8 r3; u8 g3; u8 b3; u8 pad3;
    s16 x2; s16 y2;
    u8 r4; u8 g4; u8 b4; u8 pad4;
    s16 w2; s16 h2;
} PrimData;

void func_8007AA30(PrimData *);
void func_8007A8B4(s32, PrimData *);
extern s32 D_800A374C;
extern PrimData *D_800A38B4;

void func_8003553C(void) {
    PrimData *s0;
    s32 ot;

    s0 = D_800A38B4;
    func_8007AA30(s0);
    s0->y2 = 0xF0;
    s0->h2 = 0xF0;
    s0->r1 = 0; s0->g1 = 0; s0->b1 = 0x80;
    s0->r2 = 0; s0->g2 = 0; s0->b2 = 0x80;
    s0->r3 = 0; s0->g3 = 0; s0->b3 = 0;
    s0->r4 = 0; s0->g4 = 0; s0->b4 = 0;
    ot = D_800A374C;
    s0->x1 = 0; s0->y1 = 0;
    s0->w1 = 0x280; s0->h1 = 0;
    s0->x2 = 0;
    s0->w2 = 0x280;
    func_8007A8B4(ot + 0x401C, s0);
    D_800A38B4 = (PrimData *)((u8 *)s0 + 0x24);
}
"""

# Try using D_800A38B4 = s0 + 1 with PrimData pointer arithmetic
code_struct2 = TYPEDEFS + """
typedef struct {
    u8 pad0[4];
    u8 r1; u8 g1; u8 b1; u8 pad1;
    s16 x1; s16 y1;
    u8 r2; u8 g2; u8 b2; u8 pad2;
    s16 w1; s16 h1;
    u8 r3; u8 g3; u8 b3; u8 pad3;
    s16 x2; s16 y2;
    u8 r4; u8 g4; u8 b4; u8 pad4;
    s16 w2; s16 h2;
} PrimData;

void func_8007AA30(PrimData *);
void func_8007A8B4(s32, PrimData *);
extern s32 D_800A374C;
extern u8 *D_800A38B4;

void func_8003553C(void) {
    PrimData *s0;
    s32 ot;

    s0 = (PrimData *)D_800A38B4;
    func_8007AA30(s0);
    s0->y2 = 0xF0;
    s0->h2 = 0xF0;
    s0->r1 = 0; s0->g1 = 0; s0->b1 = 0x80;
    s0->r2 = 0; s0->g2 = 0; s0->b2 = 0x80;
    s0->r3 = 0; s0->g3 = 0; s0->b3 = 0;
    s0->r4 = 0; s0->g4 = 0; s0->b4 = 0;
    ot = D_800A374C;
    s0->x1 = 0; s0->y1 = 0;
    s0->w1 = 0x280; s0->h1 = 0;
    s0->x2 = 0;
    s0->w2 = 0x280;
    func_8007A8B4(ot + 0x401C, s0);
    D_800A38B4 = (u8 *)s0 + 0x24;
}
"""

test(code_vol, "volatile", target_o)
test(code_struct, "struct", target_o)
test(code_struct2, "struct2", target_o)
