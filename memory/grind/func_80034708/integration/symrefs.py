"""Where is each merged per-word symbol still referenced by something that is BUILT?"""
import re, glob, os
SYMS = ['D_8010277A', 'D_8010277C', 'D_8010277D', 'D_8010277E', 'D_8010277F', 'D_80102780', 'D_80102781',
        'D_80102782', 'D_80102783', 'D_80102784', 'D_80102785', 'D_80102786', 'D_80102787',
        'g_practice_lesson_size_a', 'g_practice_lesson_size_b', 'g_practice_lesson_count_a',
        'g_practice_lesson_count_b', 'g_practice_lesson_flag_a', 'g_practice_lesson_flag_b',
        'g_practice_lesson_init_param_plus_1', 'g_practice_lesson_byte_6',
        'D_80106A54', 'D_80106A58', 'D_80106A5C', 'D_80106A70', 'D_80106A71', 'D_80106A72', 'D_80106A73',
        'g_file_disc_size', 'g_file_disc_type', 'g_file_flags', 'g_file_flags_byte',
        'g_default_color_r', 'g_default_color_g', 'g_default_color_b', 'g_file_disc_type_plus_4',
        'g_intro_demo_duration', 'D_800A3176', 'g_game_flag_c_800A3176', 'g_motion_id_current_800A3174']
INTEG = 'tmp/func_80034708/integ/src'
srcs = {}
for p in glob.glob('src/*.c'):
    q = os.path.join(INTEG, os.path.basename(p))
    srcs[p] = open(q if os.path.exists(q) else p, encoding='utf-8').read()
for p in glob.glob(INTEG + '/*.c'):
    if not os.path.exists('src/' + os.path.basename(p)):
        srcs[p] = open(p, encoding='utf-8').read()
inc_asm, inc_rod = set(), set()
for p, s in srcs.items():
    inc_asm |= set(re.findall(r'INCLUDE_ASM\("asm/funcs", (\w+)\)', s))
    inc_rod |= set(re.findall(r'INCLUDE_RODATA\("asm/rodata", (\w+)\)', s))
built_asm = [f'asm/funcs/{f}.s' for f in inc_asm] + [f'asm/rodata/{f}.s' for f in inc_rod]
built_asm += glob.glob('asm/data/*.s') + glob.glob('asm/*.s') + ['asm/funcs/save_vc_ctrl.s']
texts = {p: open(p, encoding='utf-8', errors='replace').read() for p in built_asm if os.path.exists(p)}
cfg = {p: open(p, encoding='utf-8').read() for p in
       ['undefined_syms_auto.txt', 'named_syms.txt', 'symbol_addrs.txt', 'sdata_syms.txt', 'undefined_funcs_auto.txt']}
for sym in SYMS:
    rx = re.compile(r'\b%s\b' % sym)
    c_refs = [p for p, s in srcs.items() if rx.search(re.sub(r'/\*.*?\*/', '', s, flags=re.S))]
    a_refs = [p for p, s in texts.items() if rx.search(s)]
    rows = [(p, [l for l in s.split('\n') if rx.search(l)]) for p, s in cfg.items() if rx.search(s)]
    print(f'{sym:40s} C={c_refs} ASM={a_refs}')
    for p, ls in rows:
        for l in ls:
            print(f'      {p}: {l.strip()[:110]}')
