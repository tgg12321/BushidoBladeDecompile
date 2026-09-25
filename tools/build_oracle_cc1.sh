#!/bin/bash
# tools/build_oracle_cc1.sh — rebuild the project's cc1 from source.
#
# The oracle compiler is GCC 2.7.2 from the pinned upstream commit, built with
# the recipe recovered by the 2026-08-07 forensics, plus TWO committed patches:
#
#   upstream : decompals/mips-gcc-2.7.2 @ 43d1cdb67ed135879869b5266f01efaaada5e35a
#   patch    : tools/cc1-plus-to-ior-narrow.patch  (narrows combine.c's
#              PLUS->IOR conversion: no rewrite of (plus REG CONST_INT); read
#              that file's header for the full rationale)
#   crashfix : tools/cc1-reorg-negate-rtx-decl.patch  (the owner-approved
#              2026-08-24 host-ABI negate_rtx declaration in reorg.c;
#              applied in EVERY mode — it is not a codegen change)
#   recipe   : every object at -O0 (`-g`), combine.o ALONE at `-O`,
#              `-fgnu89-inline` throughout (without it modern hosts fail to
#              link: c-gperf.h's is_reserved_word is `__inline` without
#              `static`)
#
# Owner rulings: docs/grind/decisions.md 2026-08-07 (d99ab6a6: reproducible
# baseline), 2026-08-24 (262930f1b: crash fix), 2026-09-25 (bcdc1648e: the
# narrowed PLUS->IOR condition replaces the old no-rewrite patch). Background
# in docs/ORACLE-COMPILER.md. The narrowed condition is the best fit to the
# evidence, not a recovered historical compiler; the question stays open there.
#
# Modes:
#   (default)        the operative oracle compiler: pristine + both patches.
#   --stock          pristine upstream + the crash fix only, PLUS->IOR patch
#                    NOT applied. Research only — on the current tree it
#                    diverges from the oracle at site A only (ings).
#   --install        after a PASSing self-check, install over
#                    tools/gcc-2.7.2/build/cc1, preserving the previous binary.
#   --record-manifest  refresh tools/cc1_tu_expectation.txt from this build.
#
# Guarantees:
#   * tools/gcc-2.7.2/ is NEVER modified. The build runs in a scratch copy whose
#     tracked files are restored to the upstream commit, so the local BB2 debug
#     hooks are absent from the oracle build by construction (they are inert,
#     but the oracle must not depend on that).
#   * Refuses to run if tools/gcc-2.7.2/'s sources have drifted from the
#     manifest in docs/ORACLE-COMPILER.md.
#   * Building never installs. Installing never skips the self-check.
set -uo pipefail

cd "$(git rev-parse --show-toplevel)" || exit 1
LIVE=tools/gcc-2.7.2
SCRATCH=tmp/cc1build
PATCH=tools/cc1-plus-to-ior-narrow.patch
CRASHFIX=tools/cc1-reorg-negate-rtx-decl.patch
EXPECT=tools/cc1_tu_expectation.txt
UPSTREAM_PIN=43d1cdb67ed135879869b5266f01efaaada5e35a
MODE=oracle
INSTALL=0
RECORD=0
for a in "$@"; do
  case "$a" in
    --stock)                       MODE=stock ;;
    --oracle|--legacy-oracle)      MODE=oracle ;;
    --install)                     INSTALL=1 ;;
    --record-manifest)             RECORD=1 ;;
    *) echo "unknown option: $a" >&2; exit 2 ;;
  esac
done

# ---------------------------------------------------------------- manifest ---
# Hashes recorded in docs/ORACLE-COMPILER.md for the LIVE (hooked) diagnostic
# tree. The compiler tree is gitignored, so drift there is invisible to git;
# this check is what makes it detectable.
read -r -d '' MANIFEST <<'EOF'
80c8088750a91b37ef53bea6da51d402c58801c8  flow.c
6f23c5eeeabc97f3049b2d414ab42b6f38b891f6  function.c
46af0a314da8ad6dcb27890ca9b2003006c70ced  global.c
9b8f822a79a1945ac4b58ebfb243017d67b828c5  jump.c
3fb248a6b2c85cd7e9e57b19f5df7daf62b3d5fe  local-alloc.c
7ddde6b0f2b65172c5cc83be6165789f445953d9  reload1.c
6e1cf6a97c169204304efd7427e066facb49d69f  reorg.c
3668555e9cb7970b335a505aca4cfdda26e8fc49  sched.c
24c5952113d88cbb96f5c9f7e7152147d1efb8a7  combine.c
EOF
echo "== verifying $LIVE against the docs/ORACLE-COMPILER.md manifest"
if ! ( cd "$LIVE" && printf '%s\n' "$MANIFEST" | sha1sum -c --quiet ); then
  cat >&2 <<'ERR'

FATAL: tools/gcc-2.7.2/ sources do not match the manifest in
docs/ORACLE-COMPILER.md. The compiler tree is gitignored, so drift here is
invisible to git — this check exists to catch exactly that.

Do NOT build a baseline compiler from a tree whose state is unknown. Either
restore the tree, or re-record the manifest (with rationale) in the same
change that altered it.
ERR
  exit 1
fi
echo "   manifest OK"

# ------------------------------------------------------------ scratch tree ---
echo "== preparing pristine scratch tree at $SCRATCH"
rm -rf "$SCRATCH"
mkdir -p "$SCRATCH"
cp -a "$LIVE/." "$SCRATCH/" || exit 1
if [ "$(readlink -f "$SCRATCH")" = "$(readlink -f "$LIVE")" ]; then
  echo "FATAL: scratch resolves to the live tree" >&2; exit 1
fi
( cd "$SCRATCH" && git checkout -- . ) || { echo "FATAL: could not restore pristine sources" >&2; exit 1; }
DIRT=$( cd "$SCRATCH" && git status --porcelain --untracked-files=no )
if [ -n "$DIRT" ]; then
  echo "FATAL: scratch tree still has modified tracked files after checkout:" >&2
  echo "$DIRT" >&2; exit 1
fi
HEADREV=$( cd "$SCRATCH" && git rev-parse HEAD )
echo "   pristine at $HEADREV"
if [ "$HEADREV" != "$UPSTREAM_PIN" ]; then
  echo "FATAL: compiler tree is not at the pinned upstream commit" >&2
  echo "       expected $UPSTREAM_PIN" >&2
  exit 1
fi

echo "== applying $CRASHFIX (every mode)"
if ! git -C "$SCRATCH" apply --check "$PWD/$CRASHFIX" 2>&1; then
  echo "FATAL: $CRASHFIX does not apply to the pinned upstream source" >&2; exit 1
fi
git -C "$SCRATCH" apply "$PWD/$CRASHFIX" || exit 1
echo "   applied ($(git -C "$SCRATCH" diff --numstat -- reorg.c | awk '{print $1}') lines added to reorg.c)"

if [ "$MODE" = oracle ]; then
  echo "== applying $PATCH"
  if ! git -C "$SCRATCH" apply --check "$PWD/$PATCH" 2>&1; then
    echo "FATAL: $PATCH does not apply to the pinned upstream source" >&2; exit 1
  fi
  git -C "$SCRATCH" apply "$PWD/$PATCH" || exit 1
  echo "   applied (combine.c +$(git -C "$SCRATCH" diff --numstat -- combine.c | awk '{print $1}') -$(git -C "$SCRATCH" diff --numstat -- combine.c | awk '{print $2}'))"
else
  echo "== --stock: PLUS->IOR patch NOT applied (research build; diverges from the oracle)"
fi

# ------------------------------------------------------------------ build ---
echo "== building (all objects -O0/-g, combine.o at -O, -fgnu89-inline)"
( cd "$SCRATCH"
  # Modern bison/gperf cannot regenerate the 1995 grammar, so the checked-in
  # generated files must stay NEWER than their sources or make tries to.
  touch c-parse.c c-parse.h c-gperf.h objc-parse.c objc-parse.h 2>/dev/null
  rm -f ./*.o cc1
  TMPDIR=/dev/shm make combine.o CFLAGS="-O -g -fgnu89-inline" >/dev/null 2>&1
  TMPDIR=/dev/shm make cc1      CFLAGS="-g -fgnu89-inline"    >/dev/null 2>&1 )
[ -f "$SCRATCH/cc1" ] || { echo "FATAL: build produced no cc1" >&2; exit 1; }
SH=$(sha1sum "$SCRATCH/cc1" | cut -d' ' -f1)
SR=$(nm --defined-only -S "$SCRATCH/cc1" 2>/dev/null | awk '$4=="simplify_rtx"{print $2}' | head -1)
SR=0x$(echo "$SR" | sed 's/^0*//')
echo "   built : $SCRATCH/cc1  ($(stat -c%s "$SCRATCH/cc1") bytes)"
echo "   sha1  : $SH"
echo "   simplify_rtx: $SR   (oracle/narrow=0x3a6a, stock=0x3a11; retired no-rewrite=0x39ab)"
# The binary's own sha1 is NOT stable across rebuilds (host-toolchain metadata
# leaks in); simplify_rtx's size and the emitted asm are the real invariants.
EXPECT_SR=$([ "$MODE" = oracle ] && echo 0x3a6a || echo 0x3a11)
if [ "$SR" != "$EXPECT_SR" ]; then
  echo "FATAL: simplify_rtx is $SR, expected $EXPECT_SR for mode=$MODE" >&2; exit 1
fi

# ------------------------------------------------------------- self-check ---
# Behaviour over the project's TUs, not bytes. Reference preference order puts
# a PRESERVED HISTORICAL binary first: comparing against the in-use build/cc1
# would be circular once the recipe's own output is installed there.
#
# Since the 2026-09-25 adoption (bcdc1648e) the historical binary implements
# the RETIRED no-rewrite condition, so it is an identity reference only where
# the two conditions agree. They agree on every TU of the tree as adopted. The
# expected divergence per mode, as measured, is listed below. A future src/
# change that lands a site where the conditions differ must add its stem here
# in the same change (e.g. func_80073C78's `+` body in text1b), and the full
# `engine verify-oracle --rebuild` stays the authoritative gate.
#   text1b: func_80073C78's `+` UV stores (Match commit after 9bc64b751) —
#           narrow and stock emit the target `ori` pair, no-rewrite `addu`.
ORACLE_EXPECT_DIFF="text1b"      # narrow vs historical no-rewrite
STOCK_EXPECT_DIFF="ings text1b"  # stock vs historical no-rewrite: site A (study § 1.1) + text1b
F="-O2 -G0 -funsigned-char -quiet -mcpu=3000 -mips1 -mno-abicalls -fno-builtin -w -mel -msoft-float"
FG8="-O2 -G8 -funsigned-char -quiet -mcpu=3000 -mips1 -mno-abicalls -fno-builtin -w -mel -msoft-float"
CPP="mipsel-linux-gnu-cpp -Iinclude -undef -Wall -lang-c -fno-builtin -Dmips -D__GNUC__=2 -D__OPTIMIZE__ -D__mips__ -D__mips -Dpsx -D__psx__ -D__psx -D_PSYQ -D__EXTENSIONS__ -D_MIPSEL -D_LANGUAGE_C -DLANGUAGE_C"
W=tmp/cc1build_check; mkdir -p $W

REF=""
# cc1.PRE-RECIPE-0f438e42 is the retired no-rewrite compiler WITH the crash
# fix (the operative binary 2026-08-24 .. 2026-09-25, kept by the adoption's
# --install). The 045c9543 binaries predate the crash fix and segfault on five
# current TUs, so they are only last-resort references.
for c in "${REF_CC1:-}" \
         "$LIVE/build/cc1.PRE-RECIPE-0f438e42" \
         "$LIVE/build/cc1.PRE-RECIPE-045c9543" \
         "$LIVE/build/cc1.ORACLE-BACKUP" \
         "/mnt/c/Users/Trenton/bb2-oracle-cc1-backup/cc1.oracle-compiler-045c9543"; do
  [ -n "$c" ] && [ -x "$c" ] && { REF="$c"; break; }
done

# Manifest expectation is keyed to the source tree: src/*.c + include/*.h.
SRCDIG=$( (sha1sum src/*.c include/*.h 2>/dev/null | sha1sum) | cut -d' ' -f1 )
: > $W/digests.txt
n=0
for stem in $(ls src/*.c | sed 's|src/||; s|\.c$||'); do
  $CPP src/$stem.c > $W/t.i 2>/dev/null
  case "$stem" in text1a_pre|text1a_post) FL="$FG8";; *) FL="$F";; esac
  "$SCRATCH/cc1" $FL $W/t.i -o $W/new.s 2>/dev/null
  # Drop the .file directive: it carries the input path, not codegen.
  grep -v '^\t\.file' $W/new.s | sha1sum | sed "s|-|$stem|" >> $W/digests.txt
  n=$((n+1))
done

rc=0
if [ -n "$REF" ]; then
  echo "== self-check: asm over $n TUs vs preserved reference $REF"
  bad=""; lines=0
  for stem in $(ls src/*.c | sed 's|src/||; s|\.c$||'); do
    $CPP src/$stem.c > $W/t.i 2>/dev/null
    case "$stem" in text1a_pre|text1a_post) FL="$FG8";; *) FL="$F";; esac
    "$SCRATCH/cc1" $FL $W/t.i -o $W/new.s 2>/dev/null
    "$REF"         $FL $W/t.i -o $W/ref.s 2>/dev/null
    cmp -s $W/new.s $W/ref.s || { bad="$bad $stem"; lines=$((lines + $(diff $W/ref.s $W/new.s | grep -c '^[<>]'))); }
  done
  echo "   differing: ${bad:-none}${bad:+  ($lines line(s))}"
  case "$MODE" in
    oracle) WANT="$ORACLE_EXPECT_DIFF" ;;
    stock)  WANT="$STOCK_EXPECT_DIFF" ;;
  esac
  norm() { echo $* | tr ' ' '\n' | sed '/^$/d' | sort | tr '\n' ' '; }
  if [ "$(norm $bad)" = "$(norm $WANT)" ]; then
    echo "   PASS — divergence from the no-rewrite reference is exactly the expected set {${WANT:-none}}."
  else
    echo "   FAIL — expected divergence {${WANT:-none}}, measured {${bad:- none} }."; rc=1
  fi
elif [ -f "$EXPECT" ] && [ "$MODE" = oracle ]; then
  echo "== self-check: no reference binary on disk; using $EXPECT"
  RECDIG=$(awk '/^# src-digest /{print $3}' "$EXPECT")
  if [ "$RECDIG" != "$SRCDIG" ]; then
    echo "   SKIPPED — the manifest was recorded against a different source tree"
    echo "             (recorded $RECDIG, current $SRCDIG). Per-TU digests move"
    echo "             whenever src/ or include/ changes, which is most commits."
    echo "             Verify with a full 'engine verify-oracle --rebuild' instead."
  elif diff <(grep -v '^#' "$EXPECT") $W/digests.txt >/dev/null; then
    echo "   PASS — all $n TU digests match the recorded expectation."
  else
    echo "   FAIL — TU digests differ from $EXPECT:"
    diff <(grep -v '^#' "$EXPECT") $W/digests.txt | head -20; rc=1
  fi
else
  echo "== self-check: SKIPPED (no reference binary and no usable manifest)"
  echo "   The full 'engine verify-oracle --rebuild' is the authoritative gate."
fi

if [ "$RECORD" = 1 ]; then
  { echo "# Per-TU asm digests emitted by the oracle compiler built from"
    echo "# tools/build_oracle_cc1.sh + tools/cc1-plus-to-ior-narrow.patch"
    echo "# + tools/cc1-reorg-negate-rtx-decl.patch."
    echo "# Keyed to the source tree: they change whenever src/ or include/ does."
    echo "# Refresh with: bash tools/build_oracle_cc1.sh --record-manifest"
    echo "# src-digest $SRCDIG"
    echo "# recorded $(date -u +%Y-%m-%dT%H:%M:%SZ)"
    cat $W/digests.txt; } > "$EXPECT"
  echo "== recorded $EXPECT ($n TUs, src-digest $SRCDIG)"
fi

if [ "$INSTALL" = 1 ]; then
  if [ $rc -ne 0 ]; then
    echo "== NOT installing: self-check did not pass" >&2; exit $rc
  fi
  [ "$MODE" = oracle ] || { echo "== refusing to install a --stock build" >&2; exit 2; }
  PREV=$(sha1sum "$LIVE/build/cc1" | cut -d' ' -f1)
  KEEP="$LIVE/build/cc1.PRE-RECIPE-${PREV:0:8}"
  echo "== installing over $LIVE/build/cc1"
  [ -f "$KEEP" ] || cp -a "$LIVE/build/cc1" "$KEEP"
  cp "$SCRATCH/cc1" "$LIVE/build/cc1"
  echo "   installed $SH   (previous $PREV retained as $(basename "$KEEP"))"
  echo "   NOW RUN: & tools/wteng.ps1 main verify-oracle --rebuild"
fi
exit $rc
