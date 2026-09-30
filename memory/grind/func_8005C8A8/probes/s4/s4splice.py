import sys
src = open('src/text1b_tu1c.c').read()
key = 'INCLUDE_ASM("asm/funcs", func_8005C8A8);'
assert key in src
open(sys.argv[2], 'w').write(src.replace(key, open(sys.argv[1]).read()))
