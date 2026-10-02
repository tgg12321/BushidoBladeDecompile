"""prep6.py <landing body> <out>: the landing body with the two Ruling 11 comment blocks removed (generator input)."""
import re, sys
s = open(sys.argv[1]).read()
n0 = s.count("/* Ruling 11")
s = re.sub(r"    /\* Ruling 11 .*?\*/\n", "", s, flags=re.S)
assert n0 == 2 and s.count("Ruling 11") == 0
open(sys.argv[2], 'w', newline='\n').write(s)
