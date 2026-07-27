param(
    [ValidateRange(3, 60)]
    [int]$SecondsPerScenario = 8
)

$ErrorActionPreference = 'Stop'
$broker = 'broker.emqx.io'
$topic = 'iot31/nhom-thanh-danh/health-band/command'
$container = 'health-band-node-red'

$scenarios = @(
    @{ Mode = 'normal';      Label = 'Baseline: normal readings' },
    @{ Mode = 'high_hr';     Label = 'Warning: high heart rate' },
    @{ Mode = 'low_spo2';    Label = 'Warning: low blood oxygen' },
    @{ Mode = 'fall';        Label = 'Critical: fall detected' },
    @{ Mode = 'low_battery'; Label = 'Maintenance: low battery' },
    @{ Mode = 'normal';      Label = 'Recovery: return to normal' }
)

function Publish-Mode {
    param([string]$Mode)

    $requestId = "guided-demo-$Mode-$([DateTimeOffset]::UtcNow.ToUnixTimeMilliseconds())"
    $payload = @{
        requestId = $requestId
        command = 'setMode'
        value = $Mode
    } | ConvertTo-Json -Compress

    $nodeScript = @"
const mqtt = require('mqtt');
const client = mqtt.connect('mqtt://$broker:1883', { clientId: 'guided_demo_' + Date.now() });
client.on('connect', () => {
  client.publish('$topic', '$payload', { qos: 1 }, (error) => {
    if (error) { console.error(error.message); process.exitCode = 2; }
    client.end();
  });
});
client.on('error', error => { console.error(error.message); process.exit(2); });
setTimeout(() => { console.error('MQTT publish timeout'); process.exit(2); }, 5000);
"@

    docker exec $container node -e $nodeScript
    if ($LASTEXITCODE -ne 0) {
        throw "Could not publish scenario '$Mode'. Check Docker and Internet access."
    }
}

Write-Host 'Health Band guided demo'
Write-Host 'Keep the Wokwi simulator and Dashboard visible side by side.'
Write-Host ''

foreach ($scenario in $scenarios) {
    Write-Host ">>> $($scenario.Label)" -ForegroundColor Cyan
    Publish-Mode -Mode $scenario.Mode
    Write-Host "Published setMode=$($scenario.Mode). Observe OLED, RGB LED, buzzer, Digital Twin, alerts and timeline."
    Start-Sleep -Seconds $SecondsPerScenario
}

Write-Host ''
Write-Host 'Guided demo completed. The device is back in Normal mode.' -ForegroundColor Green
