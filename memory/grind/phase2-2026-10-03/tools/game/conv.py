# conv.py: helpers -- rewrite raw Unk80101EC8Record accesses inside one function body
import re

# offset -> (member, declared type)
M = {
    0x04: ("index", "s16"), 0x06: ("unk_06", "s16"), 0x0A: ("unk_0A", "s16"), 0x0C: ("unk_0C", "s16"),
    0x0E: ("unk_0E", "s16"), 0x1A: ("unk_1A", "s16"), 0x2C: ("unk_24.held", "u32"),
    0x44: ("unk_44", "s16"), 0x6A: ("unk_6A", "u16"), 0x72: ("unk_72", "s16"), 0x74: ("unk_74", "s32"),
    0x94: ("unk_94", "s16"), 0xB1: ("unk_B1", "u8"),
    0xB8: ("unk_B8.vx", "s32"), 0xBC: ("unk_B8.vy", "s32"), 0xC0: ("unk_B8.vz", "s32"),
    0xC8: ("unk_C8.vx", "s32"), 0xD0: ("unk_C8.vz", "s32"),
    0xD8: ("unk_D8.x", "s32"), 0xE0: ("unk_D8.z", "s32"),
    0x104: ("unk_104.vx", "s32"), 0x108: ("unk_104.vy", "s32"), 0x10C: ("unk_104.vz", "s32"),
    0x148: ("unk_148", "s32"), 0x14C: ("unk_14C", "s16"), 0x14E: ("unk_14E", "s16"),
    0x150: ("unk_150", "s16"), 0x152: ("unk_152", "s16"), 0x154: ("unk_154", "s16"),
    0x1CA: ("unk_1C8.vy", "s16"), 0x1D8: ("unk_1D8", "s16"), 0x1DC: ("unk_1DC", "s16"),
    0x286: ("unk_286", "s16"), 0x318: ("unk_318", "s16"), 0x31A: ("unk_31A", "s16"),
    0x31C: ("unk_31C", "s16"), 0x320: ("unk_320.x", "s32"), 0x328: ("unk_320.z", "s32"),
}
SAME = {("s16", "s16"), ("u16", "u16"), ("s32", "s32"), ("u32", "u32"), ("u8", "u8"),
        ("u16", "s16"), ("s16", "u16")}  # sign-only differences: plain read tried first

def conv(body, var, log, keep=()):
    """Replace *(T *)(var + 0xN), *((T *) (var + 0xN)) and *(T *)((u8 *)var + 0xN) with var->member.
    keep: offsets whose raw-type read keeps a (T) cast, e.g. {0x0E}."""
    T = r"(s8|u8|s16|u16|s32|u32)"; N = r"(0x[0-9A-Fa-f]+|\d+)"
    patA = re.compile(r"\*\(\(\s*" + T + r"\s*\*\s*\)\s*\(\s*" + var + r"\s*\+\s*" + N + r"\s*\)\)")
    patB = re.compile(r"\*\(\s*" + T + r"\s*\*\s*\)\s*\(\s*(?:\(u8 \*\)\s*)?" + var + r"\s*\+\s*" + N + r"\s*\)")
    def sub(m):
        t, off = m.group(1), int(m.group(2), 0)
        mem, mt = M[off]
        if (t, mt) not in SAME and not (t in ("s8", "u8") and mt == "u8"):
            raise SystemExit(f"type mismatch {t} vs {mt} at {hex(off)}: {m.group(0)}")
        log.append((hex(off), t, mt))
        if off in keep and t != mt:
            return f"({t}){var}->{mem}"
        return f"{var}->{mem}"
    return patB.sub(sub, patA.sub(sub, body))

def func_slice(s, head):
    a = s.index(head); b = s.index("\n}\n", a) + 3
    return a, b

def rep(t, pairs, where):
    for o, n, c in pairs:
        assert t.count(o) == c, (where, o, t.count(o)); t = t.replace(o, n)
    return t

def tidy(body, var):
    """Drop the parentheses the raw spelling left around a lone member access: (var->m) -> var->m."""
    return re.sub(r"(?<![\w\]])\((" + var + r"->[\w.]+)\)", r"\1", body)
