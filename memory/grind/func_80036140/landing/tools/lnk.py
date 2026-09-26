"""Minimal PsyQ LNK (.OBJ) reader: the full .text byte image (all BYTES / ZEROES records, in order),
exported symbol offsets (SYMS) and, per patched .text offset, the symbol names its relocation
expression references (RELOCS). Opcode / expression tables per the public psyq-obj-parser
(pcsx-redux tools/psyq-obj-parser)."""

SYMS = {}
RELOCS = {}
_NAMES = {}


def _expr(d, p, refs):
    op = d[p]; p += 1
    if op == 0:
        return p + 4
    if op == 2:
        refs.append(('sym', int.from_bytes(d[p:p + 2], 'little')))
        return p + 2
    if op in (4, 12, 22):
        refs.append(('sec', int.from_bytes(d[p:p + 2], 'little')))
        return p + 2
    if op in (44, 46, 50, 0x2A, 0x30, 0x32, 0x34, 0x36, 0x38, 0x3A, 0x3C, 0x3E, 0x40):
        p = _expr(d, p, refs)
        return _expr(d, p, refs)
    raise ValueError(f'expr opcode {op} at {p - 1}')


def text_bytes(data: bytes) -> bytes:
    SYMS.clear(); RELOCS.clear(); _NAMES.clear()
    assert data[:3] == b'LNK', data[:3]
    p = 4
    sections, cur, text = {}, None, bytearray()
    pending = []
    last_base = None
    while p < len(data):
        op = data[p]; p += 1
        if op == 0:
            break
        elif op == 46:
            p += 1
        elif op == 16:
            idx = int.from_bytes(data[p:p + 2], 'little'); p += 5
            n = data[p]; p += 1
            sections[idx] = data[p:p + n].decode(); p += n
        elif op == 6:
            cur = sections[int.from_bytes(data[p:p + 2], 'little')]; p += 2
        elif op == 2:
            n = int.from_bytes(data[p:p + 2], 'little'); p += 2
            base = len(text) if cur == '.text' else None
            if cur == '.text':
                text += data[p:p + n]
            p += n
            last_base = base
        elif op == 8:
            n = int.from_bytes(data[p:p + 4], 'little'); p += 4
            if cur == '.text':
                text += bytes(n)
        elif op == 10:
            off = int.from_bytes(data[p + 1:p + 3], 'little'); p += 3
            refs = []
            p = _expr(data, p, refs)
            if cur == '.text' and last_base is not None:
                pending.append((last_base + off, refs))
        elif op == 12:
            si = int.from_bytes(data[p:p + 2], 'little')
            sec = int.from_bytes(data[p + 2:p + 4], 'little'); off = int.from_bytes(data[p + 4:p + 8], 'little')
            p += 8
            n = data[p]; name = data[p + 1:p + 1 + n].decode(); p += 1 + n
            SYMS[name] = (sections.get(sec), off); _NAMES[si] = name
        elif op == 14:
            si = int.from_bytes(data[p:p + 2], 'little'); p += 2
            n = data[p]; _NAMES[si] = data[p + 1:p + 1 + n].decode(); p += 1 + n
        elif op == 18:
            p += 6
            n = data[p]; p += 1 + n
        elif op == 28:
            p += 2
            n = data[p]; p += 1 + n
        elif op == 48:
            si = int.from_bytes(data[p:p + 2], 'little'); p += 8
            n = data[p]; _NAMES[si] = data[p + 1:p + 1 + n].decode(); p += 1 + n
        else:
            raise ValueError(f'opcode {op} at {p - 1}')
    for off, refs in pending:
        RELOCS[off] = [(_NAMES.get(i, f'#{i}') if k == 'sym' else sections.get(i, f'sec{i}')) for k, i in refs]
    return bytes(text)
