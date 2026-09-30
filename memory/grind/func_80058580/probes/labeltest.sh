#!/bin/bash
# labeltest.sh: scratch tree with maspsx's is_label() also recognizing GCC's .L<n>: labels; full build; compare.
set -e
R=/home/user/BushidoBladeDecompile
S=/tmp/claude-0/lbl
rm -rf $S && mkdir -p $S
cd $R && tar --exclude=./build --exclude=./.git --exclude=./tools/gcc-2.7.2 --exclude=./metrics --exclude=./memory --exclude=./tmp --exclude=./disc -cf - . | tar -xf - -C $S
ln -s $R/tools/gcc-2.7.2 $S/tools/gcc-2.7.2; ln -s $R/disc $S/disc
python3 - <<'PY'
p = "/tmp/claude-0/lbl/tools/maspsx/maspsx/__init__.py"
s = open(p).read()
old = 'def is_label(line: str):\n    return re.match(r"\\$L(b|e)?\\d+:$", line)\n'
assert old in s, "is_label not found"
s = s.replace(old, 'def is_label(line: str):\n    return re.match(r"(\\$L(b|e)?|\\.L)\\d+:$", line)\n')
open(p, "w").write(s)
PY
cd $S && make -j16 build/bb2.bin > $S/make.log 2>&1 || { tail -5 $S/make.log; exit 1; }
cmp build/bb2.bin /tmp/claude-0/ref_main.bin && echo "IDENTICAL" || python3 - <<'PY'
a=open('/tmp/claude-0/lbl/build/bb2.bin','rb').read(); b=open('/tmp/claude-0/ref_main.bin','rb').read()
print("size", len(a), len(b))
d=[i for i in range(0,min(len(a),len(b)),4) if a[i:i+4]!=b[i:i+4]]
print("first diff", hex(0x80010000+d[0]) if d else None, "count", len(d))
PY
