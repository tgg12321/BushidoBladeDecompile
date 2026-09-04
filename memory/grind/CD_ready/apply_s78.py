#!/usr/bin/env python3
"""Apply a CD_ready variant (body + declaration-surface directives) into src/system.c.

Directives (lines at the top of the variant file, stripped from the body):
  //DROPALL:<exact source line>
  //INS_BEFORE:<anchor exact line>|<text to insert (\n for newlines)>
  //REPLALL:<exact line> => <replacement line>
Body is extracted like s63/splice.py and RENAME-mapped, then spliced at the
CD_ready INCLUDE_ASM marker.
"""
import sys, re, pathlib

ROOT = pathlib.Path('.')
BAK = ROOT / 'tmp/grind/CD_ready/s61/system.c.bak'
SRC = ROOT / 'src/system.c'
MARK = 'INCLUDE_ASM("asm/funcs", CD_ready);'
RENAME = [("marionation_Exec","CD_ready"),("sys_VSync","VSync"),
          ("tslTm2LoadImage_2","puts"),("debug_printf","printf"),
          ("cdrom_ClearIrq","CD_flush"),("sys_GetVblankCount","CheckCallback"),
          ("func_80080828","getintr")]

def extract_fn(text):
    m = re.search(r"^s32 (marionation_Exec|CD_ready)\(s32 a0, u8 \*a1\)\s*\{", text, re.M)
    if not m: return None
    start = m.start(); depth = 0; i = m.end()-1
    while i < len(text):
        if text[i]=='{': depth+=1
        elif text[i]=='}':
            depth-=1
            if depth==0: return text[start:i+1]
        i+=1
    return None

def main():
    raw = pathlib.Path(sys.argv[1]).read_text()
    ops, keep = [], []
    for ln in raw.split('\n'):
        s = ln.strip()
        if s.startswith('//DROPALL:'): ops.append(('drop', s[10:].strip(), None))
        elif s.startswith('//INS_BEFORE:'):
            a,b = s[13:].split('|',1); ops.append(('ins', a.strip(), b.replace('@@', chr(10))))
        elif s.startswith('//REPLALL:'):
            a,b = s[10:].split('=>',1); ops.append(('repl', a.strip(), b.strip()))
        else: keep.append(ln)
    fn = extract_fn('\n'.join(keep))
    assert fn is not None, 'no function body'
    for a,b in RENAME: fn = re.sub(r'\b%s\b'%a, b, fn)
    out = BAK.read_text()
    for kind,a,b in ops:
        assert (a+'\n') in out, kind+' not found: '+a
        if kind=='drop': out = out.replace(a+'\n','')
        elif kind=='ins': out = out.replace(a+'\n', b+'\n'+a+'\n', 1)
        else: out = out.replace(a+'\n', b+'\n')
    assert MARK in out
    out = out.replace(MARK, fn.rstrip())
    open(str(SRC),'w',newline='\n').write(out)
    print('applied', sys.argv[1], len(fn.splitlines()), 'body lines,', len(ops), 'decl ops')

main()
