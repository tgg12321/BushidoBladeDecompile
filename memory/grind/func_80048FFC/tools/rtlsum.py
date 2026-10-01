"""rtlsum.py <dumpfile>: one line per insn: uid, compact pattern (whitespace squeezed)."""
import re, sys
txt = open(sys.argv[1]).read()
items = re.split(r"\n(?=\((?:insn|call_insn|jump_insn|code_label|note|barrier) )", txt)
for it in items:
    m = re.match(r"\((insn|call_insn|jump_insn|code_label|note|barrier) (\d+)", it)
    if not m:
        continue
    kind, uid = m.groups()
    s = re.sub(r"\s+", " ", it)
    if kind == "note":
        if "NOTE_INSN_DELETED" in s or "BLOCK" in s:
            continue
        print(uid, "NOTE", s[s.find('"'):][:60]); continue
    if kind == "code_label":
        print(uid, "LABEL"); continue
    # strip the uid/prev/next header
    body = re.sub(r"^\((?:insn|call_insn|jump_insn) \d+ \d+ \d+ ", "", s)
    body = re.sub(r"\(expr_list:REG_DEP_ANTI.*$|\(insn_list.*$", "", body)
    print(uid, kind[0], body[:150])
