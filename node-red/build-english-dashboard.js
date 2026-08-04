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
  'eventIn', 'eventParse', 'statusIn', 'statusParse', 'alertIn', 'alertParse',
  'geminiHttpIn', 'geminiAdvice', 'geminiHttpOut'
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
      node.func = [
        'var p=msg.payload;try{',
        'if(Buffer.isBuffer(p))p=p.toString("utf8");',
        'for(var i=0;i<2&&typeof p==="string";i++)p=JSON.parse(p);',
        'if(!p||typeof p!=="object"||Array.isArray(p))throw new Error("Payload is not an object");',
        'var w=[];var req=["deviceId","timestamp","seq","heartRate","spo2","bloodPressure","steps","fallDetected","battery","signalQuality","mode","profile","gender","wearing","wearMode","vitalDataValid","sleep","recovery"];',
        'req.forEach(function(k){if(p[k]===undefined||p[k]===null)w.push("Missing required field: "+k);});',
        'if(Number(p.heartRate)<0||Number(p.heartRate)>200)w.push("Heart rate is outside the supported 0-200 BPM range.");',
        'if(Number(p.spo2)<0||Number(p.spo2)>100)w.push("SpO2 is outside the supported 0-100% range.");',
        'if(p.wearing===false&&(Number(p.heartRate)!==0||Number(p.spo2)!==0))w.push("Off-wrist telemetry must not contain active vitals.");',
        'if(Number(p.battery)<0||Number(p.battery)>100)w.push("Battery is outside the supported 0-100% range.");',
        'if(Number(p.steps)<0)w.push("Step count cannot be negative.");',
        'var sq=["good","medium","poor"];if(sq.indexOf(p.signalQuality)<0)w.push("Unknown signal-quality value.");',
        'var key="dqseq_"+(p.deviceId||"unknown");var previous=global.get(key);',
        'if(previous!==undefined&&Number(p.seq)<=Number(previous))w.push("Sequence is not newer than the previous packet.");',
        'global.set(key,Number(p.seq));',
        'msg.payload=p;msg.qualityWarnings=w;return msg;',
        '}catch(e){node.warn("Telemetry JSON invalid: "+e.message,msg);return null;}'
      ].join('')
      node.wires = [['dashboardUI', 'alert1', 'ts1']]
      break
    case 'ts1':
      node.name = 'Record Last Seen'
      node.func = 'global.set("lts",Date.now());global.set("lti",Number((msg.payload||{}).samplingIntervalMs)||2000);return null;'
      break
    case 'alert1':
      node.name = 'Evaluate Alerts'
      node.func = [
        'var d=msg.payload||{};var a=[];var ac=global.get("ac")||{};var counters=global.get("alertCounters")||{};var n=Date.now();',
        'var profile=d.profile||global.get("healthProfile")||"student";',
        'var thresholds={student:{high:120,low:50,spo2:94},older_adult:{high:110,low:50,spo2:94},athlete:{high:130,low:45,spo2:93},child:{high:115,low:55,spo2:95}}[profile]||{high:120,low:50,spo2:94};',
        'var key=d.deviceId||"health-band-01";var c=counters[key]||{high:0,low:0,spo2:0,temp:0};',
        'var valid=d.wearing!==false&&d.vitalDataValid!==false;',
        'c.high=valid&&d.heartRate>thresholds.high?Math.min(3,c.high+1):0;',
        'c.low=valid&&d.heartRate>0&&d.heartRate<thresholds.low?Math.min(3,c.low+1):0;',
        'c.spo2=valid&&d.spo2>0&&d.spo2<thresholds.spo2?Math.min(3,c.spo2+1):0;',
        'c.temp=valid&&Number(d.bodyTemperatureC)>=38?Math.min(3,(c.temp||0)+1):0;',
        'counters[key]=c;global.set("alertCounters",counters);',
        'if(c.high>=3)a.push({deviceId:d.deviceId,type:"HIGH_HEART_RATE",severity:"warning",value:d.heartRate,threshold:thresholds.high,sourceSeq:d.seq,timestamp:n,message:"High heart rate confirmed across 3 samples: "+d.heartRate+" BPM"});',
        'if(c.low>=3)a.push({deviceId:d.deviceId,type:"LOW_HEART_RATE",severity:"warning",value:d.heartRate,threshold:thresholds.low,sourceSeq:d.seq,timestamp:n,message:"Low heart rate confirmed across 3 samples: "+d.heartRate+" BPM"});',
        'if(c.spo2>=3)a.push({deviceId:d.deviceId,type:"LOW_SPO2",severity:"warning",value:d.spo2,threshold:thresholds.spo2,sourceSeq:d.seq,timestamp:n,message:"Low blood oxygen confirmed across 3 samples: "+d.spo2+"%"});',
        'if(c.temp>=3)a.push({deviceId:d.deviceId,type:"HIGH_BODY_TEMPERATURE",severity:"warning",value:d.bodyTemperatureC,threshold:38,sourceSeq:d.seq,timestamp:n,message:"High body temperature confirmed across 3 samples: "+d.bodyTemperatureC+" C"});',
        'if(d.fallDetected===true)a.push({deviceId:d.deviceId,type:"FALL_DETECTED",severity:"critical",value:true,threshold:null,sourceSeq:d.seq,timestamp:n,message:"Fall detected"});',
        'if(d.battery<=20)a.push({deviceId:d.deviceId,type:"LOW_BATTERY",severity:"warning",value:d.battery,threshold:20,sourceSeq:d.seq,timestamp:n,message:"Low battery: "+d.battery+"%"});',
        'var stateKey=d.deviceId+"_a";var state=JSON.stringify(a.map(function(x){return x.type;}).sort());',
        'if(state===(ac[stateKey]||""))return null;ac[stateKey]=state;global.set("ac",ac);',
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
      node.func = 'var interval=global.get("lti")||2000;var timeout=Math.max(12000,interval*2.5);var g=Date.now()-(global.get("lts")||0);if(g>timeout&&global.get("ofs")!==true){global.set("ofs",true);var n=Date.now();return{payload:{deviceId:"health-band-01",type:"DEVICE_OFFLINE",severity:"critical",value:null,threshold:timeout,sourceSeq:0,timestamp:n,message:"Device offline"}};}if(g<=timeout&&global.get("ofs")===true)global.set("ofs",false);return null;'
      break
    case 'cmdF':
      node.name = 'Prepare Command'
      node.func = 'if(msg.dashboardCommand!==true||!msg.payload||!msg.payload.command)return null;if(msg.payload.command==="setProfile"&&msg.payload.value)global.set("healthProfile",msg.payload.value);if(msg.payload.command==="setGender"&&msg.payload.value)global.set("healthGender",msg.payload.value);if(msg.payload.command==="setWearState"&&msg.payload.value)global.set("healthWearMode",msg.payload.value);return{payload:msg.payload,topic:"iot31/nhom-thanh-danh/health-band/command"};'
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

output.push({
  id: 'geminiHttpIn',
  type: 'http in',
  z: 'tab1',
  name: 'Gemini Recommendation API',
  url: '/health-band/api/ai-recommendation',
  method: 'post',
  upload: false,
  swaggerDoc: '',
  x: 260,
  y: 440,
  wires: [['geminiAdvice']]
})

output.push({
  id: 'geminiAdvice',
  type: 'function',
  z: 'tab1',
  name: 'Anonymize and Ask Gemini',
  func: [
    'return (async function(){',
    'const apiKey=env.get("GEMINI_API_KEY");',
    'const model=env.get("GEMINI_MODEL")||"gemini-3.5-flash";',
    'msg.headers={"Content-Type":"application/json","Cache-Control":"no-store"};',
    'if(!apiKey){msg.statusCode=503;msg.payload={ok:false,message:"GEMINI_API_KEY is not configured on the Node-RED server."};return msg;}',
    'const p=(msg.payload&&typeof msg.payload==="object")?msg.payload:{};',
    'const isVietnamese=p.language==="vi";const language=isVietnamese?"Vietnamese":"English";',
    'const safe={profile:String(p.profile||"unknown").slice(0,24),gender:String(p.gender||"unknown").slice(0,12),wearCoveragePercent:Number(p.wearCoveragePercent||0),averageHeartRate:p.averageHeartRate==null?null:Number(p.averageHeartRate),averageSpO2:p.averageSpO2==null?null:Number(p.averageSpO2),averageBloodPressure:p.averageBloodPressure||null,steps:Number(p.steps||0),sleepHours:Number(p.sleepHours||0),sleepGoalHours:Number(p.sleepGoalHours||8),alerts:Array.isArray(p.alerts)?p.alerts.slice(0,10).map(String):[]};',
    'const prompt=["You are a conservative wellness coach in an educational IoT simulation.","Reply in "+language+" using exactly 3 short, practical lines.","Do not use Markdown, headings, JSON, tone labels, style labels, diagnosis, prescriptions, or unsupported health claims.","Mention once that blood pressure and sleep are simulated estimates.","Recommend professional or emergency help only if the aggregate contains a clear red flag.","Aggregate JSON:",JSON.stringify(safe)].join("\\n");',
    'const https=global.get("https");if(!https)throw new Error("Node-RED HTTPS module is unavailable.");',
    'const body=JSON.stringify({contents:[{parts:[{text:prompt}]}],generationConfig:{temperature:0.2,maxOutputTokens:250}});',
    'const data=await new Promise(function(resolve,reject){const request=https.request({hostname:"generativelanguage.googleapis.com",path:"/v1beta/models/"+encodeURIComponent(model)+":generateContent",method:"POST",headers:{"Content-Type":"application/json","x-goog-api-key":apiKey,"Content-Length":Buffer.byteLength(body)}},function(response){let raw="";response.setEncoding("utf8");response.on("data",function(chunk){raw+=chunk;});response.on("end",function(){let parsed={};try{parsed=JSON.parse(raw||"{}");}catch(error){return reject(new Error("Gemini returned invalid JSON."));}if(response.statusCode<200||response.statusCode>=300)return reject(new Error((parsed.error&&parsed.error.message)||("Gemini HTTP "+response.statusCode)));resolve(parsed);});});request.on("error",reject);request.setTimeout(25000,function(){request.destroy(new Error("Gemini request timed out after 25 seconds."));});request.write(body);request.end();});',
    'const responseParts=(((data.candidates||[])[0]||{}).content||{}).parts||[];',
    'const text=responseParts.map(function(part){return part.text||"";}).join("\\n").trim();',
    'if(!text)throw new Error("Gemini returned no recommendation.");',
    'const cleaned=text.replace(/```(?:json)?|```|\\*\\*/g,"").replace(/^\\s*(tone|style|format|response language)\\s*:\\s*.*$/gim,"").trim();',
    'const aiLines=cleaned.split(/\\r?\\n|(?<=[.!?])\\s+(?=[A-ZÀ-Ỹ])/).map(function(line){return line.trim().replace(/^[-•\\d.]+\\s*/,"");}).filter(function(line){const complete=/[.!?]$/.test(line)&&line.length>12&&line.length<220&&!/[()]/.test(line);const languageOk=!isVietnamese||/[À-ỹ]|\\b(hãy|bạn|nên|duy trì|theo dõi|ngủ|sức khỏe)\\b/i.test(line);return complete&&languageOk;});',
    'const localLines=isVietnamese?["Tóm tắt: dữ liệu hiện tại cần được theo dõi theo xu hướng, không kết luận từ một lần đo.","Hành động: duy trì vận động vừa phải, ngủ theo mục tiêu đã đặt và kiểm tra lại khi có cảnh báo lặp lại.","Lưu ý: huyết áp và giấc ngủ trong đồ án là ước tính mô phỏng, không dùng để chẩn đoán."]:["Summary: follow the trend rather than interpreting a single reading.","Action: maintain moderate activity, follow the sleep goal, and review repeated alerts.","Note: blood pressure and sleep are simulated estimates, not diagnostic measurements."];',
    'const lines=aiLines.slice(0,1);while(lines.length<2)lines.push(localLines[lines.length]);lines.push(localLines[2]);',
    'const recommendation=lines.slice(0,3).map(function(line,index){return (index+1)+". "+line;}).join("\\n");',
    'msg.statusCode=200;msg.payload={ok:true,recommendation:recommendation,model:model,privacy:"anonymized aggregates only",disclaimer:"Educational wellness guidance; not medical advice."};return msg;',
    '})().catch(function(error){msg.statusCode=502;msg.payload={ok:false,message:error.message};return msg;});'
  ].join(''),
  outputs: 1,
  timeout: 30,
  noerr: 0,
  initialize: '',
  finalize: '',
  libs: [],
  x: 520,
  y: 440,
  wires: [['geminiHttpOut']]
})

output.push({
  id: 'geminiHttpOut',
  type: 'http response',
  z: 'tab1',
  name: 'Gemini API Response',
  statusCode: '',
  headers: {},
  x: 780,
  y: 440,
  wires: []
})

fs.writeFileSync(flowPath, JSON.stringify(output, null, 2) + '\n', 'utf8')
fs.writeFileSync(exportPath, JSON.stringify(output, null, 2) + '\n', 'utf8')
console.log(`Built English Dashboard flow with ${output.length} nodes.`)
