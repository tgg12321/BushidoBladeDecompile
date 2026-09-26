"""Assemble an asm file with real ASPSX 2.34 and print decoded .text (gp vs lui per access).
usage (WSL, repo root): python3 tmp/research36140/asm_obj.py <file.s> [-G8]"""
import subprocess, sys
from pathlib import Path
sys.path.insert(0, 'tools/maspsx/aspsx')
sys.path.insert(0, 'tmp/research36140')
import util
from run_asm_tests import decode

s = Path(sys.argv[1]); g = sys.argv[2] if len(sys.argv) > 2 else '-G8'
o = s.with_suffix('.obj')
r = subprocess.run(['bash', 'tmp/research36140/aspsx.sh', str(s), str(o), g], capture_output=True, text=True)
print(r.stderr.strip())
t = util.read_text_section(o.read_bytes())
for i in range(0, len(t), 4):
    w = int.from_bytes(t[i:i + 4], 'little')
    print(f'{i:04x} {w:08x} {decode(w)}')
