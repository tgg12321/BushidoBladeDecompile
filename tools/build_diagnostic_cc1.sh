#!/bin/bash
# tools/build_diagnostic_cc1.sh — rebuild the INSTRUMENTED diagnostic cc1 so it
# agrees with the oracle compiler.
#
# tools/gcc-2.7.2/cc1 is the diagnostic compiler: the same GCC 2.7.2 carrying
# the BB2_*_DEBUG hooks that ra_solver / sched_solver read their dumps from.
# It was built WITHOUT tools/cc1-no-plus-to-ior.patch, so on ings and
# code6cac_b it described a compiler the project does not ship — the gap
# recorded as UNFAITHFUL_STEMS in tools/ra_solver/local_extract.py.
#
# This rebuilds it from the LIVE (hooked) sources PLUS the same committed
# patch, so diagnostic and oracle agree on every TU.
#
#   bash tools/build_diagnostic_cc1.sh            # build + verify in scratch
#   bash tools/build_diagnostic_cc1.sh --install  # then install over tools/gcc-2.7.2/cc1
#
# Unlike the oracle build this one KEEPS the hooks — that is the whole point —
# so it deliberately does NOT revert the scratch tree to pristine.
set -uo pipefail
cd "$(git rev-parse --show-toplevel)" || exit 1
LIVE=tools/gcc-2.7.2
SCRATCH=tmp/cc1diag
PATCH=tools/cc1-no-plus-to-ior.patch
ORACLE=$LIVE/build/cc1
INSTALL=0
for a in "$@"; do case "$a" in --install) INSTALL=1 ;; *) echo "unknown option: $a" >&2; exit 2 ;; esac; done

read -r -d '' MANIFEST <<'EOF'
80c8088750a91b37ef53bea6da51d402c58801c8  flow.c
6f23c5eeeabc97f3049b2d414ab42b6f38b891f6  function.c
46af0a314da8ad6dcb27890ca9b2003006c70ced  global.c
9b8f822a79a1945ac4b58ebfb243017d67b828c5  jump.c
3fb248a6b2c85cd7e9e57b19f5df7daf62b3d5fe  local-alloc.c
7ddde6b0f2b65172c5cc83be6165789f445953d9  reload1.c
73a15a5245d2e7ad55fa9e3c42d724488b1892d6  reorg.c
3668555e9cb7970b335a505aca4cfdda26e8fc49  sched.c
24c5952113d88cbb96f5c9f7e7152147d1efb8a7  combine.c
EOF
echo "== verifying $LIVE against the docs/ORACLE-COMPILER.md manifest"
( cd "$LIVE" && printf '%s\n' "$MANIFEST" | sha1sum -c --quiet ) || {
  echo "FATAL: hooked sources drifted from the manifest — refusing to build" >&2; exit 1; }
echo "   manifest OK (hooks present, as expected for the diagnostic build)"

echo "== preparing scratch tree at $SCRATCH (hooks KEPT)"
rm -rf "$SCRATCH"; mkdir -p "$SCRATCH"
cp -a "$LIVE/." "$SCRATCH/" || exit 1
[ "$(readlink -f "$SCRATCH")" != "$(readlink -f "$LIVE")" ] || { echo "FATAL: scratch == live" >&2; exit 1; }

echo "== applying $PATCH"
git -C "$SCRATCH" apply --check "$PWD/$PATCH" 2>&1 || { echo "FATAL: patch does not apply" >&2; exit 1; }
git -C "$SCRATCH" apply "$PWD/$PATCH" || exit 1
echo "   applied"
# Confirm the hooks really are still in the scratch sources.
H=$(grep -lc 'BB2_.*DEBUG' "$SCRATCH"/{flow,function,global,jump,local-alloc,reload1,reorg,sched}.c 2>/dev/null | wc -l)
echo "   hooked source files present in scratch: $H/8"
[ "$H" = 8 ] || { echo "FATAL: hooks missing from the scratch tree" >&2; exit 1; }

echo "== building (same recipe as the oracle)"
( cd "$SCRATCH"
  touch c-parse.c c-parse.h c-gperf.h objc-parse.c objc-parse.h 2>/dev/null
  rm -f ./*.o cc1
  TMPDIR=/dev/shm make combine.o CFLAGS="-O -g -fgnu89-inline" >/dev/null 2>&1
  TMPDIR=/dev/shm make cc1      CFLAGS="-g -fgnu89-inline"    >/dev/null 2>&1 )
[ -f "$SCRATCH/cc1" ] || { echo "FATAL: build produced no cc1" >&2; exit 1; }
SH=$(sha1sum "$SCRATCH/cc1" | cut -d' ' -f1)
echo "   built: $SCRATCH/cc1  ($(stat -c%s "$SCRATCH/cc1") bytes)  sha1 $SH"

echo "== fidelity check: diagnostic vs ORACLE ($ORACLE) over all TUs"
F="-O2 -G0 -funsigned-char -quiet -mcpu=3000 -mips1 -mno-abicalls -fno-builtin -w -mel -msoft-float"
FG8="-O2 -G8 -funsigned-char -quiet -mcpu=3000 -mips1 -mno-abicalls -fno-builtin -w -mel -msoft-float"
CPP="mipsel-linux-gnu-cpp -Iinclude -undef -Wall -lang-c -fno-builtin -Dmips -D__GNUC__=2 -D__OPTIMIZE__ -D__mips__ -D__mips -Dpsx -D__psx__ -D__psx -D_PSYQ -D__EXTENSIONS__ -D_MIPSEL -D_LANGUAGE_C -DLANGUAGE_C"
W=tmp/cc1diag_check; mkdir -p $W
n=0; bad=""; lines=0
for stem in $(ls src/*.c | sed 's|src/||; s|\.c$||'); do
  $CPP src/$stem.c > $W/t.i 2>/dev/null
  case "$stem" in text1a_pre|text1a_post) FL="$FG8";; *) FL="$F";; esac
  "$SCRATCH/cc1" $FL $W/t.i -o $W/new.s 2>/dev/null
  "$ORACLE"      $FL $W/t.i -o $W/ref.s 2>/dev/null
  n=$((n+1))
  cmp -s $W/new.s $W/ref.s || { bad="$bad $stem"; lines=$((lines + $(diff $W/ref.s $W/new.s | grep -c '^[<>]'))); }
done
echo "   $n TUs; divergent stems: ${bad:-none}${bad:+  ($lines line(s))}"

# For reference: what the CURRENT installed diagnostic cc1 scores on the same test.
if [ -x "$LIVE/cc1" ]; then
  obad=""
  for stem in $(ls src/*.c | sed 's|src/||; s|\.c$||'); do
    $CPP src/$stem.c > $W/t.i 2>/dev/null
    case "$stem" in text1a_pre|text1a_post) FL="$FG8";; *) FL="$F";; esac
    "$LIVE/cc1" $FL $W/t.i -o $W/old.s 2>/dev/null
    "$ORACLE"   $FL $W/t.i -o $W/ref.s 2>/dev/null
    cmp -s $W/old.s $W/ref.s || obad="$obad $stem"
  done
  echo "   (currently installed diagnostic cc1 diverges on:${obad:- none})"
fi

if [ -n "$bad" ]; then
  echo "   FAIL — the rebuilt diagnostic must agree with the oracle on every TU."
  exit 1
fi
echo "   PASS — diagnostic and oracle agree on all $n TUs; UNFAITHFUL_STEMS can be emptied."

if [ "$INSTALL" = 1 ]; then
  PREV=$(sha1sum "$LIVE/cc1" | cut -d' ' -f1)
  KEEP="$LIVE/cc1.PRE-PATCH-${PREV:0:8}"
  echo "== installing over $LIVE/cc1"
  [ -f "$KEEP" ] || cp -a "$LIVE/cc1" "$KEEP"
  cp "$SCRATCH/cc1" "$LIVE/cc1"
  echo "   installed $SH   (previous $PREV retained as $(basename "$KEEP"))"
  echo "   REMEMBER: update the cc1 hash in docs/ORACLE-COMPILER.md's manifest."
fi
