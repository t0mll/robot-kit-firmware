#pragma once
// The phone remote page: monochrome, one joystick, battery and distance readouts. No external assets.
static const char REMOTE_PAGE[] PROGMEM = R"HTML(<!doctype html>
<html lang="en"><head><meta charset="utf-8"><meta name="viewport" content="width=device-width,initial-scale=1,user-scalable=no">
<title>Rover remote</title>
<style>
:root{color-scheme:light dark;--fg:#0a0a0a;--bg:#fff;--mu:#6b6b6b}
@media(prefers-color-scheme:dark){:root{--fg:#f5f5f5;--bg:#000;--mu:#8a8a8a}}
body{margin:0;font:14px ui-monospace,monospace;background:var(--bg);color:var(--fg);display:flex;flex-direction:column;align-items:center;gap:18px;padding:24px;touch-action:none;user-select:none}
h1{font:600 18px system-ui,sans-serif;margin:0;letter-spacing:.08em;text-transform:uppercase}
#pad{width:min(80vw,320px);aspect-ratio:1;border:1px solid var(--fg);border-radius:50%;position:relative}
#knob{width:22%;aspect-ratio:1;border-radius:50%;background:var(--fg);position:absolute;left:39%;top:39%}
#stat{color:var(--mu);text-align:center;line-height:1.7}
a{color:var(--mu)}
</style></head><body>
<h1 id="name">Rover</h1>
<div id="pad"><div id="knob"></div></div>
<div id="stat">connecting…</div>
<a href="/wifi">Wi-Fi settings</a>
<script>
const pad=document.getElementById('pad'),knob=document.getElementById('knob'),stat=document.getElementById('stat');
let x=0,y=0,active=false;
function place(){knob.style.left=(39+x*39)+'%';knob.style.top=(39-y*39)+'%'}
function move(e){const r=pad.getBoundingClientRect();let dx=(e.clientX-r.left)/r.width*2-1,dy=1-(e.clientY-r.top)/r.height*2;const m=Math.hypot(dx,dy);if(m>1){dx/=m;dy/=m}x=dx;y=dy;place()}
pad.addEventListener('pointerdown',e=>{active=true;pad.setPointerCapture(e.pointerId);move(e)});
pad.addEventListener('pointermove',e=>{if(active)move(e)});
pad.addEventListener('pointerup',()=>{active=false;x=y=0;place()});
pad.addEventListener('pointercancel',()=>{active=false;x=y=0;place()});
// Tank mix: forward/back on y, turn on x.
setInterval(()=>{const l=Math.round(Math.max(-1,Math.min(1,y+x))*100),r=Math.round(Math.max(-1,Math.min(1,y-x))*100);
 fetch('/drive?l='+l+'&r='+r).catch(()=>{})},100);
setInterval(()=>{fetch('/status').then(r=>r.json()).then(s=>{document.getElementById('name').textContent=s.name;
 stat.innerHTML='mode '+s.mode+'<br>battery '+s.battery.toFixed(2)+' V &middot; distance '+(s.distance<0?'—':s.distance+' mm')}).catch(()=>{stat.textContent='connection lost'})},500);
</script></body></html>)HTML";

static const char WIFI_PAGE[] PROGMEM = R"HTML(<!doctype html>
<html lang="en"><head><meta charset="utf-8"><meta name="viewport" content="width=device-width,initial-scale=1">
<title>Rover Wi-Fi</title><style>:root{color-scheme:light dark}body{font:15px system-ui,sans-serif;padding:24px;max-width:420px;margin:auto}
input,button{font:inherit;width:100%;padding:10px;margin:6px 0 14px;box-sizing:border-box}</style></head><body>
<h1>Join your Wi-Fi</h1>
<p>The Rover drives more smoothly on your home Wi-Fi. It restarts and joins; find it on the Face.</p>
<form method="post" action="/wifi"><label>Network name<input name="ssid" required></label>
<label>Password<input name="pass" type="password"></label><button>Save and restart</button></form>
<form method="post" action="/wifi"><input type="hidden" name="forget" value="1"><button>Forget saved Wi-Fi</button></form>
<p><a href="/">Back to the remote</a></p></body></html>)HTML";
