# Bushido Blade 2 (SLUS-00663) Matching Decompilation
# Build system for PS1 (PsyQ SDK 3.5, GCC 2.7.2, ASPSX 2.34)

# -- Configuration --
SHELL := /bin/bash
.SHELLFLAGS := -o pipefail -c
.DELETE_ON_ERROR:
TARGET       := bb2
BASE_DIR     := disc
TARGET_EXE   := $(BASE_DIR)/SLUS_006.63
SPLAT_YAML   := splat.yaml

# -- Toolchain --
# PsyQ GCC 2.7.2 compiler (built from decompals/mips-gcc-2.7.2)
CC1          := tools/gcc-2.7.2/build/cc1
# maspsx ASPSX compatibility layer
PROLOGUE_FIX := python3 tools/prologue_fix.py
MULTU_PAD    := python3 tools/multu_pad.py --funcs multu_pad_funcs.txt
MASPSX       := python3 tools/maspsx/maspsx.py
MASPSX_FLAGS := --expand-div --aspsx-version=2.34 --expand-lb --expand-lb-funcs=expand_lb_funcs.txt --multu-funcs=multu_funcs.txt --expand-dest-funcs=expand_dest_funcs.txt --prefill-label-funcs=maspsx_prefill_label_funcs.txt --comm-syms=maspsx_comm_syms.txt --use-comm-section
MASPSX_FLAGS_GP := --expand-div --aspsx-version=2.34 --expand-lb --expand-lb-funcs=expand_lb_funcs.txt --multu-funcs=multu_funcs.txt --expand-dest-funcs=expand_dest_funcs.txt --prefill-label-funcs=maspsx_prefill_label_funcs.txt --comm-syms=maspsx_comm_syms.txt --use-comm-section

# GNU MIPS cross-tools
AS           := mipsel-linux-gnu-as
LD           := mipsel-linux-gnu-ld
OBJCOPY      := mipsel-linux-gnu-objcopy
CPP          := mipsel-linux-gnu-cpp

# -- Compiler Flags --
# -O2: standard optimization level for PsyQ games
# -G0: disable GP-relative addressing (default for most files)
# -G8: cc1 small-data threshold for the files listed in GP_FILES
# -funsigned-char: common PsyQ convention
# -mcpu=3000: target R3000A
# -mel: MANDATORY. The prebuilt cc1's mips-mips-gnu triple defaults BIG-endian;
# -mel flips BYTES_BIG_ENDIAN (spill-slot layout, bitfield direction, lwl/lwr).
# Load-bearing for the oracle match — do not remove (see AGENTS.md).
# -msoft-float: MANDATORY. The same triple defaults HARD-float; the PS1 has no
# FPU and PsyQ cc1psx prints "# Cc1 defaults: -mgas -msoft-float". Hard float
# leaves 32 FP regs allocatable, doubling loop.c's invariant-hoist threshold
# (122 vs 58) — adopted 2026-09-07 (docs/grind/decisions.md). Do not remove.
CC_FLAGS     := -O2 -G0 -funsigned-char -quiet -mcpu=3000 -mips1 -mno-abicalls -fno-builtin -w -mel -msoft-float
CC_FLAGS_GP  := -O2 -G8 -funsigned-char -quiet -mcpu=3000 -mips1 -mno-abicalls -fno-builtin -w -mel -msoft-float
AS_FLAGS     := -Iinclude -march=r3000 -mtune=r3000 -no-pad-sections -O1 -G0
CPP_FLAGS    := -Iinclude -undef -Wall -lang-c -fno-builtin
CPP_DEFS     := -Dmips -D__GNUC__=2 -D__OPTIMIZE__ -D__mips__ -D__mips -Dpsx -D__psx__ -D__psx \
                -D_PSYQ -D__EXTENSIONS__ -D_MIPSEL -D_LANGUAGE_C -DLANGUAGE_C
LD_FLAGS     := -nostdlib --no-check-sections

# -- Directories --
ASM_DIR      := asm
SRC_DIR      := src
BUILD_DIR    := build
INCLUDE_DIR  := include

# -- Output --
ELF          := $(BUILD_DIR)/$(TARGET).elf
BIN          := $(BUILD_DIR)/$(TARGET).bin
EXE          := $(BUILD_DIR)/$(TARGET).exe

# -- Collect source files --
# Assembly files (non-decompiled functions)
S_FILES      := $(wildcard $(ASM_DIR)/*.s)
S_O_FILES    := $(patsubst $(ASM_DIR)/%.s,$(BUILD_DIR)/$(ASM_DIR)/%.o,$(S_FILES))

# Data assembly files
DATA_S_FILES := $(wildcard $(ASM_DIR)/data/*.s)
DATA_O_FILES := $(patsubst $(ASM_DIR)/data/%.s,$(BUILD_DIR)/$(ASM_DIR)/data/%.o,$(DATA_S_FILES))

# C source files (decompiled functions), found recursively. A TU id is the path
# under src/ without .c (`text1b`, `main/psxsdk/libcomb/comb`); its object is
# build/src/<id>.o, the path bb2.ld links (engine/tus.py).
C_FILES      := $(sort $(shell find $(SRC_DIR) -name '*.c'))
C_IDS        := $(patsubst $(SRC_DIR)/%.c,%,$(C_FILES))
C_O_FILES    := $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/$(SRC_DIR)/%.o,$(C_FILES))

# Per-function asm objects linked directly (explicit opt-in — NOT a wildcard;
# asm/funcs/ holds all 1437 split functions and we want only the listed ones).
# Used where a function cannot live inside a C translation unit: text1a is the
# project's sole -G8 file, and under -G8 cc1 buffers function bodies to a temp
# file (TARGET_FILE_SWITCHING) so a file-scope __asm__ floats to the top of the
# TU instead of staying in place. Empty since 2026-10-01: save_vc_ctrl (the
# only member) moved to its own one-function TU src/text1a_svc.c when it was
# de-authorized and re-queued. Kept for any future raw-asm object.
LINKED_ASM_FUNCS :=
ASM_FUNC_O_FILES := $(patsubst %,$(BUILD_DIR)/$(ASM_DIR)/funcs/%.o,$(LINKED_ASM_FUNCS))

# All objects
ALL_O_FILES  := $(S_O_FILES) $(DATA_O_FILES) $(C_O_FILES) $(ASM_FUNC_O_FILES)

# -- Top-level targets --
.PHONY: all clean setup check clean-check

all: check

# Build and verify match
check: $(EXE) $(TARGET).sha1
	@sha1sum -c $(TARGET).sha1 || { echo "MISMATCH: $(TARGET) does not match"; exit 1; }
	@python3 -m engine.tus check
	@python3 -m engine.buildstamp record-current
	@echo "OK: $(TARGET) matches!"

# Force a clean rebuild before checking the final hash.
clean-check:
	$(MAKE) clean
	$(MAKE) check

# -- SHA1 checksum file --
$(TARGET).sha1:
	@echo "62efab4f73f992798c43e8c730aa43baa10bb4fa  $(EXE)" > $@

# -- Link --
$(ELF): $(ALL_O_FILES) $(TARGET).ld undefined_funcs_auto.txt undefined_syms_auto.txt named_syms.txt
	$(LD) $(LD_FLAGS) -Map $(BUILD_DIR)/$(TARGET).map -T $(TARGET).ld \
		-T undefined_funcs_auto.txt -T undefined_syms_auto.txt -T named_syms.txt \
		-o $@

# Extract raw binary from ELF
$(BIN): $(ELF)
	$(OBJCOPY) -O binary -j .main $< $@

# Construct PS-X EXE (prepend original 0x800-byte header)
$(EXE): $(BIN) $(TARGET_EXE) tools/make_psexe.py
	python3 tools/make_psexe.py $(TARGET_EXE) $< $@

# -- Per-file GP-relative opt-in --
# List C files (TU ids: path under src/ without .c) that need GP-relative addressing.
# Our cc1 runs these at -G8 (gp itself is maspsx's decision on each file's own definitions).
GP_FILES := main/309CC main/31548 main/31CFC main/31D3C main/24F08 main/26730 main/26940 main/5ED34 main/368E4

# -- Small data: maspsx -G8 except Sony library code (owner rulings Q65, Q69) --
# maspsx runs -G8 for every file: a file uses gp for a small symbol only if it DEFINES it (Sony
# ASPSX 2.34), statics are its own .sbss, small initialized objects its own .sdata (cc1psx -G8).
# Sony PsyQ library code was compiled -G0 and keeps it. The set is evidence, not a choice:
# generated by tools/psyq_library_files.py from the provenance census (memory/closer/
# psyq-library-census.md library span) and the link map; `--check` verifies it.
PSYQ_LIBRARY_FILES := main/psxsdk/libcomb/comb main/psxsdk/libgpu/sys gpu ings2 main system main/psxsdk/libc2/prnt main/psxsdk/libc2/sprintf main/psxsdk/libapi/c67 main/psxsdk/libapi/c112 main/psxsdk/libapi/c159 main/psxsdk/libapi/a08 main/psxsdk/libapi/a09 main/psxsdk/libapi/a11 main/psxsdk/libapi/a12 main/psxsdk/libapi/a36 main/psxsdk/libapi/a37 main/psxsdk/libapi/a39 main/psxsdk/libapi/a50 main/psxsdk/libapi/a52 main/psxsdk/libapi/a53 main/psxsdk/libapi/a54 main/psxsdk/libapi/a65 main/psxsdk/libapi/a66 main/psxsdk/libapi/a67 main/psxsdk/libapi/a91 main/psxsdk/libapi/counter main/psxsdk/libapi/pad main/psxsdk/libapi/a18 main/psxsdk/libapi/a19 main/psxsdk/libapi/a20 main/psxsdk/libapi/a21 main/psxsdk/libapi/l02 main/psxsdk/libapi/l03 main/psxsdk/libapi/patch main/psxsdk/libapi/c68 main/psxsdk/libapi/chclrpad main/psxsdk/libc2/memcpy main/psxsdk/libc2/rand main/psxsdk/libc2/strcpy main/psxsdk/libc2/strlen main/psxsdk/libc2/printf main/psxsdk/libapi/a71 main/psxsdk/libapi/a72 main/psxsdk/libc2/ctype main/psxsdk/libc2/memchr main/psxsdk/libc2/putchar

# -- Per-file lb/lh expansion opt-in --
# ASPSX expands lb→lbu+sll+sra and lh→lhu+sll+sra in certain contexts.
# These flags replicate that behavior via maspsx for files that need it.
EXPAND_LB_FILES := main/175A4 main/17AFC main/24BF0 main/24F08 main/25788
EXPAND_LH_FILES :=

# -- Rodata alignment: object-relative, one rule for every C object --
# Sony ASPSX pads `.align 3` (switch tables) relative to each object's start and
# PSYLINK places objects on 4-byte boundaries (measured with the PsyQ 3.5 tools,
# docs/grind/rodata-align-2026-09-30.md). GNU as already pads relative to the
# section start, so setting every object's .rodata alignment to 4 after `as`
# reproduces it. Owner ruling 2026-09-30 (.claude/rules/rodata-object-alignment.md);
# replaces the retired per-file `.align 3 -> .align 2` sed.
RODATA_OBJ_ALIGN := --set-section-alignment .rodata=4

# -- Per-file -fno-strength-reduce opt-in --
# Some functions were originally compiled without GCC's loop strength-reduction.
# These files get the flag applied; other files in NO_SR_FILES below.
NO_SR_FILES :=

# Every per-file list names TU ids. An id that is not a TU stops the build: a
# moved or renamed file must take its flags along, never silently lose them.
FLAG_LISTS := GP_FILES PSYQ_LIBRARY_FILES EXPAND_LB_FILES EXPAND_LH_FILES NO_SR_FILES
$(foreach l,$(FLAG_LISTS),$(foreach f,$($(l)),$(if $(filter $(f),$(C_IDS)),,$(error $(l) names '$(f)', which is not a TU (no $(SRC_DIR)/$(f).c)))))

# Helper: resolve CC/MASPSX flags based on whether file needs GP-relative
cc_flags_for = $(if $(filter $1,$(GP_FILES)),$(CC_FLAGS_GP),$(CC_FLAGS))$(if $(filter $1,$(NO_SR_FILES)), -fno-strength-reduce)
maspsx_flags_for = $(if $(filter $1,$(GP_FILES)),$(MASPSX_FLAGS_GP),$(MASPSX_FLAGS))$(if $(filter $1,$(PSYQ_LIBRARY_FILES)),, -G8)$(if $(filter $1,$(EXPAND_LB_FILES)), --expand-lb)$(if $(filter $1,$(EXPAND_LH_FILES)), --expand-lh)

# Shared pipeline dependencies for every C object. Without these, changing
# pipeline/toolchain config can leave stale objects in place because
# make only notices src/**/*.c timestamps.
PIPELINE_DEPS := Makefile \
	$(CC1) engine/buildconfig.py \
	tools/prologue_config.json \
	expand_lb_funcs.txt multu_funcs.txt multu_pad_funcs.txt expand_dest_funcs.txt maspsx_prefill_label_funcs.txt maspsx_comm_syms.txt \
	tools/prologue_fix.py tools/multu_pad.py \
	$(wildcard tools/maspsx/*.py tools/maspsx/maspsx/*.py) \
	$(wildcard include/* asm/funcs/*.s) $(shell find $(SRC_DIR) -name '*.h')

# -- Compile C source (decompiled functions) --
# Pipeline: cpp | cc1 | prologue_fix | maspsx | multu_pad | as -> .o, then objcopy (.rodata alignment 4)
$(BUILD_DIR)/$(SRC_DIR)/%.o: $(SRC_DIR)/%.c $(PIPELINE_DEPS)
	@mkdir -p $(dir $@)
	$(CPP) $(CPP_FLAGS) $(CPP_DEFS) $< | $(CC1) $(call cc_flags_for,$*) | $(PROLOGUE_FIX) | $(MASPSX) $(call maspsx_flags_for,$*) | $(MULTU_PAD) | $(AS) $(AS_FLAGS) -o $@
	$(OBJCOPY) $(RODATA_OBJ_ALIGN) $@

# -- Assemble .s files (non-decompiled asm) --
$(BUILD_DIR)/$(ASM_DIR)/%.o: $(ASM_DIR)/%.s $(wildcard include/*)
	@mkdir -p $(dir $@)
	$(AS) $(AS_FLAGS) $< -o $@

$(BUILD_DIR)/$(ASM_DIR)/data/%.o: $(ASM_DIR)/data/%.s $(wildcard include/*)
	@mkdir -p $(dir $@)
	$(AS) $(AS_FLAGS) $< -o $@

# -- Splat: re-split the binary --
setup:
	python3 -m splat split $(SPLAT_YAML)

# -- Clean --
clean:
	rm -rf $(BUILD_DIR)
