"""Chain-extender variants (dead-store-fake-exception.md 'combine-foldable chain-extender') on the
split-symbol chassis f1 (four function-scope table pointers) and a4 (alias at the loop top)."""
import os
D = 'tmp/func_800620B8/s4/'
f1 = open('tmp/func_800620B8/s/f1.c', encoding='utf-8').read()
a4 = open('tmp/func_800620B8/s2/a4.c', encoding='utf-8').read()


def sub(s, a, b, n=1):
    assert s.count(a) == n, (a, s.count(a))
    return s.replace(a, b)


out = {}
for name, src, p in (('f1', f1, 'strip32'), ('a4', a4, 'frames')):
    use = '(s32)%s[(u32)D_800A32B8 %% 6]' % p
    # E1: (x - p) + p round trip on the use
    out[name + '_e1'] = sub(src, use, '(s32)%s[(u32)D_800A32B8 %% 6] - (s32)%s + (s32)%s /* FAKE */' % (p, p, p))
    # E2: p + (x - p)
    out[name + '_e2'] = sub(src, use, '(s32)%s + ((s32)%s[(u32)D_800A32B8 %% 6] - (s32)%s) /* FAKE */' % (p, p, p))
    # E3: self-assignment right before the use
    out[name + '_e3'] = sub(src, '        sel_a:\n', '        sel_a:\n            %s = %s; /* FAKE */\n' % (p, p))
    # E4: byte-offset round trip through u8 * with a runtime offset
    out[name + '_e4'] = sub(src, use, '(s32)((u8 *)%s + ((s32)%s[(u32)D_800A32B8 %% 6] - (s32)%s)) /* FAKE */' % (p, p, p))
    # E5: index round trip: &p[n] computed as &p[n + k] - k with k a live local-free constant
    out[name + '_e5'] = sub(src, use, '(s32)(%s + ((u32)D_800A32B8 %% 6 + 1) - 1) /* FAKE */' % p)
    # E6: the SOTN four-style round trip through the pointer value
    out[name + '_e6'] = sub(src, use, '(s32)%s[(u32)D_800A32B8 %% 6] + (s32)%s - (s32)%s /* FAKE */' % (p, p, p))
# f1 only: pre-loop self-assignment and a pre-loop detour on the set
out['f1_e7'] = sub(f1, '    strip32 = D_8009BA00;\n', '    strip32 = D_8009BA00;\n    strip32 = strip32; /* FAKE */\n')
out['f1_e8'] = sub(f1, '    strip32 = D_8009BA00;\n', '    strip32 = D_8009BA00 + 1;\n    strip32 = strip32 - 1; /* FAKE */\n')
for k, v in out.items():
    open(D + k + '.c', 'w', encoding='utf-8', newline='\n').write(v)
print(','.join(D + k + '.c' for k in out))
