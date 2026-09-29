#!/bin/bash
# usage: camp.sh launch <dir> <label> [jobs] | harvest <dir> [--stop ...] | status
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
source .venv/bin/activate
case "$1" in
  launch) python3 tools/permuter_campaign.py launch --func func_800187F4 --dir "$2" --label "$3" -j ${4:-4} --stop-on-zero ;;
  harvest) shift; python3 tools/permuter_campaign.py harvest --dir "$@" ;;
  status) python3 tools/permuter_campaign.py status ;;
esac
