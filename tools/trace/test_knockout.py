#!/usr/bin/env python3
"""Emulator-free knockout regression tests; every write stays in a temp dir."""
import argparse
import contextlib
import csv
import io
import json
from pathlib import Path
import sys
import shutil
import threading
import time
import unittest
import uuid
from unittest.mock import patch

sys.dont_write_bytecode = True
sys.path.insert(0, str(Path(__file__).resolve().parent))
import knockout as k


class KnockoutTest(unittest.TestCase):
    def setUp(self):
        # Python 3.12's Windows mkdtemp(mode=0700) ACL excludes the sandbox
        # identity. Use an ordinary mkdir in a unique workspace temp directory.
        self.root = k.REPO / 'tmp' / ('knockout-test-' + uuid.uuid4().hex)
        self.root.mkdir(parents=True)
        self.addCleanup(shutil.rmtree, self.root)
        self.out = self.root / 'out'
        self.behavior = {}
        self.active = self.peak = 0
        self.lock = threading.Lock()
        self.runs = []
        self.launch_args = []
        for flag in ('emulator', 'cue', 'bios'):
            path = self.root / flag
            path.write_bytes(b'fake input')
            self.launch_args.extend(['--' + flag, str(path)])

    def fake_run(self, directory, spec, passthrough):
        with self.lock:
            self.active += 1
            self.peak = max(self.peak, self.active)
            self.runs.append((directory.name, spec, passthrough))
        try:
            time.sleep(.02)
            directory.mkdir()
            k.write_json(directory / 'spec.json', spec)
            func = directory.name
            mode = self.behavior.get(func, '')
            if mode == 'no_manifest':
                return 1
            events, stubs = [], []
            if 'stubs' in spec:
                st = spec['stubs'][0]
                stubs = [dict(at=func, addr=0x80010000, ret=st['ret'],
                              when=st.get('phase', 'entry'), at_end='intact',
                              applied=dict(ok=True, frame=0))]
                if mode != 'uncalled':
                    events.append(dict(ev='hit', addr=0x80010000,
                                       phase='menu', stub_call=True))
                if mode == 'leak':
                    events.append(dict(ev='hit', addr=0x80010008, stub_leak=True))
            capture = directory / 'capture.jsonl'
            capture.write_text(''.join(json.dumps(e) + '\n' for e in events), encoding='utf-8')
            outputs = [capture]
            for shot in spec['script']['screenshots']:
                image = directory / f"capture.{shot['name']}.png"
                outputs.append(image)
                if mode == 'hang' or (mode == 'missing_shot' and shot['name'] == 'knockout.100'):
                    continue
                pixels = bytearray(12)
                if mode == 'changed':
                    pixels[9:12] = b'\xff\x00\x00'
                k.runkit.png(str(image), 2, 2, bytes(pixels))
            man = dict(q124_complete=mode not in ('hang', 'leak', 'incomplete'),
                       spec=k.runkit.file_rec(str(directory / 'spec.json')),
                       outputs=[k.runkit.file_rec(str(p)) for p in outputs],
                       bios={'sha1': 'different' if mode == 'incomparable' else 'same'},
                       result=dict(outcome='ok', script_ended=mode != 'hang',
                                   stubs=stubs, stub_leaks=int(mode == 'leak')))
            k.write_json(directory / 'capture.manifest.json', man)
            return 1 if mode == 'bad_exit' else 0
        finally:
            with self.lock:
                self.active -= 1

    def batch(self, funcs=('unchanged', 'changed', 'third'), extra=()):
        script = dict(end_frame=200, inputs=[], phases=[dict(frame=0, name='boot'),
                                                       dict(frame=50, name='menu')],
                      screenshots=[])
        with patch.object(k.scen, 'compile_script', return_value=script), \
                patch.object(k, 'run_capture', side_effect=self.fake_run):
            rc = k.main(['run', '--scenario', 'fake', '--funcs', ','.join(funcs),
                         '--frames', '100,120', '--out', str(self.out),
                         '--jobs', '2', '--ret', 'none', *self.launch_args, *extra])
        with (self.out / 'knockout.tsv').open(encoding='utf-8', newline='') as f:
            rows = list(csv.DictReader(f, delimiter='\t'))
        return rc, rows

    def test_three_jobs_two_and_pixel_observations(self):
        self.behavior['changed'] = 'changed'
        rc, rows = self.batch()
        self.assertEqual(rc, 0)
        self.assertEqual(self.peak, 2)
        self.assertEqual([r[0] for r in self.runs].count('baseline'), 1)
        self.assertEqual(len(self.runs), 4)
        self.assertEqual(len(rows), 6)
        self.assertEqual(rows[0]['changed'], '0')
        self.assertEqual(rows[2]['changed'], '1')
        self.assertEqual(rows[2]['total'], '4')
        self.assertEqual(json.loads(rows[2]['bbox']), [1, 1, 1, 1])
        scripts = [r[1]['script'] for r in self.runs]
        self.assertTrue(all(s == scripts[0] for s in scripts))
        self.assertGreater(scripts[0]['screenshots'][-1]['frame'], 120)
        self.assertEqual(k.check(self.out), 0)

    def test_hang_not_alive(self):
        self.behavior['hang'] = 'hang'
        rc, rows = self.batch(('hang',))
        self.assertEqual(rc, 1)
        self.assertEqual(rows[0]['status'], 'not_alive')
        self.assertIn('script not ended', rows[0]['reason'])
        self.assertEqual(rows[0]['changed'], '')
        self.assertNotIn('| screenshot |', (self.out / 'knockout.md').read_text())
        self.assertEqual(k.check(self.out), 0)

    def test_stub_never_called(self):
        self.behavior['uncalled'] = 'uncalled'
        _, rows = self.batch(('uncalled',))
        self.assertEqual(rows[0]['called'], '0')
        self.assertEqual(rows[0]['status'], 'not_alive')
        self.assertIn('never called', rows[0]['reason'])

    def test_leak(self):
        self.behavior['leak'] = 'leak'
        _, rows = self.batch(('leak',))
        self.assertEqual(rows[0]['leak'], '1')
        self.assertEqual(rows[0]['vsync_live'], '0')
        self.assertEqual(rows[0]['changed'], '')

    def test_not_comparable(self):
        self.behavior['mismatch'] = 'incomparable'
        rc, rows = self.batch(('mismatch',))
        self.assertEqual(rc, 1)
        self.assertEqual(rows[0]['status'], 'inconclusive')
        self.assertEqual(rows[0]['changed'], '')
        self.assertIn('NOT COMPARABLE: different bios', (self.out / 'knockout.md').read_text())

    def test_auto_prototypes(self):
        src = self.root / 'src'
        src.mkdir()
        (src / 'test.c').write_text('void a(int x) {}\ns32 b(void);\n'
                                  'Thing *c(void) {}\nfloat d(void);\n'
                                  'INCLUDE_ASM("asm/funcs", missing);\n'
                                  's32 wrapper(void) {\n return b();\n}\n', encoding='utf-8')
        self.assertIsNone(k.auto_ret('a', src))
        self.assertEqual(k.auto_ret('b', src), 0)
        self.assertEqual(k.auto_ret('c', src), 0)
        for func in ('missing', 'd'):
            with self.assertRaises(ValueError):
                k.auto_ret(func, src)

    def test_auto_refuses_before_launch(self):
        with patch.object(k, 'auto_ret', side_effect=ValueError('unreadable')), \
                patch.object(k, 'run_capture') as run, contextlib.redirect_stderr(io.StringIO()):
            rc = k.main(['run', '--scenario', 'fake', '--funcs', 'missing',
                         '--frames', '100', '--out', str(self.out), *self.launch_args])
        self.assertEqual(rc, 1)
        run.assert_not_called()
        self.assertFalse(self.out.exists())

    def test_modified_png(self):
        self.batch(('one',))
        image = self.out / 'one/capture.knockout.100.png'
        k.runkit.png(str(image), 1, 1, b'\xff\x00\x00')
        with contextlib.redirect_stderr(io.StringIO()):
            self.assertEqual(k.main(['check', str(self.out)]), 1)

    def test_modified_capture_manifest_and_tsv(self):
        self.batch(('one',))
        for rel in ('one/capture.jsonl', 'one/capture.manifest.json', 'knockout.tsv'):
            p = self.out / rel
            original = p.read_bytes()
            p.write_bytes(original + b'\n')
            with contextlib.redirect_stderr(io.StringIO()):
                self.assertEqual(k.main(['check', str(self.out)]), 1)
            p.write_bytes(original)
        self.assertEqual(k.check(self.out), 0)

    def test_phase_and_passthrough(self):
        _, rows = self.batch(('one',), ('--phase', 'menu', '--ret', '0',
                                        '--slot-wait', '12', '--duration', '42'))
        self.assertEqual(self.runs[1][1]['stubs'], [dict(at='one', phase='menu', ret=0)])
        self.assertEqual(self.runs[1][2], [*self.launch_args[:2],
                                         *self.launch_args[4:6],
                                         *self.launch_args[2:4], '--slot-wait', '12'])
        self.assertEqual(self.runs[1][1]['duration_s'], 42)
        self.assertNotIn('--duration', self.runs[1][2])
        self.assertEqual(rows[0]['ret'], '0')
        self.assertIn('end_frame 200 past requested frames 100,120', rows[0]['reason'])

    def test_run_command_headless(self):
        with patch.object(k.subprocess, 'run') as run:
            run.return_value.returncode = 0
            k.run_capture(self.root / 'capture', {'script': {}}, ['--bios', 'b'])
        cmd = run.call_args.args[0]
        self.assertIn('--headless', cmd)
        self.assertEqual(cmd[cmd.index('--launch') + 1], 'pcsx-redux')

    def test_missing_requested_shot(self):
        self.behavior['missing'] = 'missing_shot'
        rc, rows = self.batch(('missing',))
        self.assertEqual(rc, 1)
        self.assertEqual(rows[0]['status'], 'inconclusive')
        self.assertEqual(rows[0]['changed'], '')
        self.assertEqual(k.check(self.out), 0)

    def test_funcs_file_auto_and_launch_failure(self):
        funcs = self.root / 'funcs.txt'
        funcs.write_text('a\nb\n', encoding='utf-8')
        script = dict(end_frame=150, inputs=[], phases=[], screenshots=[])
        with patch.object(k.scen, 'compile_script', return_value=script), \
                patch.object(k, 'auto_ret', side_effect=[None, 0]), \
                patch.object(k, 'run_capture', side_effect=self.fake_run):
            rc = k.main(['run', '--scenario', 'fake', '--funcs-file', str(funcs),
                         '--frames', '100', '--out', str(self.out), '--jobs', '2',
                         *self.launch_args])
        self.assertEqual(rc, 0)
        self.assertEqual([r[1]['stubs'][0]['ret'] for r in self.runs[1:]], [None, 0])
        with patch.object(k.subprocess, 'run', side_effect=OSError('missing exe')):
            self.assertEqual(k.run_capture(self.root / 'failed', {}, []), 1)

    def test_no_manifest_batch_check(self):
        self.behavior['one'] = 'no_manifest'
        rc, rows = self.batch(('one',))
        self.assertEqual(rc, 1)
        self.assertEqual(rows[0]['status'], 'not_alive')
        self.assertIn('no capture manifest: one', rows[0]['reason'])
        self.assertEqual(k.check(self.out), 0)

    def test_deleted_recorded_png(self):
        self.batch(('one',))
        (self.out / 'one/capture.knockout.100.png').unlink()
        with contextlib.redirect_stderr(io.StringIO()):
            self.assertEqual(k.check(self.out), 1)

    def test_present_recorded_missing_png(self):
        self.behavior['one'] = 'hang'
        self.batch(('one',))
        k.runkit.png(str(self.out / 'one/capture.knockout.100.png'), 1, 1, b'\x00' * 3)
        with contextlib.redirect_stderr(io.StringIO()) as err:
            self.assertEqual(k.check(self.out), 1)
        self.assertIn('recorded missing but present', err.getvalue())

    def rewrite_rows(self, change):
        path = self.out / 'knockout.tsv'
        with path.open(encoding='utf-8', newline='') as f:
            rows = list(csv.DictReader(f, delimiter='\t'))
        for row in rows:
            change(row)
        with path.open('w', encoding='utf-8', newline='') as f:
            writer = csv.DictWriter(f, fieldnames=k.FIELDS, delimiter='\t')
            writer.writeheader()
            writer.writerows(rows)
        # Keep the outer seal consistent to exercise semantic cross-checks.
        manifest = self.out / 'knockout.manifest.json'
        inventory = k.read_json(manifest)
        inventory['knockout.tsv'] = k.runkit.sha1_file(str(path))
        k.write_json(manifest, inventory)

    def test_absence_cannot_be_ok(self):
        self.behavior['one'] = 'hang'
        self.batch(('one',))
        self.rewrite_rows(lambda row: row.update(status='ok'))
        with contextlib.redirect_stderr(io.StringIO()) as err:
            self.assertEqual(k.check(self.out), 1)
        self.assertIn('absence not permitted', err.getvalue())

    def test_no_manifest_requires_recorded_reason(self):
        self.behavior['one'] = 'no_manifest'
        self.batch(('one',))
        self.rewrite_rows(lambda row: row.update(reason='other failure'))
        with contextlib.redirect_stderr(io.StringIO()) as err:
            self.assertEqual(k.check(self.out), 1)
        self.assertIn('absence not permitted', err.getvalue())

    def test_baseline_failure_skips_stubs(self):
        for mode in ('bad_exit', 'incomplete', 'hang', 'no_manifest'):
            with self.subTest(mode=mode):
                self.out = self.root / mode
                self.runs.clear()
                self.behavior['baseline'] = mode
                rc, rows = self.batch(('one', 'two'))
                self.assertEqual(rc, 1)
                self.assertEqual([r[0] for r in self.runs], ['baseline'])
                self.assertTrue(all(r['status'] == 'not_alive' for r in rows))
                self.assertIn('baseline failed', rows[0]['reason'])
                self.assertEqual(k.check(self.out), 0)

    def test_launch_input_preflight(self):
        command = ['run', '--scenario', 'fake', '--funcs', 'one', '--ret', 'none',
                   '--frames', '100', '--out', str(self.out)]
        for flag in ('emulator', 'cue', 'bios'):
            for invalid in (None, str(self.root / 'nonexistent'), str(self.root)):
                args = []
                for name in ('emulator', 'cue', 'bios'):
                    value = invalid if name == flag else str(self.root / name)
                    if value is not None:
                        args.extend(['--' + name, value])
                with self.subTest(flag=flag, invalid=invalid), \
                        patch.object(k, 'run_capture') as run, \
                        contextlib.redirect_stderr(io.StringIO()) as err:
                    self.assertEqual(k.main(command + args), 1)
                    self.assertIn('--' + flag, err.getvalue())
                    run.assert_not_called()
                    self.assertFalse(self.out.exists())

    def test_phase_and_name_preflight(self):
        script = dict(end_frame=200, inputs=[], screenshots=[],
                      phases=[dict(frame=0, name='boot'), dict(frame=50, name='menu')])
        for funcs, phase, frame in [('one', 'boot', '100'), ('one', 'menu', '50'),
                                    ('one', 'menu', '49'), ('Foo,foo', None, '100'),
                                    ('one,one', None, '100')]:
            command = ['run', '--scenario', 'fake', '--funcs', funcs, '--ret', 'none',
                       '--frames', frame, '--out', str(self.out), *self.launch_args]
            if phase:
                command.extend(['--phase', phase])
            with self.subTest(funcs=funcs, phase=phase, frame=frame), \
                    patch.object(k.scen, 'compile_script', return_value=script), \
                    patch.object(k, 'run_capture') as run, \
                    contextlib.redirect_stderr(io.StringIO()):
                self.assertEqual(k.main(command), 1)
                run.assert_not_called()
                self.assertFalse(self.out.exists())


if __name__ == '__main__':
    unittest.main()
