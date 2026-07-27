const fs = require('fs')
const path = require('path')

const root = __dirname
const flowPath = path.join(root, 'data', 'flows.json')
const exportPath = path.join(root, 'health-band-flow.json')
const backupPath = path.join(root, 'data', 'flows.before-english-dashboard.json')
const templatePath = path.join(root, 'dashboard-template.html')

const flows = JSON.parse(fs.readFileSync(flowPath, 'utf8').replace(/^\uFEFF/, ''))
const template = fs.readFileSync(templatePath, 'utf8')

if (!fs.existsSync(backupPath)) {
  fs.copyFileSync(flowPath, backupPath)
}

const removedIds = new Set([
  'gVitals', 'gChart', 'gCtrl', 'split1',
  'gHr', 'gSpo2', 'gBat', 'tSteps', 'tStatus', 'chart1',
  'btnN', 'btnH', 'btnL', 'btnF', 'btnB', 'btnR',
  'gDashboard', 'dashboardUI',
  'eventIn', 'eventParse', 'statusIn', 'statusParse', 'alertIn', 'alertParse'
])

const output = flows.filter(node => !removedIds.has(node.id))

for (const node of output) {
  switch (node.id) {
    case 'tab1':
      node.label = 'Health Band'
      break
    case 'base1':
      node.name = 'Health Band'
      node.headerContent = 'page'
      node.titleBarStyle = 'default'
      node.navigationStyle = 'icon'
      break
    case 'page1':
      node.name = 'Overview'
      node.icon = 'heart-pulse'
      break
    case 'theme1':
      node.name = 'Health Band Clinical'
      node.colors = {
        surface: '#ffffff',
        primary: '#2563eb',
        bgPage: '#eef3f8',
        groupBg: '#eef3f8',
        groupOutline: '#eef3f8'
      }
      node.sizes = {
        density: 'default',
        pagePadding: '12px',
        groupGap: '12px',
        groupBorderRadius: '18px',
        widgetGap: '12px'
      }
      break
    case 'mqttIn':
      node.name = 'Telemetry'
      break
    case 'json1':
      node.name = 'Parse Telemetry'
      node.wires = [['dashboardUI', 'alert1', 'ts1']]
      break
    case 'ts1':
      node.name = 'Record Last Seen'
      break
    case 'alert1':
      node.name = 'Evaluate Alerts'
      node.func = [
        'var d=msg.payload||{};var a=[];var ac=global.get("ac")||{};var n=Date.now();',
        'if(d.heartRate>120)a.push({deviceId:d.deviceId,type:"HIGH_HEART_RATE",severity:"warning",value:d.heartRate,threshold:120,sourceSeq:d.seq,timestamp:n,message:"High heart rate: "+d.heartRate+" BPM"});',
        'if(d.heartRate<50)a.push({deviceId:d.deviceId,type:"LOW_HEART_RATE",severity:"warning",value:d.heartRate,threshold:50,sourceSeq:d.seq,timestamp:n,message:"Low heart rate: "+d.heartRate+" BPM"});',
        'if(d.spo2<94)a.push({deviceId:d.deviceId,type:"LOW_SPO2",severity:"warning",value:d.spo2,threshold:94,sourceSeq:d.seq,timestamp:n,message:"Low blood oxygen: "+d.spo2+"%"});',
        'if(d.fallDetected===true)a.push({deviceId:d.deviceId,type:"FALL_DETECTED",severity:"critical",value:true,threshold:null,sourceSeq:d.seq,timestamp:n,message:"Fall detected"});',
        'if(d.battery<=20)a.push({deviceId:d.deviceId,type:"LOW_BATTERY",severity:"warning",value:d.battery,threshold:20,sourceSeq:d.seq,timestamp:n,message:"Low battery: "+d.battery+"%"});',
        'var k=d.deviceId+"_a";var c=JSON.stringify(a.map(function(x){return x.type;}).sort());',
        'if(c===(ac[k]||""))return null;ac[k]=c;global.set("ac",ac);',
        'return a.length?[a.map(function(x){return{payload:x};})]:null;'
      ].join('')
      break
    case 'alertMqtt':
      node.name = 'Publish Alert'
      break
    case 'injOff':
      node.name = 'Check Offline Status'
      break
    case 'offFunc':
      node.name = 'Detect Offline Device'
      node.func = 'var g=Date.now()-(global.get("lts")||0);if(g>8000&&global.get("ofs")!==true){global.set("ofs",true);var n=Date.now();return{payload:{deviceId:"health-band-01",type:"DEVICE_OFFLINE",severity:"critical",value:null,threshold:null,sourceSeq:0,timestamp:n,message:"Device offline"}};}if(g<=8000&&global.get("ofs")===true)global.set("ofs",false);return null;'
      break
    case 'cmdF':
      node.name = 'Prepare Command'
      node.func = 'if(msg.dashboardCommand!==true||!msg.payload||!msg.payload.command)return null;return{payload:msg.payload,topic:"iot31/nhom-thanh-danh/health-band/command"};'
      break
    case 'cmdM':
      node.name = 'Publish Command'
      break
  }
}

output.push({
  id: 'eventIn',
  type: 'mqtt in',
  z: 'tab1',
  name: 'Device Events',
  topic: 'iot31/nhom-thanh-danh/health-band/event',
  qos: '0',
  datatype: 'auto-detect',
  broker: 'broker1',
  nl: false,
  rap: true,
  rh: 0,
  inputs: 0,
  x: 230,
  y: 220,
  wires: [['eventParse']]
})

output.push({
  id: 'eventParse',
  type: 'function',
  z: 'tab1',
  name: 'Parse Device Event',
  func: 'var p=msg.payload;try{if(Buffer.isBuffer(p))p=p.toString("utf8");for(var i=0;i<2&&typeof p==="string";i++)p=JSON.parse(p);if(!p||typeof p!=="object")throw new Error("Event is not an object");msg.payload=p;msg.kind="event";return msg;}catch(e){node.warn("Invalid device event: "+e.message,msg);return null;}',
  outputs: 1,
  timeout: 0,
  noerr: 0,
  initialize: '',
  finalize: '',
  libs: [],
  x: 470,
  y: 220,
  wires: [['dashboardUI']]
})

output.push({
  id: 'statusIn',
  type: 'mqtt in',
  z: 'tab1',
  name: 'Device Status',
  topic: 'iot31/nhom-thanh-danh/health-band/status',
  qos: '1',
  datatype: 'auto-detect',
  broker: 'broker1',
  nl: false,
  rap: true,
  rh: 0,
  inputs: 0,
  x: 230,
  y: 280,
  wires: [['statusParse']]
})

output.push({
  id: 'statusParse',
  type: 'function',
  z: 'tab1',
  name: 'Parse Device Status',
  func: 'var p=msg.payload;try{if(Buffer.isBuffer(p))p=p.toString("utf8");for(var i=0;i<2&&typeof p==="string";i++)p=JSON.parse(p);if(!p||typeof p!=="object")throw new Error("Status is not an object");msg.payload=p;msg.kind="status";return msg;}catch(e){node.warn("Invalid device status: "+e.message,msg);return null;}',
  outputs: 1,
  timeout: 0,
  noerr: 0,
  initialize: '',
  finalize: '',
  libs: [],
  x: 470,
  y: 280,
  wires: [['dashboardUI']]
})

output.push({
  id: 'alertIn',
  type: 'mqtt in',
  z: 'tab1',
  name: 'Alert Events',
  topic: 'iot31/nhom-thanh-danh/health-band/alert',
  qos: '1',
  datatype: 'auto-detect',
  broker: 'broker1',
  nl: false,
  rap: true,
  rh: 0,
  inputs: 0,
  x: 230,
  y: 340,
  wires: [['alertParse']]
})

output.push({
  id: 'alertParse',
  type: 'function',
  z: 'tab1',
  name: 'Parse Alert Event',
  func: 'var p=msg.payload;try{if(Buffer.isBuffer(p))p=p.toString("utf8");for(var i=0;i<2&&typeof p==="string";i++)p=JSON.parse(p);if(!p||typeof p!=="object")throw new Error("Alert is not an object");msg.payload=p;msg.kind="alert";return msg;}catch(e){node.warn("Invalid alert event: "+e.message,msg);return null;}',
  outputs: 1,
  timeout: 0,
  noerr: 0,
  initialize: '',
  finalize: '',
  libs: [],
  x: 470,
  y: 340,
  wires: [['dashboardUI']]
})

output.push({
  id: 'gDashboard',
  type: 'ui-group',
  name: 'Health Band Dashboard',
  page: 'page1',
  width: '12',
  height: '1',
  order: 0,
  showTitle: false,
  className: 'health-band-shell',
  visible: 'true',
  disabled: 'false'
})

output.push({
  id: 'dashboardUI',
  type: 'ui-template',
  z: 'tab1',
  group: 'gDashboard',
  page: '',
  ui: '',
  name: 'English Health Dashboard',
  order: 0,
  width: 12,
  height: 25,
  head: '',
  format: template,
  storeOutMessages: true,
  passthru: false,
  resendOnRefresh: true,
  templateScope: 'local',
  className: 'health-band-dashboard',
  x: 700,
  y: 140,
  wires: [['cmdF']]
})

fs.writeFileSync(flowPath, JSON.stringify(output, null, 2) + '\n', 'utf8')
fs.writeFileSync(exportPath, JSON.stringify(output, null, 2) + '\n', 'utf8')
console.log(`Built English Dashboard flow with ${output.length} nodes.`)
