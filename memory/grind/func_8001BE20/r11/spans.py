import re, sys
def spans(path, regs):
    t = open(path).read()
    blocks = re.split(r'\n(?=\((?:insn|jump_insn|call_insn|code_label|note|barrier) )', t)
    order = []
    for b in blocks:
        m = re.match(r'\((\w+) (\d+)', b)
        if not m: continue
        rs = set(int(x) for x in re.findall(r'\(reg(?:/[a-z])*:\w+ (\d+)\)', b))
        order.append((m.group(1), int(m.group(2)), rs))
    lab = [i for i,(k,u,rs) in enumerate(order) if k == 'code_label']
    for r in regs:
        idx = [i for i,(k,u,rs) in enumerate(order) if r in rs]
        print(f"  pseudo {r}: first pos {idx[0]} (uid {order[idx[0]][1]}), last pos {idx[-1]} (uid {order[idx[-1]][1]}), refs {len(idx)}" if idx else f"  pseudo {r}: none")
for tag, regs in (("be20_reuse_stock", (95, 75, 76, 479)), ("be20_split_stock", (96, 76, 77, 480))):
    print(tag); spans(f"{tag}/f.lreg", regs)
