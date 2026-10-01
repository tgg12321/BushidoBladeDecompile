#!/bin/bash
# atstep.sh <tag> <cmd...>: scratch clone only - run a read-only command with the tree at <tag>, then restore HEAD.
A="/tmp/q56/adopt tree"; TAG="$1"; shift
cd "$A" && HEAD_REF=$(git rev-parse HEAD) && git checkout -q "$TAG" || exit 1
bash -c "$*"
git checkout -q q56-adopt && git reset -q --hard "$HEAD_REF"
