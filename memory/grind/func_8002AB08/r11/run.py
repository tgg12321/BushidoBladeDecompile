# run.py <cand.c>... : engine sandbox --disable all for each candidate; print score + hunk summary
import sys, subprocess, re, time
for c in sys.argv[1:]:
    t = time.time()
    p = subprocess.run([sys.executable, '-m', 'engine.cli', 'sandbox', 'func_8002AB08', '--disable', 'all', '--diff', '--candidate', c],
                       capture_output=True, text=True)
    out = p.stdout + p.stderr
    open(c + '.out', 'w').write(out)
    s = re.search(r'"score": (\d+)', out)
    h = re.search(r'(\d+ source-level .*)', out)
    n = re.search(r'target (\d+) insns · ours (\d+) insns', out)
    print(f"{c}: score={s.group(1) if s else '?'} {n.group(0) if n else ''} | {h.group(1) if h else out[-300:]} ({time.time()-t:.0f}s)")
