"""keys.py <commit-msg> <file> <func>...: test clone - layer-2 body keys at the chain commit named <commit-msg>."""
import subprocess, sys
T = "/tmp/q56r2/t"
sys.path.insert(0, T)
from engine import layer2
h = subprocess.run(["git", "-C", T, "log", "--format=%H", f"--grep=^{sys.argv[1]}$"], capture_output=True, text=True).stdout.split()[0]
txt = subprocess.run(["git", "-C", T, "show", f"{h}:{sys.argv[2]}"], capture_output=True, text=True).stdout
for f in sys.argv[3:]:
    src = layer2.body_source(txt, f, read=lambda p: None)
    print(f, src and layer2._key(src[1]))
