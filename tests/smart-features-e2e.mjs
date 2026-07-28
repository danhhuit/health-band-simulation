import fs from "node:fs/promises"
import { execFile, spawn } from "node:child_process"
import { promisify } from "node:util"

let playwright
try {
  playwright = await import("playwright")
} catch {
  playwright = await import("file:///D:/IOTs/.codex-work/health-band-audit/node_modules/playwright/index.mjs")
}
const { chromium } = playwright

const execFileAsync = promisify(execFile)
const appUrl = "http://localhost:1880/dashboard/overview?view=smart"
const topicPrefix = "iot31/nhom-thanh-danh/health-band/"
const evidenceDir = "D:\\IOTs\\projects\\health-band-simulation\\tests\\evidence\\smart"
const reportPath = `${evidenceDir}\\smart-features-results.json`
const edgePath = "C:\\Program Files (x86)\\Microsoft\\Edge\\Application\\msedge.exe"
const results = []
const browserErrors = []
let sequence = Date.now()

await fs.mkdir(evidenceDir, { recursive: true })

function result(id, feature, passed, detail) {
  results.push({
    id,
    feature,
    status: passed ? "PASS" : "FAIL",
    detail,
    executedAt: new Date().toISOString()
  })
}

async function check(id, feature, test) {
  try {
    result(id, feature, true, String(await test()))
  } catch (error) {
    result(id, feature, false, error.message)
  }
}

async function publish(topic, payload) {
  const source = `
const mqtt=require("mqtt");
const client=mqtt.connect("mqtt://broker.emqx.io:1883",{clientId:"smart_e2e_"+Date.now()+"_"+Math.random()});
client.on("connect",()=>client.publish("${topicPrefix}${topic}",JSON.stringify(${JSON.stringify(payload)}),{qos:0},()=>client.end(false,{},()=>process.exit(0))));
client.on("error",error=>{console.error(error.message);process.exit(2)});
setTimeout(()=>process.exit(3),12000);`
  await execFileAsync("docker", ["exec", "health-band-node-red", "node", "-e", source], { timeout: 15000 })
}

async function telemetry(overrides = {}) {
  const payload = {
    deviceId: "health-band-01",
    timestamp: sequence * 2,
    seq: sequence++,
    heartRate: 80,
    spo2: 98,
    steps: 1450,
    fallDetected: false,
    battery: 86,
    signalQuality: "good",
    mode: "normal",
    profile: "student",
    powerMode: "normal",
    samplingIntervalMs: 2000,
    ...overrides
  }
  await publish("telemetry", payload)
  return payload
}

async function telemetryBurst(overrides = {}, count = 5) {
  const messages = []
  for (let index = 0; index < count; index += 1) {
    messages.push({
      deviceId: "health-band-01",
      timestamp: sequence * 2,
      seq: sequence++,
      heartRate: 80,
      spo2: 98,
      steps: 1450,
      fallDetected: false,
      battery: 86,
      signalQuality: "good",
      mode: "normal",
      profile: "student",
      powerMode: "normal",
      samplingIntervalMs: 2000,
      ...overrides
    })
  }
  const source = `
const mqtt=require("mqtt");
const values=${JSON.stringify(messages)};
const client=mqtt.connect("mqtt://broker.emqx.io:1883",{clientId:"smart_burst_"+Date.now()+"_"+Math.random()});
client.on("connect",()=>{
  let index=0;
  const send=()=>{
    if(index>=values.length){setTimeout(()=>client.end(false,{},()=>process.exit(0)),100);return;}
    client.publish("${topicPrefix}telemetry",JSON.stringify(values[index++]),{qos:0},()=>setTimeout(send,40));
  };
  send();
});
client.on("error",error=>{console.error(error.message);process.exit(2)});
setTimeout(()=>process.exit(3),12000);`
  await execFileAsync("docker", ["exec", "health-band-node-red", "node", "-e", source], { timeout: 15000 })
}

function startTelemetryStream(overrides = {}, count = 60) {
  const values = []
  for (let index = 0; index < count; index += 1) {
    values.push({
      deviceId: "health-band-01",
      timestamp: sequence * 2,
      seq: sequence++,
      heartRate: 140,
      spo2: 98,
      steps: 1450,
      fallDetected: false,
      battery: 86,
      signalQuality: "good",
      mode: "high_hr",
      profile: "student",
      powerMode: "normal",
      samplingIntervalMs: 2000,
      ...overrides
    })
  }
  const source = `
const mqtt=require("mqtt");
const values=${JSON.stringify(values)};
const client=mqtt.connect("mqtt://broker.emqx.io:1883",{clientId:"smart_stream_"+Date.now()+"_"+Math.random()});
client.on("connect",()=>{
  let index=0;
  const send=()=>{
    if(index>=values.length){client.end(false,{},()=>process.exit(0));return;}
    client.publish("${topicPrefix}telemetry",JSON.stringify(values[index++]),{qos:0},()=>setTimeout(send,100));
  };
  send();
});
client.on("error",error=>{console.error(error.message);process.exit(2)});
setTimeout(()=>process.exit(3),12000);`
  return spawn("docker", ["exec", "health-band-node-red", "node", "-e", source])
}

function waitForCommand(expectedCommand) {
  let readyResolve
  const ready = new Promise(resolve => { readyResolve = resolve })
  const done = new Promise((resolve, reject) => {
    const source = `
const mqtt=require("mqtt");
const client=mqtt.connect("mqtt://broker.emqx.io:1883",{clientId:"smart_sub_"+Date.now()+"_"+Math.random()});
client.on("connect",()=>client.subscribe("${topicPrefix}command",()=>console.log("READY")));
client.on("message",(topic,buffer)=>{
  try {
    const value=JSON.parse(buffer.toString());
    if(value.command==="${expectedCommand}"){console.log("MATCH="+JSON.stringify(value));client.end(true);setTimeout(()=>process.exit(0),50);}
  } catch(error) {}
});
setTimeout(()=>process.exit(3),15000);`
    const child = spawn("docker", ["exec", "health-band-node-red", "node", "-e", source])
    let output = ""
    child.stdout.on("data", chunk => {
      output += chunk
      if (output.includes("READY")) readyResolve()
    })
    child.on("error", reject)
    child.on("exit", code => {
      const match = output.match(/MATCH=(\{.*\})/)
      if (match) resolve(JSON.parse(match[1]))
      else resolve({ error: `Expected ${expectedCommand}; exit=${code}; output=${output}` })
    })
  })
  return { ready, done }
}

const browser = await chromium.launch({
  executablePath: edgePath,
  headless: true,
  args: ["--no-sandbox", "--disable-gpu"]
})
const context = await browser.newContext({
  viewport: { width: 1600, height: 1100 },
  acceptDownloads: true
})
await context.addInitScript(() => {
  localStorage.setItem("healthBandLanguage", "en")
})
const page = await context.newPage()
page.on("console", message => {
  if (message.type() === "error") browserErrors.push(message.text())
})
page.on("pageerror", error => browserErrors.push(error.message))

await page.goto(appUrl, { waitUntil: "domcontentloaded", timeout: 30000 })
await page.waitForSelector(".smart-view", { timeout: 20000 })
await telemetry()
await page.waitForFunction(() => document.querySelector(".smart-view")?.innerText.includes("1,450"))

await check("SMART-01", "F1 explainable health score", async () => {
  const score = await page.locator(".score-ring strong").innerText()
  const factors = await page.locator(".score-factors span").count()
  if (!/^\d+$/.test(score) || factors !== 4) throw new Error(`score=${score}; factors=${factors}`)
  return `score=${score}/100 with ${factors} explainable factors`
})

await check("SMART-02", "F2 custom step goal and persistence", async () => {
  const input = page.locator(".goal-input input")
  await input.fill("8200")
  await page.getByRole("button", { name: "Save goal" }).click()
  const stored = await page.evaluate(() => localStorage.getItem("healthBandStepGoal"))
  if (stored !== "8200") throw new Error(`stored=${stored}`)
  await page.reload({ waitUntil: "domcontentloaded" })
  await page.waitForSelector(".smart-view")
  const value = await page.locator(".goal-input input").inputValue()
  if (value !== "8200") throw new Error(`reloaded=${value}`)
  return "8,200-step target persisted after reload"
})

await check("SMART-03", "F8 profile selection over MQTT", async () => {
  const subscriber = waitForCommand("setProfile")
  await subscriber.ready
  await page.getByRole("button", { name: /Athlete/ }).click()
  const command = await subscriber.done
  if (command.value !== "athlete") throw new Error(JSON.stringify(command))
  const text = await page.locator(".threshold-list").innerText()
  if (!text.includes("130 BPM")) throw new Error(text)
  return JSON.stringify(command)
})

await check("SMART-04", "F3 ten-sample trend analysis", async () => {
  for (let index = 0; index < 10; index += 1) {
    await telemetry({ heartRate: 70 + index * 2, spo2: 99 - Math.floor(index / 5), steps: 1500 + index * 20, profile: "athlete" })
  }
  await page.waitForTimeout(500)
  const text = await page.locator(".trend-intelligence").innerText()
  if (!text.includes("Heart rate") || !text.includes("Step activity")) throw new Error(text)
  return text.replace(/\n+/g, " | ").slice(0, 240)
})

await check("SMART-05", "F4 alert after three abnormal samples", async () => {
  await telemetry({ heartRate: 80, profile: "student" })
  await telemetry({ heartRate: 138, mode: "high_hr", profile: "student" })
  await telemetry({ heartRate: 139, mode: "high_hr", profile: "student" })
  await page.waitForTimeout(250)
  let count = await page.locator(".alert-item").filter({ hasText: "High heart rate" }).count()
  if (count !== 0) throw new Error("High-HR alert appeared before sample 3")
  await telemetry({ heartRate: 140, mode: "high_hr", profile: "student" })
  await page.waitForFunction(() => document.querySelector(".alerts-panel")?.innerText.includes("High heart rate"), null, { timeout: 8000 })
  count = await page.locator(".alert-item").filter({ hasText: "High heart rate" }).count()
  if (count < 1) throw new Error("High-HR alert missing after sample 3")
  return "No alert after 2 samples; alert visible after sample 3"
})

await check("SMART-06", "F5 acknowledge alert over MQTT", async () => {
  await page.locator(".view-tabs button").nth(0).click()
  const subscriber = waitForCommand("ackAlert")
  await subscriber.ready
  const stream = startTelemetryStream()
  await page.waitForFunction(() => document.querySelector(".alerts-panel")?.innerText.includes("High heart rate"), null, { timeout: 8000 })
  await page.locator(".alert-item").filter({ hasText: "High heart rate" }).getByRole("button", { name: "Acknowledge" }).click()
  const command = await subscriber.done
  stream.kill()
  if (command.value !== "HIGH_HEART_RATE") throw new Error(JSON.stringify(command))
  await page.waitForSelector(".ack-badge")
  await page.locator(".view-tabs button").nth(4).click()
  return JSON.stringify(command)
})

await check("SMART-07", "F6 fall countdown and cancellation", async () => {
  await telemetry({ heartRate: 105, spo2: 95, fallDetected: true, mode: "fall" })
  await page.waitForSelector(".emergency-banner.pending", { timeout: 8000 })
  const subscriber = waitForCommand("emergencyAction")
  await subscriber.ready
  await page.getByRole("button", { name: /I am safe/ }).click()
  const command = await subscriber.done
  if (command.value !== "cancel") throw new Error(JSON.stringify(command))
  await page.waitForFunction(() => !document.querySelector(".emergency-banner"))
  return "10-second emergency flow opened and was cancelled safely"
})

await check("SMART-08", "F7 data-quality monitor", async () => {
  await telemetry({ seq: 1, signalQuality: "unknown" })
  await page.waitForFunction(() => document.querySelector(".quality-panel")?.innerText.includes("Sequence"), null, { timeout: 5000 })
  const text = await page.locator(".quality-panel").innerText()
  if (!text.includes("Unknown signal-quality")) throw new Error(text)
  return text.replace(/\n+/g, " | ").slice(0, 260)
})

await check("SMART-09", "F10 adaptive Eco mode over MQTT", async () => {
  await telemetry({ seq: sequence++, signalQuality: "good", powerMode: "normal" })
  const subscriber = waitForCommand("setPowerMode")
  await subscriber.ready
  await page.locator(".power-panel .switch").click()
  const command = await subscriber.done
  if (command.value !== "eco") throw new Error(JSON.stringify(command))
  return JSON.stringify(command)
})

await check("SMART-10", "F11 smart non-diagnostic suggestions", async () => {
  const count = await page.locator(".coach-list > div").count()
  const note = await page.locator(".coach-panel .medical-note").innerText()
  if (count < 1 || !note.includes("do not replace medical advice")) throw new Error(`count=${count}; note=${note}`)
  return `${count} suggestions with safety disclaimer`
})

await check("SMART-11", "F12 normal-versus-current comparison", async () => {
  const rows = await page.locator(".comparison-table > div").count()
  if (rows < 5) throw new Error(`rows=${rows}`)
  return `${rows - 1} metrics compared`
})

await check("SMART-12", "F9 export session report", async () => {
  const downloadPromise = page.waitForEvent("download")
  await page.getByRole("button", { name: "Export report" }).click()
  const download = await downloadPromise
  const path = await download.path()
  const report = JSON.parse(await fs.readFile(path, "utf8"))
  if (!report.healthScore || report.stepGoal !== 8200) throw new Error(JSON.stringify(report))
  return `report=${download.suggestedFilename()}; score=${report.healthScore}; goal=${report.stepGoal}`
})

await check("SMART-13", "Additional guided automatic demo", async () => {
  const subscriber = waitForCommand("setMode")
  await subscriber.ready
  await page.getByRole("button", { name: "Start automatic demo" }).click()
  const command = await subscriber.done
  if (command.value !== "normal") throw new Error(JSON.stringify(command))
  await page.getByRole("button", { name: "Stop demo" }).click()
  return JSON.stringify(command)
})

await page.screenshot({ path: `${evidenceDir}\\smart-coach-desktop.png`, fullPage: true })

await check("SMART-14", "Additional clear-session control", async () => {
  page.once("dialog", dialog => dialog.accept())
  await page.getByRole("button", { name: "Clear session" }).click()
  const state = await page.evaluate(() => ({
    goal: localStorage.getItem("healthBandStepGoal"),
    profile: localStorage.getItem("healthBandProfile")
  }))
  if (state.goal !== null || state.profile !== null) throw new Error(JSON.stringify(state))
  return "Session history, goal and profile were cleared"
})

await check("SMART-15", "Six application pages and responsive layout", async () => {
  const tabs = await page.locator(".view-tabs button").count()
  if (tabs !== 6) throw new Error(`tabs=${tabs}`)
  await page.setViewportSize({ width: 390, height: 844 })
  const overflow = await page.evaluate(() => document.documentElement.scrollWidth > document.documentElement.clientWidth + 2)
  if (overflow) throw new Error("Horizontal overflow at 390px")
  await page.screenshot({ path: `${evidenceDir}\\smart-coach-mobile.png`, fullPage: false })
  return "6 pages; no horizontal overflow at 390px"
})

await check("SMART-16", "No browser JavaScript errors", async () => {
  if (browserErrors.length) throw new Error(browserErrors.join(" | "))
  return "No console or page errors"
})

await context.close()
await browser.close()
await fs.writeFile(reportPath, JSON.stringify(results, null, 2), "utf8")

const passed = results.filter(item => item.status === "PASS").length
console.log(JSON.stringify({ total: results.length, passed, failed: results.length - passed, reportPath }, null, 2))
if (passed !== results.length) process.exitCode = 1
