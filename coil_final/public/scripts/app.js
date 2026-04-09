/* ═══════════════════════════════════════════════
   CROW C&C — app.js
   Routes:
     POST /connect/<string>/<int>     → handleConnect()
     PUT  /telecommand/               → sendTelecommand()
     GET  /telementry_request/        → requestTelemetry()
   ═══════════════════════════════════════════════ */

'use strict';

/* ── State ─────────────────────────────────────── */
let state = {
  connected: false,
  ip: null,
  port: null,
};

/* ── Clock ─────────────────────────────────────── */
function tickClock() {
  const now = new Date();
  const hh = String(now.getHours()).padStart(2, '0');
  const mm = String(now.getMinutes()).padStart(2, '0');
  const ss = String(now.getSeconds()).padStart(2, '0');
  document.getElementById('clock').textContent = `${hh}:${mm}:${ss} UTC`;
}
tickClock();
setInterval(tickClock, 1000);

/* ── Logging ───────────────────────────────────── */
function log(type, message) {
  const vp = document.getElementById('logViewport');

  // Remove placeholder
  const empty = vp.querySelector('.log-empty');
  if (empty) empty.remove();

  const now = new Date();
  const ts = `${String(now.getHours()).padStart(2,'0')}:${String(now.getMinutes()).padStart(2,'0')}:${String(now.getSeconds()).padStart(2,'0')}`;

  const row = document.createElement('div');
  row.className = 'log-entry';
  message = message.replace(/\n/g, '<br>');

  row.innerHTML = `
    <span class="log-ts">${ts}</span>
    <span class="log-type-${type}">[${type}]</span>
    <span class="log-msg">${message}</span>
  `;
  vp.appendChild(row);
  vp.scrollTop = vp.scrollHeight;

  // Status bar center update
  document.getElementById('sbCenter').textContent = `${type}: ${message}`.slice(0, 60);
}

function escHtml(s) {
  return String(s).replace(/&/g,'&amp;').replace(/</g,'&lt;').replace(/>/g,'&gt;');
}

function clearLog() {
  const vp = document.getElementById('logViewport');
  vp.innerHTML = '<div class="log-empty">Awaiting data…</div>';
  document.getElementById('sbCenter').textContent = '—';
}

/* ── Connection status UI ──────────────────────── */
function setConnected(ip, port) {
  state.connected = true;
  state.ip = ip;
  state.port = port;

  document.getElementById('connDot').classList.add('connected');
  document.getElementById('connStatus').textContent = 'CONNECTED';
  document.getElementById('connLabel').textContent = `${ip}:${port}`;
  document.getElementById('btnConnect').disabled = true;
  document.getElementById('ipAddr').disabled = true;
  document.getElementById('portNum').disabled = true;

  // Enable Telecommand buttons
  const telecommandButtons = document.querySelectorAll('.panel-telecommand button');
  telecommandButtons.forEach(button => {
    button.disabled = false;
  });

  // Enable Telemetry buttons
  const telemetryButtons = document.querySelectorAll('.panel-log button');
  telemetryButtons.forEach(button => {
    button.disabled = false;
  });
  
    // Enable sliders
  const sliders = document.querySelectorAll('.slider');
  sliders.forEach(slider => {
    slider.disabled = false;
  });
}

function setDisconnected() {
  state.connected = false;
  state.ip = null;
  state.port = null;

  document.getElementById('connDot').classList.remove('connected');
  document.getElementById('connStatus').textContent = 'DISCONNECTED';
  document.getElementById('connLabel').textContent = '—';
  document.getElementById('btnConnect').disabled = false;

  // Disable IP and Port fields
  document.getElementById('ipAddr').disabled = false;
  document.getElementById('portNum').disabled = false;

  // Disable Telecommand buttons (if necessary)
  const telecommandButtons = document.querySelectorAll('.panel-telecommand button');
  telecommandButtons.forEach(button => {
    button.disabled = true;
  });

  // Disable Telemetry buttons
  const telemetryButtons = document.querySelectorAll('.panel-log button');
  telemetryButtons.forEach(button => {
    button.disabled = true;
  });

  // Disable sliders
  const sliders = document.querySelectorAll('.slider');
  sliders.forEach(slider => {
    slider.disabled = true;
  });
}
/* ══════════════════════════════════════════════
   POST /connect/<string>/<int>
══════════════════════════════════════════════ */
async function handleConnect() {
  const ip = document.getElementById('ipAddr').value.trim();
  const port = parseInt(document.getElementById('portNum').value, 10);

  // Check if the toggle is checked (Ctype 3 when checked, Ctype 2 when not)
  const ctype = document.getElementById('toggleCtype').checked ? 2 : 3;

  // Validate the inputs
  if (!ip) { 
    log('ERR', 'IP address is required');
    return; 
  }
  if (isNaN(port) || port < 1 || port > 65535) { 
    log('ERR', 'Invalid port number (1–65535)');
    return; 
  }

  // Construct the URL dynamically using IP and port
  const url = `/connect/${encodeURIComponent(ip)}/${port}`;
  log('TX', `POST ${url}`);

  // Prepare the JSON body with Ctype
  const body = JSON.stringify({ Ctype: ctype });

  try {
    // Send the POST request to the correct URL with the Ctype data
    const res = await fetch(url, {
      method: 'POST',  // Ensure the method is POST
      headers: {
        'Content-Type': 'application/json'  // Ensuring correct content type
      },
      body: body  // Send the body with Ctype
    });

    // Handle the response
    if (res.ok) {
      let responseBody = '';
      try { responseBody = await res.text(); } catch (_) {}
      setConnected(ip, port);
      log('RX', `HTTP ${res.status} — ${responseBody || 'Connected'}`);
    } else {
	  let responseBody = '';
      try { responseBody = await res.text(); } catch (_) {}
      log('ERR', `HTTP ${res.status} — ${responseBody || 'Connect failed'}`);
    }
  } catch (err) {
    log('INFO', `No server response (${err.message}) — updating UI only`);
    setConnected(ip, port);
  }
}

// Function to handle the routing form submission
async function handleRoute() {
  // Get the values from the form
  const lisIPaddr = document.getElementById("lisipAddr").value.trim();
  const lisPort = parseInt(document.getElementById("lisportNum").value, 10);
  const sendIPaddr = document.getElementById("sendipAddr").value.trim();
  const sendPort = parseInt(document.getElementById("senportNum").value,10);
  
  const ctype = document.getElementById('toggleCtype2').checked ? 2 : 3;

   // Validate the inputs
  if (!lisIPaddr || !sendIPaddr) { 
    log('ERR', 'IP address is required');
    return; 
  }
  if (isNaN(lisPort) || lisPort < 1 || lisPort > 65535) { 
    log('ERR', 'Invalid lis port number (1–65535)');
    return; 
  }
  if (isNaN(sendPort) || sendPort < 1 || sendPort > 65535) { 
    log('ERR', 'Invalid lis port number (1–65535)');
    return; 
  }
  
    // Construct the URL dynamically using IP and port
  const url = `/routing_table/${encodeURIComponent(lisIPaddr)}/${lisPort}/${encodeURIComponent(sendIPaddr)}/${sendPort}`;
  log('TX', `POST ${url}`);

  // Prepare the JSON body with Ctype
  const body = JSON.stringify({ Ctype: ctype });

  try {
    // Send the POST request to the correct URL with the Ctype data
    const res = await fetch(url, {
      method: 'POST',  // Ensure the method is POST
      headers: {
        'Content-Type': 'application/json'  // Ensuring correct content type
      },
      body: body  // Send the body with Ctype
    });

    // Handle the response
    if (res.ok) {
      let responseBody = '';
      try { responseBody = await res.text(); } catch (_) {}
      setConnected(lisIPaddr, lisPort);
      log('RX', `HTTP ${res.status} — ${responseBody || 'routed'}`);
    } else {
	  let responseBody = '';
      try { responseBody = await res.text(); } catch (_) {}
      log('ERR', `HTTP ${res.status} — ${responseBody || 'Connect failed'}`);
    }
  } catch (err) {
    log('INFO', `No server response (${err.message}) — updating UI only`);
    setConnected(lisIPaddr, lisPort);
  }
}

function handleDisconnect() {
  log('INFO', `Disconnecting from ${state.ip}:${state.port}`);
  setDisconnected();
}

async function handleSleepAndDisconnect() {
	// Now disconnect the connection and disable relevant UI elements
  log('INFO', `Disconnecting from ${state.ip}:${state.port}`);
  setDisconnected();
  
  // First, send the Sleep command
  await sendTelecommand(5);  // SLEEP command
  
}
/* ══════════════════════════════════════════════
   PUT /telecommand/
   Buttons: FORWARD · BACKWARD · LEFT · RIGHT · SLEEP
══════════════════════════════════════════════ */
async function sendTelecommand(cmd) {
  const speed    = parseInt(document.getElementById('sliderSpeed').value, 10);
  const duration = parseInt(document.getElementById('sliderDuration').value, 10);

  const payload = {
    command:  cmd,
    speed:    speed,
    duration: duration,
  };

  const url = '/telecommand/';
  log('TX', `PUT ${url}  cmd=${cmd} speed=${speed} dur=${duration}ms`);

  // Visual feedback — flash the button
  const btn = document.querySelector(`[data-cmd="${cmd}"]`);
  if (btn) {
    btn.style.transform = 'scale(0.9)';
    setTimeout(() => { btn.style.transform = ''; }, 120);
  }

  try {
    const res = await fetch(url, {
      method: 'PUT',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify(payload),
    });

    if (res.ok) {
      let body = '';
      try { body = await res.text(); } catch (_) {}
      log('RX', `HTTP ${res.status} — ${body || 'ACK'}`);
    } else {
	  let body = '';
      try { body = await res.text(); } catch (_) {}
      log('ERR', `HTTP ${res.status} — ${body ||'Command rejected'}`);
    }
  } catch (err) {
    log('INFO', `No server (${err.message}) — cmd queued locally`);
  }
}

/* ══════════════════════════════════════════════
   GET /telementry_request/
   "REQ HK" button in the log panel header
══════════════════════════════════════════════ */
async function requestTelemetry() {
  const url = '/telemetry_request/';
  log('TX', `GET ${url}`);

  try {
    const res = await fetch(url, { method: 'GET' });
    if (res.ok) {
      let body = '';
      try { body = await res.text(); } catch (_) {}
      log('RX', `HTTP ${res.status} — ${body || 'HK data received'}`);
    } else {
	  let body = '';
      try { body = await res.text(); } catch (_) {}
      log('ERR', `HTTP ${res.status} — ${body ||'Telemetry request failed'}`);
    }
  } catch (err) {
    log('INFO', `No server (${err.message}) — GET attempted`);
  }
}

/* ── Slider display ────────────────────────────── */
function updateSlider(sliderId, valId, value) {
  document.getElementById(valId).textContent = value;
}

/* ── Keyboard shortcuts ────────────────────────── */
document.addEventListener('keydown', (e) => {
  // Only if focus is not in an input field
  if (document.activeElement.tagName === 'INPUT') return;

  switch (e.key) {
    case 'ArrowUp':    e.preventDefault(); sendTelecommand('FORWARD');  break;
    case 'ArrowDown':  e.preventDefault(); sendTelecommand('BACKWARD'); break;
    case 'ArrowLeft':  e.preventDefault(); sendTelecommand('LEFT');     break;
    case 'ArrowRight': e.preventDefault(); sendTelecommand('RIGHT');    break;
    case 's': case 'S': sendTelecommand('SLEEP'); break;
    case 't': case 'T': requestTelemetry();        break;
    case 'Escape':      handleDisconnect();         break;
  }
});

/* ── Boot message ──────────────────────────────── */
log('INFO', 'CROW C&C initialised — ready');
log('INFO', 'Keyboard: ↑↓←→ drive  S=sleep  T=telemetry  ESC=disconnect');
