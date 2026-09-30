#!/bin/bash
# align_exp.sh <mode> : experimental build (NOT committed) of the "object-relative .align" model.
#   mode=none   : remove the per-file sed (.align 3 -> .align 2), nothing else
#   mode=global : remove the sed AND set every C object's .rodata section alignment to 4 after `as`
# Prints the bb2.bin sha1 and whether it equals the reference.
cd /home/user/BushidoBladeDecompile || exit 1
mode=$1
sed -e 's/^RODATA_ALIGN2_FILES := .*/RODATA_ALIGN2_FILES :=/' Makefile > tmp/Makefile.align
if [ "$mode" = global ]; then
python3 - <<'EOF'
p = "tmp/Makefile.align"
s = open(p).read()
old = "$(MULTU_PAD) | $(AS) $(AS_FLAGS) -o $@\n"
assert s.count(old) == 1
s = s.replace(old, old + "\t$(OBJCOPY) --set-section-alignment .rodata=4 $@\n")
open(p, "w", newline="\n").write(s)
EOF
fi
rm -rf build
make -f tmp/Makefile.align -j16 build/bb2.bin > tmp/align_exp.log 2>&1
echo "make rc=$?"
sha1sum build/bb2.bin
cmp build/bb2.bin /tmp/claude-0/ref_main.bin && echo "IDENTICAL to reference" || cmp -l build/bb2.bin /tmp/claude-0/ref_main.bin | wc -l
