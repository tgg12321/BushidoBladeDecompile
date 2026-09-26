"""The maspsx COMMON gate (maspsx_comm_syms.txt, owner ruling 2026-09-26 Q9) applied to a tree.
usage: python3 tmp/func_80036140/gate.py <tree-root>   (tree-root '.' = the real repo, under the lock)
Edits: tools/maspsx/maspsx/__init__.py, tools/maspsx/maspsx.py, maspsx_comm_syms.txt, Makefile
(--comm-syms on both MASPSX_FLAGS lines + PIPELINE_DEPS), engine/buildconfig.py (mirror).
Idempotence is NOT supported: run once on a pristine tree."""
import sys
from pathlib import Path

R = Path(sys.argv[1])


def rw(rel, fn):
    p = R / rel
    s = p.read_text(encoding='utf-8')
    t = fn(s)
    assert t != s, rel
    p.write_bytes(t.encode('utf-8'))


def rep(s, old, new, count=1):
    assert s.count(old) == count, (old[:60], s.count(old))
    return s.replace(old, new)


# ---- maspsx core -------------------------------------------------------------------------------
def core(s):
    s = rep(s, '        sdata_exclude_map=None,\n    ):',
            '        sdata_exclude_map=None,\n        comm_sym_map=None,\n    ):')
    s = rep(s, '        self.sdata_exclude_map = sdata_exclude_map or {}\n',
            '        self.sdata_exclude_map = sdata_exclude_map or {}\n'
            '        # Per-function COMMON gate (maspsx_comm_syms.txt): func -> symbols that were\n'
            '        # tentative definitions in that function\'s original translation unit.\n'
            '        self.comm_sym_map = comm_sym_map or {}\n')
    s = rep(s, '    def _sdata_allowed_for_current_func(self, symbol=None) -> bool:',
            '    def _is_comm(self, symbol) -> bool:\n'
            '        """ASPSX 2.34 never gives a COMMON (`.comm`) symbol gp at an offset (`sym+N`).\n'
            '        True for a symbol declared `.comm` in this file (upstream behaviour) or listed\n'
            '        for the current function in maspsx_comm_syms.txt (owner ruling 2026-09-26,\n'
            '        .claude/rules/maspsx-gate-lists.md): our C declares such variables `extern`,\n'
            '        so their storage class must come from the list. Only ever removes gp, only\n'
            '        from `sym+N` operands; the base access `sym` is unaffected."""\n'
            '        return symbol in self.comm_symbols or symbol in self.comm_sym_map.get(self.current_func, ())\n'
            '\n'
            '    def _sdata_allowed_for_current_func(self, symbol=None) -> bool:')
    s = rep(s, 'gp_allowed = self.gp_allow_offset or symbol not in self.comm_symbols',
            'gp_allowed = self.gp_allow_offset or not self._is_comm(symbol)', count=3)
    return s


def cli(s):
    s = rep(s, '    # decomp.me debugging\n',
            '    parser.add_argument("--comm-syms", type=str, default=None,\n'
            '                        help="Path to file listing, per function, symbols that were COMMON "\n'
            '                             "(tentative definitions) in the original translation unit "\n'
            '                             "(func: sym1, sym2): ASPSX 2.34 never gives such a symbol gp at an "\n'
            '                             "offset (sym+N), only at its base (owner ruling 2026-09-26). "\n'
            '                             "Per-function-scoped; never global.")\n'
            '    # decomp.me debugging\n')
    s = rep(s, '    # Load per-function lb/lh expansion lists\n',
            '    # Load the per-function COMMON gate (same format as --sdata-exclude)\n'
            '    comm_sym_map = {}\n'
            '    if args.comm_syms:\n'
            '        with open(args.comm_syms, "r", encoding="utf") as f:\n'
            '            for line in f:\n'
            '                line = line.strip()\n'
            '                if not line or line.startswith("#"):\n'
            '                    continue\n'
            '                if ":" in line:\n'
            '                    func_name, syms = line.split(":", 1)\n'
            '                    comm_sym_map.setdefault(func_name.strip(), set()).update(\n'
            '                        s.strip() for s in syms.split(",") if s.strip())\n'
            '\n'
            '    # Load per-function lb/lh expansion lists\n')
    s = rep(s, '        sdata_exclude_map=sdata_exclude_map,\n    )',
            '        sdata_exclude_map=sdata_exclude_map,\n        comm_sym_map=comm_sym_map,\n    )')
    return s


rw('tools/maspsx/maspsx/__init__.py', core)
rw('tools/maspsx/maspsx.py', cli)

# ---- the list ----------------------------------------------------------------------------------
(R / 'maspsx_comm_syms.txt').write_bytes(Path('tmp/func_80036140/maspsx_comm_syms.txt').read_bytes())

# ---- Makefile + mirror -------------------------------------------------------------------------
def mk(s):
    s = rep(s, ' --prefill-label-funcs=maspsx_prefill_label_funcs.txt\n',
            ' --prefill-label-funcs=maspsx_prefill_label_funcs.txt --comm-syms=maspsx_comm_syms.txt\n', count=2)
    s = rep(s, 'expand_dest_funcs.txt maspsx_prefill_label_funcs.txt \\\n',
            'expand_dest_funcs.txt maspsx_prefill_label_funcs.txt maspsx_comm_syms.txt \\\n')
    return s


rw('Makefile', mk)
