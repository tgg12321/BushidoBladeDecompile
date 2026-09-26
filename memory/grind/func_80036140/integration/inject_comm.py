import sys
syms = sys.argv[1].split(',')
data = sys.stdin.read()
sys.stdout.write(''.join(f'\t.comm\t{s},4\n' for s in syms) + data)
