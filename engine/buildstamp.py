"""Content-bound evidence that a full build used the current inputs.

Only pipeline.build_all creates this receipt, after successful compilation and
an unchanged-input check. Neither an old matching executable nor oracle-lock
can certify new source. Partial/manual builds must be followed by a full verify.
"""
from __future__ import annotations

import hashlib
import json
import os
from pathlib import Path
import shutil
import sys

from . import buildconfig as cfg

STAMP = Path('build/verified-inputs.json')
CONFIGS = (
    'Makefile', 'bb2.ld', 'bb2.sha1', 'sdata_syms.txt', 'sdata_funcs.txt',
    'sdata_exclude.txt', 'expand_lb_funcs.txt', 'expand_dest_funcs.txt',
    'multu_funcs.txt', 'multu_pad_funcs.txt', 'maspsx_prefill_label_funcs.txt',
    'undefined_funcs_auto.txt', 'undefined_syms_auto.txt', 'named_syms.txt',
    'tools/prologue_config.json', 'delay_slot_ra_funcs.txt', 'frame_fix_funcs.txt',
    'tools/prologue_fix.py', 'tools/multu_pad.py', 'tools/make_psexe.py',
    'engine/pipeline.py', 'engine/buildconfig.py', 'engine/buildstamp.py',
    cfg.TARGET_EXE, cfg.CC1,
)


def digest(path: Path) -> str:
    with path.open('rb') as f:
        return hashlib.file_digest(f, 'sha256').hexdigest()


def inputs() -> dict[str, str | None]:
    paths = {Path(p) for p in CONFIGS}
    for directory, patterns in [('src', ('*.c', '*.h')), ('include', ('*',)),
                                ('asm', ('*.s',)), ('tools/maspsx', ('*.py',))]:
        for pattern in patterns:
            paths.update(p for p in Path(directory).rglob(pattern) if p.is_file())
    result = {p.as_posix(): digest(p) if p.is_file() else None
              for p in sorted(paths)}
    # Identify the actual executables, including tools outside the repository.
    for command in (cfg.CPP, cfg.AS, cfg.LD, cfg.OBJCOPY, 'bash', 'sed', 'python3'):
        resolved = shutil.which(command)
        result['tool:' + command] = digest(Path(resolved)) if resolved else None
    result['tool:python-runtime'] = digest(Path(sys.executable))
    return result


def artifacts() -> dict[str, str]:
    required = [Path('build/bb2.exe'), Path('build/bb2.elf'), Path('build/bb2.bin')]
    required += [Path('build') / p.with_suffix('.o') for p in Path('src').glob('*.c')]
    required += [Path('build') / p.with_suffix('.o') for d in ('asm', 'asm/data')
                 for p in Path(d).glob('*.s')]
    required += [Path(f'build/asm/funcs/{name}.o') for name in cfg.LINKED_ASM_FUNCS]
    return {p.as_posix(): digest(p) for p in sorted(required)}


def invalidate() -> None:
    STAMP.unlink(missing_ok=True)


def record(before: dict) -> None:
    after = inputs()
    if before != after:
        raise RuntimeError('Build inputs changed during compilation; refusing attestation')
    data = {'schema': 1, 'inputs': after, 'artifacts': artifacts()}
    STAMP.parent.mkdir(parents=True, exist_ok=True)
    temp = STAMP.with_suffix('.tmp')
    temp.write_text(json.dumps(data, indent=2) + '\n', encoding='utf-8')
    os.replace(temp, STAMP)


def check() -> dict:
    try:
        data = json.loads(STAMP.read_text(encoding='utf-8'))
        if data.get('schema') != 1:
            return {'fresh': False, 'reason': 'unknown build attestation schema'}
        current = inputs()
        expected = data['inputs']
        changed = sorted(k for k in expected.keys() | current.keys()
                         if expected.get(k) != current.get(k))
        if changed:
            return {'fresh': False, 'reason': 'build inputs changed', 'changed': changed}
        if data['artifacts'] != artifacts():
            return {'fresh': False, 'reason': 'build artifacts changed'}
        return {'fresh': True}
    except (OSError, ValueError, KeyError, TypeError, AttributeError) as exc:
        return {'fresh': False, 'reason': f'missing/invalid build attestation or artifact: {exc}'}


def _main() -> int:
    if len(sys.argv) != 2 or sys.argv[1] not in {'record-current', 'check'}:
        print('usage: python3 -m engine.buildstamp {record-current|check}', file=sys.stderr)
        return 2
    if sys.argv[1] == 'record-current':
        record(inputs())
        result = check()
    else:
        result = check()
    print(json.dumps(result, sort_keys=True))
    return 0 if result['fresh'] else 1


if __name__ == '__main__':
    raise SystemExit(_main())
