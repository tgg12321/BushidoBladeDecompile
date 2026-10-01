#!/bin/bash
# rewind.sh TAG: scratch clone ("/tmp/q56/adopt tree") only - move the scratch branch back to a step tag.
cd "/tmp/q56/adopt tree" && git reset -q --hard "$1" && git clean -qfd -e tmp -e build && git log --oneline -1
