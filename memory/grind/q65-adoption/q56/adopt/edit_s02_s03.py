#!/usr/bin/env python3
"""one-off: update s02 docstring (M2 no longer held) and chain s03b into s03."""
p = "tmp/q56/adopt/s02_apply.py"
s = open(p, encoding="utf-8").read()
a = """  HELD (PLAN.md): M2 code6cac_c2+config, M3 text1a_c_tu2+text1a_c2+text1a_b+sound+text1b,
  M4 text1b_tu2+text1b_b - same kind of evidence, but merging surfaces 32 per-file declaration
  conflicts in completed functions (tmp/q56/adopt/conflicts_M2M3M4.txt)."""
b = """  (d) code6cac_c2 + config -> code6cac_c2.c (see below).
  HELD (PLAN.md): M3 text1a_c_tu2+text1a_c2+text1a_b+sound+text1b, M4 text1b_tu2+text1b_b - same
  kind of evidence, but merging surfaces 30 per-file declaration conflicts in completed functions
  (tmp/q56/adopt/conflicts_M2M3M4.txt); their shared objects are all zero-byte bss, which the
  interim tentative definitions of step 5 cover."""
assert s.count(a) == 1
open(p, "w", newline="\n", encoding="utf-8").write(s.replace(a, b))
p = "tmp/q56/adopt/s03_apply.py"
s = open(p, encoding="utf-8").read()
a = 'print("step 3 applied")'
b = '''import subprocess
subprocess.run([sys.executable, str(Path(__file__).parent / "s03b_fixtest.py"), str(R)], check=True)
print("step 3 applied")'''
assert s.count(a) == 1
open(p, "w", newline="\n", encoding="utf-8").write(s.replace(a, b))
print("ok")
