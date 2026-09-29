import sys, runpy
# applytp.py <file.c> : apply memory/grind/func_8005E54C/match0/tu_patch.py's patch() to a spliced TU copy in place
p = sys.argv[1]
fn = runpy.run_path('memory/grind/func_8005E54C/match0/tu_patch.py')['patch']
s = open(p, newline='\n').read()
open(p, 'w', newline='\n').write(fn(s))
