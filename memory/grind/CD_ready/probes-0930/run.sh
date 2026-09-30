set -e
source .venv/bin/activate
# usage: run.sh <name> <gen args...>
n="$1"; shift
python3 tmp/CD_ready/gen.py --name "$n" "$@"
python3 tmp/CD_cw/score_full.py "tmp/CD_ready/full_$n.c" CD_ready CD_cw CD_sync CD_datasync ${DIFF:+--diff}
