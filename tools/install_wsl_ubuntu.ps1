# Run this from an elevated PowerShell AFTER rebooting:
#   Set-ExecutionPolicy Bypass -Scope Process
#   .\tools\install_wsl_ubuntu.ps1

Write-Host "Installing Ubuntu 24.04 for WSL..."
wsl --install -d Ubuntu-24.04

Write-Host ""
Write-Host "Done! Ubuntu will open for initial user setup."
Write-Host "After creating your user, run:"
Write-Host '  cd /mnt/c/Users/Trenton/Desktop/"Bushido Blade 2 Decompile"'
Write-Host "  bash tools/setup_wsl.sh"
