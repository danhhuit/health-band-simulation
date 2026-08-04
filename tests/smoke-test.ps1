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
    '.env.example'
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

$diagram = Get-Content -LiteralPath (Join-Path $root 'diagram.json') -Raw -Encoding UTF8 | ConvertFrom-Json
$partTypes = @($diagram.parts | ForEach-Object type)
foreach ($sensorType in @(
    'wokwi-mpu6050',
    'wokwi-ds18b20',
    'wokwi-photoresistor-sensor',
    'board-bmp180'
)) {
    Assert-True -Condition ($partTypes -contains $sensorType) -Message "Wokwi sensor is present: $sensorType"
}
Assert-True -Condition (@($diagram.parts | Where-Object id -eq 'hapticIndicator').Count -eq 1) -Message 'Simulated haptic output is present'

$firmwareSource = Get-Content -LiteralPath (Join-Path $root 'firmware-wokwi\src\main.cpp') -Raw -Encoding UTF8
foreach ($field in @(
    'bodyTemperatureC',
    'ambientLightLux',
    'pressurePa',
    'altitudeM',
    'location',
    'hapticActive',
    'bloodPressure',
    'currentGender',
    'PROFILE_CHILD',
    'selectedWearMode',
    'currentSleepStage',
    'Preferences',
    'saveCheckpoint'
)) {
    Assert-True -Condition ($firmwareSource.Contains($field)) -Message "Firmware publishes extended field: $field"
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
    Assert-True -Condition ($flows.Count -ge 25) -Message 'Runtime flow contains dashboard, MQTT and Gemini API nodes'
    $commandFilter = $flows | Where-Object id -eq 'cmdF'
    Assert-True -Condition ($commandFilter.func -match 'dashboardCommand') -Message 'Command feedback-loop guard is active'
    Assert-True -Condition (@($flows | Where-Object id -eq 'geminiHttpIn').Count -eq 1) -Message 'Gemini API gateway exists'
} catch {
    Assert-True -Condition $false -Message 'Node-RED flow API is reachable'
}

try {
    Invoke-RestMethod -Method Post -Uri 'http://localhost:1880/health-band/api/ai-recommendation' -ContentType 'application/json' -Body '{"profile":"child","gender":"female"}' | Out-Null
    Assert-True -Condition $true -Message 'Gemini API gateway responds'
} catch {
    $message = $_.ErrorDetails.Message
    Assert-True -Condition ($message -match 'GEMINI_API_KEY') -Message 'Gemini API gateway safely reports missing server-side key'
}

$settingsSource = Get-Content -LiteralPath (Join-Path $root 'node-red\data\settings.js') -Raw -Encoding UTF8
Assert-True -Condition ($settingsSource -match 'localfilesystem') -Message 'Node-RED persistent context storage is enabled'

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
