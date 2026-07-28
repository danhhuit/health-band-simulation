param(
    [int]$DurationSeconds = 0
)

$ErrorActionPreference = 'Stop'

$container = docker inspect health-band-node-red 2>$null | ConvertFrom-Json
if (-not $container.State.Running) {
    throw 'The health-band-node-red container is not running. Run: docker compose up -d'
}

Write-Host 'Health Band Live MQTT Log' -ForegroundColor Cyan
Write-Host 'Source: broker.emqx.io / iot31/nhom-thanh-danh/health-band/#'
Write-Host 'Stop log: Ctrl+C'
Write-Host ''

$source = @'
const mqtt = require("mqtt");
const prefix = "iot31/nhom-thanh-danh/health-band/";
const client = mqtt.connect("mqtt://broker.emqx.io:1883", {
  clientId: "health_band_log_" + Date.now()
});

function stamp() {
  return new Date().toLocaleTimeString("en-GB", { hour12: false });
}

client.on("connect", () => {
  console.log(`[${stamp()}] [MQTT] Connected to broker.emqx.io:1883`);
  client.subscribe(prefix + "#", error => {
    if (error) {
      console.error(`[${stamp()}] [MQTT] Subscribe failed: ${error.message}`);
      process.exit(2);
    }
    console.log(`[${stamp()}] [MQTT] Subscribed to ${prefix}#`);
  });
});

client.on("message", (topic, buffer) => {
  const suffix = topic.slice(prefix.length);
  let value;
  try {
    value = JSON.parse(buffer.toString());
  } catch {
    console.log(`[${stamp()}] [${suffix.toUpperCase()}] ${buffer.toString()}`);
    return;
  }

  if (suffix === "telemetry") {
    console.log(
      `[${stamp()}] [TELEMETRY] seq=${value.seq} HR=${value.heartRate} BPM ` +
      `SpO2=${value.spo2}% steps=${value.steps} battery=${value.battery}% ` +
      `mode=${value.mode} profile=${value.profile} power=${value.powerMode}`
    );
    return;
  }
  if (suffix === "status") {
    console.log(
      `[${stamp()}] [STATUS] ${value.online ? "ONLINE" : "OFFLINE"} ` +
      `firmware=${value.firmwareVersion} mode=${value.activeMode} interval=${value.samplingIntervalMs}ms`
    );
    return;
  }
  if (suffix === "event") {
    console.log(
      `[${stamp()}] [EVENT] ${value.eventType} command=${value.command || "-"} ` +
      `value=${value.value || "-"} message="${value.message || ""}"`
    );
    return;
  }
  if (suffix === "alert") {
    console.log(`[${stamp()}] [ALERT] ${value.type} ${value.message || ""}`);
    return;
  }
  if (suffix === "command") {
    console.log(`[${stamp()}] [COMMAND] ${JSON.stringify(value)}`);
  }
});

client.on("error", error => {
  console.error(`[${stamp()}] [MQTT] Error: ${error.message}`);
});

process.on("SIGINT", () => {
  client.end(true, () => process.exit(0));
});

const durationSeconds = Number(process.env.HEALTH_BAND_LOG_SECONDS || 0);
if (durationSeconds > 0) {
  setTimeout(() => client.end(true, () => process.exit(0)), durationSeconds * 1000);
}
'@

$encodedSource = [Convert]::ToBase64String([Text.Encoding]::UTF8.GetBytes($source))
$runner = "eval(Buffer.from('$encodedSource','base64').toString('utf8'))"
docker exec -i -e "HEALTH_BAND_LOG_SECONDS=$DurationSeconds" health-band-node-red node -e $runner
