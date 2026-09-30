import sys
# Insert the fix1 game.h hunk (added lines only) after the D_8009B490 declaration.
pt = open('memory/grind/func_8005C8A8/fix1-merges.patch').read()
start = pt.index('+++ b/include/game.h')
end = pt.index('\ndiff ', start)
hunk = pt[start:end].splitlines()
added = [l[1:] for l in hunk[2:] if l.startswith('+')]
p = sys.argv[1]
g = open(p).read()
anchor = 'extern Unk8009B400Record D_8009B490[2][2];\n'
assert anchor in g
g = g.replace(anchor, anchor + '\n' + '\n'.join(added) + '\n', 1)
open(p, 'w').write(g)
