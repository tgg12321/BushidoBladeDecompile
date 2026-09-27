#!/bin/bash
# Apply the five-commit landing to the REAL tree (lock holder only), snapshotting each commit's files.
set -eo pipefail
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"; source .venv/bin/activate
H=tmp/func_80036140; S=$H/snap
test -z "$(git status --porcelain --untracked-files=no -- Makefile engine include src asm bb2.ld tools .claude $(ls *.txt))" \
  || { echo "tree not clean in landing paths"; git status --short -- Makefile engine include src asm bb2.ld tools .claude $(ls *.txt); exit 1; }
test -z "$(git diff --cached --name-only)" || { echo "index not empty"; exit 1; }
git rev-parse HEAD > $H/land_rev.txt
rm -rf $S; mkdir -p $S
snap() {  # step files...
  local st=$1; shift; mkdir -p $S/$st; printf '%s\n' "$@" > $S/$st.files
  for f in "$@"; do mkdir -p "$S/$st/$(dirname $f)"; cp "$f" "$S/$st/$f"; done
}
A="tools/maspsx/maspsx/__init__.py tools/maspsx/maspsx.py tools/maspsx/tests/test_comm_syms.py maspsx_comm_syms.txt Makefile engine/buildconfig.py engine/cheats.py engine/oracle.py engine/dossier.py engine/buildstamp.py engine/queue.py engine/test_engine.py tools/check_root_cleanliness.py tools/desync_audit.py tools/naming_wave.py tools/libscan/manifest.py tools/grinder/grindlib.py tools/hooks/park_src_guard.py tools/hooks/main_reintegration_lock.py tools/hooks/worktree_contamination_guard.py .claude/rules/integration-handoff-self-serve.md .claude/rules/maspsx-gate-lists.md"
B="src/code6cac_b2_post.c src/code6cac_b4.c src/code6cac_b4_post.c Makefile engine/buildconfig.py bb2.ld"
C="include/code6cac.h src/code6cac_b4.c src/code6cac_b4_post.c undefined_syms_auto.txt named_syms.txt asm/data/91C98.data.s"
D="src/code6cac_b4_post.c src/code6cac_b5.c src/code6cac_b5_post.c Makefile engine/buildconfig.py bb2.ld undefined_syms_auto.txt named_syms.txt .claude/rules/maspsx-gate-lists.md"
E="include/code6cac.h src/code6cac_b5.c undefined_syms_auto.txt named_syms.txt asm/data/91C98.data.s"
python3 $H/gate.py . && python3 $H/register.py . && snap A $A
python3 $H/apply2.py . split && snap B $B
python3 $H/apply2.py . merge && snap C $C
python3 $H/apply2.py . match && snap D $D
python3 $H/apply2.py . cdres && snap E $E
git status --short | grep -v metrics/
