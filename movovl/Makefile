# MOVOVL.EXE overlay build — standalone, invoked as `make -C movovl`.
# All-asm first build per docs/movovl/APPLY-PLAN.md: the split must byte-match
# the overlay oracle (movovl.sha1) with ZERO C before any draft is wired in.
# The .header section is part of the link (splat split the 0x800 PS-EXE header
# as data), so objcopy of the full ELF yields the complete 124,928-byte file.

AS      := mipsel-linux-gnu-as
LD      := mipsel-linux-gnu-ld
OBJCOPY := mipsel-linux-gnu-objcopy
ASFLAGS := -Iinclude -march=r3000 -mtune=r3000 -no-pad-sections -O1 -G0

ASM_S := $(wildcard asm/*.s) $(wildcard asm/data/*.s)
OBJS  := $(patsubst asm/%.s,build/asm/%.o,$(ASM_S))

all: check

build/asm/%.o: asm/%.s
	@mkdir -p $(dir $@)
	$(AS) $(ASFLAGS) $< -o $@

build/movovl.elf: $(OBJS) movovl.ld
	$(LD) -nostdlib --no-check-sections -Map build/movovl.map \
		-T movovl.ld \
		-T undefined_funcs_auto.movovl.txt -T undefined_syms_auto.movovl.txt \
		-o $@

build/MOVOVL.EXE: build/movovl.elf
	$(OBJCOPY) -O binary $< $@

check: build/MOVOVL.EXE
	cd build && sha1sum -c ../movovl.sha1

clean:
	rm -rf build

.PHONY: all check clean
