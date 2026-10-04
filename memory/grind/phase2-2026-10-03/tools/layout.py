"""layout.py: struct layouts as cc1 computes them. For a TU, every `typedef struct {...} NAME` and
`struct TAG {...}` visible at file scope in its preprocessed text gets offsetof/sizeof of each named
non-bitfield member (and the element size of array members), read back from a compiled initializer.
layouts(tu, pre_text) -> {type_name: {"size": n, "members": [{name, off, size, esize, base, stars, arr}]}}
Results are cached per (struct text) in-process."""
import hashlib, re, subprocess
from citems import strip_cs
from ctok import Toks, typedef_names, read_type, read_declarators

CC1 = ["tools/gcc-2.7.2/build/cc1", "-O2", "-G0", "-funsigned-char", "-quiet", "-mcpu=3000", "-mips1",
       "-mno-abicalls", "-fno-builtin", "-w", "-mel", "-msoft-float"]


def parse_structs(clean):
    """[(c_name, [(member, base, stars, arr, bitfield)])] for file-scope struct definitions."""
    T = Toks(clean)
    tn = typedef_names(clean)
    out, k, depth = [], 0, 0
    while k < len(T.t):
        x = T.t[k]
        if x == "{":
            depth += 1
        elif x == "}":
            depth -= 1
        elif depth == 0 and x in ("struct", "union") and k + 1 < len(T.t):
            j = k + 1
            tag = None
            if re.match(r"[A-Za-z_]", T.t[j]):
                tag = T.t[j]
                j += 1
            if j < len(T.t) and T.t[j] == "{":
                e = T.match(j)
                typedef = k > 0 and T.t[k - 1] == "typedef"
                names = []
                if typedef:
                    q = e + 1
                    while q < len(T.t) and T.t[q] != ";":
                        if re.match(r"[A-Za-z_]", T.t[q]) and T.t[q - 1] in ("}", ","):
                            names.append(T.t[q])
                        q += 1
                cname = names[0] if names else (f"{x} {tag}" if tag else None)
                if cname:
                    out.append((cname, members(T, j + 1, e, tn), x == "union"))
                    if tag and names:
                        pass
                k = e + 1
                continue
        k += 1
    return out


def members(T, a, b, tn):
    res, k = [], a
    while k < b:
        r = read_type(T, k, tn)
        if not r:
            k += 1
            continue
        base, j = r
        # bitfields: `u32 addr : 24;`
        q = j
        while q < b and T.t[q] != ";":
            q += 1
        span = T.t[j:q]
        if ":" in span:
            for n in [s for s in span if re.match(r"[A-Za-z_]", s)][:1]:
                res.append((n, base, 0, False, True))
            k = q + 1
            continue
        decls, j2 = read_declarators(T, j)
        for name, stars, arr, init, proto in decls:
            if name:
                res.append((name, base, stars, arr, False))
        k = max(j2, j) + 1
    return res


_cache = {}


def layouts(tu, pre):
    clean = strip_cs(pre)
    structs = parse_structs(clean)
    todo = []
    for cname, mem, is_union in structs:
        key = hashlib.sha1((cname + repr(mem)).encode()).hexdigest()
        if key not in _cache:
            todo.append((key, cname, mem, is_union))
    if todo:
        exprs, idx = [], []
        for key, cname, mem, is_union in todo:
            exprs.append(f"sizeof({cname})")
            for (n, base, stars, arr, bit) in mem:
                if bit:
                    continue
                exprs.append(f"(unsigned)&((({cname} *)0)->{n})")
                exprs.append(f"sizeof((({cname} *)0)->{n})")
                exprs.append(f"sizeof((({cname} *)0)->{n}[0])" if arr else "0")
            idx.append((key, cname, mem, is_union))
        src = pre + "\nunsigned p2_layout[] = {\n" + ",\n".join(exprs) + "\n};\n"
        r = subprocess.run(CC1, input=src.encode(), capture_output=True)
        asm = r.stdout.decode()
        a = asm.find("p2_layout:")
        words = [int(w, 0) for w in re.findall(r"\.word\s+(\S+)", asm[a:])] if a >= 0 else []
        if len(words) != len(exprs):
            # fall back one struct at a time (an unparsable struct must not sink the rest)
            for key, cname, mem, is_union in idx:
                _cache[key] = one(pre, cname, mem, is_union)
        else:
            w = iter(words)
            for key, cname, mem, is_union in idx:
                size = next(w)
                ms = []
                for (n, base, stars, arr, bit) in mem:
                    if bit:
                        continue
                    off, sz, es = next(w), next(w), next(w)
                    ms.append(dict(name=n, off=off, size=sz, esize=es if arr else sz, base=base, stars=stars, arr=arr))
                _cache[key] = dict(size=size, members=ms, union=is_union)
    out = {}
    for cname, mem, is_union in structs:
        key = hashlib.sha1((cname + repr(mem)).encode()).hexdigest()
        if _cache.get(key):
            out[cname] = _cache[key]
    return out


def one(pre, cname, mem, is_union):
    exprs = [f"sizeof({cname})"]
    good = [m for m in mem if not m[4]]
    for (n, base, stars, arr, bit) in good:
        exprs += [f"(unsigned)&((({cname} *)0)->{n})", f"sizeof((({cname} *)0)->{n})",
                  f"sizeof((({cname} *)0)->{n}[0])" if arr else "0"]
    src = pre + "\nunsigned p2_layout[] = {\n" + ",\n".join(exprs) + "\n};\n"
    asm = subprocess.run(CC1, input=src.encode(), capture_output=True).stdout.decode()
    a = asm.find("p2_layout:")
    words = [int(w, 0) for w in re.findall(r"\.word\s+(\S+)", asm[a:])] if a >= 0 else []
    if len(words) != len(exprs):
        return None
    w = iter(words)
    size = next(w)
    ms = []
    for (n, base, stars, arr, bit) in good:
        off, sz, es = next(w), next(w), next(w)
        ms.append(dict(name=n, off=off, size=sz, esize=es if arr else sz, base=base, stars=stars, arr=arr))
    return dict(size=size, members=ms, union=is_union)
