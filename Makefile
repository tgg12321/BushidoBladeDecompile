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
FIX_LWL      := python3 tools/fix_lwl.py
REGFIX       := python3 tools/regfix.py
REGFIX_STAGE2:= REGFIX_CONFIG=regfix_stage2.txt python3 tools/regfix.py
ASMFIX       := python3 tools/asmfix.py
MASPSX       := python3 tools/maspsx/maspsx.py
MASPSX_FLAGS := --expand-div --aspsx-version=2.34 --sdata-syms=sdata_syms.txt --sdata-funcs=sdata_funcs.txt --sdata-exclude=sdata_exclude.txt --expand-lb --expand-lb-funcs=expand_lb_funcs.txt
MASPSX_FLAGS_GP := --expand-div --aspsx-version=2.34 --dont-force-G0 --sdata-syms=sdata_syms.txt -G8

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
CC_FLAGS     := -O2 -G0 -funsigned-char -quiet -mcpu=3000 -mips1 -mno-abicalls -fno-builtin -w
CC_FLAGS_GP  := -O2 -G8 -funsigned-char -quiet -mcpu=3000 -mips1 -mno-abicalls -fno-builtin -w
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

# All objects
ALL_O_FILES  := $(S_O_FILES) $(DATA_O_FILES) $(C_O_FILES)

# -- Top-level targets --
.PHONY: all clean setup check clean-check validate

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
GP_FILES :=

# -- Per-file lb/lh expansion opt-in --
# ASPSX expands lb→lbu+sll+sra and lh→lhu+sll+sra in certain contexts.
# These flags replicate that behavior via maspsx for files that need it.
EXPAND_LB_FILES := code6cac_b
EXPAND_LH_FILES :=

# -- Per-file fix_lwl opt-in --
# GCC 2.7.2 emits lwl/lwr/swl/swr with big-endian offsets; XOR with 3 for little-endian.
# Only enable for files containing lwl/lwr in maspsx output (without compensating inline asm).
FIX_LWL_FILES := code6cac

# -- Per-file rodata alignment fix --
# GCC 2.7.2 emits .align 3 (8-byte) for switch tables in .rodata.
# When rodata is split across objects, this creates unwanted padding.
# Downgrade to .align 2 (4-byte) for files whose rodata is sandwiched.
RODATA_ALIGN2_FILES := code6cac code6cac_b code6cac_c code6cac_c0 code6cac_c_ab code6cac_c_mid code6cac_c2 text1a text1a_b text1a_c text1a_c2 main

# Helper: resolve CC/MASPSX flags based on whether file needs GP-relative
cc_flags_for = $(if $(filter $1,$(GP_FILES)),$(CC_FLAGS_GP),$(CC_FLAGS))
maspsx_flags_for = $(if $(filter $1,$(GP_FILES)),$(MASPSX_FLAGS_GP),$(MASPSX_FLAGS))$(if $(filter $1,$(EXPAND_LB_FILES)), --expand-lb)$(if $(filter $1,$(EXPAND_LH_FILES)), --expand-lh)
fix_lwl_for = $(if $(filter $1,$(FIX_LWL_FILES)),$(FIX_LWL) |,)
rodata_align_fix = $(if $(filter $1,$(RODATA_ALIGN2_FILES)),sed "s/\.align\t3/.align\t2/" |,)

# Shared pipeline dependencies for every C object. Without these, changing
# regfix/asmfix/toolchain config can leave stale objects in place because
# make only notices src/*.c timestamps.
PIPELINE_DEPS := Makefile \
	tools/prologue_config.json \
	regfix.txt regfix_stage2.txt asmfix.txt \
	sdata_syms.txt sdata_funcs.txt sdata_exclude.txt expand_lb_funcs.txt \
	tools/prologue_fix.py tools/fix_lwl.py tools/regfix.py tools/asmfix.py \
	tools/maspsx/maspsx.py tools/maspsx/maspsx/__init__.py

# -- Compile C source (decompiled functions) --
# Pipeline: cpp | cc1 | prologue_fix | maspsx | [fix_lwl] | [sed align fix] | regfix | regfix_stage2 | asmfix | as -> .o
$(BUILD_DIR)/$(SRC_DIR)/%.o: $(SRC_DIR)/%.c $(PIPELINE_DEPS)
	@mkdir -p $(dir $@)
	$(CPP) $(CPP_FLAGS) $(CPP_DEFS) $< | $(CC1) $(call cc_flags_for,$*) | $(PROLOGUE_FIX) | $(MASPSX) $(call maspsx_flags_for,$*) | $(call fix_lwl_for,$*) $(call rodata_align_fix,$*) $(REGFIX) | $(REGFIX_STAGE2) | $(ASMFIX) | $(AS) $(AS_FLAGS) -o $@

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

# -- Validate regfix.txt rules against current pipeline output --
# Catches label-renumber drift, stale indices, post-swap pattern mismatches.
# See feedback_label_renumber_breaks_regfix.md.
validate:
	python3 tools/validate_regfix.py --live

# -- Clean --
clean:
	rm -rf $(BUILD_DIR)
