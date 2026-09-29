"""Excerpt func_800187F4 .lreg windows around the cse class-head decisions: the `/ 8` expansion (the insn
adding const 7) and the focus-0 squared-distance compare (the insn comparing with const 1024; the first)."""
import re, sys
def body(t):
    return re.search(r"^;; Function func_800187F4\n(.*?)(?=^;; Function |\Z)", t, re.S | re.M).group(1)
for d in sys.argv[1:]:
    b = body(open(d + "/f.i.lreg").read())
    blks = [x for x in re.split(r"\n(?=\((?:insn|jump_insn|call_insn|code_label|note|barrier) )", b) if x.startswith("(")]
    def show(pred, before, after, title):
        idx = [i for i, x in enumerate(blks) if pred(re.sub(r"\s+", " ", x))]
        i = idx[0]
        print(f"--- {d.split('/')[-1]}: {title} (insns around uid {re.match(r'.(\w+) (\d+)', blks[i]).group(2)})")
        for x in blks[max(0, i - before): i + after + 1]:
            if x.startswith("(note"):
                continue
            print("   " + re.sub(r"\s+", " ", x)[:200])
    show(lambda x: re.search(r"\(plus:SI \(reg[^()]*\) \(const_int 7\)\)", x) is not None, 4, 2, "delta / 8 expansion")
    show(lambda x: "(const_int 1024)" in x, 5, 3, "focus-0 squared distance: sum, copy, compare")
