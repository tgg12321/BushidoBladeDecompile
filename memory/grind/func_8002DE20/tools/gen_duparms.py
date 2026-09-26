"""Per-value twin + duplicated-statement-into-arms (sanctioned family, jump2 cross-jump).

Every JOIN-POINT test (T4, T7, T9, T11: the tests reached from several failing
arms of the previous group) gets its two per-value writes duplicated into every
arm that reaches it: after each innermost `if (..) { return 1; }` (its fall-through)
and in an `else { }` of each enclosing `if`. The copy after the group is removed.
Each such local is then written in 2-3 blocks (multi-block, so global for
local-alloc); jump2's cross-jump can re-merge the copies at the join.
Nested / single-predecessor tests (T1-T3, T5, T6, T8, T10, T12) have no arms to
duplicate into and stay as in the twin.

usage: python3 gen_duparms.py <final_pv.c> <out.c> [T4,T7,T9,T11]
"""
import sys

inp, out = sys.argv[1], sys.argv[2]
joins = set((sys.argv[3] if len(sys.argv) > 3 else "T4,T7,T9,T11").split(","))
lines = open(inp, encoding="utf-8").read().split("\n")
JOIN_FIRST = {"T4": "    cross_a4 = ", "T7": "    cross_a7 = ", "T9": "    cross_a9 = ",
              "T11": "    cross_a11 = "}


def ind(s):
    return len(s) - len(s.lstrip())


for tag in ["T11", "T9", "T7", "T4"]:  # bottom-up keeps earlier indices valid
    if tag not in joins:
        continue
    w0 = next(i for i, ln in enumerate(lines) if ln.startswith(JOIN_FIRST[tag]))
    W = [lines[w0].strip(), lines[w0 + 1].strip()]
    # the group ends right before w0 (a comment line may sit between)
    end = w0 - 1
    while lines[end].strip().startswith("/*"):
        end -= 1
    assert lines[end] == "    }", (tag, lines[end])
    start = end
    while not (ind(lines[start]) == 4 and lines[start].lstrip().startswith("if (")):
        start -= 1
    grp = lines[start:end + 1]
    new = []
    opens = []  # stack of (indent, innermost?)
    for k, ln in enumerate(grp):
        s = ln.strip()
        if s.startswith("if (") and s.endswith("{"):
            innermost = grp[k + 1].strip() == "return 1;"
            opens.append((ind(ln), innermost))
            new.append(ln)
        elif s == "}" and opens and ind(ln) == opens[-1][0]:
            i0, innermost = opens.pop()
            if innermost:
                new.append(ln)
                new += [" " * i0 + w + " /* FAKE: duplicated into the arm */" for w in W]
            else:
                new.append(" " * i0 + "} else {")
                new += [" " * (i0 + 4) + w + " /* FAKE: duplicated into the arm */" for w in W]
                new.append(ln)
        else:
            new.append(ln)
    lines = lines[:start] + new + lines[end + 1:w0] + lines[w0 + 2:]
open(out, "w", encoding="utf-8", newline="\n").write("\n".join(lines))
print(out, sorted(joins))
