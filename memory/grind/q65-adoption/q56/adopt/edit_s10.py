#!/usr/bin/env python3
"""one-off: s10 = v1 s04 minus the declaration fixes (now step 9); s11 = v1 s05 renamed."""
import re
p = "tmp/q56/adopt/s10_apply.py"
s = open(p, encoding="utf-8").read()
a = s.index("def one_declaration_fixes():")
b = s.index("one_declaration_fixes()\n", a + 30) + len("one_declaration_fixes()\n")
s = s[:a] + s[b:]
s = s.replace('"""Step 4 (Q65 adoption): the switch to the per-file gp model.',
              '"""Step 10 (Q65 adoption): the switch to the per-file gp model.')
s = s.replace("(the fixes above included: this proves them byte-neutral\n# under the lists)", "")
s = s.replace("usage: s04_apply.py <tree>   (the tree's build/ must be the step-3 oracle build)",
              "Needs rule amendments A1 (K2 = every object of a file's per-file static block) and A2 (objects between\n"
              "a file's gp objects are its definitions: la/indexed-only objects and sized gap fillers) - PLAN.md.\n"
              "usage: s10_apply.py <tree>")
s = s.replace('"pre-switch tree (with the declaration fixes) is not at the oracle"', '"pre-switch tree is not at the oracle"')
s = s.replace('open("/tmp/q56/s04_log.txt", "w")', 'open("/tmp/q56/s10_log.txt", "w")')
s = s.replace('print("step 4 applied")', 'print("step 10 applied")')
open(p, "w", newline="\n", encoding="utf-8").write(s)
p = "tmp/q56/adopt/s11_apply.py"
s = open(p, encoding="utf-8").read()
s = s.replace('"""Step 5 (Q65 adoption)', '"""Step 11 (Q65 adoption)')
s = s.replace("follow step 2's moves", "follow steps 2-6's moves")
s = s.replace("usage: s05_apply.py <tree>", "usage: s11_apply.py <tree>")
s = s.replace('# the files step 2 split / merged / cut', '# the files steps 2-6 split / merged / cut')
s = s.replace('print("step 5 applied")', 'print("step 11 applied")')
open(p, "w", newline="\n", encoding="utf-8").write(s)
print("ok")
