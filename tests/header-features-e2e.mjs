import fs from "node:fs/promises"

let playwright
try {
  playwright = await import("playwright")
} catch {
  playwright = await import("file:///D:/IOTs/.codex-work/health-band-audit/node_modules/playwright/index.mjs")
}
const { chromium } = playwright

const appUrl = "http://localhost:1880/dashboard/overview"
const evidenceDir = "D:\\IOTs\\projects\\health-band-simulation\\tests\\evidence\\header"
const edgePath = "C:\\Program Files (x86)\\Microsoft\\Edge\\Application\\msedge.exe"
const results = []
const browserErrors = []

await fs.mkdir(evidenceDir, { recursive: true })

async function check(id, name, test) {
  try {
    results.push({ id, name, status: "PASS", detail: String(await test()) })
  } catch (error) {
    results.push({ id, name, status: "FAIL", detail: error.message })
  }
}

const browser = await chromium.launch({
  executablePath: edgePath,
  headless: true,
  args: ["--no-sandbox", "--disable-gpu"]
})
const context = await browser.newContext({ viewport: { width: 1500, height: 980 } })
await context.addInitScript(() => {
  localStorage.setItem("healthBandLanguage", "en")
})
const page = await context.newPage()
page.on("console", message => {
  if (message.type() === "error") browserErrors.push(message.text())
})
page.on("pageerror", error => browserErrors.push(error.message))

await page.goto(appUrl, { waitUntil: "domcontentloaded", timeout: 30000 })
await page.waitForSelector(".health-app", { timeout: 20000 })

await check("HEADER-01", "Moon button enables dark theme", async () => {
  await page.locator(".theme-toggle").click()
  await page.waitForFunction(() =>
    document.querySelector(".health-app")?.classList.contains("dark-theme") &&
    document.body.classList.contains("health-band-dark-theme")
  )
  const stored = await page.evaluate(() => localStorage.getItem("healthBandTheme"))
  if (stored !== "dark") throw new Error(`stored=${stored}`)
  await page.waitForTimeout(500)
  await page.screenshot({ path: `${evidenceDir}\\dashboard-dark-theme.png`, fullPage: false })
  return "Dark theme applied to app and outer Node-RED page"
})

await check("HEADER-02", "Dark theme persists after reload", async () => {
  await page.reload({ waitUntil: "domcontentloaded" })
  await page.waitForSelector(".health-app.dark-theme")
  const stored = await page.evaluate(() => localStorage.getItem("healthBandTheme"))
  if (stored !== "dark") throw new Error(`stored=${stored}`)
  return "healthBandTheme=dark persisted"
})

await check("HEADER-03", "Notification bell opens center with unread badge", async () => {
  const badge = page.locator(".notification-badge")
  await badge.waitFor({ timeout: 5000 })
    const before = await badge.innerText()
    await page.locator(".notification-trigger").click()
    await page.waitForSelector(".notification-panel")
    await page.waitForTimeout(350)
    const count = await page.locator(".notification-item").count()
  if (count < 1) throw new Error("Notification list is empty")
  await page.screenshot({ path: `${evidenceDir}\\notification-center.png`, fullPage: false })
  return `badge=${before}; notifications=${count}`
})

await check("HEADER-04", "Mark all notifications as read", async () => {
  await page.getByRole("button", { name: "Mark all as read" }).click()
  const unread = await page.locator(".notification-item.unread").count()
  const badge = await page.locator(".notification-badge").count()
  if (unread !== 0 || badge !== 0) throw new Error(`unread=${unread}; badge=${badge}`)
  return "All notifications marked as read and badge hidden"
})

await check("HEADER-05", "Clear notification list", async () => {
  await page.getByRole("button", { name: "Clear", exact: true }).click()
  await page.locator(".notification-trigger").click()
  await page.waitForSelector(".notification-empty")
  return "Notification empty state is visible"
})

await page.locator(".notification-trigger").click()
await check("HEADER-06", "Sun button restores light theme", async () => {
  await page.locator(".theme-toggle").click()
  await page.waitForFunction(() => !document.querySelector(".health-app")?.classList.contains("dark-theme"))
  const stored = await page.evaluate(() => localStorage.getItem("healthBandTheme"))
  if (stored !== "light") throw new Error(`stored=${stored}`)
  await page.waitForTimeout(500)
  await page.screenshot({ path: `${evidenceDir}\\dashboard-light-theme.png`, fullPage: false })
  return "Light theme restored and persisted"
})

await check("HEADER-07", "No browser errors", async () => {
  if (browserErrors.length) throw new Error(browserErrors.join(" | "))
  return "No console or page errors"
})

await browser.close()
await fs.writeFile(`${evidenceDir}\\header-features-results.json`, JSON.stringify(results, null, 2), "utf8")

const passed = results.filter(item => item.status === "PASS").length
console.log(JSON.stringify({ total: results.length, passed, failed: results.length - passed }, null, 2))
if (passed !== results.length) process.exitCode = 1
