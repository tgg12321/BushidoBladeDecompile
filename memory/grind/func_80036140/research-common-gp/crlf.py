import sys
d = open(sys.argv[1], 'rb').read().replace(b'\r\n', b'\n').replace(b'\n', b'\r\n')
open(sys.argv[2], 'wb').write(d)
