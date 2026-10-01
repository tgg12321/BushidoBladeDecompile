# tuc.py <inc-dir> <stem>...: build each TU (src unchanged) with <inc-dir> ahead of include/, compare .text bytes with build/src/<stem>.o
import sys, os, subprocess
sys.path.insert(0, '.')
from engine import buildconfig as cfg
inc = sys.argv[1]
cfg.CPP_FLAGS = '-I%s ' % inc + cfg.CPP_FLAGS
from engine import pipeline, cheats
for stem in sys.argv[2:]:
    wd = 'tmp/func_80058580/h/tuc/%s' % stem
    os.makedirs(wd, exist_ok=True)
    ov = cheats.empty_overrides(wd + '/cfg')
    out = wd + '/%s.o' % stem
    try:
        pipeline.build_c_object(stem, out, cheat_overrides=ov)
    except Exception as e:
        print(stem, 'BUILD FAIL', str(e)[:200]); continue
    def sec(o):
        subprocess.run(['mipsel-linux-gnu-objcopy', '-O', 'binary', '-j', '.text', o, o + '.text'], check=True)
        subprocess.run(['mipsel-linux-gnu-objcopy', '-O', 'binary', '-j', '.data', o, o + '.data'], check=True)
        return open(o + '.text', 'rb').read() + open(o + '.data', 'rb').read()
    a = sec(out); b = sec('build/src/%s.o' % stem)
    print(stem, 'SAME' if a == b else 'DIFF (%d vs %d bytes)' % (len(a), len(b)))
