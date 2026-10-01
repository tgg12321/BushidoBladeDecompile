# usage: python mkvar.py base.c out.c spec.py  (spec defines REPL = [(old,new),...])
import sys
base,out,spec=sys.argv[1:4]
s=open(base).read()
ns={}
exec(open(spec).read(),ns)
for a,b in ns['REPL']:
    assert a in s, ('MISSING', a[:80])
    s=s.replace(a,b,1)
open(out,'w',newline='\n').write(s)
