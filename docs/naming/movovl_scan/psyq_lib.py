#!/usr/bin/env python3
"""psyq_lib.py — minimal SN Systems PsyQ LIB/OBJ parser (Closer W3, 2026-07-09).

Parses a PsyQ .LIB ("LIB\\x01" container), extracts a named module's OBJ
("LNK\\x02" stream), reconstructs its sections, XDEF symbols and relocations.

Usage:
  python3 psyq_lib.py LIBCD.LIB BIOS            # dump symbols + section sizes
  python3 psyq_lib.py LIBCD.LIB BIOS --sym CD_sync --words N
"""
import struct, sys


def lib_modules(b):
    assert b[:4] == b"LIB\x01", "not a PsyQ LIB"
    pos = 4
    while pos < len(b):
        name = b[pos:pos + 8].decode("ascii", "replace").strip()
        date, objoff, nxt = struct.unpack_from("<III", b, pos + 8)
        yield name, b[pos + objoff: pos + nxt]
        pos += nxt


class Obj:
    def __init__(self):
        self.sections = {}      # id -> {"name":, "bytes": bytearray}
        self.xdefs = []         # (name, section_id, offset)
        self.xrefs = {}         # symbol_index -> name
        self.relocs = []        # (section_id, type, offset, expr_repr)
        self.locals = []


def parse_expr(b, p, xrefs):
    op = b[p]; p += 1
    if op == 0:
        v = struct.unpack_from("<I", b, p)[0]; return p + 4, f"0x{v:x}"
    if op == 2:
        v = struct.unpack_from("<H", b, p)[0]; return p + 2, f"sym:{xrefs.get(v, v)}"
    if op == 4:
        v = struct.unpack_from("<H", b, p)[0]; return p + 2, f"secbase:{v}"
    if op == 12:
        v = struct.unpack_from("<H", b, p)[0]; return p + 2, f"secstart:{v}"
    if op == 22:
        v = struct.unpack_from("<H", b, p)[0]; return p + 2, f"secend:{v}"
    if op in (44, 46, 50, 54, 58):  # add/sub/div/etc binary
        p, l = parse_expr(b, p, xrefs)
        p, r = parse_expr(b, p, xrefs)
        o = {44: "+", 46: "-", 50: "/"}.get(op, f"op{op}")
        return p, f"({l}{o}{r})"
    raise ValueError(f"unknown expr opcode {op} at {p-1}")


def parse_obj(b):
    assert b[:3] == b"LNK", "not an SN OBJ"
    o = Obj()
    p = 4  # "LNK" + version byte
    cur = None
    block_base = {}  # section -> start offset of most recent BYTES block
    def name_at(p):
        n = b[p]; return p + 1 + n, b[p + 1:p + 1 + n].decode("ascii", "replace")
    while p < len(b):
        op = b[p]; p += 1
        if op == 0:      # END
            break
        elif op == 2:    # BYTES
            n = struct.unpack_from("<H", b, p)[0]; p += 2
            block_base[cur] = len(o.sections[cur]["bytes"])
            o.sections[cur]["bytes"] += b[p:p + n]; p += n
        elif op == 6:    # SWITCH section
            cur = struct.unpack_from("<H", b, p)[0]; p += 2
            o.sections.setdefault(cur, {"name": f"sec{cur}", "bytes": bytearray()})
        elif op == 8:    # ZEROES
            n = struct.unpack_from("<I", b, p)[0]; p += 4
            o.sections[cur]["bytes"] += bytes(n)
        elif op == 10:   # RELOCATION
            t = b[p]; p += 1
            off = struct.unpack_from("<H", b, p)[0]; p += 2
            p, e = parse_expr(b, p, o.xrefs)
            o.relocs.append((cur, t, block_base.get(cur, 0) + off, e))
        elif op == 12:   # XDEF
            si, sec = struct.unpack_from("<HH", b, p); p += 4
            off = struct.unpack_from("<I", b, p)[0]; p += 4
            p, nm = name_at(p)
            o.xdefs.append((nm, sec, off)); o.xrefs[si] = nm
        elif op == 14:   # XREF
            si = struct.unpack_from("<H", b, p)[0]; p += 2
            p, nm = name_at(p)
            o.xrefs[si] = nm
        elif op == 16:   # SECTION
            si, grp = struct.unpack_from("<HH", b, p); p += 4
            align = b[p]; p += 1
            p, nm = name_at(p)
            o.sections.setdefault(si, {"name": nm, "bytes": bytearray()})
            o.sections[si]["name"] = nm
        elif op == 18:   # LOCAL_SYMBOL
            sec = struct.unpack_from("<H", b, p)[0]; p += 2
            off = struct.unpack_from("<I", b, p)[0]; p += 4
            p, nm = name_at(p)
            o.locals.append((nm, sec, off))
        elif op == 28:   # FILENAME
            p += 2; p, _ = name_at(p)
        elif op == 46:   # PROGRAMTYPE
            p += 1
        elif op == 48:   # UNINITIALIZED (bss symbol)
            si, sec = struct.unpack_from("<HH", b, p); p += 4
            sz = struct.unpack_from("<I", b, p)[0]; p += 4
            p, nm = name_at(p)
            o.xrefs[si] = nm
        elif op == 50: p += 2                    # INC_SLD_LINENUM
        elif op == 52: p += 3                    # INC_SLD_LINENUM_BY_BYTE
        elif op == 54: p += 4                    # INC_SLD_LINENUM_BY_WORD
        elif op == 56: p += 6                    # SET_SLD_LINENUM
        elif op == 58: p += 8                    # SET_SLD_LINENUM_FILE
        elif op == 60: p += 2                    # END_SLD
        else:
            raise ValueError(f"unknown opcode {op} at {p-1} (0x{p-1:x})")
    return o


def get_module(libpath, modname):
    b = open(libpath, "rb").read()
    for name, data in lib_modules(b):
        if name == modname:
            return parse_obj(data)
    raise SystemExit(f"module {modname} not in {libpath}")


if __name__ == "__main__":
    libpath, mod = sys.argv[1], sys.argv[2]
    o = get_module(libpath, mod)
    for sid, s in sorted(o.sections.items()):
        print(f"section {sid} {s['name']:10s} {len(s['bytes'])} bytes")
    for nm, sec, off in sorted(o.xdefs, key=lambda x: (x[1], x[2])):
        sname = o.sections.get(sec, {}).get("name", sec)
        print(f"XDEF {nm:24s} {sname}+0x{off:x}")
    if "--sym" in sys.argv:
        sym = sys.argv[sys.argv.index("--sym") + 1]
        nw = int(sys.argv[sys.argv.index("--words") + 1]) if "--words" in sys.argv else 32
        nm, sec, off = next(x for x in o.xdefs if x[0] == sym)
        data = o.sections[sec]["bytes"]
        rl = {r[2]: (r[1], r[3]) for r in o.relocs if r[0] == sec}
        for i in range(nw):
            a = off + i * 4
            w = struct.unpack_from("<I", data, a)[0]
            note = ""
            for d in range(4):
                if a + d in rl:
                    t, e = rl[a + d]
                    note = f"  reloc t{t} {e}"
            print(f"+{i*4:04x} {w:08x}{note}")
