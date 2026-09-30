set -e
source .venv/bin/activate
python3 tmp/CD_cw/gen.py
for v in "$@"; do python3 tmp/CD_cw/score_full.py tmp/CD_cw/full_$v.c CD_cw CD_sync CD_datasync CdLastPos; done
