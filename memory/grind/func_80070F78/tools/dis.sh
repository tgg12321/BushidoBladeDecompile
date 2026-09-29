cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
# usage: dis.sh <name> <from> <to>   (line range of function disassembly)
mipsel-linux-gnu-objdump -d -r --no-show-raw-insn tmp/func_80070F78/sc/$1/text1b.o | awk '/<func_80070F78>:/{f=1} f' | grep -v "^\s*$" | sed -n "$2,$3p"
