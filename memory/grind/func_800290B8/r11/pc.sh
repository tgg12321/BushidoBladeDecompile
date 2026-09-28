#!/bin/bash
cd "$(dirname "$0")/../.."
source .venv/bin/activate 2>/dev/null
python3 tools/permuter_campaign.py "$@"
