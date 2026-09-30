#!/bin/bash
# dos.sh <workdir> <command...> : run a DOS command line in headless DOSBox with <workdir> as C:.
# Output of the command is captured to <workdir>/OUT.TXT.
W="$1"; shift
CMD="$*"
cat > "$W/dosbox.conf" <<EOF
[sdl]
output=surface
[dosbox]
memsize=32
[cpu]
cycles=max
[autoexec]
mount c "$W"
c:
$CMD > OUT.TXT
exit
EOF
SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy timeout 120 dosbox -conf "$W/dosbox.conf" -noconsole >/dev/null 2>&1
cat "$W/OUT.TXT" 2>/dev/null | tr -d '\r'
