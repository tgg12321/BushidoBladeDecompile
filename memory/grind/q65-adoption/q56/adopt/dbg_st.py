import sys, os
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from splitc_lib import statements
raw = open(sys.argv[1]).read()
L = raw.split("\n")
a, b = int(sys.argv[2]), int(sys.argv[3])
for st in statements(raw, 0, len(L)):
    if st["b"] >= a - 1 and st["a"] <= b - 1:
        print(st["kind"], st["a"] + 1, st["b"] + 1, repr(" ".join(st["text"].split())[:100]))
