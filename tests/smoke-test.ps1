$ErrorActionPreference = 'Stop'
$root = Split-Path -Parent $PSScriptRoot
$platformIo = Join-Path $env:USERPROFILE '.platformio\penv\Scripts\platformio.exe'
$failures = [System.Collections.Generic.List[string]]::new()

function Assert-True {
    param(
        [bool]$Condition,
        [string]$Message
    )

    if ($Condition) {
        Write-Host "[PASS] $Message" -ForegroundColor Green
    } else {
        Write-Host "[FAIL] $Message" -ForegroundColor Red
        $failures.Add($Message)
    }
}

Write-Host 'Health Band smoke test'

$requiredFiles = @(
    'diagram.json',
    'wokwi.toml',
    'firmware-wokwi\src\main.cpp',
    'firmware-wokwi\platformio.ini',
    'node-red\data\flows.json',
    'data\telemetry.schema.json',
    'data\status.schema.json',
    'data\command.schema.json',
    'data\alert.schema.json',
    'data\event.schema.json'
)

foreach ($file in $requiredFiles) {
    Assert-True -Condition (Test-Path (Join-Path $root $file)) -Message "Required file exists: $file"
}

foreach ($file in @(
    'diagram.json',
    'node-red\data\flows.json',
    'data\telemetry.schema.json',
    'data\status.schema.json',
    'data\command.schema.json',
    'data\alert.schema.json',
    'data\event.schema.json'
)) {
    try {
        Get-Content -LiteralPath (Join-Path $root $file) -Raw -Encoding UTF8 | ConvertFrom-Json | Out-Null
        Assert-True -Condition $true -Message "Valid JSON: $file"
    } catch {
        Assert-True -Condition $false -Message "Valid JSON: $file"
    }
}

Assert-True -Condition (Test-Path $platformIo) -Message 'PlatformIO command is installed'
if (Test-Path $platformIo) {
    & $platformIo run --project-dir (Join-Path $root 'firmware-wokwi')
    Assert-True -Condition ($LASTEXITCODE -eq 0) -Message 'ESP32 firmware builds successfully'
}

try {
    $container = docker inspect health-band-node-red | ConvertFrom-Json
    Assert-True -Condition ($container.State.Running -eq $true) -Message 'Node-RED container is running'
    Assert-True -Condition ($container.State.Health.Status -eq 'healthy') -Message 'Node-RED container is healthy'
} catch {
    Assert-True -Condition $false -Message 'Node-RED container is available'
}

try {
    $flows = Invoke-RestMethod -Uri 'http://localhost:1880/flows'
    Assert-True -Condition ($flows.Count -ge 22) -Message 'Runtime flow contains all dashboard and MQTT nodes'
    $commandFilter = $flows | Where-Object id -eq 'cmdF'
    Assert-True -Condition ($commandFilter.func -match 'dashboardCommand') -Message 'Command feedback-loop guard is active'
} catch {
    Assert-True -Condition $false -Message 'Node-RED flow API is reachable'
}

try {
    $page = Invoke-WebRequest -UseBasicParsing -Uri 'http://localhost:1880/dashboard/overview'
    Assert-True -Condition ($page.StatusCode -eq 200) -Message 'Dashboard is reachable'
} catch {
    Assert-True -Condition $false -Message 'Dashboard is reachable'
}

if ($failures.Count -gt 0) {
    Write-Host ''
    Write-Host "Smoke test failed: $($failures.Count) check(s)." -ForegroundColor Red
    exit 1
}

Write-Host ''
Write-Host 'Smoke test passed. Start Wokwi and complete the scenario tests.' -ForegroundColor Green
