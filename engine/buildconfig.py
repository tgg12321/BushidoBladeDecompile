"""Explicit build configuration — the single source of truth for how a C file
becomes a byte-identical object.

This DATA mirrors the Makefile exactly as of the commit recorded in the oracle
manifest. The Makefile is retained only until `engine build` is proven at full
byte-parity (Phase 0, task 4); after that this module is canonical.

Keep this as plain data. The pipeline assembly lives in pipeline.py so that the
"what" (config) and the "how" (command construction) stay separable — which is
also what lets the Phase 1 cheat-invisible sandbox swap cheat stages without
touching any of these values.
"""

# -- Ground truth -----------------------------------------------------------
ORACLE_SHA1 = "62efab4f73f992798c43e8c730aa43baa10bb4fa"
TARGET_EXE = "disc/SLUS_006.63"

# -- Toolchain --------------------------------------------------------------
CC1 = "tools/gcc-2.7.2/build/cc1"
CPP = "mipsel-linux-gnu-cpp"
AS = "mipsel-linux-gnu-as"
LD = "mipsel-linux-gnu-ld"
OBJCOPY = "mipsel-linux-gnu-objcopy"
OBJDUMP = "mipsel-linux-gnu-objdump"
NM = "mipsel-linux-gnu-nm"

# PS-EXE layout: the raw binary loads at LOAD_ADDR; build/bb2.exe prepends a
# HEADER_SIZE-byte PS-X EXE header. So vram V lives at file offset
# HEADER_SIZE + (V - LOAD_ADDR) in build/bb2.exe.
LOAD_ADDR = 0x80010000
HEADER_SIZE = 0x800

# Python pipeline stages (invoked exactly as the Makefile invokes them)
MASPSX = "python3 tools/maspsx/maspsx.py"
PROLOGUE_FIX = "python3 tools/prologue_fix.py"
FIX_LWL = "python3 tools/fix_lwl.py"
MULTU_PAD = "python3 tools/multu_pad.py --funcs multu_pad_funcs.txt"
REGFIX = "python3 tools/regfix.py"
REGFIX_STAGE2 = "REGFIX_CONFIG=regfix_stage2.txt python3 tools/regfix.py"
ASMFIX = "python3 tools/asmfix.py"

# The three "cheat" stages, named so the Phase 1 sandbox can target them.
CHEAT_STAGES = ("regfix", "regfix_stage2", "asmfix")

# -- Flags ------------------------------------------------------------------
CC_FLAGS = "-O2 -G0 -funsigned-char -quiet -mcpu=3000 -mips1 -mno-abicalls -fno-builtin -w"
CC_FLAGS_GP = "-O2 -G8 -funsigned-char -quiet -mcpu=3000 -mips1 -mno-abicalls -fno-builtin -w"
AS_FLAGS = "-Iinclude -march=r3000 -mtune=r3000 -no-pad-sections -O1 -G0"
CPP_FLAGS = "-Iinclude -undef -Wall -lang-c -fno-builtin"
CPP_DEFS = (
    "-Dmips -D__GNUC__=2 -D__OPTIMIZE__ -D__mips__ -D__mips -Dpsx -D__psx__ -D__psx "
    "-D_PSYQ -D__EXTENSIONS__ -D_MIPSEL -D_LANGUAGE_C -DLANGUAGE_C"
)
LD_FLAGS = "-nostdlib --no-check-sections"
MASPSX_FLAGS = (
    "--expand-div --aspsx-version=2.34 --sdata-syms=sdata_syms.txt "
    "--sdata-funcs=sdata_funcs.txt --sdata-exclude=sdata_exclude.txt --expand-lb "
    "--expand-lb-funcs=expand_lb_funcs.txt --multu-funcs=multu_funcs.txt "
    "--expand-dest-funcs=expand_dest_funcs.txt "
    "--label-nop-funcs=maspsx_label_nop_funcs.txt"
)
MASPSX_FLAGS_GP = (
    "--expand-div --aspsx-version=2.34 --dont-force-G0 --sdata-syms=sdata_syms.txt -G8"
)

# -- Per-file opt-ins (C file stem, no path/extension) ----------------------
# Mirrors the Makefile GP_FILES / EXPAND_LB_FILES / FIX_LWL_FILES /
# RODATA_ALIGN2_FILES / NO_SR_FILES lists. Byte-parity (task 4) is the proof
# these are correct; do not edit without re-running `engine parity`.
GP_FILES = set()
EXPAND_LB_FILES = {"code6cac_b"}
EXPAND_LH_FILES = set()
FIX_LWL_FILES = {"code6cac", "text1b"}
RODATA_ALIGN2_FILES = {
    "code6cac", "code6cac_b", "code6cac_c", "code6cac_c0", "code6cac_c_ab",
    "code6cac_c_mid", "code6cac_c2", "text1a", "text1a_b", "text1a_c",
    "text1a_c2", "text1b_b", "main",
}
NO_SR_FILES = set()

# -- Linker -----------------------------------------------------------------
LD_SCRIPT = "bb2.ld"
LD_SYM_FILES = ["undefined_funcs_auto.txt", "undefined_syms_auto.txt", "named_syms.txt"]
