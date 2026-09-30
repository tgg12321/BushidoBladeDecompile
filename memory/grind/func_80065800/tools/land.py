"""func_80065800 landing edits (run from the repo root, ONLY while holding the landing lock).
1. text1b_tu1c.c: aggregate merge of the per-mode tables (D_800F0BA8 s16[18], D_800F0CA0
   Unk800F0C10Record[18]) across every consumer, then the func_80065800 body.
2. include/game.h: the two array declarations.
3. text1b_tu1c.c: drop the hand-transcribed jtbl_800158F8 / jtbl_80015940 (now compiled);
   text1a_b_pre_rodata_b.c: comment only.
4. undefined_syms_auto.txt / named_syms.txt: retire the per-word alias rows for the merged bytes."""
import re
from pathlib import Path

NL = chr(10)


def rd(p):
    t = Path(p).read_bytes().decode('utf-8')
    assert '\r' not in t, p
    return t


def wr(p, t):
    Path(p).write_bytes(t.encode('utf-8'))


def sub1(t, a, b):
    assert t.count(a) == 1, a[:100]
    return t.replace(a, b)


def in_tmr(a):
    return 0x800F0BA8 <= a < 0x800F0BCC and a % 2 == 0


def in_rec(a):
    return 0x800F0CA0 <= a < 0x800F0D78 and a % 4 == 0


def tmr(a):
    return '@@TMR@@[%d]' % ((a - 0x800F0BA8) // 2)


def rec(a):
    off = a - 0x800F0CA0
    return '@@POS@@[%d].unk%X' % (off // 12, off % 12)


# ---- 1. text1b_tu1c.c -------------------------------------------------------------
P = 'src/text1b_tu1c.c'
t = rd(P)
out, dropped = [], 0
for line in t.split(NL):
    m = re.match(r'^\s*extern (?:s16|u16|s32|u32) D_(800F0[0-9A-F]{3});\s*$', line)
    if m and (in_tmr(int(m.group(1), 16)) or in_rec(int(m.group(1), 16))):
        dropped += 1
        continue
    out.append(line)
t = NL.join(out)
t = re.sub(r'\(u16 \*\)&D_(800F0[0-9A-F]{3})\b',
           lambda m: '&' + tmr(int(m.group(1), 16)) if in_tmr(int(m.group(1), 16)) else m.group(0), t)
t = re.sub(r'\*\(s16 \*\)&D_(800F0[0-9A-F]{3})\b',
           lambda m: tmr(int(m.group(1), 16)) if in_tmr(int(m.group(1), 16)) else m.group(0), t)


def repl(m):
    a = int(m.group(1), 16)
    if in_tmr(a):
        return tmr(a)
    if in_rec(a):
        return rec(a)
    return m.group(0)


t = re.sub(r'\bD_(800F0[0-9A-F]{3})\b', repl, t)
t = re.sub(r'\bu16 \*(\w+) = &@@TMR@@\[', r's16 *\1 = &@@TMR@@[', t)
t = sub1(t, '    u16 *v1;\n    s32 v0;\n    func_80065800(0xC);\n    v1 = &@@TMR@@[12];\n    v0 = *v1 + 1;\n    *v1 = v0;\n',
         '    s16 *v1;\n    s32 v0;\n    func_80065800(0xC);\n    v1 = &@@TMR@@[12];\n    *v1 = *v1 + 1;\n')
t = sub1(t, 'if ((s16)@@TMR@@[14] >= 11)', 'if (@@TMR@@[14] >= 11)')
t = t.replace('@@TMR@@', 'D_800F0BA8').replace('@@POS@@', 'D_800F0CA0')
left = [l for l in t.split(NL) if re.search(r'\(u16 \*\)&D_800F0BA8|\(s16 \*\)&D_800F0BA8|u16 \*\w+ = &D_800F0BA8|\(s16\)D_800F0BA8', l)]
assert not left, left
inc = 'INCLUDE_ASM("asm/funcs", func_80065800);\n'
# The two transcribed switch tables (moved here by the 5c543ce1d boundary commit) become the
# compiler's own tables for the C body, at the same place in text1b_tu1c.o's .rodata.
ja = t.index('/* jtbl_800158F8: 18 words (72B) @ 0x800158F8 */')
jb = t.index(inc)
assert t.count('jtbl_800158F8') == 2 and t.count('jtbl_80015940') == 2 and ja < jb
t = t[:ja] + t[jb:]
t = sub1(t, inc, rd('tmp/f65800/final.c'))
wr(P, t)
print('text1b_tu1c.c: dropped %d per-word externs' % dropped)

# ---- 2. include/game.h ---------------------------------------------------------------
G = 'include/game.h'
h = rd(G)
h = sub1(h, 'extern Unk800F0C10Record D_800F0C10[4][3];\n', """extern Unk800F0C10Record D_800F0C10[4][3];

/* Per-effect-mode state, modes 0..0x11 (func_80065800 and its per-mode init and step
 * functions). Object model evidence (the original binary): asm/funcs/func_80065800.s
 * :39-45 addresses the position records as base + mode*12 (sll/addu/sll, then
 * %lo(D_800F0CA0)($at)) and :147-152 the timers as base + mode*2; modes 8/9 read
 * element mode-2, base - 4 + mode*2 (:351-356). The init functions
 * func_80064E90..func_800652AC fill the records and initialise the timers; the step
 * functions func_800652F4..func_800657B0 advance the timers. Replaces the splat
 * per-word scalars D_800F0BA8..D_800F0BCA and D_800F0CA0..D_800F0D74. */
extern s16 D_800F0BA8[18];
extern Unk800F0C10Record D_800F0CA0[18];
""")
wr(G, h)

# ---- 3. text1a_b_pre_rodata_b.c: comment only ------------------------------------------
R = 'src/text1a_b_pre_rodata_b.c'
r = rd(R)
r = sub1(r, "(func_80061064's string, func_80065800's two tables, func_8006B578's\n * compiler-generated switch table",
         "(func_80061064's string, func_80065800's two compiler-generated switch\n * tables, func_8006B578's compiler-generated switch table")
wr(R, r)

# ---- 4. symbol rows ---------------------------------------------------------------------
U = 'undefined_syms_auto.txt'
u = rd(U)
keep, gone = [], 0
for line in u.split(NL):
    m = re.match(r'^D_(800F0[0-9A-F]{3}) = 0x800F0[0-9A-F]{3};$', line)
    if m:
        a = int(m.group(1), 16)
        if (in_tmr(a) or in_rec(a)) and a not in (0x800F0BA8, 0x800F0CA0):
            gone += 1
            continue
    keep.append(line)
wr(U, NL.join(keep))
print('undefined_syms_auto.txt: retired %d rows' % gone)

N = 'named_syms.txt'
n = rd(N)
keep, gone = [], []
for line in n.split(NL):
    m = re.match(r'^(g_\w+)\s*= 0x(800F0[0-9A-F]{3});', line)
    if m:
        a = int(m.group(2), 16)
        if in_tmr(a) or in_rec(a):
            gone.append(m.group(1))
            continue
    keep.append(line)
n = NL.join(keep)
n = sub1(n, "/* Existing g_motion_ex_counter_p1 (D_800F0BC0) and _p2 (D_800F0BC4) are at offsets +0x18/+0x1C from D_800F0BA8. */\n",
         "/* The per-id 12-byte state records and 2-byte counters are the arrays D_800F0CA0[18] and D_800F0BA8[18]\n * (include/game.h; merged 2026-09-30 with func_80065800, which retired their per-word alias names). */\n")
n = sub1(n, """/* === Motion-ex state IDs 2-7 (2026-05-17) -- 18 highs === */
/* Extension of the existing motion-ex state cluster (IDs 0-1 named */
/* earlier in this naming sprint).  Each ID has 12-byte state block: */
/*   +0x0 = pos, +0x4 = extra, +0x8 = data_ptr */
/* Spanning 0x800F0CA0 (id0) through 0x800F0CFC (id7) = 24 entries x 4 bytes. */
""", "")
n = sub1(n, "/* === Motion-ex counter id15 + text1b glyph data (2026-05-17) -- 3 highs === */\n",
         "/* === text1b glyph data (2026-05-17) === */\n")
n = sub1(n, "g_motion_ex_counter_id15_plus_6                         = 0x800F0BCC;  /* +6 from g_motion_ex_counter_id15 (0x800F0BC6) */",
         "g_motion_ex_counter_id15_plus_6                         = 0x800F0BCC;  /* the halfword just past D_800F0BA8[18] */")


def prose(m):
    a = int(m.group(1), 16)
    if in_tmr(a):
        return 'D_800F0BA8[%d]' % ((a - 0x800F0BA8) // 2)
    if in_rec(a):
        off = a - 0x800F0CA0
        return 'D_800F0CA0[%d]%s' % (off // 12, '' if off % 12 == 0 else '.unk%X' % (off % 12))
    return m.group(0)


lines = n.split(NL)
for i, line in enumerate(lines):
    if '/*' in line:
        c = line.index('/*')
        body = line[c:]
        body = re.sub(r'\bD_(800F0[0-9A-F]{3})\b(?!\[)', prose, body)
        body = body.replace('(paired with g_motion_ex_counter_p2)', '(paired with D_800F0BA8[14])')
        body = body.replace('D_800F0BA8[13]/BC6', 'D_800F0BA8[13]/[15]')
        lines[i] = line[:c] + body
n = NL.join(lines)
wr(N, n)
print('named_syms.txt: retired %d rows: %s' % (len(gone), ' '.join(gone)))
