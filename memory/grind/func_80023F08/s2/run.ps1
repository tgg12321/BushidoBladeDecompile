param([string]$v)
pwsh tmp/orch/sbx.ps1 func_80023F08 tmp/func_80023F08/$v.c 2>&1 | Out-File -Encoding utf8 tmp/func_80023F08/$v.out
Get-Content tmp/func_80023F08/$v.out | Select-String -Pattern '"score"|insns ·|source-level ·|error|Error' | Select-Object -First 8
