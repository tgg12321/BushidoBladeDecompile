#!/usr/bin/env python3
"""psyqobj.py — read the whole .text of a Psy-Q LNK v2 object (every BYTES/ZEROES record of .text, in
order). tools/maspsx/aspsx/util.py returns only the first chunk and stops at unknown records; this walks
the record stream (opcodes after pcsx-redux tools/psyq-obj-parser)."""

def _pstr(d, p):
    n = d[p]; return d[p + 1:p + 1 + n].decode("latin1"), p + 1 + n

def _expr(d, p):
    op = d[p]; p += 1
    if op == 0:
        return p + 4
    if op in (2, 4, 6, 8, 0x0A, 0x0C, 0x0E, 0x10):  # one u16 operand (symbol / section refs)
        return p + 2
    if op >= 0x2C:          # binary operators: two sub-expressions
        p = _expr(d, p)
        return _expr(d, p)
    raise ValueError(f"unknown expression op {op:#x} at {p - 1}")

def read_text(d: bytes) -> bytes:
    assert d[:3] == b"LNK" and d[3] == 2
    p, secs, cur, out = 4, {}, None, bytearray()
    while p < len(d):
        op = d[p]; p += 1
        if op == 0:
            break
        elif op == 2:
            n = int.from_bytes(d[p:p + 2], "little"); p += 2
            if secs.get(cur) == ".text":
                out += d[p:p + n]
            p += n
        elif op == 6:
            cur = int.from_bytes(d[p:p + 2], "little"); p += 2
        elif op == 8:
            n = int.from_bytes(d[p:p + 4], "little"); p += 4
            if secs.get(cur) == ".text":
                out += b"\0" * n
        elif op == 10:
            p += 1 + 2
            p = _expr(d, p)
        elif op == 12:
            p += 2 + 2 + 4; _, p = _pstr(d, p)
        elif op == 14:
            p += 2; _, p = _pstr(d, p)
        elif op == 16:
            idx = int.from_bytes(d[p:p + 2], "little"); p += 2 + 2 + 1
            secs[idx], p = _pstr(d, p)
        elif op == 18:
            p += 2 + 4; _, p = _pstr(d, p)
        elif op == 28:
            p += 2; _, p = _pstr(d, p)
        elif op == 46:
            p += 1
        elif op == 48:
            p += 2 + 2 + 4; _, p = _pstr(d, p)
        else:
            raise ValueError(f"unknown record {op} at {p - 1}")
    return bytes(out)
