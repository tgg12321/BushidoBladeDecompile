"""
Verify that a decompiled C function produces byte-identical output to the original.

Compiles a C file through the full PsyQ pipeline, links it at the correct address,
and compares the resulting bytes against the original binary.

Usage:
  python3 tools/check_func.py <c_file> [func_name ...]
  python3 tools/check_func.py src/ings.c                    # check all functions in file
  python3 tools/check_func.py src/ings.c func_800167AC      # check one function
"""

import sys
import os
import subprocess
import struct
import re
import tempfile
import shutil

# Paths
PROJECT_ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
ORIGINAL_EXE = os.path.join(PROJECT_ROOT, "disc", "SLUS_006.63")
CC1 = os.path.join(PROJECT_ROOT, "tools", "gcc-2.7.2", "build", "cc1")
MASPSX = os.path.join(PROJECT_ROOT, "tools", "maspsx", "maspsx.py")
REGFIX = os.path.join(PROJECT_ROOT, "tools", "regfix.py")
REGFIX_STAGE2 = os.path.join(PROJECT_ROOT, "regfix_stage2.txt")
ASM_FILE = os.path.join(PROJECT_ROOT, "asm", "6CAC.s")
UNDEF_FUNCS = os.path.join(PROJECT_ROOT, "undefined_funcs_auto.txt")
UNDEF_SYMS = os.path.join(PROJECT_ROOT, "undefined_syms_auto.txt")
SYMBOL_ADDRS = os.path.join(PROJECT_ROOT, "symbol_addrs.txt")

# Toolchain
CPP = "mipsel-linux-gnu-cpp"
AS = "mipsel-linux-gnu-as"
LD = "mipsel-linux-gnu-ld"
OBJCOPY = "mipsel-linux-gnu-objcopy"
NM = "mipsel-linux-gnu-nm"

# Compiler flags (must match Makefile exactly)
CPP_FLAGS = "-Iinclude -undef -Wall -lang-c -fno-builtin".split()
CPP_DEFS = ("-Dmips -D__GNUC__=2 -D__OPTIMIZE__ -D__mips__ -D__mips -Dpsx "
            "-D__psx__ -D__psx -D_PSYQ -D__EXTENSIONS__ -D_MIPSEL "
            "-D_LANGUAGE_C -DLANGUAGE_C").split()
CC_FLAGS = ("-O2 -G0 -funsigned-char -quiet -mcpu=3000 -mips1 "
            "-mno-abicalls -fno-builtin -w").split()
MASPSX_FLAGS = "--expand-div --aspsx-version=2.34".split()
AS_FLAGS = "-Iinclude -march=r3000 -mtune=r3000 -no-pad-sections -O1 -G0".split()

LOAD_ADDR = 0x80010000
EXE_HEADER = 0x800


def get_all_func_info():
    """Parse function addresses and sizes from the asm file and split func files."""
    funcs = {}

    # From main asm (nonmatching metadata)
    with open(ASM_FILE, "r") as f:
        content = f.read()
    for m in re.finditer(r'nonmatching\s+(\w+)\s*,\s*(0x[0-9A-Fa-f]+|\d+)', content):
        name = m.group(1)
        size = int(m.group(2), 0)
        addr_m = re.match(r'func_([0-9A-Fa-f]+)', name)
        if addr_m:
            addr = int(addr_m.group(1), 16)
            funcs[name] = (addr, size)

    # From split function files (for functions already removed from main asm)
    import glob
    funcs_dir = os.path.join(PROJECT_ROOT, "asm", "funcs")
    for path in glob.glob(os.path.join(funcs_dir, "func_*.s")):
        name = os.path.splitext(os.path.basename(path))[0]
        if name in funcs:
            continue
        with open(path) as f:
            lines = f.readlines()
        addrs = []
        for line in lines:
            m2 = re.search(r'/\*\s+[0-9A-Fa-f]+\s+([0-9A-Fa-f]+)\s+', line)
            if m2:
                addrs.append(int(m2.group(1), 16))
        if addrs:
            start = addrs[0]
            size = addrs[-1] + 4 - start
            funcs[name] = (start, size)

    return funcs


def get_original_bytes(addr, size):
    """Read bytes from the original EXE at the given VRAM address."""
    file_offset = EXE_HEADER + (addr - LOAD_ADDR)
    with open(ORIGINAL_EXE, "rb") as f:
        f.seek(file_offset)
        return f.read(size)


def compile_c_file(c_file, output_obj):
    """Compile a C file through the full PsyQ pipeline: cpp | cc1 | maspsx | as"""
    # Step 1: Preprocess
    cpp_cmd = [CPP] + CPP_FLAGS + CPP_DEFS + [c_file]
    cpp_result = subprocess.run(cpp_cmd, capture_output=True, cwd=PROJECT_ROOT)
    if cpp_result.returncode != 0:
        print(f"  CPP failed:\n{cpp_result.stderr.decode()}")
        return False

    # Step 2: Compile with GCC 2.7.2
    cc1_cmd = [CC1] + CC_FLAGS
    cc1_result = subprocess.run(cc1_cmd, input=cpp_result.stdout,
                                 capture_output=True, cwd=PROJECT_ROOT)
    if cc1_result.returncode != 0:
        print(f"  CC1 failed:\n{cc1_result.stderr.decode()}")
        return False

    # Step 3: ASPSX compatibility (maspsx)
    maspsx_cmd = ["python3", MASPSX] + MASPSX_FLAGS
    maspsx_result = subprocess.run(maspsx_cmd, input=cc1_result.stdout,
                                    capture_output=True, cwd=PROJECT_ROOT)
    if maspsx_result.returncode != 0:
        print(f"  MASPSX failed:\n{maspsx_result.stderr.decode()}")
        return False

    # Step 4: Regfix stage 1
    regfix_cmd = ["python3", REGFIX]
    regfix_result = subprocess.run(regfix_cmd, input=maspsx_result.stdout,
                                    capture_output=True, cwd=PROJECT_ROOT)
    if regfix_result.returncode != 0:
        print(f"  REGFIX failed:\n{regfix_result.stderr.decode()}")
        return False

    # Step 5: Regfix stage 2
    env = os.environ.copy()
    env["REGFIX_CONFIG"] = REGFIX_STAGE2
    regfix2_cmd = ["python3", REGFIX]
    regfix2_result = subprocess.run(regfix2_cmd, input=regfix_result.stdout,
                                     capture_output=True, cwd=PROJECT_ROOT, env=env)
    if regfix2_result.returncode != 0:
        print(f"  REGFIX stage 2 failed:\n{regfix2_result.stderr.decode()}")
        return False

    # Step 6: Assemble
    as_cmd = [AS] + AS_FLAGS + ["-o", output_obj]
    as_result = subprocess.run(as_cmd, input=regfix2_result.stdout,
                                capture_output=True, cwd=PROJECT_ROOT)
    if as_result.returncode != 0:
        print(f"  AS failed:\n{as_result.stderr.decode()}")
        return False

    return True


def get_obj_functions(obj_file):
    """Get list of (offset, size, name) for text functions in an object file."""
    cmd = [NM, "-n", obj_file]
    result = subprocess.run(cmd, capture_output=True, text=True, cwd=PROJECT_ROOT)
    if result.returncode != 0:
        return []

    symbols = []
    for line in result.stdout.strip().split("\n"):
        parts = line.split()
        if len(parts) >= 3 and parts[1] == "T":
            symbols.append((int(parts[0], 16), parts[2]))

    # Get text section size
    cmd2 = ["mipsel-linux-gnu-objdump", "-h", obj_file]
    r2 = subprocess.run(cmd2, capture_output=True, text=True, cwd=PROJECT_ROOT)
    text_size = 0
    for line in r2.stdout.split("\n"):
        if ".text" in line:
            parts = line.split()
            for i, p in enumerate(parts):
                if p == ".text":
                    text_size = int(parts[i-1], 16)
                    break

    funcs = []
    for i, (addr, name) in enumerate(symbols):
        if i + 1 < len(symbols):
            size = symbols[i + 1][0] - addr
        else:
            size = text_size - addr
        funcs.append((addr, size, name))

    return funcs


def generate_func_symbols(all_funcs, obj_func_names, sym_file):
    """Write a linker script defining addresses for all functions not in the object."""
    with open(sym_file, "w") as f:
        for name, (addr, size) in sorted(all_funcs.items(), key=lambda x: x[1][0]):
            if name not in obj_func_names:
                f.write(f"{name} = 0x{addr:08X};\n")


def link_at_address(obj_file, addr, output_bin, text_size, all_funcs, obj_func_names):
    """Link an object at a specific address and extract the binary."""
    tmpdir = tempfile.mkdtemp()
    try:
        ld_script = os.path.join(tmpdir, "check.ld")
        func_syms = os.path.join(tmpdir, "func_syms.ld")
        elf_out = os.path.join(tmpdir, "check.elf")

        with open(ld_script, "w") as f:
            f.write(f"SECTIONS {{\n")
            f.write(f"    .text 0x{addr:08X} : SUBALIGN(4) {{ *(.text) }}\n")
            f.write(f"    /DISCARD/ : {{ *(*) }}\n")
            f.write(f"}}\n")

        # Generate symbol definitions for all external functions
        generate_func_symbols(all_funcs, obj_func_names, func_syms)

        ld_cmd = [LD, "-nostdlib", "--no-check-sections",
                  "--unresolved-symbols=ignore-all",
                  "--noinhibit-exec",
                  "-T", ld_script,
                  "-T", func_syms,
                  "-T", UNDEF_FUNCS,
                  "-T", UNDEF_SYMS,
                  obj_file, "-o", elf_out]
        result = subprocess.run(ld_cmd, capture_output=True, text=True, cwd=PROJECT_ROOT)
        if result.returncode != 0:
            stderr = result.stderr
            errors = [l for l in stderr.split("\n")
                     if l and "warning" not in l.lower()]
            if errors:
                print(f"  LD failed:\n{stderr}")
                return False

        cmd = [OBJCOPY, "-O", "binary", "-j", ".text", elf_out, output_bin]
        result = subprocess.run(cmd, capture_output=True, cwd=PROJECT_ROOT)
        if result.returncode != 0:
            print(f"  objcopy failed: {result.stderr.decode()}")
            return False

        return True
    finally:
        shutil.rmtree(tmpdir)


def compare_bytes(expected, actual, addr):
    """Compare two byte sequences and report differences."""
    if expected == actual:
        return True

    min_len = min(len(expected), len(actual))
    diffs = []
    for i in range(min_len):
        if expected[i] != actual[i]:
            diffs.append(i)

    if len(expected) != len(actual):
        print(f"  Size mismatch: expected {len(expected)} bytes, got {len(actual)} bytes")

    if diffs:
        print(f"  {len(diffs)} differing bytes ({len(diffs)//4} instructions):")
        shown = set()
        for d in diffs[:40]:
            inst_off = (d // 4) * 4
            if inst_off in shown:
                continue
            shown.add(inst_off)
            if inst_off + 4 <= min_len:
                exp_word = struct.unpack("<I", expected[inst_off:inst_off+4])[0]
                act_word = struct.unpack("<I", actual[inst_off:inst_off+4])[0]
                vram = addr + inst_off
                print(f"    0x{vram:08X}: expected 0x{exp_word:08X}, got 0x{act_word:08X}")

    return False


def main():
    if len(sys.argv) < 2:
        print(f"Usage: {sys.argv[0]} <c_file> [func_name ...]")
        sys.exit(1)

    c_file = sys.argv[1]
    requested_funcs = sys.argv[2:] if len(sys.argv) > 2 else None

    # Get all known function info
    all_funcs = get_all_func_info()

    # Compile the C file
    with tempfile.NamedTemporaryFile(suffix=".o", delete=False) as tmp:
        obj_path = tmp.name

    try:
        if not compile_c_file(c_file, obj_path):
            sys.exit(1)

        # Get functions defined in the object
        obj_funcs = get_obj_functions(obj_path)
        if not obj_funcs:
            print("No functions found in compiled object")
            sys.exit(1)

        # Filter to requested functions
        if requested_funcs:
            obj_funcs = [(o, s, n) for o, s, n in obj_funcs if n in requested_funcs]

        if not obj_funcs:
            print("No matching functions found")
            sys.exit(1)

        # Find the first function's address for linking
        first_func = obj_funcs[0]
        if first_func[2] not in all_funcs:
            print(f"  {first_func[2]}: not found in asm metadata")
            sys.exit(1)

        link_addr = all_funcs[first_func[2]][0]

        # Link at the correct address
        with tempfile.NamedTemporaryFile(suffix=".bin", delete=False) as tmp2:
            bin_path = tmp2.name

        text_size = sum(s for _, s, _ in obj_funcs)
        obj_func_names = set(n for _, _, n in get_obj_functions(obj_path))
        if not link_at_address(obj_path, link_addr, bin_path, text_size,
                               all_funcs, obj_func_names):
            sys.exit(1)

        with open(bin_path, "rb") as f:
            linked_bytes = f.read()
        os.unlink(bin_path)

        # Check each function
        all_ok = True
        for func_off, func_size, func_name in obj_funcs:
            if func_name not in all_funcs:
                print(f"  {func_name}: SKIP (not in asm metadata)")
                continue

            expected_addr, expected_size = all_funcs[func_name]
            expected = get_original_bytes(expected_addr, expected_size)

            # Extract this function's bytes from linked binary
            offset_in_bin = expected_addr - link_addr
            actual = linked_bytes[offset_in_bin:offset_in_bin + expected_size]

            if compare_bytes(expected, actual, expected_addr):
                print(f"  {func_name}: OK")
            else:
                print(f"  {func_name}: MISMATCH")
                all_ok = False

        sys.exit(0 if all_ok else 1)

    finally:
        if os.path.exists(obj_path):
            os.unlink(obj_path)


if __name__ == "__main__":
    main()
