param(
  [switch]$Full
)

$ErrorActionPreference = "Stop"
$projectRoot = Split-Path -Parent $PSScriptRoot
$dashboardUrl = "http://localhost:1880/dashboard/overview"

function Write-Step([string]$message) {
  Write-Host "`n[Health Band pre-demo] $message" -ForegroundColor Cyan
}

function Assert-ExitCode([string]$action) {
  if ($LASTEXITCODE -ne 0) {
    throw "$action failed (exit code $LASTEXITCODE)."
  }
}

Set-Location $projectRoot

Write-Step "Checking Docker"
docker info *> $null
Assert-ExitCode "Docker Desktop check"

Write-Step "Starting Node-RED"
docker compose up -d
Assert-ExitCode "Node-RED startup"

$deadline = (Get-Date).AddSeconds(60)
do {
  $state = docker inspect -f "{{.State.Status}} {{if .State.Health}}{{.State.Health.Status}}{{end}}" health-band-node-red 2>$null
  if ($state -match "running healthy") { break }
  Start-Sleep -Seconds 2
} while ((Get-Date) -lt $deadline)

if ($state -notmatch "running healthy") {
  docker logs --tail 80 health-band-node-red
  throw "Node-RED did not become healthy within 60 seconds."
}
Write-Host "[PASS] Node-RED is healthy" -ForegroundColor Green

Write-Step "Building ESP32 firmware"
& "$env:USERPROFILE\.platformio\penv\Scripts\platformio.exe" run --project-dir ".\firmware-wokwi"
Assert-ExitCode "PlatformIO build"

Write-Step "Building and deploying the Dashboard flow"
node ".\node-red\build-english-dashboard.js"
Assert-ExitCode "Dashboard flow build"
curl.exe -sS -X POST -H "Content-Type: application/json" --data-binary "@node-red/data/flows.json" "http://localhost:1880/flows" *> $null
Assert-ExitCode "Dashboard flow deploy"

Start-Sleep -Seconds 3
$response = Invoke-WebRequest -UseBasicParsing -Uri $dashboardUrl -TimeoutSec 15
if ($response.StatusCode -ne 200) {
  throw "Dashboard returned HTTP $($response.StatusCode)."
}
Write-Host "[PASS] Dashboard is reachable: $dashboardUrl" -ForegroundColor Green

if ($Full) {
  Write-Step "Running smoke test"
  & powershell -ExecutionPolicy Bypass -File ".\tests\smoke-test.ps1"
  Assert-ExitCode "Smoke test"
}

Write-Host "`nREADY FOR DEMO" -ForegroundColor Green
Write-Host "Next: Start Wokwi, wait for MQTT connection, then open $dashboardUrl" -ForegroundColor Yellow
