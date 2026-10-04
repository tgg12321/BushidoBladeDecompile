import sys
sys.path.insert(0, "tools/libscan")
import psyq_lib as P
want = set(sys.argv[2:])
b = open(sys.argv[1], "rb").read()
for name, objb in P.lib_modules(b):
    if want and name not in want:
        continue
    o = P.parse_obj(objb)
    secs = {sid: (s["name"], len(s["bytes"])) for sid, s in o.sections.items()}
    text = [v for v in secs.values() if v[0] == ".text"]
    xs = sorted(((sec, off, nm) for nm, sec, off in o.xdefs), key=lambda x: (x[0], x[1]))
    xd = ", ".join(f"{nm}{'' if secs.get(sec,('',))[0]=='.text' else '['+str(secs.get(sec,('?',))[0])+']'}+0x{off:x}" for sec, off, nm in xs)
    print(f"{name:10s} text=0x{(text[0][1] if text else 0):x} {xd}")
