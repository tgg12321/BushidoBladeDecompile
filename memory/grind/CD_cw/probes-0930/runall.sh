set -e
source .venv/bin/activate
F=$(mipsel-linux-gnu-nm build/src/system.o | awk '$2=="T"{print $3}' | tr '\n' ' ')
python3 tmp/CD_cw/score_full.py "${1:-tmp/CD_cw/full_final.c}" $F | grep -v "score=0 " || true
echo "funcs: $(echo $F | wc -w)"
