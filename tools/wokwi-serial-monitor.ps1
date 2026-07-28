$ErrorActionPreference = 'Stop'

$python = Join-Path $env:USERPROFILE '.platformio\penv\Scripts\python.exe'
if (-not (Test-Path $python)) {
    throw 'PlatformIO Python was not found. Install/repair the PlatformIO IDE extension first.'
}

$port = 4000
$connection = Get-NetTCPConnection -LocalPort $port -State Listen -ErrorAction SilentlyContinue
if (-not $connection) {
    throw 'Wokwi RFC2217 port 4000 is not open. Start Wokwi Simulator before running this monitor.'
}

Write-Host 'Health Band Wokwi Serial Monitor' -ForegroundColor Cyan
Write-Host 'Connection: rfc2217://localhost:4000 @ 115200 baud'
Write-Host 'Stop monitor: Ctrl+C'
Write-Host ''

& $python -m serial.tools.miniterm 'rfc2217://localhost:4000' 115200 --eol LF --raw
