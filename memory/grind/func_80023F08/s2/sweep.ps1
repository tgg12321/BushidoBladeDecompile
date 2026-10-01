param([string[]]$vs)
foreach ($v in $vs) {
  pwsh tmp/orch/sbx.ps1 func_80023F08 tmp/func_80023F08/$v.c 2>&1 | Out-File -Encoding utf8 tmp/func_80023F08/$v.out
  $sc = (Get-Content tmp/func_80023F08/$v.out | Select-String -Pattern '"score"' | Select-Object -First 1).Line
  $ins = (Get-Content tmp/func_80023F08/$v.out | Select-String -Pattern 'insns ·' | Select-Object -First 1).Line
  Write-Output "$v $sc $ins"
}
