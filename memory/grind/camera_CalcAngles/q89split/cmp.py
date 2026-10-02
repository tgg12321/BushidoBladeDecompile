"""Per-function compare of two objdump -dr listings (order-free; addresses stripped)."""
import re, sys
def parse(p):
    fn, cur = {}, None
    for line in open(p):
        m = re.match(r"^[0-9a-f]+ <(\w+)>:", line)
        if m:
            cur = m.group(1); fn[cur] = []; continue
        if cur and line.strip():
            s = re.sub(r"^\s*[0-9a-f]+:\s*", "", line.rstrip())
            s = re.sub(r"\b[0-9a-f]+ <(\.L\w+|\w+)\+0x[0-9a-f]+>", r"<\1>", s)  # branch targets: label, not address
            s = re.sub(r"^\s*[0-9a-f]+: (R_MIPS)", r"\1", s)
            fn[cur].append(s)
    return fn
a, b = parse(sys.argv[1]), parse(sys.argv[2])
order_a = list(a); order_b = list(b)
diff = [f for f in a if f in b and a[f] != b[f]]
print("functions: G0 %d, G8 %d; only-G0 %s only-G8 %s" % (len(a), len(b), sorted(set(a) - set(b)), sorted(set(b) - set(a))))
print("same function order:", order_a == order_b)
print("differing functions:", diff)
