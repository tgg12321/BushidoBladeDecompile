# tostruct.py <in.c> <out.c> : convert record byte-offset casts to member accesses of a test struct R
import sys, re
s = open(sys.argv[1]).read()
fields = {  # off: (type, name)
 0xC: ('s16','c'), 0xE: ('u16','e'), 0x3C: ('s32','f3c'), 0x40: ('s16','frame'), 0x6A: ('u16','move'),
 0x8C: ('s16','f8c'), 0x92: ('s16','f92'), 0x96: ('s16','f96'), 0xA1: ('u8','win[3]'), 0xAD: ('u8','fad'), 0xAE: ('u8','fae'),
 0xF4: ('Vec3i32','pos'), 0x114: ('Vec4i32','vel[2]'), 0x134: ('Vec4i32','nudge'), 0x1C8: ('SVec4i16','rot'),
 0x1D8: ('s16','yaw'), 0x210: ('LeafPos','seg[3]'), 0x234: ('LeafPos','seg2[2]'), 0x26C: ('s16','f26c'),
 0x286: ('s16','f286'), 0x34A: ('u8','f34a'),
}
sizes = {'s16':2,'u16':2,'s32':4,'u8':1,'Vec3i32':12,'Vec4i32':16,'SVec4i16':8,'LeafPos':12}
off = 0; body = []
for o in sorted(fields):
    t, n = fields[o]
    if o > off: body.append(f"    u8 pad{off:X}[0x{o:X} - 0x{off:X}];")
    body.append(f"    {t} {n};")
    cnt = int(re.search(r'\[(\d+)\]', n).group(1)) if '[' in n else 1
    off = o + sizes[t] * cnt
body.append(f"    u8 pad{off:X}[0x44C - 0x{off:X}];")
st = "typedef struct {\n" + "\n".join(body) + "\n} R8002AB08;\n"
name = {o: re.sub(r'\[.*', '', n) for o, (t, n) in fields.items()}
def R(v): return f"((R8002AB08 *){v})"
rep = [
 (r"\*\(s16 \*\)\((other|self) \+ 0xC\)", lambda m: f"{R(m[1])}->c"),
 (r"\*\(u16 \*\)\((other|self) \+ 0xE\)", lambda m: f"{R(m[1])}->e"),
 (r"\*\(s32 \*\)\((other|self) \+ 0x3C\)", lambda m: f"{R(m[1])}->f3c"),
 (r"\*\(s16 \*\)\((other|self) \+ 0x40\)", lambda m: f"{R(m[1])}->frame"),
 (r"\*\(u16 \*\)\((other|self) \+ 0x6A\)", lambda m: f"{R(m[1])}->move"),
 (r"\*\(s16 \*\)\((other|self) \+ 0x8C\)", lambda m: f"{R(m[1])}->f8c"),
 (r"\*\(s16 \*\)\((other|self) \+ 0x92\)", lambda m: f"{R(m[1])}->f92"),
 (r"\*\(s16 \*\)\((other|self) \+ 0x96\)", lambda m: f"{R(m[1])}->f96"),
 (r"\*\(u8 \*\)\((other|self) \+ 0xA1\)", lambda m: f"{R(m[1])}->win[0]"),
 (r"\*\(u8 \*\)\((other|self) \+ 0xA2\)", lambda m: f"{R(m[1])}->win[1]"),
 (r"\*\(u8 \*\)\((other) \+ alt \+ 0xA1\)", lambda m: f"{R(m[1])}->win[alt]"),
 (r"\*\(u8 \*\)\((other) \+ alt \+ 0xA3\)", lambda m: f"{R(m[1])}->win[alt + 2]"),
 (r"\*\(u8 \*\)\((other|self) \+ 0xAD\)", lambda m: f"{R(m[1])}->fad"),
 (r"\*\(u8 \*\)\((other|self) \+ 0xAE\)", lambda m: f"{R(m[1])}->fae"),
 (r"\*\(s32 \*\)\((other|self) \+ 0xF4\)", lambda m: f"{R(m[1])}->pos.x"),
 (r"\*\(s32 \*\)\((other|self) \+ 0xFC\)", lambda m: f"{R(m[1])}->pos.z"),
 (r"\*\(s32 \*\)\((other) \+ \((\w+) << 4\) \+ 0x114\)", lambda m: f"{R(m[1])}->vel[{m[2]}].vx"),
 (r"\*\(s32 \*\)\((other) \+ \((\w+) << 4\) \+ 0x11C\)", lambda m: f"{R(m[1])}->vel[{m[2]}].vz"),
 (r"\(s32 \*\)\(other \+ 0x114 \+ alt \* 0x10\)", lambda m: f"&{R('other')}->vel[alt].vx"),
 (r"\(s32 \*\)\(other \+ 0x114\)", lambda m: f"&{R('other')}->vel[0].vx"),
 (r"\*\(s32 \*\)\((other|self) \+ 0x134\)", lambda m: f"{R(m[1])}->nudge.vx"),
 (r"\*\(s32 \*\)\((other|self) \+ 0x13C\)", lambda m: f"{R(m[1])}->nudge.vz"),
 (r"\*\(s16 \*\)\((other|self) \+ 0x1CA\)", lambda m: f"{R(m[1])}->rot.vy"),
 (r"\*\(s16 \*\)\((other|self) \+ 0x1D8\)", lambda m: f"{R(m[1])}->yaw"),
 (r"\(\(LeafPos \*\)\((other) \+ 0x210\)\)\[(\w+)\]", lambda m: f"{R(m[1])}->seg[{m[2]}]"),
 (r"\(\(LeafPos \*\)\((other) \+ 0x234\)\)\[(\w+)\]", lambda m: f"{R(m[1])}->seg2[{m[2]}]"),
 (r"\*\(s32 \*\)\((other) \+ 0x21C\)", lambda m: f"{R(m[1])}->seg[1].x"),
 (r"\*\(s32 \*\)\((other) \+ 0x220\)", lambda m: f"{R(m[1])}->seg[1].y"),
 (r"\*\(s32 \*\)\((other) \+ 0x224\)", lambda m: f"{R(m[1])}->seg[1].z"),
 (r"\*\(s16 \*\)\((other|self) \+ 0x26C\)", lambda m: f"{R(m[1])}->f26c"),
 (r"\*\(s16 \*\)\((other|self) \+ 0x286\)", lambda m: f"{R(m[1])}->f286"),
 (r"\*\(u8 \*\)\((other|self) \+ 0x34A\)", lambda m: f"{R(m[1])}->f34a"),
]
for pat, f in rep:
    s = re.sub(pat, f, s)
left = re.findall(r"\((?:other|self) \+[^)]*\)", s)
print("unconverted:", left)
s = s.replace("extern void func_8002CA8C", st + "extern void func_8002CA8C", 1)
open(sys.argv[2], 'w', newline='\n').write(s)
