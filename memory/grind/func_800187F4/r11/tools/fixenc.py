import sys
for p in sys.argv[1:]:
    b = open(p, 'rb').read()
    out = []
    n = 0
    for line in b.split(b'\n'):
        try:
            out.append(line.decode('utf-8'))
        except UnicodeDecodeError:
            out.append(line.decode('cp1252')); n += 1
    open(p, 'wb').write('\n'.join(out).encode('utf-8'))
    print(p, 'cp1252 lines fixed:', n)
