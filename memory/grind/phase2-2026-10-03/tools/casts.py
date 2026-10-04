#!/usr/bin/env python3
"""casts.py OUTDIR : census of raw-offset casts in every linked src/**/*.c (run in WSL, repo root).

A site is a pointer cast `(T *)` (T a scalar, pointer or struct type) applied to a base plus a constant
byte offset, or used as a field view:
  deref     *(T *)(BASE + OFF)        load or store of a T at BASE+OFF           (kind deref-load/-store)
  index     ((T *)BASE)[N]            the T at BASE + N*sizeof(T)                (kind index-load/-store)
  member    ((S *)BASE)->m            a struct view cast onto an untyped base    (kind struct-cast)
  addr      (T *)(BASE + OFF)         a pointer to BASE+OFF, not dereferenced    (kind addr)
  ptradd    (u8 *)BASE + OFF          byte-pointer arithmetic outside a cast     (kind ptradd)
`deref`/`index` with OFF 0 and no arithmetic are type puns (kind pun-load/-store); a non-constant
term in the offset makes the site `computed` (array arithmetic, not a field).
For each site: the base expression, its root identifier and that identifier's declared type (locals,
parameters, TU globals, headers), and the struct the base points at where determinable (cmodel.resolve):
  decl            the root is declared as that struct (pointer / object / array): the cast discards a type
  decl-elsewhere  the root is a global this TU declares untyped but another TU or header declares as it
  member          the base is a member of a typed struct whose member type is that struct
  return          the base is a call whose prototype returns a pointer to it
  assign(k/n)     k of the n assignments to the untyped local carry it (none carries another struct)
  caller(k/n)     k of the n C call sites pass it to the untyped parameter (none passes another)
  ambiguous(...)  assignments / callers carry different structs;  scratchpad: a 0x1F800xxx literal
then the member at OFF in that struct's cc1 layout (cmodel.leaf): exact (same size and signedness) /
sign (same size, other signedness) / size / inside (OFF inside a member) / pad (an unk_ byte-pad array:
a member to add) / beyond (OFF >= sizeof) / addr-of (addr/ptradd sites). `fake`: site = a FAKE comment
within 3 lines above, fn = elsewhere in the function; `cast_note`: a function comment ties a cast to
codegen. Writes OUTDIR/casts.tsv, casts_by_struct.tsv, casts_by_file.tsv."""
import collections, os, re, sys
sys.path.insert(0, ".")
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from ctok import read_type, EXPR_KW
from cmodel import *
from cmodel import OPERAND_END

OUT = sys.argv[1]

rows = []
for u in units.values():
    T = u.T
    for f in u.funcs:
        q, e = f["body"]
        skip = set()
        for k in range(q, e):
            if T.t[k] != "(" or k in skip or T.t[k - 1] == "sizeof":
                continue
            r = read_type(T, k + 1, u.tn)
            if not r:
                continue
            base, j = r
            stars = 0
            while j < e and T.t[j] in ("*", "const", "volatile"):
                stars += T.t[j] == "*"
                j += 1
            if stars == 0 or T.t[j] != ")" or base in ("struct", "union"):
                continue
            ce = j
            pstars = stars - 1
            psz = scal(base, pstars)
            acc = base + (" " + "*" * pstars if pstars else "")
            accsize = psz[0] if psz else (allstructs[base]["size"] if base in allstructs and not pstars else None)
            accsign = psz[1] if psz else None
            o = ce + 1
            kind = None
            off, computed, base_tt = 0, False, None
            if T.t[o] == "(":
                oe = T.match(o)
                inner = T.t[o + 1:oe]
                terms = split_add(inner)
                if len(terms) >= 2:
                    base_tt = terms[0][1] if terms[0][0] == 1 else None
                    cs = [const_val(t) for s_, t in terms[1:]]
                    if base_tt is None:
                        continue
                    if all(c is not None for c in cs):
                        off = sum(s_ * c for (s_, t), c in zip(terms[1:], cs))
                    else:
                        computed = True
                        off = sum(s_ * c for (s_, t), c in zip(terms[1:], cs) if c is not None)
                else:
                    base_tt = inner
                opend = oe
                # do not count the head cast of the base term again
                bt0 = o + 1
                if T.t[bt0] == "(":
                    skip.add(bt0)
            elif re.match(r"[A-Za-z_&]", T.t[o]):
                j2 = o + (1 if T.t[o] == "&" else 0)
                j2 += 1
                while j2 < e and T.t[j2] in ("[", "->", "."):
                    j2 = T.match(j2) + 1 if T.t[j2] == "[" else j2 + 2
                base_tt = T.t[o:j2]
                opend = j2 - 1
                if T.t[opend + 1] in ("+", "-") and T.t[k - 1] != "*" and not (T.t[k - 1] == "(" and T.t[opend + 1] == ")"):
                    # (u8 *)p + N : pointer arithmetic on the cast
                    tt2, d = [], 0
                    for i2 in range(opend + 2, e):
                        x = T.t[i2]
                        if x in "([":
                            d += 1
                        elif x in ")]":
                            if d == 0:
                                break
                            d -= 1
                        if d == 0 and x in (",", ";", "?", ":", "==", "!=", "<", ">", "&&", "||", "="):
                            break
                        tt2.append(x)
                    c = const_val(tt2)
                    if c is not None and T.t[opend + 1] in ("+", "-"):
                        kind = "ptradd"
                        off = (c if T.t[opend + 1] == "+" else -c) * (accsize or 1)
                    elif tt2:
                        kind = "ptradd"
                        computed = True
            else:
                continue
            prev = T.t[k - 1]
            pprev = T.t[k - 2]
            after = T.t[opend + 1] if opend + 1 < len(T.t) else ""
            if kind is None:
                if prev == "*" and (not OPERAND_END.match(pprev) or pprev in EXPR_KW):
                    a2 = after
                    store = a2 in ("=", "+=", "-=", "|=", "&=", "^=", "<<=", ">>=", "*=", "/=", "++", "--") or \
                        T.t[k - 2] in ("++", "--")
                    kind = ("deref" if (off or computed or len(split_add(T.t[o + 1:opend])) > 1) else "pun") + \
                        ("-store" if store else "-load")
                elif prev == "(" and after == ")" and opend + 2 < len(T.t) and T.t[opend + 2] == "[":
                    ie = T.match(opend + 2)
                    c = const_val(T.t[opend + 3:ie])
                    if c is None:
                        computed = True
                    else:
                        off += c * (accsize or 0)
                    a2 = T.t[ie + 1] if ie + 1 < len(T.t) else ""
                    store = a2 in ("=", "+=", "-=", "|=", "&=", "^=", "<<=", ">>=", "++", "--")
                    kind = "index-store" if store else "index-load"
                elif prev == "(" and after == ")" and opend + 2 < len(T.t) and T.t[opend + 2] == "->":
                    kind = "struct-cast"
                    mname = T.t[opend + 3]
                    if base in allstructs:
                        m = next((m for m in allstructs[base]["members"] if m["name"] == mname), None)
                        acc = f"{base}->{mname}"
                        accsize, accsign = None, None
                        if m:
                            off += m["off"]
                else:
                    if not (off or computed):
                        continue   # a plain pointer conversion, not a field view
                    kind = "addr"
            s, how, root, decl = resolve(u, f, base_tt or [])
            if computed:
                match, path = "computed", (leaf(s, off, None, None)[1] if s else "")
            elif s and kind == "struct-cast":
                match, path = ("same-struct" if s == base else "other-struct:" + base), ""
            elif s:
                match, path = leaf(s, off, accsize if kind not in ("addr", "ptradd") else None,
                                   accsign if kind not in ("addr", "ptradd") else None)
                if kind in ("addr", "ptradd") and match == "aggregate":
                    match = "addr-of"
            else:
                match, path = "", ""
            line = T.line(k)
            fake = "site" if any(line - 3 <= fl <= line for fl in f["fake_lines"]) else \
                ("fn" if f["fake_lines"] else "")
            rows.append(dict(file=u.path, line=line, func=f["name"], kind=kind, acc=acc,
                             size=accsize if accsize is not None else "", base=" ".join(base_tt or []),
                             root=root, decl=decl, off=("computed" if computed and not off else
                                                        f"0x{off:X}" if off >= 0 else f"-0x{-off:X}"),
                             struct=s or "", how=how, match=match, member=path, fake=fake,
                             cast_note="y" if f["cast_note"] else ""))

cols = ["file", "line", "func", "kind", "acc", "size", "base", "root", "decl", "off", "struct", "how",
        "match", "member", "fake", "cast_note"]
with open(f"{OUT}/casts.tsv", "w", encoding="utf-8", newline="\n") as fh:
    fh.write("# " + "\t".join(cols) + "   (memory/grind/phase2-2026-10-03/tools/casts.py)\n")
    for r in rows:
        fh.write("\t".join(str(r[c]).replace("\t", " ") for c in cols) + "\n")
field = [r for r in rows if r["off"] != "computed"]
bys = collections.defaultdict(collections.Counter)
for r in rows:
    how = r["how"].split("(")[0]
    key = r["struct"] or ("(" + how + ")")
    bys[key]["sites"] += 1
    bys[key][r["match"] or "unresolved"] += 1
    bys[key]["how:" + how] += 1
with open(f"{OUT}/casts_by_struct.tsv", "w", encoding="utf-8", newline="\n") as fh:
    keys = ["exact", "sign", "size", "inside", "pad", "beyond", "hole", "addr-of", "computed", "same-struct",
            "other-struct", "unresolved"]
    fh.write("# struct\tsites\t" + "\t".join(keys) + "\thow\n")
    for s, c in sorted(bys.items(), key=lambda x: -x[1]["sites"]):
        oth = sum(v for k2, v in c.items() if k2.startswith("other-struct"))
        fh.write(f"{s}\t{c['sites']}\t" + "\t".join(str(oth if k2 == 'other-struct' else c[k2]) for k2 in keys) +
                 "\t" + ",".join(f"{k2[4:]}={v}" for k2, v in sorted(c.items()) if k2.startswith("how:")) + "\n")
byf = collections.defaultdict(collections.Counter)
for r in rows:
    byf[r["file"]]["sites"] += 1
    byf[r["file"]][r["kind"].split("-")[0]] += 1
    byf[r["file"]]["resolved"] += bool(r["struct"])
    byf[r["file"]]["fake_site"] += r["fake"] == "site"
with open(f"{OUT}/casts_by_file.tsv", "w", encoding="utf-8", newline="\n") as fh:
    ks = ["deref", "pun", "index", "struct", "addr", "ptradd", "resolved", "fake_site"]
    fh.write("# file\tsites\t" + "\t".join(ks) + "\n")
    for fl, c in sorted(byf.items(), key=lambda x: -x[1]["sites"]):
        fh.write(f"{fl}\t{c['sites']}\t" + "\t".join(str(c[k2]) for k2 in ks) + "\n")
kc = collections.Counter(r["kind"] for r in rows)
print(len(rows), "sites;", dict(kc))
print("resolved:", sum(1 for r in rows if r["struct"]), collections.Counter(r["how"].split("(")[0] for r in rows).most_common())
print("match:", collections.Counter(r["match"] for r in rows if r["struct"]).most_common())
