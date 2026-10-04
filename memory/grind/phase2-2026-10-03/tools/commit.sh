#!/bin/bash
# commit.sh MSGFILE APPENDFILE FUNC... : stage memory/grind/<FUNC>/layer2.jsonl for every FUNC
# (each must exist and be modified/untracked), append APPENDFILE (or "-" for none) to MSGFILE,
# commit with MSGFILE, then verify the commit holds exactly those ledgers and the message tail.
# Aborts on the first failure (set -e). Run from Git Bash in the repo root.
set -euo pipefail
cd "/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
msg=$1; app=$2; shift 2
[ -f "$msg" ] || { echo "no message file $msg"; exit 1; }
n=0
for f in "$@"; do
  p="memory/grind/$f/layer2.jsonl"
  [ -f "$p" ] || { echo "MISSING ledger $p"; exit 1; }
  git status --short -- "$p" | grep -q . || { echo "ledger unchanged: $p"; exit 1; }
  git add "$p"; n=$((n+1))
done
if [ "$app" != "-" ]; then
  [ -f "$app" ] || { echo "no append file $app"; exit 1; }
  printf '\n' >> "$msg"; cat "$app" >> "$msg"
fi
# nothing outside the expected set may be unstaged in src/include/named_syms
if git diff --name-only | grep -v '^metrics/events.jsonl$' | grep -q .; then
  echo "unstaged tracked changes present:"; git diff --name-only; exit 1
fi
git commit -q -F "$msg"
got=$(git show --stat --format= HEAD | grep -c 'memory/grind/.*/layer2.jsonl' || true)
echo "HEAD $(git log --oneline -1)"
echo "ledgers in commit: $got (expected $n)"
[ "$got" = "$n" ] || { echo "LEDGER COUNT MISMATCH"; exit 1; }
git show --stat --format= HEAD | tail -1
echo "--- message tail:"; git log -1 --format=%B | tail -4
