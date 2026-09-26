"""Copy tools/maspsx to tmp/research36140/maspsx_comm and add the proposed per-function COMMON gate:
  --comm-syms FILE   lines `func: sym1, sym2` -- inside `func`, the listed symbols are treated as
                     COMMON (.comm) for ASPSX 2.34's gp decision only: base access may use gp (if
                     otherwise allowed), `sym+N` never does. No storage is emitted.
usage (repo root): python3 tmp/research36140/make_patched_maspsx.py"""
import shutil
from pathlib import Path

dst = Path('tmp/research36140/maspsx_comm')
if dst.exists():
    shutil.rmtree(dst)
shutil.copytree('tools/maspsx', dst, ignore=shutil.ignore_patterns('aspsx', 'tests', '.git', '__pycache__'))

init = dst / 'maspsx' / '__init__.py'
s = init.read_text(encoding='utf-8')
old = 'gp_allowed = self.gp_allow_offset or symbol not in self.comm_symbols'
assert s.count(old) == 3, s.count(old)
s = s.replace(old, 'gp_allowed = self.gp_allow_offset or not self._is_comm(symbol)')
s = s.replace('        sdata_exclude_map=None,\n    ):',
              '        sdata_exclude_map=None,\n        comm_sym_map=None,\n    ):', 1)
s = s.replace('        self.sdata_exclude_map = sdata_exclude_map or {}\n',
              '        self.sdata_exclude_map = sdata_exclude_map or {}\n'
              '        self.comm_sym_map = comm_sym_map or {}\n', 1)
s = s.replace('    def _sdata_allowed_for_current_func(self, symbol=None) -> bool:',
              '    def _is_comm(self, symbol) -> bool:\n'
              '        # ASPSX 2.34: a COMMON (.comm) symbol is never gp-relative at an offset.\n'
              '        return symbol in self.comm_symbols or symbol in self.comm_sym_map.get(self.current_func, ())\n\n'
              '    def _sdata_allowed_for_current_func(self, symbol=None) -> bool:', 1)
init.write_text(s, encoding='utf-8', newline='\n')

cli = dst / 'maspsx.py'
c = cli.read_text(encoding='utf-8')
c = c.replace('    # decomp.me debugging\n',
              '    parser.add_argument("--comm-syms", type=str, default=None)\n    # decomp.me debugging\n', 1)
c = c.replace('    # Load per-function lb/lh expansion lists\n',
              '    comm_sym_map = {}\n'
              '    if args.comm_syms:\n'
              '        with open(args.comm_syms, "r", encoding="utf") as f:\n'
              '            for line in f:\n'
              '                line = line.strip()\n'
              '                if line and not line.startswith("#") and ":" in line:\n'
              '                    fn, syms = line.split(":", 1)\n'
              '                    comm_sym_map.setdefault(fn.strip(), set()).update(\n'
              '                        x.strip() for x in syms.split(",") if x.strip())\n\n'
              '    # Load per-function lb/lh expansion lists\n', 1)
c = c.replace('        sdata_exclude_map=sdata_exclude_map,\n    )',
              '        sdata_exclude_map=sdata_exclude_map,\n        comm_sym_map=comm_sym_map,\n    )', 1)
assert 'comm_sym_map=comm_sym_map' in c and 'args.comm_syms' in c
cli.write_text(c, encoding='utf-8', newline='\n')
print('ok', dst)
