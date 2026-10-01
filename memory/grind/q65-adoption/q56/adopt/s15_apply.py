#!/usr/bin/env python3
"""Step 15 (Q65 adoption): the switch to the per-file gp model. Byte-identical by construction + check.

Evidence and rule: tmp/q56/MODEL.md, PLAN.md. Sony ASPSX 2.34 gives a file gp for a small symbol only if
the file DEFINES it; Sony PSYLINK lays out each file's initialized small data (.sdata) and statics (.lcomm
-> .sbss) in link order, then the COMMON (tentative) block. BB2's small-data area has exactly that shape:
  0x800A30CC..0x800A3308  per-file .sdata blocks (initialized definitions),
  0x800A3308..0x800A3618  per-file .sbss blocks (statics),
  0x800A3618..0x800A3800  the COMMON block (tentative definitions).
This step:
  1. defines, in each C file, the small data its code reaches gp-relative (the oracle build's GPREL16
     relocations): an initialized definition (value from the original EXE) in the .sdata region, a
     `static` in the .sbss region, a tentative definition in the COMMON region. A file's .sdata / .sbss
     block is contiguous, so every object inside the span its gp objects cover is defined there too
     (objects its code reaches by lui/%lo or la, and unreferenced "filler" objects, sized from the gap);
  2. moves those bytes out of asm/data/91C98.data.s: the blob is cut into pieces around the C blocks,
     and bb2.ld places each file's .sdata / .sbss at its address;
  3. passes -G8 to maspsx for every file except Sony PsyQ library code (owner ruling Q69: compiled -G0; the
     set comes from the provenance census over the link map, tools/psyq_library_files.py), and retires
     sdata_syms.txt / sdata_funcs.txt / sdata_exclude.txt.
Files whose .sbss block contains an object another file ALSO reaches gp-relative (a static shared across
our files: the held merges M3/M4) keep TENTATIVE definitions there (interim; all zero-byte bss).
Needs rule amendments A1 (K2 = every object of a file's per-file static block) and A2 (objects between
a file's gp objects are its definitions: la/indexed-only objects and sized gap fillers) - PLAN.md.
usage: s15_apply.py <tree>"""
import json, os, re, subprocess, sys
R = sys.argv[1]
os.chdir(R)
H = os.path.dirname(os.path.abspath(__file__))
NL = "\n"
SD0, SD1, ST1, HI = 0x800A30CC, 0x800A3308, 0x800A3618, 0x800A3800  # HI: end of the image (and of the data blob)
BLOB0 = 0x800A1498
EXE = open("disc/SLUS_006.63", "rb").read()
LOG = []


def log(s):
    LOG.append(s); print(s)


def b(a, n=1):
    o = a - 0x80010000 + 0x800
    return EXE[o:o + n]


# the plan reads an oracle build of this tree 
subprocess.run("rm -rf build && make -j16 build/bb2.exe > /tmp/q56/s04_prebuild.log 2>&1", shell=True, check=True)
assert subprocess.run("sha1sum build/bb2.exe", shell=True, capture_output=True, text=True).stdout.split()[0] == \
    "62efab4f73f992798c43e8c730aa43baa10bb4fa", "pre-switch tree is not at the oracle"
subprocess.run("rm -rf /tmp/q56/pre04obj && cp -r build/src /tmp/q56/pre04obj", shell=True, check=True)
subprocess.run([sys.executable, f"{H}/defs_plan.py", R], check=True, capture_output=True)
P = json.load(open("/tmp/q56/defs_plan.json"))

# ---- 0. who names each small-data symbol, and how (for the orphan test, E2 and truthful filler comments) ----
# REFS[name] = set of (file, function, kind): kind gp (GPREL16), la (lui + addiu), indexed (lui, then the base
# register is added to before the %lo access), direct (lui + %lo load/store on the same register: a direct
# access the rule would make gp for a <= 8-byte definition in that file), word (a data word in a C object).
REFS = {}
_INSN = re.compile(r"^\s*([0-9a-f]+):\s+[0-9a-f]{8}\s+(\S+)\s*(.*)$")
_RELOC = re.compile(r"^\s*[0-9a-f]+:\s+R_MIPS_(\w+)\s+(\S+)$")
for _o in sorted(os.listdir("/tmp/q56/pre04obj")):
    if not _o.endswith(".o"):
        continue
    _f = _o[:-2]
    _out = subprocess.run(f"mipsel-linux-gnu-objdump -dr --no-show-raw-insn -M no-aliases /tmp/q56/pre04obj/{_o}",
                          shell=True, capture_output=True, text=True).stdout.splitlines()
    _func, _prev, _hi = None, None, {}
    for _l in _out:
        _m = re.match(r"^[0-9a-f]+ <([^>]+)>:$", _l)
        if _m:
            _func, _prev, _hi = _m.group(1), None, {}
            continue
        _m = re.match(r"^\s*([0-9a-f]+):\s+(\S+)\s*(.*)$", _l)
        if _m and not _l.strip().split(":", 1)[1].strip().startswith("R_MIPS_"):
            _prev = (_m.group(2), _m.group(3))
            op, ops = _prev
            regs = [x.strip() for x in ops.split(",")]
            # an addu into a register that holds a %hi(S) makes later %lo(S)(reg) accesses indexed
            if op in ("addu", "add") and regs and regs[0] in _hi:
                _hi[regs[0]] = (_hi[regs[0]][0], True)
            continue
        _m = _RELOC.match(_l)
        if not _m or _prev is None:
            continue
        kind, sym = _m.group(1), re.sub(r"\+0x[0-9a-f]+$", "", _m.group(2))
        op, ops = _prev
        regs = [x.strip() for x in ops.split(",")]
        if kind == "GPREL16":
            k = "gp"
        elif kind == "HI16":
            _hi[regs[0]] = (sym, False)
            k = None
        elif kind == "LO16":
            if op == "addiu":
                k = "la"
            else:
                bm = re.search(r"\((\w+)\)", ops)
                hv = _hi.get(bm.group(1)) if bm else None
                k = "indexed" if (hv and hv[1]) else "direct"
        elif kind == "32":
            k = "word"
        else:
            k = None
        if k:
            REFS.setdefault(sym, set()).add((_f, _func or "?", k))
# image addresses -> the C-defined object there (pre-switch build): a small-data word whose original value is
# such an address is spelled as that object's address (layer-2 round 2, step-15 finding 5)
SYM_AT = {}
for _l in subprocess.run("mipsel-linux-gnu-nm build/bb2.elf", shell=True, capture_output=True, text=True).stdout.splitlines():
    _p = _l.split()
    if len(_p) == 3:   # the image is one .main section, so data shows as T; c_definition_of keeps C data only
        SYM_AT.setdefault(int(_p[0], 16), []).append(_p[2])
NEED_EXTERN = {}   # file -> extern lines its initializers need
CURRENT_FILE = [None]


def vtext(v, es, signed, t=None):
    """an initializer value: an image address that is a C-defined object's address is spelled as that address"""
    if es == 4 and 0x80010000 <= v < 0x800A3800 and v in SYM_AT:
        for nm in SYM_AT[v]:
            cd = c_definition_of(nm)
            if cd:
                if cd[0] != f"src/{CURRENT_FILE[0]}.c":
                    NEED_EXTERN.setdefault(CURRENT_FILE[0], []).append(cd[1])
                expr = nm if cd[1].endswith("[];") else f"&{nm}"
                log(f"ADDRESS {CURRENT_FILE[0]} value {v:#010x} spelled ({t or 's32'}){expr} ({cd[0]})")
                return f"({t or 's32'}){expr}" if not (t and "*" in t) else (expr if cd[1].endswith("[];") else f"({t}){expr}")
    if t and "*" in t:
        return f"({t})0x{v:08X}" if v else "0"
    return fmt(v, es, signed)


# data words in the asm data files that name a symbol (a pointer in data whose owning C file is not known)
DPTR = {}
for _fn in sorted(os.listdir("asm/data")):
    for _m in re.finditer(r"/\*\s*[0-9A-F]+\s+([0-9A-F]{8})\s+[0-9A-F]+\s*\*/\s*\.word\s+(\w+)", open(f"asm/data/{_fn}").read()):
        DPTR.setdefault(_m.group(2), []).append((_fn, int(_m.group(1), 16)))

BASE = {"s8": (1, True), "u8": (1, False), "char": (1, False), "s16": (2, True), "u16": (2, False),
        "short": (2, True), "s32": (4, True), "u32": (4, False), "int": (4, True), "long": (4, True)}


def parse_decl(d, name):
    """'extern T name[dims];' -> (T, dims-list or [], text-type). None if not a scalar/array of scalar."""
    m = re.match(r"extern\s+(.*?)\b%s\b((?:\s*\[[^\]]*\])*)\s*;$" % re.escape(name), d)
    if not m:
        return None
    t = " ".join(m.group(1).split())
    dims = re.findall(r"\[([^\]]*)\]", m.group(2))
    return t, dims


SRC_TEXT = {p: open(p).read() for p in sorted([f"src/{g}" for g in os.listdir("src") if g.endswith(".c")] +
                                             [f"include/{h}" for h in os.listdir("include") if h.endswith(".h")])}


def decl_anywhere(nm):
    """the first `extern T nm...;` any source file or header declares (types follow existing declarations), as
    (path, decl text) - for an object a file reaches gp-relative without declaring it itself"""
    for p, t in SRC_TEXT.items():
        m = re.search(r"^\s*(extern\s+[^;{}()]*?\b%s\b(?:\s*\[[^\]]*\])*\s*;)" % re.escape(nm), t, re.M)
        if m:
            return p, " ".join(m.group(1).split())
    return None


def c_definition_of(nm):
    """the C definition (`[const] T nm[...]` at file scope, not extern) of an image object, as an extern line"""
    for p, t in SRC_TEXT.items():
        if not p.startswith("src/"):
            continue
        m = re.search(r"^((?:const\s+)?[A-Za-z_]\w*(?:\s+\w+)*?)\s+\**\s*%s\s*(\[[^\]]*\])?\s*=" % re.escape(nm), t, re.M)
        if m and not m.group(1).startswith(("extern", "static", "return")):
            return p, f"extern {' '.join(m.group(1).split())} {nm}{'[]' if m.group(2) else ''};"
    return None


CPP = ("mipsel-linux-gnu-cpp -Iinclude -undef -Wall -lang-c -fno-builtin -Dmips -D__GNUC__=2 -D__OPTIMIZE__ "
       "-D__mips__ -D__mips -Dpsx -D__psx__ -D__psx -D_PSYQ -D__EXTENSIONS__ -D_MIPSEL -D_LANGUAGE_C -DLANGUAGE_C")
CC1 = "tools/gcc-2.7.2/build/cc1 -O2 -G0 -funsigned-char -quiet -mcpu=3000 -mips1 -mno-abicalls -fno-builtin -w -mel -msoft-float"
_SZ = {}


def sizeof(f, nm, t, dims):
    """sizeof the declared type, measured by our cc1 on the file's own preprocessed text."""
    key = (f, t, tuple(dims))
    if key in _SZ:
        return _SZ[key]
    if dims and not dims[-1].strip():
        return None
    probe = subprocess.run(f"{CPP} src/{f}.c 2>/dev/null", shell=True, capture_output=True, text=True).stdout
    probe += f"\nextern {t} __q56_obj{''.join('[%s]' % d for d in dims)};\nint __q56_sz = sizeof(__q56_obj);\n"
    out = subprocess.run(CC1, shell=True, input=probe, capture_output=True, text=True).stdout
    m = re.search(r"__q56_sz:\s*\n\s*\.word\s+(\d+)", out)
    _SZ[key] = int(m.group(1)) if m else None
    return _SZ[key]


def elem(t):
    tt = t.replace("volatile ", "").replace("const ", "").replace("signed ", "").replace("unsigned ", "u").strip()
    if "*" in tt:
        return 4, False, True
    if tt in BASE:
        return BASE[tt][0], BASE[tt][1], False
    return None


def lcomm_align(n):
    """(A8, owner ruling Q79, rules: 8c57bc4ab): Sony ASPSX 2.34 + PSYLINK place every `.lcomm` static
    4-aligned whatever its size (lcomm_align_probe); maspsx models it (step 12)"""
    return 4


def obj_align(reg, size, dims, e):
    """the alignment the build gives an object: maspsx's .lcomm model aligns a static by its size; an
    initialized object takes cc1's alignment (an array is word-aligned by DATA_ALIGNMENT, a scalar its size)."""
    if reg == "static":
        return lcomm_align(size)
    return 4 if dims else (e[0] if e else 4)


# the data blob's own object boundaries (splat dlabels; each marks an object some code or data names)
BLOB_LABELS = {}
_lab = None
for _l in open("asm/data/91C98.data.s"):
    _m = re.match(r"^dlabel (\w+)", _l)
    if _m:
        _lab = _m.group(1); continue
    _m = re.search(r"/\*\s*[0-9A-F]+\s+([0-9A-F]{8})", _l)
    if _m and _lab:
        BLOB_LABELS[int(_m.group(1), 16)] = _lab
        _lab = None
_LABEL_ADDRS = sorted(BLOB_LABELS)


def label_end(a):
    """the end of the original object at a: the next blob label after a (or the image end)"""
    import bisect
    k = bisect.bisect_right(_LABEL_ADDRS, a)
    return _LABEL_ADDRS[k] if k < len(_LABEL_ADDRS) else HI


# ---- 1. objects per file and region -----------------------------------------------------------------
gpown = {}   # addr -> set(files reaching it gp)
for f, uses in P.items():
    for nm, u in uses.items():
        if u["gp"]:
            gpown.setdefault(u["addr"], set()).add(f)


def region(a):
    return "sdata" if a < SD1 else ("static" if a < ST1 else "common")


defs = {}      # file -> list of dict(kind, addr, size, text)
spans = []     # (start, end, file, section)
tentative = {} # file -> [text]
held = set()
for f, uses in sorted(P.items()):
    for reg in ("sdata", "static"):
        own = sorted((u["addr"], nm) for nm, u in uses.items() if u["gp"] and u["region"] == reg)
        if not own:
            continue
        # objects of this file in this region (by address; one name per address, the first declared one)
        objs = {}
        for nm, u in uses.items():
            if u["region"] != reg or not u["decls"]:
                continue
            pd = parse_decl(u["decls"][0], nm)
            if pd is None:
                log(f"UNPARSED decl {f} {nm}: {u['decls'][0]}")
                continue
            objs.setdefault(u["addr"], []).append((nm, pd, u["gp"]))
        # a gp-reached object this file does not declare takes the type another file declares for it (types
        # follow existing declarations; layer-2 round 2, step-15 finding 3)
        for nm, u in uses.items():
            if u["region"] != reg or not u["gp"] or u["decls"] or u["addr"] in objs:
                continue
            da = decl_anywhere(nm)
            if da:
                pd = parse_decl(da[1], nm)
                if pd:
                    objs.setdefault(u["addr"], []).append((nm, pd, True))
                    log(f"TYPED-ELSEWHERE {f} {nm}: {da[1]} ({da[0]}; {f} reaches it gp-relative without a declaration)")
        # an object another file ALSO reaches gp-relative cannot be this file's static: when it sits at an
        # end of the block it stays a tentative definition (interim, held merge) and the rest is the block
        shared = sorted(a for a, fs in gpown.items() if region(a) == reg and f in fs and len(fs) > 1)
        if reg == "static" and shared:
            for a, nm in own:
                if a in shared:
                    tentative.setdefault(f, []).append((a, nm))
            own = [(a, nm) for a, nm in own if a not in shared]
            if not own:
                continue
        lo = own[0][0]
        others = sorted(a for a, fs in gpown.items() if region(a) == reg and f not in fs and a > lo)
        stop = others[0] if others else (SD1 if reg == "sdata" else ST1)
        hi_obj = max(a for a, _ in own)
        if reg == "static" and (any(lo < a < hi_obj for a in shared) or any(lo < a < hi_obj for a in others)):
            held.add(f)
            for a, nm in own:
                tentative.setdefault(f, []).append((a, nm))
            log(f"HELD static block {f} (shared {', '.join(hex(x) for x in shared) or '-'}): tentative interim")
            continue
        if reg == "sdata" and (shared or any(lo < a < hi_obj for a in others)):
            log(f"BLOCKED sdata block {f}: shared {', '.join(hex(x) for x in shared)}")
            continue
        items = []
        a = lo
        # the block runs from the first to the last object the file reaches gp-relative; objects it only
        # reaches by lui/%lo or la past that point are left in the data blob (ownership not shown)
        addrs = sorted(x for x in objs if lo <= x <= hi_obj)
        # the last object this file names inside [lo, stop): the block runs to its end
        last_end = lo
        cur = lo
        for x in addrs:
            if x < cur:
                log(f"OVERLAP {f} {hex(x)} inside previous object (alias) - left to the symbol file")
                continue
            if x > cur:
                items.append(("fill", cur, x - cur, None))
            nm, (t, dims), isgp = objs[x][0]
            if len(objs[x]) > 1:
                log(f"ALIAS {f} {hex(x)}: {[o[0] for o in objs[x]]} - defines {nm}")
            e = elem(t)
            if e is None and reg == "sdata":
                log(f"NON-SCALAR initialized {f} {nm}: {t}{''.join('[%s]' % d for d in dims)}")
                raise SystemExit(1)
            if dims and not dims[-1].strip():
                es = e[0] if e else sizeof(f, nm, t, dims[:-1] + ["1"])
                nxt = next((y for y in addrs if y > x), None)
                if nxt is None:
                    # no later object of this file: the extent is the original's own object boundary
                    nxt = label_end(x)
                n = max(1, (nxt - x) // es)
                dims = dims[:-1] + [str(n)]
                log(f"SIZED-FROM-GAP {f} {nm}: incomplete array -> [{n}] (next object at {hex(nxt)})")
            size = sizeof(f, nm, t, dims)
            if size is None:
                log(f"NO-SIZE {f} {nm}: {t} {dims}")
                raise SystemExit(1)
            # evidence check: no gp access of this file reaches past the object (cross-symbol storage, refused)
            offs = uses[nm].get("gp_offsets", [])
            if offs and max(offs) >= size:
                log(f"OVERRUN {f} {nm}: gp offset {max(offs)} >= size {size} - size by evidence first")
                raise SystemExit(1)
            items.append(("obj", x, size, (nm, t, dims, e, isgp)))
            cur = x + size
            last_end = cur
        # the block runs to the end of the LAST object the file reaches gp-relative, declared in C or not
        # (an object reached gp only from INCLUDE_ASM code has no C declaration yet; its extent is the
        # original's object boundary, the blob label after it)
        if hi_obj >= cur:
            hend = min(label_end(hi_obj), stop)
            items.append(("fill", cur, hend - cur, None))
            log(f"BLOCK-END {f} {reg}: extends to {hex(hend)} (last gp-reached object {hex(hi_obj)} is declared in no C file; its extent is the original's label)")
            cur = last_end = hend
        end = last_end
        le = min(label_end(max(hi_obj, lo)), stop)
        if le - end >= 4:   # a shorter tail is alignment (END-PAD / A8 tail), not left-over data
            log(f"BLOCK-TAIL {f} {reg} {hex(end)} ({le - end} B): after the last object of the block (inside the "
                f"original's label run ending {hex(le)}), left in the data blob")
        # a gap the build's own alignment of the next object already produces is padding, not an object
        # (per-file-gp-model.md, gap clause); a block that ends on an odd byte is followed by the linker's
        # SUBALIGN(2) padding before the next input section (no filler byte)
        # (A8): in a static block, a static's tail to the next 4-byte boundary is padding; a gap run beyond it
        # (A2) is still an object
        if reg == "static":
            split_items = []
            for it in items:
                if it[0] == "fill" and it[1] % 4:
                    t4 = min(it[2], 4 - it[1] % 4)
                    log(f"PADDING {f} static {hex(it[1])} ({t4} B): the tail of the static before it to the next "
                        f"4-byte boundary (A8)")
                    if it[2] > t4:
                        split_items.append(("fill", it[1] + t4, it[2] - t4, None))
                    continue
                split_items.append(it)
            items = split_items
        kept = []
        for k, it in enumerate(items):
            if it[0] == "fill" and k + 1 < len(items) and items[k + 1][0] == "obj":
                nx = items[k + 1]
                al = obj_align(reg, nx[2], nx[3][2], nx[3][3])
                if ((it[1] + al - 1) // al) * al == it[1] + it[2]:
                    log(f"PADDING {f} {reg} {hex(it[1])} ({it[2]} B): the next object's {al}-alignment produces it")
                    continue
            kept.append(it)
        items = kept
        if end % 2:
            log(f"END-PAD {f} {reg} block ends at {hex(end)}: linker SUBALIGN(2) pads to {hex(end + 1)}")
        spans.append((lo, end, f, ".sdata" if reg == "sdata" else ".sbss"))
        defs.setdefault(f, []).append((reg, items))
    # COMMON region: tentative definitions for every name reached gp-relative
    for nm, u in uses.items():
        if u["gp"] and u["region"] == "common":
            tentative.setdefault(f, []).append((u["addr"], nm))


# ---- 1b. (A1) orphans: "an orphan object joins the adjacent block of the one file referencing it; only objects
# <= 8 bytes live in these blocks". An object in the static region outside every block, named by exactly one
# file's code, joins that file's block when it is adjacent to it (directly, or across that object's own
# alignment padding), is <= 8 bytes, and E2 holds (the file's accesses are la / indexed only: a direct lui/%lo
# load or store would be gp under a <= 8-byte static). Applied repeatedly from each block edge; anything else
# in the run stays in the data blob and is logged (ORPHAN-LEFT, borderline).
def orphan_ok(nm, F, size, a):
    refs = REFS.get(nm, set())
    files = {r[0] for r in refs}
    if DPTR.get(nm):
        return f"named by a data word in asm data {[(fn, hex(x)) for fn, x in DPTR[nm]]}"
    if files != {F}:
        return f"named by {sorted(files) or 'nothing'}" + ("" if files else " (unreferenced)")
    if size > 8:
        return f"{size} bytes > 8 (a static that large is .bss, not this block)"
    bad = sorted((fn, k) for _, fn, k in refs if k not in ("la", "indexed"))
    if bad:
        return f"E2: accesses {bad} (only la / indexed are predicted for a <= 8-byte static)"
    if a % lcomm_align(size):
        return f"misaligned for a {size}-byte static"
    return None


def block_entry(F):
    for k, (reg, its) in enumerate(defs.get(F, [])):
        if reg == "static":
            return k
    return None


def orphan_item(F, a, size):
    """the joined object: typed from F's own declaration when it has one, else a label-named filler"""
    nm = BLOB_LABELS[a]
    u = P.get(F, {}).get(nm)
    if u and u["decls"]:
        pd = parse_decl(u["decls"][0], nm)
        if pd and pd[1] and not pd[1][-1].strip():
            pd = (pd[0], pd[1][:-1] + [str(size // (elem(pd[0])[0] if elem(pd[0]) else 1))])
        if pd:
            t, dims = pd
            sz = sizeof(F, nm, t, dims)
            if sz == size:
                return ("obj", a, size, (nm, t, dims, elem(t), False))
    return ("fill", a, size, None)


ORPHANS = []
for _round in range(2):
    st = sorted(s for s in spans if s[3] == ".sbss")
    for i in range(len(st) - 1):
        (xlo, xend, X, _), (ylo, yend, Y, _) = st[i], st[i + 1]
        labs = [x for x in _LABEL_ADDRS if xend <= x < ylo]
        objs_ = [(x, min(label_end(x), ylo)) for x in labs]
        # from Y's start downwards
        newlo, add = ylo, []
        for x, e in reversed(objs_):
            if e != newlo:
                break
            why = orphan_ok(BLOB_LABELS[x], Y, e - x, x)
            if why:
                break
            add.insert(0, orphan_item(Y, x, e - x))
            newlo = x
        if add:
            k = block_entry(Y)
            defs[Y][k] = ("static", add + defs[Y][k][1])
            spans[spans.index(st[i + 1])] = (newlo, yend, Y, ".sbss")
            for it in add:
                log(f"ORPHAN-JOIN {Y} static {hex(it[1])} ({it[2]} B, {BLOB_LABELS[it[1]]}): named only by {Y}, la/indexed")
            st[i + 1] = (newlo, yend, Y, ".sbss")
        # from X's end upwards
        newend, add = xend, []
        for x, e in objs_:
            if x >= st[i + 1][0]:
                break
            al = lcomm_align(e - x)
            if not (newend <= x < ((newend + al - 1) // al) * al + 1 and x % al == 0):
                break
            why = orphan_ok(BLOB_LABELS[x], X, e - x, x)
            if why:
                break
            if x > newend:
                log(f"PADDING {X} static {hex(newend)} ({x - newend} B): the joined orphan's {al}-alignment produces it")
            add.append(orphan_item(X, x, e - x))
            newend = e
        if add:
            k = block_entry(X)
            defs[X][k] = ("static", defs[X][k][1] + add)
            spans[spans.index(st[i])] = (xlo, newend, X, ".sbss")
            for it in add:
                log(f"ORPHAN-JOIN {X} static {hex(it[1])} ({it[2]} B, {BLOB_LABELS[it[1]]}): named only by {X}, la/indexed")
            st[i] = (xlo, newend, X, ".sbss")
# what is left between blocks and named by code: logged, stays in the data blob
st = sorted(s for s in spans if s[3] == ".sbss")
for i in range(len(st) - 1):
    for x in [x for x in _LABEL_ADDRS if st[i][1] <= x < st[i + 1][0]]:
        nm = BLOB_LABELS[x]
        if REFS.get(nm) or DPTR.get(nm):
            sz = min(label_end(x), st[i + 1][0]) - x
            files = sorted({r[0] for r in REFS.get(nm, set())})
            why = orphan_ok(nm, files[0] if len(files) == 1 else "-", sz, x)
            nb = "adjacent to its block" if files and files[0] in (st[i][2], st[i + 1][2]) else "not adjacent to its file's block"
            ORPHANS.append((x, sz, nm, files, why or nb))
            log(f"ORPHAN-LEFT {hex(x)} ({sz} B, {nm}) named by {files}: {why or nb} - stays in the data blob "
                f"(A9, owner ruling Q81)")


def fmt(v, size, signed):
    if signed and v >= 1 << (8 * size - 1):
        return str(v - (1 << (8 * size)))
    return hex(v) if v > 9 else str(v)


# the data blob's own object boundaries (splat dlabels): a filler never spans one, and takes its name
BLOB_LABELS = {}
_lab = None
for _l in open("asm/data/91C98.data.s"):
    _m = re.match(r"^dlabel (\w+)", _l)
    if _m:
        _lab = _m.group(1); continue
    _m = re.search(r"/\*\s*[0-9A-F]+\s+([0-9A-F]{8})", _l)
    if _m and _lab:
        BLOB_LABELS[int(_m.group(1), 16)] = _lab
        _lab = None
_LABEL_ADDRS = sorted(BLOB_LABELS)


def one_object(a, n, static_kw, init, nm):
    """ONE exact-size object for the run [a, a+n), or None if no single object can sit at a."""
    if n == 1:
        t, k, es = "u8", 1, 1
    elif n == 2 and a % 2 == 0:
        t, k, es = "s16", 1, 2
    elif n == 4 and a % 4 == 0:
        t, k, es = "s32", 1, 4
    elif n == 8 and a % 4 == 0:
        t, k, es = "s32", 2, 4
    else:
        t, k, es = "u8", n, 1
    al = (lcomm_align(n) if not init else (4 if k > 1 else es))
    if a % al or n > 8:
        return None
    if init:
        vals = [int.from_bytes(b(a + i * es, es), "little") for i in range(k)]
        if k == 1:
            return f"{static_kw}{t} {nm} = {vtext(vals[0], es, t[0] == 's', t)};"
        return f"{static_kw}{t} {nm}[{k}] = {{ {', '.join(vtext(v, es, t[0] == 's', t) for v in vals)} }};"
    return f"{static_kw}{t} {nm}" + (f"[{k}]" if k > 1 else "") + ";"


def filler_decls(start, size, static_kw, init):
    """per-file-gp-model.md gap clause: the run [start, start+size) is cut at the data blob's own labels
    (a label is an object some code or data references, named by it); each piece is ONE object of its exact
    size, the unreferenced ones named D_<start>. A piece no single object can occupy (alignment) or longer
    than 8 bytes is split under (A6) (owner ruling Q71), below."""
    import bisect
    out, a, end = [], start, start + size
    while a < end:
        if static_kw and a % 4:
            # (A8): a static run never starts inside a 4-byte slot - those bytes are the tail of the static
            # that starts the slot (padding), even where the blob carries a label there
            t4 = min(end - a, 4 - a % 4)
            log(f"PADDING static {hex(a)} ({t4} B): the tail of the static before it to the next 4-byte "
                f"boundary (A8){' - blob label ' + BLOB_LABELS[a] if a in BLOB_LABELS else ''}")
            a += t4
            continue
        k_ = bisect.bisect_right(_LABEL_ADDRS, a)
        nxt = min(_LABEL_ADDRS[k_] if k_ < len(_LABEL_ADDRS) else end, end)
        nm = BLOB_LABELS.get(a, "D_%08X" % a)
        d = one_object(a, nxt - a, static_kw, init, nm)
        if d:
            out.append(d)
        else:
            log(f"A6 {static_kw or 'sdata '}run {hex(a)} ({nxt - a} B, {nm}): no single object fits; split")
            out += old_filler_decls(a, nxt - a, static_kw, init)
        a = nxt
    return out


def old_filler_decls(start, size, static_kw, init):
    """per-file-gp-model.md (A6), owner ruling Q71: a run no single object can occupy splits deterministically
    into the FEWEST aligned pieces, each <= 8 bytes and placed by the build exactly at its start; among
    splits with that many pieces, the one whose pieces are largest earliest. Named D_<addr>."""
    end = start + size
    best = {end: (0, [])}
    for a in range(end - 1, start - 1, -1):
        cands = []
        for n in range(min(8, end - a), 0, -1):
            if (a + n) in best and one_object(a, n, static_kw, init, "D_%08X" % a):
                c, rest = best[a + n]
                cands.append((c + 1, [-n] + [-x for x in rest], n, rest))
        if cands:
            c, _, n, rest = min(cands)
            best[a] = (c, [n] + rest)
    out, a = [], start
    for n in best[start][1]:
        d = one_object(a, n, static_kw, init, "D_%08X" % a)
        out.append(d)
        log(f"A6 piece {hex(a)} {n} B: {d}")
        a += n
    return out


DATA_MODEL_NOTES = {
    # layer-2 round 2, step-15 finding 4: bytes that belong to a larger object than its C declaration says;
    # defined here as the declaration has it, the evidence logged for a later aggregate typing
    "D_800A3224": "the w/h halves of the 8-byte RECT at D_800A3220 (code6cac_c2 passes &D_800A3220 to LoadImage, "
                  "whose callee reads x/y/w/h); include/code6cac.h declares D_800A3220 u32",
    "D_800A3290": "the second word of the 8-byte record at D_800A328C (text1b stores &D_800A328C as a descriptor's "
                  "p_static for func_8007352C; its neighbours D_800A327C/3284/3294 are 8-byte records); text1b "
                  "declares D_800A328C s32",
}


def fill_comment(nm, f):
    """what names a filler object, from the pre-switch objects' relocations and the asm data words"""
    if nm in DATA_MODEL_NOTES:
        log(f"DATA-MODEL {f} {nm}: {DATA_MODEL_NOTES[nm]} - defined as its own object, retype later")
        return f"/* {DATA_MODEL_NOTES[nm]}; not named by code - logged (s15 DATA-MODEL) */"
    refs, dp = REFS.get(nm, set()), DPTR.get(nm, [])
    parts = []
    gpf = sorted({fn for g, fn, k in refs if k == "gp"})
    if gpf:
        parts.append("reached gp-relative by " + ", ".join(gpf) + " (declared in no C file)")
    for kind, what in (("la", "address taken (lui/addiu)"), ("indexed", "indexed (lui + %lo)"),
                       ("direct", "direct lui/%lo access"), ("word", "a data word")):
        fs = sorted({(g, fn) for g, fn, k in refs if k == kind})
        if fs:
            parts.append(what + " in " + ", ".join(fn if g == f else f"{g}:{fn}" for g, fn in fs))
    if dp:
        parts.append("named by the pointer word at " + ", ".join(f"{x:#010X} ({fn})" for fn, x in dp) +
                     " (A9, owner ruling Q80, rules: 8c57bc4ab: this file's K3 global by layout - it lies between"
                     " this file's gp-reached objects; the pointer table's owning file is open)")
        log(f"A9 {f} {nm}: named by a pointer word in asm data {[(fn, hex(x)) for fn, x in dp]}: defined here "
            f"as a K3 global (owner ruling Q80); the table's owning file is open")
    if not parts:
        return "/* not named by any code or data: size from the gap */"
    return "/* " + "; ".join(parts) + ": size from the blob label */"


def fill_lines(a, size, f, reg):
    out = []
    for d in filler_decls(a, size, "static " if reg == "static" else "", reg == "sdata"):
        nm = re.search(r"\b(\w+)(?:\[\d+\])?\s*(?:=|;)", d).group(1)
        out.append(d + "  " + fill_comment(nm, f))
    return out


# ---- 2. write the definitions into the C files --------------------------------------------------------
hdr_removed = {}
FILL_NAMES = {}   # (file, region) -> names the gap fillers define


def drop_externs(nm, f, src):
    """K2: a static is defined only in F - F's file-scope `extern`, every other file's `extern` (file or block
    scope) and a header's go (a header line also decided cc1's static emission order)"""
    pat0 = r"^extern\s+[^;{}()]*\b%s\b(?:\s*\[[^\]]*\])*\s*;[^\n]*\n" % re.escape(nm)
    pat1 = r"^[ \t]*extern\s+[^;{}()]*\b%s\b(?:\s*\[[^\]]*\])*\s*;[^\n]*\n" % re.escape(nm)
    src, n = re.subn(pat1, "", src, flags=re.M)   # F's own file- and block-scope externs (the static precedes them)
    if n:
        log(f"EXTERN-REMOVED src/{f}.c: {n} extern line(s) of {nm} (its own static now)")
    for g in sorted(os.listdir("src")):
        if not g.endswith(".c") or g == f"{f}.c":
            continue
        gt = open(f"src/{g}").read()
        gt2, gn = re.subn(pat1, "", gt, flags=re.M)
        if gn:
            open(f"src/{g}", "w", newline=NL).write(gt2)
            log(f"EXTERN-REMOVED src/{g}: {gn} extern line(s) of {nm} (now a static of {f})")
    for hp in sorted(os.listdir("include")):
        if not hp.endswith(".h"):
            continue
        ht = open(f"include/{hp}").read()
        ht2, hn = re.subn(pat0, "", ht, flags=re.M)
        if hn:
            open(f"include/{hp}", "w", newline=NL).write(ht2)
            hdr_removed.setdefault(hp, []).append(nm)
            log(f"HEADER include/{hp}: extern {nm} removed (now a static of {f})")
    return src


for f in sorted(set(defs) | set(tentative)):
    path = f"src/{f}.c"
    src = open(path).read()
    CURRENT_FILE[0] = f
    top, bottom = [], []
    for reg, items in defs.get(f, []):
        if reg == "static":
            top.append("/* Q65: this file's statics (.sbss, allocated per file in link order by PSYLINK), in address order. */")
        else:
            bottom.append("/* Q65: this file's initialized small data (.sdata), in address order; values from the original EXE. */")
        for kind, a, size, o in items:
            if kind == "fill":
                fl = fill_lines(a, size, f, reg)
                (top if reg == "static" else bottom).extend(fl)
                for l in fl:
                    fnm = re.search(r"\b(\w+)(?:\[\d+\])?\s*(?:=|;)", l).group(1)
                    FILL_NAMES.setdefault((f, reg), []).append(fnm)
                    if reg == "static":
                        src = drop_externs(fnm, f, src)
                continue
            nm, t, dims, e, isgp = o
            dimtxt = "".join(f"[{d}]" for d in dims)
            if reg == "static":
                top.append(f"static {t} {nm}{dimtxt};")
                src = drop_externs(nm, f, src)
                continue
            es, signed, isptr = e
            count = 1
            for d in dims:
                count *= int(d, 0)
            vals = [int.from_bytes(b(a + i * es, es), "little") for i in range(count)]
            if isptr:
                vtxt = [vtext(v, es, False, t) for v in vals]
            else:
                vtxt = [vtext(v, es, signed, t) for v in vals]
            init = vtxt[0] if not dims else "{ " + ", ".join(vtxt) + " }"
            bottom.append(f"{t} {nm}{dimtxt} = {init};")
    tents = sorted(tentative.get(f, []))
    if tents:
        bottom.append("/* Q65: tentative definitions (COMMON) of the small data this file reaches gp-relative." +
                      (" Interim for the .sbss block held with merge M3/M4 (PLAN.md)." if f in held else "") + " */")
        for a, nm in tents:
            d = P[f][nm]["decls"][0]
            pd = parse_decl(d, nm)
            if pd is None:
                log(f"UNPARSED tentative {f} {nm}: {d}")
                raise SystemExit(1)
            t, dims = pd
            bottom.append(f"{t} {nm}{''.join('[%s]' % x for x in dims)};")
    if top:
        # right before the first file-scope statement that names any of these statics (types they use
        # are declared above it; a later block-scope `extern` for one of them refers to the static)
        sys.path.insert(0, H)
        from splitc_lib import statements
        lines = src.split(NL)
        names_ = [re.search(r"(\w+)(?:\[[^\]]*\])*\s*;", l).group(1) for l in top if l.startswith("static ")]
        pat = re.compile(r"\b(%s)\b" % "|".join(map(re.escape, names_)))
        first = None
        for st in statements(src, 0, len(lines)):
            if pat.search(st["text"]):
                first = st["a"]
                break
        assert first is not None, f
        # keep a comment block that heads that statement with it
        while first > 0 and lines[first - 1].strip().startswith(("/*", "*", "//")):
            first -= 1
        lines[first:first] = top + [""]
        src = NL.join(lines)
    if NEED_EXTERN.get(f):
        bottom = [l for l in dict.fromkeys(NEED_EXTERN[f])] + bottom
    if bottom:
        src = src.rstrip(NL) + NL + NL + NL.join(bottom) + NL
    open(path, "w", newline=NL).write(src)
    log(f"defined in {f}: {len(top)} static lines, {len(bottom)} sdata/tentative lines")

json.dump({"spans": [(hex(a), hex(e), f, s) for a, e, f, s in sorted(spans)], "held": sorted(held),
           "items": {f: [(reg, [(k, hex(a), sz, (o[0] if o else None)) for k, a, sz, o in its]) for reg, its in bl]
                     for f, bl in defs.items()},
           "tentative": {f: [(hex(a), nm) for a, nm in sorted(l)] for f, l in tentative.items()}},
          open("/tmp/q56/s04_spans.json", "w"), indent=1)

# K1 (Q62) condition: a tentative definition's object is zero in the original image (or wholly above the
# image end 0x800A3800, uninitialized by construction)
for f, lst in sorted(tentative.items()):
    for a, nm in lst:
        if a < HI and any(b(a, 4)):
            log(f"K1-NONZERO {f} {nm} at {a:#x}: original bytes {b(a, 4).hex()}")

# ---- 2b. symbol-file rows: a C-defined object takes its address from link order, not a row ---------------
# (K2: "no per-symbol address pin or linker-script symbol assignment"; K3 objects likewise). A static is
# referenced only from its own file (checked here against every other object's relocations); tentative (K1)
# objects keep their rows (their address comes from the symbol file, owner ruling Q62).
refs_by_obj = {}
for o in os.listdir("/tmp/q56/pre04obj"):
    if o.endswith(".o"):
        out = subprocess.run(f"mipsel-linux-gnu-objdump -r /tmp/q56/pre04obj/{o}", shell=True, capture_output=True, text=True).stdout
        refs_by_obj[o[:-2]] = set(re.sub(r"\+0x[0-9a-f]+$", "", l.split()[-1]) for l in out.splitlines()
                                  if re.match(r"^[0-9a-f]+\s+R_MIPS_", l))
asm_refs = {}
for fn in os.listdir("asm/funcs"):
    for m in re.finditer(r"%(?:hi|lo|gp_rel)\((\w+)", open(f"asm/funcs/{fn}", errors="replace").read()):
        asm_refs.setdefault(m.group(1), set()).add(fn[:-2])
defined_names = {}
for f, blocks in defs.items():
    for reg, items in blocks:
        for kind, a, size, o in items:
            if kind == "obj":
                defined_names[o[0]] = (f, reg)
for (f, reg), nms in FILL_NAMES.items():
    for nm in nms:
        defined_names.setdefault(nm, (f, reg))
k2_violations = []
for nm, (f, reg) in sorted(defined_names.items()):
    others = sorted(o for o, rs in refs_by_obj.items() if o != f and nm in rs)
    if reg == "static" and others:
        k2_violations.append((nm, f, others))
        log(f"K2-VIOLATION {nm} (static of {f}) is referenced from {others}")
rows_removed = []
for sf in ("undefined_syms_auto.txt", "named_syms.txt"):
    lines = open(sf).read().split(NL)
    keep = []
    for l in lines:
        m = re.match(r"^\s*(\w+)\s*=\s*0x[0-9A-Fa-f]+\s*;", l)
        if m and m.group(1) in defined_names and not any(v[0] == m.group(1) for v in k2_violations):
            rows_removed.append((sf, m.group(1)))
            log(f"ROW-REMOVED {sf} {m.group(1)}: defined in C by {defined_names[m.group(1)][0]}")
            continue
        keep.append(l)
    open(sf, "w", newline=NL).write(NL.join(keep))
log(f"symbol-file rows removed for C-defined objects: {len(rows_removed)}")

# ---- 3. cut the data blob around the C blocks ---------------------------------------------------------
blob = open("asm/data/91C98.data.s").read().split(NL)
head_end = next(i for i, l in enumerate(blob) if l.startswith(".section"))
HEAD = blob[:head_end + 1]
items = []   # (addr, kind, text): kind in label/data
lab = None
for l in blob[head_end + 1:]:
    m = re.match(r"^dlabel (\w+)", l)
    if m:
        lab = m.group(1); continue
    if l.startswith(("nonmatching", "enddlabel", ".align")) or not l.strip():
        continue
    m = re.match(r"^\s*/\*\s*[0-9A-F]+\s+([0-9A-F]{8})(?:\s+[0-9A-F]+)?\s*\*/\s*(\.\w+)\s*(.*)$", l)
    if not m:
        continue
    a, d, arg = int(m.group(1), 16), m.group(2), m.group(3)
    size = {".word": 4, ".short": 2, ".byte": 1}.get(d)
    if d == ".asciz":
        size = len(bytes(arg.strip()[1:-1], "latin1").decode("unicode_escape")) + 1
    items.append((a, size, l.strip(), lab))
    lab = None
cuts = sorted(spans)
pieces, prev = [], BLOB0
for a, e, f, sec in cuts:
    pieces.append((prev, a)); prev = e + (e % 2)
pieces.append((prev, HI))


def piece_text(lo, hi):
    # `.align 0`: no automatic alignment of .word/.short (a piece may start at a 2-mod-4 address; every
    # byte position is explicit: data lines at their original addresses, gaps as .space)
    out, cur, openlab = list(HEAD) + ["    .align 0"], lo, None
    for a, size, text, labname in items:
        if not (lo <= a < hi):
            continue
        if a > cur:
            out.append(f"    .space {a - cur}")
        if labname:
            if openlab:
                out.append(f"enddlabel {openlab}")
            out += ["", f"nonmatching {labname}", "", f"dlabel {labname}"]
            openlab = labname
        out.append("    " + text)
        cur = a + size
    if cur < hi:
        out.append(f"    .space {hi - cur}")
    if openlab:
        out.append(f"enddlabel {openlab}")
    return NL.join(out) + NL


names = []
for lo, hi in pieces:
    if hi <= lo:
        continue
    nm = "91C98" if lo == BLOB0 else "%X" % (lo - 0x80010000 + 0x800)
    open(f"asm/data/{nm}.data.s", "w", newline=NL).write(piece_text(lo, hi))
    if lo >= SD0:
        labs = [x for x in _LABEL_ADDRS if lo <= x < hi]
        desc = ", ".join(f"{BLOB_LABELS[x]}" + (f" (named by {sorted({r[0] for r in REFS.get(BLOB_LABELS[x], set())})})"
                         if REFS.get(BLOB_LABELS[x]) else "") for x in labs[:10]) + (f" and {len(labs) - 10} more labels" if len(labs) > 10 else "") or "no label"
        k1 = sorted({(a, n) for fl in tentative.values() for a, n in fl if lo <= a < hi})
        if k1:
            k1f = sorted({f for f, fl in tentative.items() for a, n in fl if lo <= a < hi})
            log(f"BLOB-PIECE {nm} [{hex(lo)}, {hex(hi)}): overlaps the COMMON block - {len(k1)} K1 tentative "
                f"definitions ({', '.join(n for _, n in k1[:6])}{', ...' if len(k1) > 6 else ''}) that {len(k1f)} files "
                f"reach gp-relative take their addresses here from symbol-file rows (owner ruling Q62); the piece "
                f"keeps the original bytes of the range (labels: {desc})")
        else:
            log(f"BLOB-PIECE {nm} [{hex(lo)}, {hex(hi)}): {desc} - outside every file's block (no file reaches it "
                f"gp-relative and no block's contiguity covers it), so it stays data")
    names.append((lo, nm))
ld = open("bb2.ld").read()
seq = []
pi = {lo: nm for lo, nm in names}
allpos = sorted([(lo, f"        build/asm/data/{nm}.data.o(.data);") for lo, nm in names] +
                [(a, f"        build/src/{f}.o({sec});") for a, e, f, sec in cuts])
old = "        build/asm/data/91C98.data.o(.data);"
assert ld.count(old) == 1
ld = ld.replace(old, NL.join(t for _, t in allpos))
open("bb2.ld", "w", newline=NL).write(ld)
log(f"blob cut into {len(names)} pieces around {len(cuts)} C blocks")

# ---- 4. the flag switch and the list retirement --------------------------------------------------------
# owner ruling Q69: maspsx -G8 for every file except Sony PsyQ library code (compiled -G0), the library set
# derived from the provenance census and this tree's link map (tools/psyq_library_files.py, added here)
import shutil
shutil.copy(f"{H}/psyq_library_files.py", "tools/psyq_library_files.py")
os.chmod("tools/psyq_library_files.py", 0o755)
r = subprocess.run([sys.executable, "tools/psyq_library_files.py"], capture_output=True, text=True)
assert r.returncode == 0, r.stderr
LIB = r.stdout.split()
log(f"PSYQ_LIBRARY_FILES (Q69): {' '.join(LIB)}")
assert not set(LIB) & (set(defs) | set(tentative)), "a library file defines small data"
# (A4): the landing commit records every src/*.c file's classification with its .text range
tab = subprocess.run([sys.executable, "tools/psyq_library_files.py", "--table"], capture_output=True, text=True).stdout
with open("docs/grind/gp-model-2026-09-30.md", "a", newline=NL) as fh:
    fh.write("\n## (A4) maspsx -G8 classification at the landing commit (owner ruling Q69)\n\n"
             "From `tools/psyq_library_files.py --table`: a file is Sony library code (-G0) when its link-map "
             "`.text` is non-empty and lies inside the census library span 0x80078948..0x8008D070; every other "
             "file gets -G8.\n\n| file | .text | maspsx |\n|---|---|---|\n" + tab)
mk = open("Makefile").read()
for var in ("MASPSX_FLAGS", "MASPSX_FLAGS_GP"):
    m = re.search(r"^%s :=(.*)$" % var, mk, re.M)
    v = m.group(1)
    for x in (" --sdata-syms=sdata_syms.txt", " --sdata-funcs=sdata_funcs.txt", " --sdata-exclude=sdata_exclude.txt"):
        assert x in v
        v = v.replace(x, "")
    mk = mk[:m.start(1)] + v + mk[m.end(1):]
mk = mk.replace("\tsdata_syms.txt sdata_funcs.txt sdata_exclude.txt expand_lb_funcs.txt", "\texpand_lb_funcs.txt")
anchor = "# -- Per-file lb/lh expansion opt-in --"
assert mk.count(anchor) == 1
mk = mk.replace(anchor, "# -- Small data: maspsx -G8 except Sony library code (owner rulings Q65, Q69) --\n"
                "# maspsx runs -G8 for every file: a file uses gp for a small symbol only if it DEFINES it (Sony\n"
                "# ASPSX 2.34), statics are its own .sbss, small initialized objects its own .sdata (cc1psx -G8).\n"
                "# Sony PsyQ library code was compiled -G0 and keeps it. The set is evidence, not a choice:\n"
                "# generated by tools/psyq_library_files.py from the provenance census (memory/closer/\n"
                "# psyq-library-census.md library span) and the link map; `--check` verifies it.\n"
                "PSYQ_LIBRARY_FILES := " + " ".join(LIB) + "\n\n" + anchor)
old = "maspsx_flags_for = $(if $(filter $1,$(GP_FILES)),$(MASPSX_FLAGS_GP),$(MASPSX_FLAGS))"
assert mk.count(old) == 1
mk = mk.replace(old, old + "$(if $(filter $1,$(PSYQ_LIBRARY_FILES)),, -G8)")
mk = mk.replace("# -G8: enable GP-relative for files that need it (uses sdata_syms.txt filtering)",
                "# -G8: cc1 small-data threshold for the files listed in GP_FILES")
mk = mk.replace("# These are compiled with -G8 and use sdata_syms.txt for selective GP-rel.",
                "# Our cc1 runs these at -G8 (gp itself is maspsx's decision on each file's own definitions).")
open("Makefile", "w", newline=NL).write(mk)
bc = open("engine/buildconfig.py").read()
for x in ('"--expand-div --aspsx-version=2.34 --sdata-syms=sdata_syms.txt "\n    "--sdata-funcs=sdata_funcs.txt --sdata-exclude=sdata_exclude.txt --expand-lb "',):
    assert bc.count(x) == 2, "buildconfig flag text changed"
    bc = bc.replace(x, '"--expand-div --aspsx-version=2.34 --expand-lb "')
bc = bc.replace("EXPAND_LH_FILES = set()", "EXPAND_LH_FILES = set()\n# Mirrors the Makefile PSYQ_LIBRARY_FILES (owner ruling Q69): Sony library code, maspsx without -G8;\n"
                "# every other file gets -G8. Generated by tools/psyq_library_files.py (--check verifies).\n"
                "PSYQ_LIBRARY_FILES = {" + ", ".join(f'"{g}"' for g in LIB) + "}")
open("engine/buildconfig.py", "w", newline=NL).write(bc)
pl = open("engine/pipeline.py").read()
old = "    maspsx_flags = cfg.MASPSX_FLAGS_GP if stem in cfg.GP_FILES else cfg.MASPSX_FLAGS"
assert pl.count(old) == 1
pl = pl.replace(old, old + "\n    if stem not in cfg.PSYQ_LIBRARY_FILES:\n        maspsx_flags += \" -G8\"")
# engine test: the library set agrees across Makefile / buildconfig, and with the evidence when linked
te = open("engine/test_engine.py").read()
a1 = "def test_maspsx_fingerprint() -> None:"
assert te.count(a1) == 1
te = te.replace(a1, open(f"{H}/test_psyq_library_files.txt").read() + "\n\n" + a1)
a2 = "    test_maspsx_fingerprint()\n"
assert te.count(a2) == 1
te = te.replace(a2, "    test_psyq_library_files()\n" + a2)
open("engine/test_engine.py", "w", newline=NL).write(te)
open("engine/pipeline.py", "w", newline=NL).write(pl)
for x in ("sdata_syms.txt", "sdata_funcs.txt", "sdata_exclude.txt"):
    os.remove(x)
# engine: the retired lists leave the pipeline-input sets (build stamp, oracle identity, queue fingerprint)
for p, pats in (("engine/buildstamp.py", [("'sdata_syms.txt', 'sdata_funcs.txt',\n    'sdata_exclude.txt', ", "")]),
                ("engine/oracle.py", [('    "sdata_syms.txt", "sdata_funcs.txt", "sdata_exclude.txt",\n', "")]),
                ("engine/queue.py", [('"sdata_syms.txt", "sdata_funcs.txt", "sdata_exclude.txt",', "")])):
    t = open(p).read()
    for a, bb in pats:
        if t.count(a) != 1:
            log(f"NOTE {p}: pattern not found once ({t.count(a)}): {a!r}")
            continue
        t = t.replace(a, bb)
    open(p, "w", newline=NL).write(t)
open("/tmp/q56/s10_log.txt", "w").write(NL.join(LOG) + NL)
open(f"{H}/s15_log.txt", "w", newline=NL).write(NL.join(LOG) + NL)
# the commit body: every disposition the generator took (layer-2 round 1, step-15 finding 6)
msg = ["Rule: .claude/rules/per-file-gp-model.md (Q65, Q67-Q72; A8/A9 = Q79-Q81, rules: 8c57bc4ab). Generator: s15_apply.py (docstring); evidence",
       "docs/grind/gp-model-2026-09-30.md; full generator log banked as memory/grind/q65-adoption/q56/adopt/s15_log.txt.",
       "Explicit-relocation asm excluded from E1/E2/split: memory/grind/q65-adoption/q56/adopt/explicit_exclusions.md.",
       "Owner rulings Q86/Q87 (rules: a17a2c144): the eight small read-only items in files that get -G8 stay in",
       ".rodata (record only, byte-neutral); D_800A3224 / D_800A3290 stay defined as declared with their evidence",
       "comments (the RECT / 8-byte record retype is owed after the adoption).",
       "Changed completed bodies (block-scope externs of the new statics removed, EXTERN-REMOVED below), each with",
       "its own layer-2: keys in memory/grind/q65-adoption/q56/adopt/body_hashes.txt (step15).", ""]
for title, pfx in (("Blocks extended to their last gp-reached object, declared in no C file", "BLOCK-END"),
                   ("gp-reached objects typed by another file's declaration", "TYPED-ELSEWHERE"),
                   ("Bytes after a block's last object inside the original's label run, left in the data blob", "BLOCK-TAIL"),
                   ("Small-data words whose original value is a C object's address, spelled as that address", "ADDRESS"),
                   ("Object-model notes (evidence for a later type; defined here as declared)", "DATA-MODEL"),
                   ("Symbol-file rows removed (objects now defined in C)", "ROW-REMOVED"),
                   ("Data-blob pieces between the C blocks", "BLOB-PIECE"),
                   ("Alignment padding inside a block (no object: the next object's alignment produces it)", "PADDING"),
                   ("Block ends on an odd byte (linker SUBALIGN(2) padding, no object)", "END-PAD"),
                   ("(A6, Q71) runs no single object fits, split into the fewest aligned pieces", "A6 "),
                   ("(A1) orphan objects joined to the one block that names them", "ORPHAN-JOIN"),
                   ("(A1; A9, owner ruling Q81) objects named by code but left in the data blob", "ORPHAN-LEFT"),
                   ("(A9, owner ruling Q80) objects named by a pointer in asm data, defined by layout", "A9 "),
                   ("Arrays sized from the original's object boundary", "SIZED-FROM-GAP"),
                   ("(K2) externs of the new statics removed (own file and other files)", "EXTERN-REMOVED"),
                   ("(K2) header externs removed", "HEADER"),
                   ("Held / blocked blocks", "HELD"), ("Blocked", "BLOCKED"), ("K2 violations", "K2-VIOLATION"),
                   ("K1 non-zero", "K1-NONZERO"), ("Aliases", "ALIAS"), ("Overlaps", "OVERLAP")):
    rows = [l for l in LOG if l.startswith(pfx)]
    if rows:
        msg.append(f"{title} ({len(rows)}):")
        msg += ["- " + r for r in rows]
        msg.append("")
msg += [l for l in LOG if l.startswith(("defined in ", "symbol-file rows", "blob cut", "PSYQ_LIBRARY_FILES"))]
open(f"{H}/s15_msg.txt", "w", newline=NL).write(NL.join(msg) + NL)
print("step 15 applied")
