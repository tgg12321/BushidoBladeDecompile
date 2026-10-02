"""post.py <files...>: x/z-copy form -> nocopy form in place (drop the copies, read vec directly)."""
import re, sys
for f in sys.argv[1:]:
    s = open(f).read()
    n = len(re.findall(r"^\s*s32 x = vec\.vx;\n", s, flags=re.M))
    s = re.sub(r"^\s*s32 x = vec\.vx;\n", "", s, flags=re.M)
    s = re.sub(r"^\s*s32 z = vec\.vz;\n", "", s, flags=re.M)
    s = re.sub(r"\bz\b", "vec.vz", s)
    s = re.sub(r"\bx\b", "vec.vx", s)
    assert n == 2, (f, n)
    open(f, 'w', newline='\n').write(s)
