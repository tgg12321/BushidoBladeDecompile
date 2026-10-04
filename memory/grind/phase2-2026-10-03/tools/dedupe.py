"""dedupe.py [--apply] [--include LINE[@AFTER]] HEADER[,HEADER...] FILE...

Removes from each FILE the file-scope items the HEADER(s) already provide, verbatim in meaning:
  - typedef / struct / union / enum definitions: identical text (comments and whitespace ignored);
  - `extern` object declarations: identical text (comments and whitespace ignored);
  - function prototypes (with or without `extern`): identical type (parameter names ignored);
  - `#define` lines: identical text.
A comment block directly above a removed item (no blank line between) is removed too when the
same comment text (whitespace ignored) is in a HEADER. A "Declarations from the file this module was
split from" / "Declarations from the old ..." banner left with nothing under it is removed. Runs of
blank lines left behind collapse to one. --include adds an #include line after the last top-of-file
#include (when the file does not already have it). Block-scope declarations are never touched.
Without --apply prints a unified diff."""
import difflib, os, re, sys
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from citems import items, strip_cs

KW = set("struct union enum const volatile unsigned signed int char short long void".split())


def ws(s):
    return " ".join(s.split())


def proto_key(src):
    """type key of a prototype `[extern] RET NAME(PARAMS);` or None."""
    s = ws(strip_cs(src)).rstrip(";").strip()
    s = re.sub(r"^extern\s+", "", s)
    if s.startswith(("typedef", "static")):
        return None
    m = re.match(r"^(.*?\b)([A-Za-z_]\w*)\s*\((.*)\)$", s)
    if not m or "(*" in m.group(1) or m.group(1).strip() == "":
        return None
    ret, name, params = m.group(1), m.group(2), m.group(3)
    if "=" in ret:
        return None
    out, depth, cur = [], 0, ""
    for ch in params:
        if ch == "," and depth == 0:
            out.append(cur); cur = ""
            continue
        depth += ch == "("
        depth -= ch == ")"
        cur += ch
    out.append(cur)
    ps = []
    for p in out:
        p = p.strip()
        if "(" not in p:
            ids = re.findall(r"[A-Za-z_]\w*", p)
            nonkw = [i for i in ids if i not in KW]
            if len(nonkw) >= 2 and ids[-1] not in KW and re.search(r"\b" + ids[-1] + r"\s*(\[[^\]]*\])?$", p):
                p = re.sub(r"\b" + ids[-1] + r"\s*(?=(\[[^\]]*\])?$)", "", p).strip()
        ps.append(ws(p.replace("*", " * ")).replace(" *", "*").replace("* ", "*"))
    return ("F", name, ws(ret.replace("*", " * ")).replace(" *", "*").replace("* ", "*"), tuple(ps))


def item_key(lines, it):
    src = "\n".join(lines[it["start"] - 1:it["end"]])
    c = ws(strip_cs(src))
    if it["kind"] == "pp":
        return ("P", c) if c.startswith("#define") or c.startswith("# define") else None
    if it["kind"] != "decl":
        return None
    if c.startswith("typedef") or re.match(r"(struct|union|enum)\s+\w+\s*\{", c):
        return ("T", c)
    pk = proto_key(src)
    if pk:
        return pk
    if c.startswith("extern"):
        return ("O", c)
    return None


def comment_blocks(text):
    return {ws(m.group(0)) for m in re.finditer(r"/\*.*?\*/", text, re.S)}


def header_keys(paths):
    keys, comments = set(), set()
    for p in paths:
        t = open(p, encoding="utf-8").read()
        its, lines = items(t)
        for it in its:
            k = item_key(lines, it)
            if k:
                keys.add(k)
        comments |= comment_blocks(t)
    return keys, comments


BANNER = re.compile(r"^/\* Declarations from (the file this module was split from|the old) .*\*/$")


def process(path, keys, comments, include):
    text = open(path, encoding="utf-8").read()
    its, lines = items(text)
    drop = set()
    for it in its:
        if it["kind"] not in ("decl", "pp"):
            continue
        k = item_key(lines, it)
        if k is None or k not in keys:
            continue
        drop.update(range(it["start"], it["end"] + 1))
        # attached comment lines directly above (within the item's lead span)
        j = it["start"] - 1
        while j >= it["lead"] and lines[j - 1].strip() != "":
            j -= 1
        # lines j+1 .. start-1 are a contiguous non-blank run (comments, by construction of lead);
        # its last comment block (ending right above the item, starting on its own line) goes when
        # a header carries the same comment
        run = "\n".join(lines[j:it["start"] - 1])
        blocks = list(re.finditer(r"/\*.*?\*/", run, re.S))
        if blocks and run[blocks[-1].end():].strip() == "" and ws(blocks[-1].group(0)) in comments:
            before = run[:blocks[-1].start()]
            if before.split("\n")[-1].strip() == "":
                drop.update(range(j + 1 + before.count("\n"), it["start"]))
    # a blank line that ends up next to another blank line only because of a removal goes too
    new = []
    prev_dropped = False
    for i, l in enumerate(lines, 1):
        if i in drop:
            prev_dropped = True
            continue
        if l.strip() == "" and new and new[-1].strip() == "" and prev_dropped:
            continue
        new.append(l)
        prev_dropped = prev_dropped and l.strip() == ""
    # banner with nothing (but blank lines) under it before the next non-declaration item
    out = []
    for i, l in enumerate(new):
        if BANNER.match(l.strip()):
            k = i + 1
            incom = False
            while k < len(new):
                s = new[k].strip()
                if incom:
                    incom = "*/" not in s
                elif s.startswith("/*"):
                    incom = "*/" not in s
                elif s != "":
                    break
                k += 1
            nxt = new[k] if k < len(new) else ""
            if not re.match(r"\s*(extern\b|typedef\b|[A-Za-z_][\w\s\*]*\(.*\)\s*;)", nxt):
                if out and out[-1].strip() == "" and k < len(new) and new[i + 1].strip() == "":
                    out.pop()
                continue
        out.append(l)
    res = out
    if include and not any(l.strip() == include for l in res):
        last = max((i for i, l in enumerate(res[:60]) if l.startswith("#include")), default=None)
        res.insert(last + 1 if last is not None else 0, include)
    return text, "\n".join(res)


def main():
    args = sys.argv[1:]
    apply = "--apply" in args
    args = [a for a in args if a != "--apply"]
    includes = []
    while args and args[0] == "--include":
        includes.append(args[1]); args = args[2:]
    hdrs = args[0].split(",")
    keys, comments = header_keys(hdrs)
    for f in args[1:]:
        old, new = None, None
        text = open(f, encoding="utf-8").read()
        cur = text
        for inc in includes or [None]:
            open(f + ".tmp_dedupe", "w", encoding="utf-8", newline="\n").write(cur)
            _, cur = process(f + ".tmp_dedupe", keys, comments, inc)
        import os
        os.remove(f + ".tmp_dedupe")
        if cur != text:
            if apply:
                open(f, "w", encoding="utf-8", newline="\n").write(cur)
            else:
                sys.stdout.writelines(difflib.unified_diff(text.splitlines(True), cur.splitlines(True), f, f, n=1))


if __name__ == "__main__":
    main()
