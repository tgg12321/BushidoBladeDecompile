#!/bin/bash
cd "/mnt/c/Users/Trenton/Desktop/bb2-worktrees/ln-naming-a"
source .venv/bin/activate
python3 tools/naming_member_apply.py --dry-run docs/naming/phase3/pending/var-members-rest/pairs.txt > docs/naming/phase3/pending/var-members-rest/dry.txt 2>&1
echo "exit=$?" >> docs/naming/phase3/pending/var-members-rest/dry.txt
