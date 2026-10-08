#!/usr/bin/env pwsh
# tools/codex_eng.ps1 <make | engine-subcommand args...>
#
# The ONE build/score door for a sandboxed Codex worker (skill: codex-worker). Codex's Windows
# sandbox cannot start WSL; the Codex allow-rule ~/.codex-claude/rules/bb2-codex-eng.rules (installed by
# tools/codex_worker.py) runs exactly `pwsh -NoProfile -File <MAIN>/tools/codex_eng.ps1 ...`
# OUTSIDE the sandbox. This copy is main's, which the sandbox cannot write.
#
# All checks live in `codex_worker.py shadow --from-pin`: only while a codex_worker run is live
# (pin in main's tmp/, with the run's pid), it harvests that run's scratch worktree through main's
# worktree registry (trusting nothing inside it), refuses unless the change is confined to
# src/ include/ asm/, copies it into the run's Claude-owned trusted worktree and builds THERE, with
# a scrubbed environment and a whitelist of commands and arguments. Nothing in the scratch tree
# is ever executed.
$ErrorActionPreference = 'Stop'
$main = (Resolve-Path (Join-Path $PSScriptRoot '..')).Path
if (-not (Test-Path (Join-Path $main '.git') -PathType Container)) {
    [Console]::Error.WriteLine("codex_eng: REFUSED -- run main's copy ($main is not the main checkout)"); exit 2
}
# Codex's cwd is its scratch tree: leave it before starting anything, and stop Windows from
# resolving executables out of the current directory.
Set-Location $main
$env:NoDefaultCurrentDirectoryInExePath = '1'
& python -I (Join-Path $PSScriptRoot 'codex_worker.py') shadow --from-pin @args
exit $LASTEXITCODE
