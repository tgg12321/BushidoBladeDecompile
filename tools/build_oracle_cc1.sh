#!/bin/bash
# tools/build_oracle_cc1.sh — rebuild the project's cc1 from source.
#
# The oracle compiler is GCC 2.7.2 from the pinned upstream commit, built with
# the recipe recovered by the 2026-08-07 forensics, plus ONE committed patch,
# a host-only crash fix that changes no codegen:
#
#   upstream : decompals/mips-gcc-2.7.2 @ 43d1cdb67ed135879869b5266f01efaaada5e35a
#   crashfix : tools/cc1-reorg-negate-rtx-decl.patch  (the owner-approved
#              2026-08-24 host-ABI negate_rtx declaration in reorg.c; it only
#              lets a 64-bit host run the compiler)
#   recipe   : every object at -O0 (`-g`), combine.o ALONE at `-O`,
#              `-fgnu89-inline` throughout (without it modern hosts fail to
#              link: c-gperf.h's is_reserved_word is `__inline` without
#              `static`)
#
# Owner rulings: docs/grind/decisions.md 2026-08-07 (d99ab6a6: reproducible
# baseline), 2026-08-24 (262930f1b: crash fix), 2026-09-26 Q17 ("consider a
# compiler patch a cheat"): no codegen-changing patch is applied. The
# PLUS->IOR patches (tools/cc1-plus-to-ior-narrow.patch, adopted 2026-09-25
# under bcdc1648e, and the earlier no-rewrite patch it replaced) are
# SUPERSEDED and kept for history only. Background in docs/ORACLE-COMPILER.md.
#
# Modes:
#   (default)        the operative oracle compiler: pristine + the crash fix.
#   --stock          accepted as an alias of the default (since Q17 the oracle
#                    IS the stock build).
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
CRASHFIX=tools/cc1-reorg-negate-rtx-decl.patch
EXPECT=tools/cc1_tu_expectation.txt
UPSTREAM_PIN=43d1cdb67ed135879869b5266f01efaaada5e35a
MODE=oracle
INSTALL=0
RECORD=0
for a in "$@"; do
  case "$a" in
    --stock|--oracle)              MODE=oracle ;;
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

echo "== applying $CRASHFIX"
if ! git -C "$SCRATCH" apply --check "$PWD/$CRASHFIX" 2>&1; then
  echo "FATAL: $CRASHFIX does not apply to the pinned upstream source" >&2; exit 1
fi
git -C "$SCRATCH" apply "$PWD/$CRASHFIX" || exit 1
echo "   applied ($(git -C "$SCRATCH" diff --numstat -- reorg.c | awk '{print $1}') lines added to reorg.c)"

# Q17: nothing else is applied. Fail loudly if the tracked sources differ from
# the pinned upstream by anything but EXACTLY the crash-fix hunk (reverse it:
# the tree must then be clean), so a codegen patch cannot creep back in.
git -C "$SCRATCH" apply -R "$PWD/$CRASHFIX" && git -C "$SCRATCH" diff --quiet || {
  echo "FATAL: scratch tree differs from upstream by more than the crash fix:" >&2
  git -C "$SCRATCH" diff --stat >&2; exit 1; }
git -C "$SCRATCH" apply "$PWD/$CRASHFIX" || exit 1
# Untracked source-like files must be exactly the configure/gen* outputs the
# 2026-08-07 tree already carried (their CONTENTS are not hashed here; the
# asm self-check below and verify-oracle are what pin behaviour).
GENERATED="bc-arity.h bc-opcode.h bc-opname.h config.h hconfig.h insn-attr.h insn-attrtab.c insn-codes.h insn-config.h insn-emit.c insn-extract.c insn-flags.h insn-opinit.c insn-output.c insn-peep.c insn-recog.c options.h specs.h tconfig.h tm.h"
UNTRACKED=$(git -C "$SCRATCH" ls-files --others -- '*.c' '*.h' '*.y' '*.def' '*.md' '*.in' | sort | tr '\n' ' ')
if [ "$UNTRACKED" != "$(echo $GENERATED | tr ' ' '\n' | sort | tr '\n' ' ')" ]; then
  echo "FATAL: untracked source files in the scratch tree differ from the known generated set:" >&2
  echo "  have: $UNTRACKED" >&2; exit 1
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
echo "   simplify_rtx: $SR   (oracle = stock = 0x3a11; retired narrow patch 0x3a6a, retired no-rewrite 0x39ab)"
# The binary's own sha1 is NOT stable across rebuilds (host-toolchain metadata
# leaks in); simplify_rtx's size and the emitted asm are the real invariants.
EXPECT_SR=0x3a11
if [ "$SR" != "$EXPECT_SR" ]; then
  echo "FATAL: simplify_rtx is $SR, expected $EXPECT_SR (unpatched combine.c)" >&2; exit 1
fi

# ------------------------------------------------------------- self-check ---
# Behaviour over the project's TUs, not bytes. Reference preference order puts
# a PRESERVED HISTORICAL binary first: comparing against the in-use build/cc1
# would be circular once the recipe's own output is installed there.
#
# Every preserved reference is a PATCHED compiler, so it is an identity
# reference only where its patch and stock agree. The expected divergence per
# reference, as measured on the tree of the Q17 unpatch commit, is below. A
# future src/ change that lands a site where they differ must update the list
# in the same change; the full `engine verify-oracle --rebuild` stays the
# authoritative gate.
#   cc1.PRE-RECIPE-aa04d761 — the retired NARROW PLUS->IOR compiler (operative
#           2026-09-25 .. 2026-09-26, kept by the Q17 --install). It differs
#           from stock only at (plus REG CONST_INT) disjoint-bit sites; its one
#           dependent function, func_800174F4, returned to INCLUDE_ASM in the
#           same change, so it expects NO divergence.
#   cc1.PRE-RECIPE-0f438e42 — the retired NO-REWRITE compiler with the crash
#           fix (operative 2026-08-24 .. 2026-09-25). Expects:
#     text1b: func_80073C78's `+` UV stores — stock emits the target `ori`
#             pair, no-rewrite `addu`.
#     ings:   `main`'s natural `((D_800A36F1 - 1) << 8) + 0x80` (site C) —
#             stock keeps the unfolded `addiu -1; sll 8; addiu 0x80`,
#             no-rewrite folds to `sll; addiu -128`.
NARROW_REF_EXPECT_DIFF=""
NOREWRITE_REF_EXPECT_DIFF="ings text1b"
F="-O2 -G0 -funsigned-char -quiet -mcpu=3000 -mips1 -mno-abicalls -fno-builtin -w -mel -msoft-float"
FG8="-O2 -G8 -funsigned-char -quiet -mcpu=3000 -mips1 -mno-abicalls -fno-builtin -w -mel -msoft-float"
CPP="mipsel-linux-gnu-cpp -Iinclude -undef -Wall -lang-c -fno-builtin -Dmips -D__GNUC__=2 -D__OPTIMIZE__ -D__mips__ -D__mips -Dpsx -D__psx__ -D__psx -D_PSYQ -D__EXTENSIONS__ -D_MIPSEL -D_LANGUAGE_C -DLANGUAGE_C"
W=tmp/cc1build_check; mkdir -p $W

REF=""
# The 045c9543 binaries predate the crash fix and segfault on five current
# TUs, so they are only last-resort references; no expectation is recorded for
# them (set REF_EXPECT_DIFF when using one, or with REF_CC1).
for c in "${REF_CC1:-}" \
         "$LIVE/build/cc1.PRE-RECIPE-aa04d761" \
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
  case "$(basename "$REF")" in
    cc1.PRE-RECIPE-aa04d761) WANT="$NARROW_REF_EXPECT_DIFF" ;;
    cc1.PRE-RECIPE-0f438e42) WANT="$NOREWRITE_REF_EXPECT_DIFF" ;;
    *)                       WANT="${REF_EXPECT_DIFF:-<no expectation recorded>}" ;;
  esac
  norm() { echo $* | tr ' ' '\n' | sed '/^$/d' | sort | tr '\n' ' '; }
  if [ "$(norm $bad)" = "$(norm $WANT)" ]; then
    echo "   PASS — divergence from $(basename "$REF") is exactly the expected set {${WANT:-none}}."
  else
    echo "   FAIL — expected divergence {${WANT:-none}}, measured {${bad:- none} }."; rc=1
  fi
elif [ -f "$EXPECT" ]; then
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
    echo "# tools/build_oracle_cc1.sh: pinned upstream + the host-only crash fix"
    echo "# tools/cc1-reorg-negate-rtx-decl.patch (no codegen patch; owner Q17)."
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
  PREV=$(sha1sum "$LIVE/build/cc1" | cut -d' ' -f1)
  KEEP="$LIVE/build/cc1.PRE-RECIPE-${PREV:0:8}"
  echo "== installing over $LIVE/build/cc1"
  [ -f "$KEEP" ] || cp -a "$LIVE/build/cc1" "$KEEP"
  # cp-then-rename: a plain cp fails ("Text file busy") while another
  # process runs the compiler; the rename leaves running processes on the old inode.
  cp "$SCRATCH/cc1" "$LIVE/build/cc1.new" && mv -f "$LIVE/build/cc1.new" "$LIVE/build/cc1" || {
    echo "FATAL: could not install $LIVE/build/cc1" >&2; exit 1; }
  echo "   installed $SH   (previous $PREV retained as $(basename "$KEEP"))"
  echo "   NOW RUN: & tools/wteng.ps1 main verify-oracle --rebuild"
fi
exit $rc
