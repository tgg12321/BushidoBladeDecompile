#!/usr/bin/env python3
# Two small hand-off debt items, as two commits.
#   engine: engine/layer2._asm_addr reads a function's address from the first machine column AFTER its glabel
#           (a module's leading data words may precede it in the same .s); tools/relocsim.py takes the target
#           words from the glabel on; engine test test_layer2_asm_addr_after_glabel.
#   phase2: D_8007E08C (MSC00's two leading words) folded into asm/funcs/InitGeom.s ahead of its glabel
#           (asm/funcs/D_8007E08C.s and msc00.c's separate include block go); SpuGetVoiceVolume gets PsyQ's
#           prototype in libspu.h (short *volL / *volR: s_gvv.c's definition and ut_vvol.c's locals follow).
# usage: fsmall1.py engine|phase2   writes tmp/p2/fsmall_<part>/ (scratch only); BASE_REV env (default f2b56796b; the engine part was generated on 6607ec419)
import os, subprocess, sys
NL = chr(10)
BASE_REV = os.environ.get("BASE_REV", "f2b56796b")
PART = sys.argv[1]
OUT = "tmp/p2/fsmall_%s/" % PART

def show(p):
    return subprocess.run(["git", "show", "%s:%s" % (BASE_REV, p)], capture_output=True, check=True, text=True,
                          encoding="utf-8").stdout

def rep(t, pairs):
    for a, b in pairs:
        assert t.count(a) == 1, a[:70]
        t = t.replace(a, b)
    return t

def engine():
    out = {}
    out["engine/layer2.py"] = rep(show("engine/layer2.py"), [(
        'def _asm_addr(text: str | None, func: str) -> str | None:\n'
        '    g = _GLABEL_RE.search(text or "")\n'
        '    m = _ASM_VADDR.search(text or "") if g and g.group(1) == func else None\n',
        'def _asm_addr(text: str | None, func: str) -> str | None:\n'
        '    """`func`\'s address: the first machine column AFTER its glabel (a module\'s leading data words may\n'
        '    sit ahead of the glabel in the same .s, e.g. MSC00\'s D_8007E08C in InitGeom.s)."""\n'
        '    g = _GLABEL_RE.search(text or "")\n'
        '    m = _ASM_VADDR.search(text, g.end()) if g and g.group(1) == func else None\n')])
    test = (
        '\n\ndef test_layer2_asm_addr_after_glabel() -> None:\n'
        '    """A .s may carry its module\'s leading data words ahead of the glabel (InitGeom.s holds MSC00\'s\n'
        '    D_8007E08C); the function\'s address is the first machine column after its glabel."""\n'
        '    from engine import layer2\n'
        '    text = ("dlabel D_8007E08C\\n"\n'
        '            "    /* 6E88C 8007E08C 50730915 */ .word 0x15097350\\n"\n'
        '            "    /* 6E890 8007E090 9C9F4000 */ .word 0x00409F9C\\n"\n'
        '            "enddlabel D_8007E08C\\n"\n'
        '            "glabel InitGeom\\n"\n'
        '            "    /* 6E894 8007E094 0A80013C */  lui        $at, %hi(D_8009C798)\\n")\n'
        '    eq("addresses: the first machine column after the glabel, not the leading data words",\n'
        '       layer2._asm_addr(text, "InitGeom"), "8007E094")\n'
        '    eq("addresses: a plain glabel file reads as before",\n'
        '       layer2._asm_addr("glabel f\\n    /* 1 80010000 00000000 */  nop\\n", "f"), "80010000")\n'
        '    eq("addresses: another function\'s file gives nothing", layer2._asm_addr(text, "D_8007E08C"), None)\n'
        '\n\ndef main() -> int:')
    t = rep(show("engine/test_engine.py"), [("\n\ndef main() -> int:", test)])
    out["engine/test_engine.py"] = rep(t, [("    test_layer2_addresses()\n",
                                            "    test_layer2_addresses()\n    test_layer2_asm_addr_after_glabel()\n")])
    out["tools/relocsim.py"] = rep(show("tools/relocsim.py"), [(
        'tw, base = [], None\nfor line in TGT.read_text().splitlines():\n'
        '    m = re.match(r"^\\s*/\\*\\s*\\S+\\s+([0-9A-Fa-f]{8})\\s+([0-9A-Fa-f]{8})\\s*\\*/", line)\n    if m:\n',
        'tw, base = [], None\n'
        'seen = False   # words from the function\'s glabel on (a module\'s leading data may precede it)\n'
        'for line in TGT.read_text().splitlines():\n'
        '    if re.match(r"^\\s*glabel\\s+%s\\s*$" % re.escape(func), line):\n        seen = True\n'
        '    m = re.match(r"^\\s*/\\*\\s*\\S+\\s+([0-9A-Fa-f]{8})\\s+([0-9A-Fa-f]{8})\\s*\\*/", line)\n    if m and seen:\n')])
    return out

# docs/naming/build_census.py: the census universe reads each function from its glabel (as
# engine/layer2._asm_addr does); a leading dlabel block is its own entry, as with its own .s.
CENSUS_OLD = '''glabel_re = re.compile(r"^glabel\\s+(\\S+)")
addr_re = re.compile(r"^\\s*/\\*\\s*[0-9A-Fa-f]+\\s+([0-9A-Fa-f]{8})\\s")
for fn in sorted(os.listdir(FUNCDIR)):
    if not fn.endswith(".s"):
        continue
    path = os.path.join(FUNCDIR, fn)
    with open(path, "r", encoding="utf-8", errors="replace") as fh:
        lines = fh.readlines()
    name = None
    addr = None
    insns = 0
    for ln in lines:
        m = glabel_re.match(ln)
        if m and name is None:
            name = m.group(1)
            continue
        m = addr_re.match(ln)
        if m:
            insns += 1
            if addr is None:
                addr = m.group(1).upper()
    if name is None:
        name = fn[:-2]
    funcs[name] = dict(addr=addr or "", stem=fn[:-2], insns=insns)
'''
CENSUS_NEW = '''glabel_re = re.compile(r"^glabel\\s+(\\S+)")
dlabel_re = re.compile(r"^dlabel\\s+(\\S+)")
addr_re = re.compile(r"^\\s*/\\*\\s*[0-9A-Fa-f]+\\s+([0-9A-Fa-f]{8})\\s")
for fn in sorted(os.listdir(FUNCDIR)):
    if not fn.endswith(".s"):
        continue
    path = os.path.join(FUNCDIR, fn)
    with open(path, "r", encoding="utf-8", errors="replace") as fh:
        lines = fh.readlines()
    # A function's address and size count from its glabel (as engine/layer2._asm_addr reads it). A
    # module's leading data block (dlabel) ahead of the glabel in the same .s -- MSC00's D_8007E08C in
    # InitGeom.s -- is its own entry, as when it had its own .s; a file with no glabel is one entry.
    if any(glabel_re.match(ln) for ln in lines):
        pre = []
        for i, ln in enumerate(lines):
            if glabel_re.match(ln):
                break
            pre.append(ln)
        lead = next((dlabel_re.match(ln).group(1) for ln in pre if dlabel_re.match(ln)), None)
        if lead and any(addr_re.match(ln) for ln in pre):
            cols = [addr_re.match(ln).group(1).upper() for ln in pre if addr_re.match(ln)]
            funcs[lead] = dict(addr=cols[0], stem=fn[:-2], insns=len(cols))
        lines = lines[i:]
    name = None
    addr = None
    insns = 0
    for ln in lines:
        m = glabel_re.match(ln)
        if m and name is None:
            name = m.group(1)
            continue
        m = addr_re.match(ln)
        if m:
            insns += 1
            if addr is None:
                addr = m.group(1).upper()
    if name is None:
        name = fn[:-2]
    funcs[name] = dict(addr=addr or "", stem=fn[:-2], insns=insns)
'''

GVV = """/* the form of psyz's decomp/src/libspu/s_gvv.c (github.com/Xeeynamo/psyz @ 973fa3460, PsyQ 4.0) */
static inline void assign(u16 val, s16 *out) {
    u32 offset = 0x8000; /* FAKE: 0x8000 held in a u32 so the adjustment is a subu; inline, uval - 0x8000 becomes an addu of the same 0x8000 register (same low 16 bits): score 2 */
    u32 uval = val;

    if (uval >= 0x4000) {
        *out = uval - offset;
    } else {
        *out = val;
    }
}

void SpuGetVoiceVolume(s32 arg0, s16 *arg1, s16 *arg2) {
    u16 left = _spu_RXX->raw[arg0 * 8];
    u16 right = _spu_RXX->raw[arg0 * 8 + 1]; /* FAKE: read ahead of the left channel's store (the two lhu together); read at its assign: score 14 */

    assign(left, arg1);
    assign(right, arg2);
}
"""

def phase2():
    out = {}
    out["asm/funcs/InitGeom.s"] = show("asm/funcs/D_8007E08C.s") + show("asm/funcs/InitGeom.s")
    m = show("src/main/psxsdk/libgte/msc00.c")
    i = m.index("/* D_8007E08C: the two leading words")
    j = m.index('INCLUDE_ASM("asm/funcs", InitGeom);')
    out["src/main/psxsdk/libgte/msc00.c"] = rep(m[:i] + m[j:], [(
        "/* PsyQ 4.0 LIBGTE MSC00: InitGeom, after the module's two leading data words D_8007E08C. .text",
        "/* PsyQ 4.0 LIBGTE MSC00: InitGeom, after the module's two leading data words D_8007E08C (both in\n"
        " * asm/funcs/InitGeom.s, ahead of its glabel). .text")])
    out["include/psxsdk/libspu.h"] = rep(show("include/psxsdk/libspu.h"), [(
        "extern void SpuGetVoiceEnvelope(s32, u16 *);\n",
        "extern void SpuGetVoiceEnvelope(s32, u16 *);\nextern void SpuGetVoiceVolume(s32, s16 *, s16 *);\n")])
    t = show("src/main/psxsdk/libspu/s_gvv.c")
    a = t.index("void SpuGetVoiceVolume(s32 arg0, u16 *arg1, u16 *arg2) {")
    b = t.index("\n}\n", a) + 3
    out["src/main/psxsdk/libspu/s_gvv.c"] = t[:a] + GVV + t[b:]
    out["docs/naming/README.md"] = rep(show("docs/naming/README.md"), [
        ("| `asm/funcs/*.s` files | 1,438 |", "| `asm/funcs/*.s` files | 1,437 |"),
        ("| minus `D_8007E08C.s`, `jtbl_comb_control.s` | 1,436 | data-as-code blob; a jump table",
         "| minus `jtbl_comb_control.s` (and InitGeom.s's leading `D_8007E08C` dlabel block) | 1,436 | MSC00's two leading data words, ahead of InitGeom's glabel; a jump table")])
    out["src/main/psxsdk/libsnd/ut_vvol.c"] = rep(show("src/main/psxsdk/libsnd/ut_vvol.c"), [
        ("    u16 raw1, raw2;\n", "    s16 raw1, raw2;\n"),
        ("        *a1 = (s16)raw1 / 129;\n        *a2 = (s16)raw2 / 129;\n", "        *a1 = raw1 / 129;\n        *a2 = raw2 / 129;\n")])
    out["docs/naming/build_census.py"] = rep(show("docs/naming/build_census.py"), [(CENSUS_OLD, CENSUS_NEW)])
    return out   # plus: asm/funcs/D_8007E08C.s deleted

def write():
    out = engine() if PART == "engine" else phase2()
    for p, x in out.items():
        q = OUT + p
        os.makedirs(os.path.dirname(q), exist_ok=True)
        open(q, "w", encoding="utf-8", newline=NL).write(x)
    print("wrote", OUT, sorted(out), "(phase2 also deletes asm/funcs/D_8007E08C.s)" if PART == "phase2" else "")

if __name__ == "__main__":
    write()
