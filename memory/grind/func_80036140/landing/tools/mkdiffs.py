"""Per-commit diffs of the five-commit landing, from the scratch states (t_steps.sh), for the reviewer.
usage (WSL, repo root): python3 tmp/func_80036140/mkdiffs.py <rev>"""
import difflib, subprocess, sys
from pathlib import Path

REV = sys.argv[1]
H = Path('tmp/func_80036140')
OUT = Path('memory/grind/func_80036140/landing/steps')
FILES = {
    'A_maspsx_gate': ['tools/maspsx/maspsx/__init__.py', 'tools/maspsx/maspsx.py', 'tools/maspsx/tests/test_comm_syms.py',
                      'maspsx_comm_syms.txt', 'Makefile', 'engine/buildconfig.py', 'engine/cheats.py', 'engine/oracle.py',
                      'engine/dossier.py', 'engine/buildstamp.py', 'engine/queue.py', 'engine/test_engine.py',
                      'tools/check_root_cleanliness.py', 'tools/desync_audit.py', 'tools/naming_wave.py',
                      'tools/libscan/manifest.py', 'tools/grinder/grindlib.py', 'tools/hooks/park_src_guard.py',
                      'tools/hooks/main_reintegration_lock.py', 'tools/hooks/worktree_contamination_guard.py',
                      '.claude/rules/integration-handoff-self-serve.md', '.claude/rules/maspsx-gate-lists.md'],
    'B_split': ['src/code6cac_b2_post.c', 'src/code6cac_b4.c', 'src/code6cac_b4_post.c', 'Makefile',
                'engine/buildconfig.py', 'bb2.ld'],
    'C_merges': ['include/code6cac.h', 'src/code6cac_b4.c', 'src/code6cac_b4_post.c', 'undefined_syms_auto.txt',
                 'named_syms.txt', 'asm/data/91C98.data.s'],
    'D_match': ['src/code6cac_b4_post.c', 'src/code6cac_b5.c', 'src/code6cac_b5_post.c', 'Makefile',
                'engine/buildconfig.py', 'bb2.ld', 'undefined_syms_auto.txt', 'named_syms.txt',
                '.claude/rules/maspsx-gate-lists.md'],
    'E_cd_result': ['include/code6cac.h', 'src/code6cac_b5.c', 'undefined_syms_auto.txt', 'named_syms.txt',
                    'asm/data/91C98.data.s'],
}
STATES = [None, 'stA', 'stB', 'stC', 'stD', 'stE']


def content(state, rel):
    if state is None:
        r = subprocess.run(['git', 'show', f'{REV}:{rel}'], capture_output=True, text=True)
        return r.stdout if r.returncode == 0 else ''
    p = H / state / rel
    return p.read_text() if p.exists() else ''


OUT.mkdir(parents=True, exist_ok=True)
for k, (name, files) in enumerate(FILES.items()):
    a, b = STATES[k], STATES[k + 1]
    out, changed = [], []
    for rel in sorted(set(f for fs in FILES.values() for f in fs)):
        x, y = content(a, rel), content(b, rel)
        if x != y:
            changed.append(rel)
            out += list(difflib.unified_diff(x.splitlines(True), y.splitlines(True), f'a/{rel}', f'b/{rel}'))
    extra = sorted(set(changed) - set(files))
    missing = sorted(set(files) - set(changed))
    (OUT / f'{name}.diff').write_text(''.join(out))
    print(f'{name}: {len(changed)} files changed; declared-but-unchanged {missing}; changed-but-undeclared {extra}')
