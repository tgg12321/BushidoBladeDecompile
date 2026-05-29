#!/bin/bash
# cc1psx wrapper: runs the original PsyQ cc1psx.exe (GCC 2.7.2.SN.1) via dosemu2
# so the build can opt-in to bytewise-target-cc1 codegen for functions where
# decompals/mips-gcc-2.7.2 (kmc-gcc-tailored) diverges from PsyQ's allocator.
#
# Drop-in replacement for cc1: reads preprocessed C on stdin, writes asm on stdout.
# Honors a subset of cc1 flags (-O2, -G0, -mcpu=3000, -mips1, -funsigned-char,
# -quiet, -w). Other flags are silently dropped (cc1psx doesn't accept them).
#
# Per-invocation cost: ~0.8s on this host. Fine for matching builds, slow for
# wholesale use — gate via CC1_PSX_FILES in the Makefile.
set -eo pipefail

CC1PSX="$(dirname "$0")/cc1psx.exe"
[ -f "$CC1PSX" ] || { echo "cc1psx_wrapper: missing $CC1PSX" >&2; exit 1; }

# Per-invocation work dir (PID + nanoseconds for parallel-safety).
# MUST live in /dev/shm, NOT /tmp: dosemu2 under WSL runs its fssvc helper with
# a landlock-isolated private /tmp, so a /tmp work dir is invisible to it
# ("fssvc_add_path: Transport Error" -> SIGABRT). /dev/shm is a shared tmpfs both
# dosemu2 and its helper can see. (Diagnosed 2026-05-29 restoring cc1psx.)
WORK="/dev/shm/cc1psx_$$_$(date +%N)"
mkdir -p "$WORK"
cp "$CC1PSX" "$WORK/CC1PSX.EXE"
trap 'rm -rf "$WORK"' EXIT

# Stash stdin to a DOS-name file
cat > "$WORK/T.C"

# Build the cc1psx command with safe flag passthrough.
# cc1psx.exe accepts: -quiet -Olevel -mcpu=N -mips{1,2,3,4} -msoft-float -funsigned-char -w
PSX_FLAGS="-quiet"
for arg in "$@"; do
    case "$arg" in
        -O*|-mcpu=*|-mips1|-mips2|-mips3|-mips4|-msoft-float|-funsigned-char|-w|-G0|-G8)
            PSX_FLAGS="$PSX_FLAGS $arg"
            ;;
        # Drop unsupported / GNU-only flags silently
        -mno-abicalls|-fno-builtin|-fno-pic|--*|-quiet)
            ;;
        # Skip flags that take their next token as a value
        -o)
            : # we always write to T.S
            ;;
    esac
done

# XDG_RUNTIME_DIR for dosemu2 (system /run/user often unwriteable under WSL).
# Force a /tmp path regardless of inherited value.
export XDG_RUNTIME_DIR="/tmp/runtime-$(id -u)"
mkdir -p "$XDG_RUNTIME_DIR/dosemu2"
chmod 700 "$XDG_RUNTIME_DIR" 2>/dev/null || true

# Run dosemu silently. -dumb suppresses TUI; -K mounts WORK as drive C:.
# dosemu2 exits non-zero on a benign "kbd: EOF from stdin" even on a SUCCESSFUL
# compile, so do NOT let its exit code abort us (set -e is on) — judge success
# by the presence of the output asm instead.
dosemu -dumb -K "$WORK" \
    -E "CC1PSX.EXE $PSX_FLAGS T.C -o T.S" \
    </dev/null >/dev/null 2>&1 || true

# DOS preserves only 8.3 lowercase on emufs; result lands as t.s
if [ -f "$WORK/T.S" ]; then
    cat "$WORK/T.S"
elif [ -f "$WORK/t.s" ]; then
    cat "$WORK/t.s"
else
    echo "cc1psx_wrapper: cc1psx produced no output (dosemu/cc1psx failed)" >&2
    exit 1
fi
