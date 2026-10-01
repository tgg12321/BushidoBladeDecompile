#!/bin/bash
# usage: camp.sh launch <func> <dir> <label> [jobs] | harvest <dir> [--stop ...] | status
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
source .venv/bin/activate
case "$1" in
  launch) python3 tools/permuter_campaign.py launch --func "$2" --dir "$3" --label "$4" -j ${5:-2} --stop-on-zero ;;
  harvest) shift; python3 tools/permuter_campaign.py harvest --dir "$@" ;;
  status) python3 tools/permuter_campaign.py status ;;
esac
