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
MASPSX       := python3 tools/maspsx/maspsx.py
MASPSX_FLAGS := --expand-div --aspsx-version=2.34

# GNU MIPS cross-tools
AS           := mipsel-linux-gnu-as
LD           := mipsel-linux-gnu-ld
OBJCOPY      := mipsel-linux-gnu-objcopy
CPP          := mipsel-linux-gnu-cpp

# -- Compiler Flags --
# -O2: standard optimization level for PsyQ games
# -G0: disable GP-relative addressing (tune later if needed)
# -funsigned-char: common PsyQ convention
# -mcpu=3000: target R3000A
CC_FLAGS     := -O2 -G0 -funsigned-char -quiet -mcpu=3000 -mips1 -mno-abicalls -fno-builtin -w
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
.PHONY: all clean setup check

all: check

# Build and verify match
check: $(EXE) $(TARGET).sha1
	@sha1sum -c $(TARGET).sha1 && echo "OK: $(TARGET) matches!" || echo "MISMATCH: $(TARGET) does not match"

# -- SHA1 checksum file --
$(TARGET).sha1:
	@echo "62efab4f73f992798c43e8c730aa43baa10bb4fa  $(EXE)" > $@

# -- Link --
$(ELF): $(ALL_O_FILES) $(TARGET).ld
	$(LD) $(LD_FLAGS) -Map $(BUILD_DIR)/$(TARGET).map -T $(TARGET).ld \
		-T undefined_funcs_auto.txt -T undefined_syms_auto.txt \
		-o $@

# Extract raw binary from ELF
$(BIN): $(ELF)
	$(OBJCOPY) -O binary -j .main $< $@

# Construct PS-X EXE (prepend original 0x800-byte header)
$(EXE): $(BIN)
	python3 tools/make_psexe.py $(TARGET_EXE) $< $@

# -- Compile C source (decompiled functions) --
# Pipeline: cpp | cc1 | maspsx | as -> .o
$(BUILD_DIR)/$(SRC_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CPP) $(CPP_FLAGS) $(CPP_DEFS) $< | $(CC1) $(CC_FLAGS) | $(MASPSX) $(MASPSX_FLAGS) | $(AS) $(AS_FLAGS) -o $@

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
