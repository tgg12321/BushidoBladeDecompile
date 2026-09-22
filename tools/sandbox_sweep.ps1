<#
.SYNOPSIS
  Score several candidate bodies for one function and print one line each
  (plus, optionally, only the SCORED diff hunks).

.DESCRIPTION
  The manual lane's inner loop is "write N spellings, score them all, read the
  diff of the best". Doing that through bare `sandbox` meant re-typing
  --candidate every time and wading through the not-scored (masked) hunks and
  cpp noise. Every variant's full output is saved to
  tmp/sandbox_sweep/<func>/<variant-name>.txt so nothing is lost (never next to
  the variant: that would dirty the ledger).

  Variants are paths to candidate bodies (the same format as
  memory/grind/<func>/candidate.c). Nothing in the tree is touched: sandbox
  scores a COPY of the src.

.EXAMPLE
  pwsh tools/sandbox_sweep.ps1 -Func CD_cw -Variants tmp/cdcw/a.c,tmp/cdcw/b.c
  pwsh tools/sandbox_sweep.ps1 -Func CD_cw -Variants memory/grind/CD_cw/candidate.c -Hunks
#>
[CmdletBinding()]
param(
    [Parameter(Mandatory = $true)] [string]$Func,
    [Parameter(Mandatory = $true)] [string[]]$Variants,
    # also print the scored (source-level / operand-only) hunks of each variant
    [switch]$Hunks
)
$ErrorActionPreference = 'Stop'
$Root = Split-Path -Parent $PSScriptRoot
$wteng = Join-Path $Root 'tools/wteng.ps1'
$outDir = Join-Path $Root "tmp/sandbox_sweep/$Func"
New-Item -ItemType Directory -Force -Path $outDir | Out-Null

# pwsh -File passes "a.c,b.c" as ONE string; accept both forms.
$list = @($Variants | ForEach-Object { $_ -split ',' } | Where-Object { $_ })

foreach ($v in $list) {
    $abs = if ([IO.Path]::IsPathRooted($v)) { $v } else { Join-Path $Root $v }
    if (-not (Test-Path $abs)) { "{0,-40} MISSING" -f $v; continue }
    # sandbox resolves --candidate relative to the repo root inside WSL.
    $rel = [IO.Path]::GetRelativePath($Root, (Resolve-Path $abs)) -replace '\\', '/'
    $out = & $wteng main sandbox $Func --disable all --diff --candidate $rel 2>&1 | Out-String -Width 240
    $out | Set-Content -Encoding utf8 (Join-Path $outDir ((Split-Path $abs -Leaf) + '.txt'))

    $score = if ($out -match '"score"\s*:\s*(\d+)') { $Matches[1] } else { '?' }
    $insns = if ($out -match '"build_insns"\s*:\s*(\d+)') { $Matches[1] } else { '?' }
    $tgt   = if ($out -match '"target_insns"\s*:\s*(\d+)') { $Matches[1] } else { '?' }
    $err = ''
    if ($score -eq '?') {
        # First compiler diagnostic, not the cpp multi-line-string warnings.
        $e = ($out -split "`n" | Where-Object { $_ -match '\.c:\d+: ' -and $_ -notmatch 'missing terminating' } |
              Select-Object -First 2 | ForEach-Object { $_.Trim() }) -join ' | '
        $err = "  BUILD FAILED: $e"
    }
    "{0,-40} score {1,4}   insns {2}/{3}{4}" -f $v, $score, $insns, $tgt, $err

    if ($Hunks -and $score -ne '?') {
        $skip = $false
        foreach ($l in ($out -split "`n")) {
            if ($l -match '^@ hunk') { $skip = $l -match 'not-scored' }
            if ($l -match '^\s+\d+ source-level') { "    $($l.Trim())" }
            if (-not $skip -and $l -match '^@ hunk|^\s+(target|ours) ') { "    $($l.TrimEnd())" }
        }
    }
}
