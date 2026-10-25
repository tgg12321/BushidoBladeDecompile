#!/usr/bin/env python3
"""Q132 stub-and-observe batches. Observations only; never assign names/roles.

Liveness fallback: no general game RAM/VSync counter was established. Require
script_ended (runkit.py:14-16: frames count GPU::Vsync from power-on), an ok
outcome, a called/intact stub without leaks, and a later screenshot. Reaching
end_frame beyond every requested frame supplies the permitted fallback; this
does not prove that game logic progressed or attribute a difference to a callee.
"""
from __future__ import annotations

import argparse
from concurrent.futures import ThreadPoolExecutor
import contextlib
import csv
import io
import json
import math
from pathlib import Path
import re
import subprocess
import sys

import gdbtrace
import runkit
sys.path.insert(0, str(Path(__file__).resolve().parents[2]))
from tools.trace import store
from scenarios import scen

REPO = Path(__file__).resolve().parents[2]
FIELDS = ['func', 'ret', 'called', 'leak', 'vsync_live', 'reason', 'status',
          'frame', 'changed', 'total', 'bbox', 'baseline_png', 'stub_png',
          'baseline_sha1', 'stub_sha1']


def write_json(path, obj):
    Path(path).write_text(json.dumps(obj, indent=2) + '\n', encoding='utf-8',
                          newline='\n')


def read_json(path):
    return json.loads(Path(path).read_text(encoding='utf-8'))


def auto_ret(func, src=None):
    """Conservative source declarations only; unknown/aggregate returns refuse."""
    types = set()
    for path in sorted(Path(src or REPO / 'src').rglob('*.c')):
        text = path.read_text(encoding='utf-8')
        text = re.sub(r'/\*.*?\*/|//[^\n]*', '', text, flags=re.S)
        pat = (r'^\s*((?:(?:[A-Za-z_]\w*)\s+)+\**\s*)' + re.escape(func)
               + r'\s*\([^;{}]*\)\s*(?:\{|;)')
        for match in re.finditer(pat, text, re.M):
            typ = re.sub(r'\b(extern|static|inline|const|volatile)\b', '',
                         match[1]).strip()
            typ = ' '.join(typ.split())
            if typ in {'return', 'else', 'goto'}:
                continue
            if typ == 'void':
                types.add('none')
            elif '*' in typ or re.fullmatch(
                    r'(?:[su](?:8|16|32)|u?int(?:8|16|32)_t|size_t|'
                    r'(?:(?:unsigned|signed) )?(?:int|short|char|long)|'
                    r'(?:(?:unsigned|signed) )?(?:short|long) int|'
                    r'unsigned|signed)', typ):
                types.add('0')
            else:
                types.add('unknown')
    if len(types) != 1 or 'unknown' in types or not types:
        raise ValueError(f'{func}: cannot read an unambiguous void/integer/pointer '
                         'C prototype in src; give --ret explicitly')
    return None if types == {'none'} else 0


def run_capture(directory, spec, passthrough):
    """Separate process per run: gdbtrace --headless owns its pool slot."""
    directory.mkdir()
    write_json(directory / 'spec.json', spec)
    cmd = [sys.executable, str(REPO / 'tools/trace/gdbtrace.py'), 'run',
           '--spec', str(directory / 'spec.json'), '--out',
           str(directory / 'capture.jsonl'), '--launch', 'pcsx-redux',
           '--headless', *passthrough]
    with (directory / 'run.log').open('w', encoding='utf-8', newline='\n') as log:
        try:
            return subprocess.run(cmd, stdout=log, stderr=subprocess.STDOUT).returncode
        except OSError as exc:
            log.write(f'launch failed: {exc}\n')
            return 1


def capture_state(directory, func):
    man = read_json(directory / 'capture.manifest.json')
    result = man.get('result', {})
    try:
        events = [json.loads(x) for x in
                  store.resolve(directory, 'capture.jsonl', man).read_text(encoding='utf-8').splitlines()]
    except store.StoreError as exc:
        return False, False, [store.problem(exc, directory / 'capture.jsonl')]
    stubs = [s for s in result.get('stubs', []) if s.get('at') == func]
    called = bool(stubs) and any(e.get('ev') == 'hit' and e.get('stub_call')
                                and e.get('addr') == stubs[0]['addr']
                                for e in events)
    leak = bool(result.get('stub_leaks') or any(e.get('stub_leak') for e in events))
    reasons = []
    if result.get('outcome') != 'ok':
        reasons.append('outcome not ok')
    if not result.get('script_ended'):
        reasons.append('script not ended')
    if not called:
        reasons.append('stub never called')
    if leak:
        reasons.append('stub leak')
    if len(stubs) != 1 or not (stubs[0].get('applied') or {}).get('ok') \
            or (stubs[0].get('applied') or {}).get('mid_stub') \
            or stubs[0].get('at_end') != 'intact':
        reasons.append('stub not applied whole and intact')
    return called, leak, reasons


def summarize(out, funcs, returns, frames, scenario, codes, baseline_problem=None):
    rows, notes = [], ['# Knockout observations', '',
                       'Liveness uses the script end_frame fallback (no RAM '
                       'counter established). No names or roles are inferred.', '']
    base = out / 'baseline'
    for func in funcs:
        other = out / func
        notes += [f'## {func}', '']
        try:
            if baseline_problem:
                raise ValueError(f'baseline failed: {baseline_problem}; stub run not launched')
            called, leak, reasons = capture_state(other, func)
            if codes[func] != 0:
                reasons.append(f'run exit {codes[func]}')
            script = read_json(other / 'spec.json')['script']
            try:
                live = store.resolve(other, 'capture.knockout.live.png')
            except store.StoreMissing:
                live = other / 'capture.knockout.live.png'
            except store.StoreUnverified as exc:
                reasons.append(store.problem(exc, other / 'capture.knockout.live.png'))
                live = other / 'capture.knockout.live.png'
            if not live.is_file() or script['end_frame'] <= max(frames):
                reasons.append('no later liveness screenshot/end_frame')
            # compare prints to a buffer so it creates no unrecorded diff PNGs.
            buf = io.StringIO()
            with contextlib.redirect_stdout(buf):
                rc = gdbtrace.compare(argparse.Namespace(
                    base=str(base / 'capture.jsonl'),
                    other=str(other / 'capture.jsonl'),
                    base_spec=str(base / 'spec.json'),
                    other_spec=str(other / 'spec.json'),
                    context=scenario, out=None))
            text = buf.getvalue()
            comparable = rc == 0 and codes['baseline'] == 0
            alive = not reasons
            status = 'not_alive' if not alive else ('ok' if comparable else 'inconclusive')
            reason = '; '.join(reasons) if reasons else (
                f"script reached end_frame {script['end_frame']} past requested "
                f"frames {','.join(map(str, frames))} (fallback)")
            if not comparable:
                reason += '; NOT COMPARABLE (see knockout.md)'
            notes += [f'Status: **{status}**. {reason}.', '']
            if status != 'ok':
                # Keep compare diagnostics, withhold its pixel table as evidence.
                text = text.split('| screenshot |')[0] + '\nDiff withheld as evidence.\n'
            notes += [text]
        except (OSError, ValueError, KeyError) as exc:
            called, leak, alive, status = False, False, False, 'not_alive'
            detail = store.problem(exc, other) if isinstance(exc, store.StoreError) else str(exc)
            reason = f'incomplete capture: {detail}'
            for directory in (base, other):
                if not (directory / 'capture.manifest.json').is_file():
                    reason += f'; no capture manifest: {directory.name}'
            notes += [reason, '', 'Diff withheld as evidence.', '']
        for frame in frames:
            a = base / f'capture.knockout.{frame}.png'
            b = other / f'capture.knockout.{frame}.png'
            row = dict(func=func, ret='none' if returns[func] is None else returns[func],
                       called=int(called), leak=int(leak), vsync_live=int(alive),
                       reason=reason, status=status, frame=frame,
                       baseline_png=a.relative_to(out).as_posix(),
                       stub_png=b.relative_to(out).as_posix())
            for key, path in [('baseline_sha1', a), ('stub_sha1', b)]:
                try:
                    row[key] = runkit.sha1_file(str(store.capture_path(path)))
                except store.StoreMissing:
                    row[key] = ''
                except store.StoreError as exc:
                    row[key] = ''
                    row.update(status='inconclusive' if row['status'] == 'ok' else row['status'],
                               reason=f"{row['reason']}; {store.problem(exc, path)}")
            if row['status'] == 'ok':
                try:
                    res, err = gdbtrace.diff_shot(str(a), str(b), None)
                except (OSError, ValueError) as exc:
                    res, err = None, str(exc)
                if err:
                    row.update(status='inconclusive', reason=err)
                    notes += [f'Frame {frame}: inconclusive ({err}). Diff withheld.', '']
                else:
                    n, total, box = res
                    row.update(changed=n, total=total, bbox=json.dumps(box))
            rows.append(row)
    with (out / 'knockout.tsv').open('w', encoding='utf-8', newline='') as f:
        writer = csv.DictWriter(f, fieldnames=FIELDS, delimiter='\t', lineterminator='\n')
        writer.writeheader()
        writer.writerows(rows)
    (out / 'knockout.md').write_text('\n'.join(notes), encoding='utf-8', newline='\n')
    # Seal capture manifests and the TSV too, so neither can silently be edited.
    inventory = {p.relative_to(out).as_posix(): runkit.sha1_file(str(p))
                 for p in sorted(out.rglob('*')) if p.is_file()}
    write_json(out / 'knockout.manifest.json', inventory)
    return 0 if all(r['status'] == 'ok' for r in rows) else 1


def run_batch(args, passthrough):
    for flag in ('emulator', 'cue', 'bios'):
        value = getattr(args, flag)
        if not value or not Path(value).is_file():
            raise ValueError(f'--{flag} must be given and name an existing file ({value!r})')
    raw = args.funcs if args.funcs is not None else Path(args.funcs_file).read_text(encoding='utf-8')
    funcs = re.split(r'[,\s]+', raw.strip())
    if not funcs or any(not re.fullmatch(r'[A-Za-z_]\w*', f) or f.casefold() == 'baseline' for f in funcs):
        raise ValueError('funcs must be C identifiers (excluding baseline)')
    if len({f.casefold() for f in funcs}) != len(funcs):
        raise ValueError('function names must be unique case-insensitively (output directories collide)')
    frames = sorted(set(int(n) for n in args.frames.split(',')))
    if not frames or frames[0] < 1 or args.jobs < 1:
        raise ValueError('frames and jobs must be positive')
    returns = {f: auto_ret(f) if args.ret == 'auto' else
               (None if args.ret == 'none' else int(args.ret, 0)) for f in funcs}
    for f in funcs:
        gdbtrace.stub_words({'at': f, 'ret': returns[f]})
    script = scen.compile_script(args.scenario)
    if args.phase and args.phase not in {p['name'] for p in script['phases']}:
        raise ValueError(f'unknown phase {args.phase}')
    if args.phase and args.phase == runkit.first_phase(script):
        raise ValueError('cannot stub at the first phase; omit --phase to stub at EXE entry')
    if args.phase and next(p['frame'] for p in script['phases'] if p['name'] == args.phase) >= frames[0]:
        raise ValueError('requested frames must be strictly after the stub phase start frame')
    script = scen.knockout_script(script, frames)
    runkit.check_script(script)
    duration = float(args.duration) if args.duration is not None else 7200
    if not math.isfinite(duration) or duration <= 0:
        raise ValueError('--duration must be finite and positive')
    out = Path(args.out).resolve()
    if out.exists() and any(out.iterdir()):
        raise ValueError('output directory must be empty (never mix captures)')
    out.mkdir(parents=True, exist_ok=True)
    spec = dict(duration_s=duration, max_events=200000, script=script)
    codes = {'baseline': run_capture(out / 'baseline', spec, passthrough)}
    problems = []
    if codes['baseline'] != 0:
        problems.append(f"exit {codes['baseline']}")
    try:
        baseline = read_json(out / 'baseline/capture.manifest.json')
        if not baseline.get('q124_complete'):
            problems.append(f"not q124_complete: {baseline.get('q124_missing', [])}")
        if not baseline.get('result', {}).get('script_ended'):
            problems.append('script not ended')
    except (OSError, ValueError) as exc:
        problems.append(f'no readable baseline manifest: {exc}')
    if problems:
        return summarize(out, funcs, returns, frames, args.scenario, codes,
                         baseline_problem='; '.join(problems))

    def stub_run(func):
        stub = dict(at=func, ret=returns[func])
        if args.phase:
            stub['phase'] = args.phase
        return run_capture(out / func, dict(spec, stubs=[stub]), passthrough)

    with ThreadPoolExecutor(max_workers=args.jobs) as pool:
        codes.update(zip(funcs, pool.map(stub_run, funcs)))
    return summarize(out, funcs, returns, frames, args.scenario, codes)


def check(out):
    out = Path(out).resolve()
    errors = []
    inventory = read_json(out / 'knockout.manifest.json')
    with (out / 'knockout.tsv').open(encoding='utf-8', newline='') as f:
        rows = list(csv.DictReader(f, delimiter='\t'))

    def recorded_absence(path, allowed):
        if not allowed:
            errors.append(f'absence not permitted by records: {path}')
        try:
            present = store.capture_path(path).exists() if store.is_bulk(path.name) else path.exists()
        except store.StoreMissing:
            present = False
        except store.StoreError as exc:
            errors.append(str(exc))
            present = True
        if present:
            errors.append(f'recorded missing but present: {path}')

    history = store.LegacyRecords()

    def never_published(path, legacy):
        # A pre-migration run's file that no commit ever carried (gitignored
        # run.log, uncopied .mcd / .script.log) is reported, not lost.
        try:
            if store.ever_committed(path) or not legacy():
                return False
        except (OSError, ValueError, store.StoreError):
            return False
        print(f'never published (legacy): {path}')
        return True

    def verify(path, sha, legacy=lambda: False):
        if store.is_bulk(path.name):
            try:
                path = store.capture_path(path)
            except store.StoreMissing as exc:
                if not never_published(path, legacy):
                    errors.append(str(exc))
                return
            except ValueError as exc:
                errors.append(str(exc))
                return
        if not path.is_file():
            if not never_published(path, legacy):
                errors.append(f'hash mismatch/missing: {path}')
        elif path.name.endswith('.manifest.json'):
            # Pre-migration inventories cite the manifest before its `stored` flags.
            if not store.manifest_cited(path, sha):
                errors.append(f'hash mismatch/missing: {path}')
        elif runkit.sha1_file(str(path)) != sha:
            errors.append(f'hash mismatch/missing: {path}')

    for rel, sha in inventory.items():
        path = (out / rel).resolve()
        if not path.is_relative_to(out):
            raise ValueError('inventory path escapes output directory')
        verify(path, sha, lambda rel=rel, sha=sha: store.inventoried_before_migration(
            out / 'knockout.manifest.json', rel, sha))
    directories = {'baseline', *(r['func'] for r in rows),
                   *(p.name for p in out.iterdir() if p.is_dir())}
    for name in sorted(directories):
        directory = out / name
        affected = rows if name == 'baseline' else [r for r in rows if r['func'] == name]
        manifest = directory / 'capture.manifest.json'
        if not manifest.is_file():
            marker = f'no capture manifest: {name}'
            recorded_absence(manifest, bool(affected) and all(
                r['status'] == 'not_alive' and marker in r['reason'].split('; ')
                for r in affected))
            continue
        man = read_json(manifest)
        outputs = {Path(r['path'].replace('\\', '/')).name: r for r in man.get('outputs', [])}
        for name, rec in outputs.items():
            if rec.get('sha1') and not rec.get('missing'):
                verify(directory / name, rec['sha1'],
                       lambda rec=rec: history.qualifies(manifest, rec))
            else:
                recorded_absence(directory / name, rec.get('missing') is True
                                 and not rec.get('sha1') and bool(affected)
                                 and all(r['status'] != 'ok' for r in affected))
        for path in [*directory.glob('capture.jsonl'), *directory.glob('*.png')]:
            if path.name not in outputs:
                errors.append(f'not in capture manifest: {path}')
        verify(directory / 'spec.json', man['spec']['sha1'])
    for row in rows:
        for prefix in ('baseline', 'stub'):
            path = (out / row[prefix + '_png']).resolve()
            if not path.is_relative_to(out):
                raise ValueError('TSV path escapes output directory')
            sha = row[prefix + '_sha1']
            if sha:
                verify(path, sha)
            else:
                recorded_absence(path, row['status'] != 'ok')
    # Catch new PNGs/captures as well as edits to inventoried files.
    for path in out.rglob('*'):
        if path.is_file() and path.name != 'knockout.manifest.json' and path.relative_to(out).as_posix() not in inventory:
            errors.append(f'unrecorded file: {path}')
    for error in errors:
        print(error, file=sys.stderr)
    return int(bool(errors))


def main(argv=None):
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    sub = ap.add_subparsers(dest='command', required=True)
    p = sub.add_parser('run')
    p.add_argument('--scenario', required=True)
    group = p.add_mutually_exclusive_group(required=True)
    group.add_argument('--funcs')
    group.add_argument('--funcs-file')
    p.add_argument('--frames', required=True)
    p.add_argument('--out', required=True)
    p.add_argument('--phase')
    p.add_argument('--ret', default='auto')
    p.add_argument('--jobs', type=int, default=1)
    # Pass through launch inputs/settings; pool chooses port/profile per run.
    for flag in ('emulator', 'bios', 'cue', 'map', 'elf', 'memcard1', 'memcard2',
                 'portable-dir', 'host', 'duration', 'connect-timeout', 'stall', 'slot-wait'):
        p.add_argument('--' + flag)
    p = sub.add_parser('check')
    p.add_argument('out')
    args = ap.parse_args(argv)
    try:
        if args.command == 'check':
            return check(args.out)
        passthrough = []
        for flag in ('emulator', 'bios', 'cue', 'map', 'elf', 'memcard1', 'memcard2',
                     'portable-dir', 'host', 'connect-timeout', 'stall', 'slot-wait'):
            value = getattr(args, flag.replace('-', '_'))
            if value is not None:
                passthrough.extend(['--' + flag, value])
        return run_batch(args, passthrough)
    except (OSError, ValueError, KeyError, scen.ScenError) as exc:
        print(f'knockout: {exc}', file=sys.stderr)
        return 1


if __name__ == '__main__':
    sys.exit(main())
