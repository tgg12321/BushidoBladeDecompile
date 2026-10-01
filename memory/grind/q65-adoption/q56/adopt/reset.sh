#!/bin/bash
# reset.sh: scratch clone only — discard uncommitted work (back to the last step commit).
cd "/tmp/q56/adopt tree" && git reset -q --hard HEAD && git clean -qfd -e tmp -e build && git log --oneline -1
