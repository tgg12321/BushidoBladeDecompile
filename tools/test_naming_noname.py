#!/usr/bin/env python3
"""Tests for tools/naming_noname.py: final rows are never offered by `free`,
non-function rows survive `prune` while their placeholder is in the tree, and
the tree census finds globals, members and locals.

    python tools/test_naming_noname.py
"""
import contextlib
import io
import os
import sys
import tempfile
import unittest

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import naming_noname as nn  # noqa: E402

HEAD = '\t'.join(nn.COLS) + '\n'
CENSUS = ('address,current_name,glabel,name_layer,src_location\n'
          '0x80010000,func_80010000,func_80010000,,src/main/a.c\n'
          '0x80010008,func_80010008,func_80010008,,src/main/a.c\n'
          '0x80010010,func_80010010,func_80010010,,src/main/a.c\n')
SRC = '''#include "common.h"
#define SCR ((Rec *)0x1F800000)
typedef struct {
    s32 kept;
    u8 unk_04[4]; /* no access */
    s16 unk06;
} Rec;
s32 D_80090000; /* gap */
void func_80010000(void) {}
void f(s32 arg0) {
    s32 var_1 = 0; /* FAKE: constant holder */
    SCR->kept = arg0 + var_1;
}
'''


def row(addr, name, kind, final='-'):
    return '\t'.join([addr, name, kind, '1', 'w1', 'r', 'e', 'abc', final]) + '\n'


class NoNameTest(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.cwd = os.getcwd()
        os.chdir(self.tmp.name)
        os.makedirs('docs/naming/phase3')
        os.makedirs('src/main')
        os.makedirs('include')
        with open(nn.CENSUS, 'w', newline='\n') as f:
            f.write(CENSUS)
        with open('src/main/a.c', 'w', newline='\n') as f:
            f.write(SRC)
        with open(nn.LEDGER, 'w', newline='\n') as f:
            f.write(HEAD + row('0x80010000', 'func_80010000', 'func', 'empty')
                    + row('0x80010008', 'func_80010008', 'func')
                    + row('0x80090000', 'D_80090000', 'data', 'gap-filler')
                    + row('Rec.unk_04', 'unk_04', 'member', 'never-accessed')
                    + row('f:var_1', 'var_1', 'local', 'const-holder')
                    + row('g:var_9', 'var_9', 'local', 'const-holder'))

    def tearDown(self):
        os.chdir(self.cwd)
        self.tmp.cleanup()

    def run_cmd(self, *argv):
        out, err = io.StringIO(), io.StringIO()
        sys.argv = ['naming_noname.py', *argv]
        with contextlib.redirect_stdout(out), contextlib.redirect_stderr(err):
            try:
                nn.main()
                code = 0
            except SystemExit as e:
                code = e.code
        return code, out.getvalue(), err.getvalue()

    def test_tree_census(self):
        tree = nn.tree_census()
        self.assertEqual(tree['data'], {'D_80090000'})
        self.assertEqual(tree['member'], {'Rec.unk_04', 'Rec.unk06'})
        self.assertEqual(tree['local'], {'f:arg0', 'f:var_1'})

    def test_free_never_offers_final(self):
        code, out, err = self.run_cmd('free', 'src/main/a.c')
        self.assertEqual((code, out.split()), (0, ['func_80010010']))
        self.assertIn('1 skipped as NO_NAME (no new evidence), 1 final', err)
        code, out, _ = self.run_cmd('free', 'src/main/a.c', '--revive', '0x80010008=named callee')
        self.assertIn('func_80010008', out.split())
        code, _, _ = self.run_cmd('free', 'src/main/a.c', '--revive', '0x80010000=trace')
        self.assertIn('final (empty)', code)
        code, out, _ = self.run_cmd('free', 'src/main/a.c', '--revive', '0x80010000=trace',
                                    '--force-final')
        self.assertIn('func_80010000', out.split())

    def test_prune_keeps_present_rows(self):
        rows = nn.load()
        gone = nn.prune(rows, nn.load_census())
        self.assertEqual(gone, ['g:var_9'])
        nn.save(rows)
        self.assertEqual([k for k in nn.load()],
                         ['0x80010000', '0x80010008', '0x80090000', 'Rec.unk_04', 'f:var_1'])

    def test_status(self):
        code, out, _ = self.run_cmd('status')
        lines = dict(l.split('\t', 1) for l in out.splitlines()[1:])
        self.assertEqual(lines['func'], '3\t1\t2\t1')
        self.assertEqual(lines['data'], '1\t1\t0\t0')
        self.assertEqual(lines['member'], '2\t1\t1\t0')
        self.assertEqual(lines['local'], '2\t1\t1\t0')


if __name__ == '__main__':
    unittest.main()
