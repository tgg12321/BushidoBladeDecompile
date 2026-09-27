"""Q24 procedure (Q21 exception condition (1), owner ruling Q24), implemented once.

Listing form: each compiler's own -S output of the same preprocessed TU, before maspsx (as calib_b.sh produces), cut to
the one function. Parsing: strip directives, blank lines, comments; label lines are removed from the instruction list
but remembered as destinations. Symbol operands are compared by resolved address: the SYM table canonicalises a probe's
own names for the same bytes (g_sc, g_sc+1, g_tb, g_match_*, ...) to the target's symbols (D_800A3898, D_800A3899, ...). Placeholders BEFORE aligning: every register operand except `$0` -> REG; every label
operand -> LBL. Alignment: difflib.SequenceMatcher(autojunk=False). After alignment, a branch/jump in an equal run is
equal to its counterpart only if its destination instruction is aligned (in an equal run) with the counterpart's
destination instruction, or both leave the function; otherwise it is treated as an instruction of a changed run with
no equal counterpart."""
import difflib, re
from pathlib import Path

SYM = [("D_800A3898+1", "D_800A3899"), ("D_800A38AA+1", "D_800A38AB"),  # same resolved address
       ("g_match_p1_score+1", "D_800A3899"), ("g_match_p1_score", "D_800A3898"),
       ("g_match_p1_tiebreaker+1", "D_800A38AB"), ("g_match_p1_tiebreaker", "D_800A38AA"),
       ("g_match_score+1", "D_800A3899"), ("g_match_score", "D_800A3898"),
       ("g_match_tiebreaker+1", "D_800A38AB"), ("g_match_tiebreaker", "D_800A38AA"),
       ("g_sc+1", "D_800A3899"), ("g_sc", "D_800A3898"), ("g_tb+1", "D_800A38AB"), ("g_tb", "D_800A38AA")]
LABEL_RE = re.compile(r"[.$]L\d+")


def parse(path):
    """-> list of dicts: line, norm, dest_label (or None)."""
    text = Path(path).read_text().replace("\r", "")
    for a, b in SYM:
        text = text.replace(a, b)
    out, labels, pending = [], {}, []
    for i, raw in enumerate(text.splitlines(), 1):
        s = raw.split("#")[0].strip()
        if not s:
            continue
        m = re.match(r"^([.$]L\d+):$", s)
        if m:
            pending.append(m.group(1))
            continue
        if s.startswith(".") or s.endswith(":"):
            continue
        for lab in pending:
            labels[lab] = len(out)
        pending = []
        dest = LABEL_RE.search(s)
        norm = LABEL_RE.sub("LBL", s)
        norm = re.sub(r"\$(?!0\b)\d+", "REG", norm)
        norm = re.sub(r"\s+", " ", norm)
        out.append({"line": i, "norm": norm, "dest_label": dest.group(0) if dest else None})
    for lab in pending:
        labels[lab] = len(out)
    for ins in out:
        ins["dest"] = labels.get(ins["dest_label"]) if ins["dest_label"] else None
    return out


def align(a, b):
    """-> (opcodes, mapping a_index->b_index for equal pairs that survive the destination check, bad set of a indices)."""
    sm = difflib.SequenceMatcher(a=[x["norm"] for x in a], b=[x["norm"] for x in b], autojunk=False)
    ops = sm.get_opcodes()
    amap = {}
    for tag, i1, i2, j1, j2 in ops:
        if tag == "equal":
            for k in range(i2 - i1):
                amap[i1 + k] = j1 + k
    bad_a, bad_b = set(), set()
    for ia, ib in amap.items():
        da, db = a[ia]["dest"], b[ib]["dest"]
        if a[ia]["dest_label"] is None:
            continue
        leaves_a, leaves_b = da is None or da >= len(a), db is None or db >= len(b)
        if leaves_a and leaves_b:
            continue
        if leaves_a != leaves_b or amap.get(da) != db:
            bad_a.add(ia)
            bad_b.add(ib)
    return ops, amap, bad_a, bad_b


def differing(a, b):
    """Indices of a (reference) and b that do not survive as equal (changed runs + destination failures)."""
    ops, amap, bad_a, bad_b = align(a, b)
    da = {i for i in range(len(a)) if i not in amap} | bad_a
    db = {j for j in range(len(b)) if j not in set(amap.values())} | bad_b
    return ops, da, db


def exempt(tgt, psx):
    """Target-side exempt indices: differing with no equal-normalised counterpart among the cc1psx differing
    instructions of the same changed run (destination failures count as having no counterpart)."""
    ops, amap, bad_a, bad_b = align(tgt, psx)
    ex_t, ex_p = set(bad_a), set(bad_b)
    for tag, i1, i2, j1, j2 in ops:
        if tag == "equal":
            continue
        other = {psx[j]["norm"] for j in range(j1, j2)}
        mine = {tgt[i]["norm"] for i in range(i1, i2)}
        ex_t |= {i for i in range(i1, i2) if tgt[i]["norm"] not in other}
        ex_p |= {j for j in range(j1, j2) if psx[j]["norm"] not in mine}
    return ops, ex_t, ex_p


def presence_exempt(gov_norms, psx):
    """Position-not-decided part: governed instruction forms (with multiplicity) not present in the cc1psx reference output
    are exempt. Returns a Counter of exempt forms."""
    from collections import Counter
    need = Counter(gov_norms)
    have = Counter(x["norm"] for x in psx)
    return Counter({f: n - have[f] for f, n in need.items() if have[f] < n})


def presence_miss(gov_norms, exempt_counter, out):
    """Position-not-decided part: a spelling misses iff some non-exempt governed form occurs fewer times in its output."""
    from collections import Counter
    need = Counter(gov_norms) - exempt_counter
    have = Counter(x["norm"] for x in out)
    return {f: (n, have[f]) for f, n in need.items() if have[f] < n}
