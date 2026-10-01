#!/bin/bash
# post5.sh: after a full series run - integrity, post_run evidence, body hashes, patch diff vs round 4.
R="/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
cp "$R/memory/grind/q65-adoption/q56/adopt/body_hashes.py" "$R/tmp/q56/adopt/body_hashes.py"
echo "== integrity"; bash "$R/tmp/q56/scratch_integrity.sh"
echo "== post_run"; bash "$R/tmp/q56/adopt/post_run.sh" 2>&1 | tail -6
echo "== body hashes"
cd "/tmp/q56/adopt tree" && source .venv/bin/activate
python3 "$R/tmp/q56/adopt/body_hashes.py" "/tmp/q56/adopt tree" > "$R/tmp/q56/adopt/body_hashes.txt" 2>&1; grep -c . "$R/tmp/q56/adopt/body_hashes.txt"
diff "$R/tmp/q56/r4_patches/body_hashes.txt" "$R/tmp/q56/adopt/body_hashes.txt" && echo "body_hashes identical to round 4"
echo "== patch diff vs round 4"
rm -rf "$R/tmp/q56/r5diff"; python3 "$R/tmp/q56/r2/pdiff.py" "$R/tmp/q56/r4_patches" "$R/tmp/q56/adopt" "$R/tmp/q56/r5diff"
