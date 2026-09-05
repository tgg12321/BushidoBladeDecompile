# Bushido Blade 2 (SLUS-00663) Matching Decompilation
# Build system for PS1 (PsyQ SDK 3.5, GCC 2.7.2, ASPSX 2.34)

# -- Configuration --
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
MASPSX_FLAGS := --expand-div --aspsx-version=2.34 --sdata-syms=sdata_syms.txt --sdata-funcs=sdata_funcs.txt --sdata-exclude=sdata_exclude.txt --expand-lb --expand-lb-funcs=expand_lb_funcs.txt --multu-funcs=multu_funcs.txt --expand-dest-funcs=expand_dest_funcs.txt --label-nop-funcs=maspsx_label_nop_funcs.txt --prefill-label-funcs=maspsx_prefill_label_funcs.txt
MASPSX_FLAGS_GP := --expand-div --aspsx-version=2.34 --sdata-syms=sdata_syms.txt --sdata-funcs=sdata_funcs.txt --sdata-exclude=sdata_exclude.txt --expand-lb --expand-lb-funcs=expand_lb_funcs.txt --multu-funcs=multu_funcs.txt --expand-dest-funcs=expand_dest_funcs.txt --label-nop-funcs=maspsx_label_nop_funcs.txt --prefill-label-funcs=maspsx_prefill_label_funcs.txt

# GNU MIPS cross-tools
AS           := mipsel-linux-gnu-as
LD           := mipsel-linux-gnu-ld
OBJCOPY      := mipsel-linux-gnu-objcopy
CPP          := mipsel-linux-gnu-cpp

# -- Compiler Flags --
# -O2: standard optimization level for PsyQ games
# -G0: disable GP-relative addressing (default for most files)
# -G8: enable GP-relative for files that need it (uses sdata_syms.txt filtering)
# -funsigned-char: common PsyQ convention
# -mcpu=3000: target R3000A
# -mel: MANDATORY. The prebuilt cc1's mips-mips-gnu triple defaults BIG-endian;
# -mel flips BYTES_BIG_ENDIAN (spill-slot layout, bitfield direction, lwl/lwr).
# Load-bearing for the oracle match — do not remove (see AGENTS.md).
CC_FLAGS     := -O2 -G0 -funsigned-char -quiet -mcpu=3000 -mips1 -mno-abicalls -fno-builtin -w -mel
CC_FLAGS_GP  := -O2 -G8 -funsigned-char -quiet -mcpu=3000 -mips1 -mno-abicalls -fno-builtin -w -mel
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

# C source files (decompiled functions)
C_FILES      := $(wildcard $(SRC_DIR)/*.c)
C_O_FILES    := $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/$(SRC_DIR)/%.o,$(C_FILES))

# Per-function asm objects linked directly (explicit opt-in — NOT a wildcard;
# asm/funcs/ holds all 1437 split functions and we want only the listed ones).
# Used where a function cannot live inside a C translation unit: text1a is the
# project's sole -G8 file, and under -G8 cc1 buffers function bodies to a temp
# file (TARGET_FILE_SWITCHING) so a file-scope __asm__ floats to the top of the
# TU instead of staying in place. bb2.ld links this object between the
# text1a_pre / text1a_post fragments.
LINKED_ASM_FUNCS := save_vc_ctrl
ASM_FUNC_O_FILES := $(patsubst %,$(BUILD_DIR)/$(ASM_DIR)/funcs/%.o,$(LINKED_ASM_FUNCS))

# All objects
ALL_O_FILES  := $(S_O_FILES) $(DATA_O_FILES) $(C_O_FILES) $(ASM_FUNC_O_FILES)

# -- Top-level targets --
.PHONY: all clean setup check clean-check

all: check

# Build and verify match
check: $(EXE) $(TARGET).sha1
	@sha1sum -c $(TARGET).sha1 && echo "OK: $(TARGET) matches!" || echo "MISMATCH: $(TARGET) does not match"

# Force a clean rebuild before checking the final hash.
clean-check: clean check

# -- SHA1 checksum file --
$(TARGET).sha1:
	@echo "62efab4f73f992798c43e8c730aa43baa10bb4fa  $(EXE)" > $@

# -- Link --
$(ELF): $(ALL_O_FILES) $(TARGET).ld
	$(LD) $(LD_FLAGS) -Map $(BUILD_DIR)/$(TARGET).map -T $(TARGET).ld \
		-T undefined_funcs_auto.txt -T undefined_syms_auto.txt -T named_syms.txt \
		-o $@

# Extract raw binary from ELF
$(BIN): $(ELF)
	$(OBJCOPY) -O binary -j .main $< $@

# Construct PS-X EXE (prepend original 0x800-byte header)
$(EXE): $(BIN)
	python3 tools/make_psexe.py $(TARGET_EXE) $< $@

# -- Per-file GP-relative opt-in --
# List C files (without path/extension) that need GP-relative addressing.
# These are compiled with -G8 and use sdata_syms.txt for selective GP-rel.
GP_FILES := text1a_pre text1a_post

# -- Per-file lb/lh expansion opt-in --
# ASPSX expands lb→lbu+sll+sra and lh→lhu+sll+sra in certain contexts.
# These flags replicate that behavior via maspsx for files that need it.
EXPAND_LB_FILES := code6cac_b
EXPAND_LH_FILES :=

# -- Per-file rodata alignment fix --
# GCC 2.7.2 emits .align 3 (8-byte) for switch tables in .rodata.
# When rodata is split across objects, this creates unwanted padding.
# Downgrade to .align 2 (4-byte) for files whose rodata is sandwiched.
RODATA_ALIGN2_FILES := code6cac code6cac_b code6cac_c code6cac_c0 code6cac_c_ab code6cac_c2 text1a_pre text1a_post text1a_b text1a_c text1a_c2 text1b_b main

# -- Per-file -fno-strength-reduce opt-in --
# Some functions were originally compiled without GCC's loop strength-reduction.
# These files get the flag applied; other files in NO_SR_FILES below.
NO_SR_FILES :=

# Helper: resolve CC/MASPSX flags based on whether file needs GP-relative
cc_flags_for = $(if $(filter $1,$(GP_FILES)),$(CC_FLAGS_GP),$(CC_FLAGS))$(if $(filter $1,$(NO_SR_FILES)), -fno-strength-reduce)
maspsx_flags_for = $(if $(filter $1,$(GP_FILES)),$(MASPSX_FLAGS_GP),$(MASPSX_FLAGS))$(if $(filter $1,$(EXPAND_LB_FILES)), --expand-lb)$(if $(filter $1,$(EXPAND_LH_FILES)), --expand-lh)
rodata_align_fix = $(if $(filter $1,$(RODATA_ALIGN2_FILES)),sed "s/\.align\t3/.align\t2/" |,)

# Shared pipeline dependencies for every C object. Without these, changing
# pipeline/toolchain config can leave stale objects in place because
# make only notices src/*.c timestamps.
PIPELINE_DEPS := Makefile \
	tools/prologue_config.json \
	sdata_syms.txt sdata_funcs.txt sdata_exclude.txt expand_lb_funcs.txt multu_funcs.txt multu_pad_funcs.txt expand_dest_funcs.txt \
	tools/prologue_fix.py tools/multu_pad.py \
	tools/maspsx/maspsx.py tools/maspsx/maspsx/__init__.py

# -- Compile C source (decompiled functions) --
# Pipeline: cpp | cc1 | prologue_fix | maspsx | [sed align fix] | multu_pad | as -> .o
$(BUILD_DIR)/$(SRC_DIR)/%.o: $(SRC_DIR)/%.c $(PIPELINE_DEPS)
	@mkdir -p $(dir $@)
	$(CPP) $(CPP_FLAGS) $(CPP_DEFS) $< | $(CC1) $(call cc_flags_for,$*) | $(PROLOGUE_FIX) | $(MASPSX) $(call maspsx_flags_for,$*) | $(call rodata_align_fix,$*) $(MULTU_PAD) | $(AS) $(AS_FLAGS) -o $@

# -- Assemble .s files (non-decompiled asm) --
$(BUILD_DIR)/$(ASM_DIR)/%.o: $(ASM_DIR)/%.s
	@mkdir -p $(dir $@)
	$(AS) $(AS_FLAGS) $< -o $@

$(BUILD_DIR)/$(ASM_DIR)/data/%.o: $(ASM_DIR)/data/%.s
	@mkdir -p $(dir $@)
	$(AS) $(AS_FLAGS) $< -o $@

# -- Splat: re-split the binary --
setup:
	python3 -m splat split $(SPLAT_YAML)

# -- Clean --
clean:
	rm -rf $(BUILD_DIR)
