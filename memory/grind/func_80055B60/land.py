# land.py: apply func_80055B60's landing edits in place (run ONLY under the landing lock).
#   src/text1b.c            INCLUDE_ASM line -> memory/grind/func_80055B60/candidate.c
#   include/code6cac.h      PracticeMenuRec members (memory/grind/func_80055B60/mkhdr.py logic), PadState moved up
#   undefined_syms_auto.txt retire D_80099D8D / D_80102790 alias rows; drop func_80055B60 from two retire notes
import re, runpy, sys

def rw(path, fn):
    s = open(path, encoding='utf-8', newline='').read()
    assert '\r' not in s
    t = fn(s)
    assert t != s, path
    open(path, 'w', encoding='utf-8', newline='\n').write(t)

def one(s, old, new):
    assert s.count(old) == 1, (old[:60], s.count(old))
    return s.replace(old, new)

cand = open('memory/grind/func_80055B60/candidate.c', encoding='utf-8').read()
rw('src/text1b.c', lambda s: one(s, 'INCLUDE_ASM("asm/funcs", func_80055B60);\n', cand))

# header: reuse mkhdr.py's transformation on the real file
src = open('memory/grind/func_80055B60/mkhdr.py').read()
src = src.replace("os.makedirs('tmp/b60/inc/include', exist_ok=True)\n", "")
src = src.replace("open('tmp/b60/inc/include/code6cac.h', 'w', encoding='utf-8', newline='\\n').write(h)",
                  "open('include/code6cac.h', 'w', encoding='utf-8', newline='\\n').write(h)")
assert "open('include/code6cac.h'" in src
exec(compile(src, 'mkhdr', 'exec'), {'__name__': 'mkhdr'})

def syms(s):
    s = one(s, "D_80099D8D = 0x80099D8D;  /* alias of D_80099D88+0x5 (StatusFlagRec record 0); retire with func_80055B60 (asm/funcs/func_80055B60.s is its only assembled referrer) */\n", "")
    s = one(s, "D_80102790 = 0x80102790;  /* alias of D_80102788+0x8; retire with func_80055B60 (asm/funcs/func_80055B60.s is its only referrer) */\n", "")
    s = one(s, "retire with func_80055B60 and func_80058580 (asm/funcs/func_80055B60.s and asm/funcs/func_80058580.s are its assembled referrers) */",
            "retire with func_80058580 (asm/funcs/func_80058580.s is its only assembled referrer) */")
    s = one(s, "retire with func_80023F08, func_8002AB08, func_80055B60 */", "retire with func_80023F08, func_8002AB08 */")
    return s
rw('undefined_syms_auto.txt', syms)
print('landed edits applied')
