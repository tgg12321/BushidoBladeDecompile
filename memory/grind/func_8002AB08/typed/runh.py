# runh.py <cand.c>... : like run.py, with tmp/func_8002AB08/typed/inc/include ahead of include/
import sys, re, io, contextlib
sys.path.insert(0, '.')
from engine import buildconfig as cfg
cfg.CPP_FLAGS = '-Itmp/func_8002AB08/typed/inc/include ' + cfg.CPP_FLAGS
from engine import cli
for c in sys.argv[1:]:
    buf = io.StringIO()
    sys.argv = ['engine.cli', 'sandbox', 'func_8002AB08', '--disable', 'all', '--diff', '--candidate', c]
    try:
        with contextlib.redirect_stdout(buf):
            cli.main()
    except SystemExit:
        pass
    out = buf.getvalue(); open(c + '.out', 'w').write(out)
    s = re.search(r'"score": (\d+)', out); h = re.search(r'(\d+ source-level[^\n]*)', out)
    print(c, 'score=' + (s.group(1) if s else '?'), h.group(1) if h else out[-300:])
